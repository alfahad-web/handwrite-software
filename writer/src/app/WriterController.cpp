#include "WriterController.h"

#include "cnc/GrblConnection.h"
#include "gcode/GcodeController.h"
#include "services/WriterProjectService.h"

#include <QDir>
#include <QFileDialog>

namespace {
void remapManualAnchors(QHash<int, QPointF> *anchors, const QString &oldText, const QString &newText) {
    if (!anchors || oldText == newText) return;
    const int oldLen = oldText.size();
    const int newLen = newText.size();
    int prefix = 0;
    while (prefix < oldLen && prefix < newLen && oldText.at(prefix) == newText.at(prefix))
        ++prefix;
    int suffix = 0;
    while (suffix < oldLen - prefix && suffix < newLen - prefix
           && oldText.at(oldLen - 1 - suffix) == newText.at(newLen - 1 - suffix))
        ++suffix;
    const int delta = newLen - oldLen;
    QHash<int, QPointF> out;
    for (auto it = anchors->constBegin(); it != anchors->constEnd(); ++it) {
        const int k = it.key();
        if (k < 0 || k >= oldLen) continue;
        if (k < prefix) {
            if (k < newLen) out.insert(k, it.value());
        } else if (k >= oldLen - suffix) {
            const int nk = k + delta;
            if (nk >= 0 && nk < newLen) out.insert(nk, it.value());
        }
    }
    *anchors = out;
}
}

WriterController::WriterController(QObject *parent) : QObject(parent) {
    m_document = new DocumentModel(this);
    m_settings = new AppSettings(this);
    m_gcode = new GcodeController(this, this);
    m_grbl = new GrblConnection(this);
    m_settings->load();
    m_lastDocumentText = m_document->text();
    connect(m_document, &DocumentModel::textChanged, this, &WriterController::onDocumentTextChanged);
    connect(m_settings, &AppSettings::anyChanged, this, [this]() {
        emit layoutInvalidated();
        bumpDirty();
    });
    connect(m_grbl, &GrblConnection::streamFinished, this, [this](bool) {
        if (m_runActive) stopRun();
    });
}

void WriterController::setViewMode(const QString &m) {
    if (m_viewMode == m) return;
    m_viewMode = m;
    emit viewModeChanged();
    bumpDirty();
}

void WriterController::setSettingsOpen(bool v) {
    if (m_settingsOpen == v) return;
    m_settingsOpen = v;
    emit settingsOpenChanged();
}

void WriterController::onDocumentTextChanged() {
    const QString newT = m_document->text();
    if (!m_blockAnchorRemap) remapManualAnchors(&m_manualAnchors, m_lastDocumentText, newT);
    m_lastDocumentText = newT;
    stopRun();
    emit layoutInvalidated();
    if (!m_blockAnchorRemap) bumpDirty();
}

void WriterController::beginProjectLoad() {
    m_blockAnchorRemap = true;
    m_suppressDirty = true;
}

void WriterController::endProjectLoad() {
    m_lastDocumentText = m_document->text();
    m_blockAnchorRemap = false;
    m_suppressDirty = false;
    emit layoutInvalidated();
}

void WriterController::setFontFolderPathAndLoad(const QString &path, bool emitMissingIfNotFound) {
    if (m_fontFolderPath == path) {
        loadFontsFromPath(path, emitMissingIfNotFound);
        return;
    }
    m_fontFolderPath = path;
    emit fontFolderPathChanged();
    loadFontsFromPath(path, emitMissingIfNotFound);
}

void WriterController::setDocumentDirty(bool dirty) {
    if (m_documentDirty == dirty) return;
    m_documentDirty = dirty;
    emit documentDirtyChanged();
}

void WriterController::bumpDirty() {
    if (m_suppressDirty) return;
    setDocumentDirty(true);
}

void WriterController::setProjectFilePath(const QString &path) {
    if (m_projectFilePath == path) return;
    m_projectFilePath = path;
    emit projectFilePathChanged();
}

void WriterController::pickFontFolder() {
    const QString dir = QFileDialog::getExistingDirectory(nullptr, QStringLiteral("Select font folder"), m_fontFolderPath);
    if (dir.isEmpty()) return;
    m_fontFolderPath = dir;
    emit fontFolderPathChanged();
    loadFontsFromPath(dir, false);
    bumpDirty();
}

void WriterController::reloadFonts() {
    if (!m_fontFolderPath.isEmpty()) loadFontsFromPath(m_fontFolderPath, false);
}

void WriterController::loadFontsFromPath(const QString &path, bool emitMissingIfNotFound) {
    if (!path.isEmpty() && !QDir(path).exists()) {
        m_fontCatalog = FontCatalog();
        m_fontStatus = QStringLiteral("Font folder not found.");
        emit fontStatusChanged();
        emit layoutInvalidated();
        if (emitMissingIfNotFound) emit fontFolderMissing(path);
        return;
    }
    QString err;
    m_fontCatalog = FontLoader::loadDirectory(path, &err);
    if (m_fontCatalog.isEmpty())
        m_fontStatus = err.isEmpty() ? QStringLiteral("No glyphs loaded.") : err;
    else
        m_fontStatus = QStringLiteral("Loaded %1 glyphs (%2 variants).")
                           .arg(m_fontCatalog.size())
                           .arg(m_fontCatalog.totalVariants());
    emit fontStatusChanged();
    emit layoutInvalidated();
}

void WriterController::setManualAnchors(const QHash<int, QPointF> &h) {
    m_manualAnchors = h;
    emit layoutInvalidated();
    bumpDirty();
}

void WriterController::setManualAnchor(int docIndex, const QPointF &anchorCm) {
    if (docIndex < 0) return;
    m_manualAnchors.insert(docIndex, anchorCm);
    emit layoutInvalidated();
    bumpDirty();
}

void WriterController::clearAllManualAnchors() {
    if (m_manualAnchors.isEmpty()) return;
    m_manualAnchors.clear();
    emit layoutInvalidated();
    bumpDirty();
}

void WriterController::startRun() {
    if (m_runActive) return;
    m_gcode->regenerate();
    if (m_grbl->connected()) {
        m_grbl->streamProgram(m_gcode->generatedGcode());
    } else {
        m_grbl->logMessage(QStringLiteral("Not connected — preview only (no CNC stream)."));
    }
    m_runPaused = false;
    m_runActive = true;
    emit runActiveChanged();
}

void WriterController::pauseRun() {
    if (!m_runActive || m_runPaused) return;
    m_runPaused = true;
    if (m_grbl->connected()) m_grbl->sendRealtimeCommand(QStringLiteral("!"));
    emit runPausedChanged();
}

void WriterController::resumeRun() {
    if (!m_runActive || !m_runPaused) return;
    m_runPaused = false;
    if (m_grbl->connected()) m_grbl->sendRealtimeCommand(QStringLiteral("~"));
    emit runPausedChanged();
}

void WriterController::stopRun() {
    if (!m_runActive && !m_grbl->streaming()) return;
    if (m_grbl->streaming()) m_grbl->cancelStream();
    const bool hadPause = m_runPaused;
    const bool wasActive = m_runActive;
    m_runActive = false;
    m_runPaused = false;
    if (hadPause) emit runPausedChanged();
    if (wasActive) emit runActiveChanged();
}

void WriterController::notifyLineHeightCollision(bool exceeds) {
    if (exceeds && !m_wasLineHeightExceeding) emit lineHeightCollisionWarning();
    m_wasLineHeightExceeding = exceeds;
}

void WriterController::markSaved() {
    setDocumentDirty(false);
}

void WriterController::resetToEmptyProject(bool resetSettingsToDefaults) {
    m_suppressDirty = true;
    stopRun();
    m_document->setText(QString());
    m_lastDocumentText = QString();
    m_manualAnchors.clear();
    m_viewMode = QStringLiteral("typing");
    emit viewModeChanged();
    if (resetSettingsToDefaults) {
        m_settings->setFeedRateCmPerS(2.0);
        m_settings->setPageWidthCm(21.0);
        m_settings->setPageHeightCm(29.7);
        m_settings->setLeftMarginCm(1.5);
        m_settings->setRightMarginCm(1.5);
        m_settings->setVerticalGapCm(0.5);
        m_settings->setHxCm(0.2);
        m_settings->setHyCm(0.5);
        m_settings->setLineHeightCm(0.45);
        m_settings->setFontUnitToCm(0.0001);
        m_settings->setJoinDistMm(0.0);
        m_settings->setPenUpZ(30.0);
        m_settings->setPenDownZ(-5.0);
    }
    m_fontFolderPath.clear();
    emit fontFolderPathChanged();
    m_fontCatalog = FontCatalog();
    m_fontStatus = QStringLiteral("No font folder selected.");
    emit fontStatusChanged();
    setProjectFilePath(QString());
    m_suppressDirty = false;
    setDocumentDirty(false);
    emit layoutInvalidated();
}

void WriterController::newWriterProject() {
    const QString selected = QFileDialog::getSaveFileName(
        nullptr,
        QStringLiteral("Create .writer file"),
        QString(),
        QStringLiteral("Writer document (*.writer)")
    );
    if (selected.isEmpty()) return;
    resetToEmptyProject(true);
    setProjectFilePath(selected);
}

void WriterController::openWriterProject() {
    const QString selected = QFileDialog::getOpenFileName(
        nullptr,
        QStringLiteral("Open .writer file"),
        QString(),
        QStringLiteral("Writer document (*.writer)")
    );
    if (selected.isEmpty()) return;
    QString err;
    if (!WriterProjectService::loadProject(selected, this, &err)) {
        emit projectIoError(err);
        return;
    }
    setProjectFilePath(selected);
    markSaved();
}

void WriterController::saveWriterProject() {
    QString path = m_projectFilePath;
    if (path.isEmpty()) {
        path = QFileDialog::getSaveFileName(
            nullptr,
            QStringLiteral("Save .writer file"),
            QString(),
            QStringLiteral("Writer document (*.writer)")
        );
        if (path.isEmpty()) return;
        setProjectFilePath(path);
    }
    QString err;
    if (!WriterProjectService::saveProject(path, this, &err)) {
        emit projectIoError(err);
        return;
    }
    markSaved();
}

void WriterController::saveWriterProjectAs() {
    const QString path = QFileDialog::getSaveFileName(
        nullptr,
        QStringLiteral("Save .writer file as"),
        m_projectFilePath,
        QStringLiteral("Writer document (*.writer)")
    );
    if (path.isEmpty()) return;
    setProjectFilePath(path);
    QString err;
    if (!WriterProjectService::saveProject(path, this, &err)) {
        emit projectIoError(err);
        return;
    }
    markSaved();
}

bool WriterController::loadWriterProjectFromPath(const QString &path) {
    QString err;
    if (!WriterProjectService::loadProject(path, this, &err)) {
        emit projectIoError(err);
        return false;
    }
    setProjectFilePath(path);
    markSaved();
    return true;
}
