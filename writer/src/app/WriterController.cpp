#include "WriterController.h"

#include <QFileDialog>

WriterController::WriterController(QObject *parent) : QObject(parent) {
    m_document = new DocumentModel(this);
    m_settings = new AppSettings(this);
    m_settings->load();
    connect(m_document, &DocumentModel::textChanged, this, [this]() {
        clearAnchorOverrides();
        stopRun();
        emit layoutInvalidated();
    });
    connect(m_settings, &AppSettings::anyChanged, this, &WriterController::layoutInvalidated);
}

void WriterController::setViewMode(const QString &m) {
    if (m_viewMode == m) return;
    m_viewMode = m;
    emit viewModeChanged();
}

void WriterController::setSettingsOpen(bool v) {
    if (m_settingsOpen == v) return;
    m_settingsOpen = v;
    emit settingsOpenChanged();
}

void WriterController::pickFontFolder() {
    const QString dir = QFileDialog::getExistingDirectory(nullptr, QStringLiteral("Select font folder"), m_fontFolderPath);
    if (dir.isEmpty()) return;
    m_fontFolderPath = dir;
    emit fontFolderPathChanged();
    loadFontsFromPath(dir);
}

void WriterController::reloadFonts() {
    if (!m_fontFolderPath.isEmpty()) loadFontsFromPath(m_fontFolderPath);
}

void WriterController::loadFontsFromPath(const QString &path) {
    QString err;
    m_fontMap = FontLoader::loadDirectory(path, &err);
    if (m_fontMap.isEmpty())
        m_fontStatus = err.isEmpty() ? QStringLiteral("No glyphs loaded.") : err;
    else
        m_fontStatus = QStringLiteral("Loaded %1 glyphs.").arg(m_fontMap.size());
    emit fontStatusChanged();
    emit layoutInvalidated();
}

void WriterController::clearAnchorOverrides() {
    if (m_anchorOverrides.isEmpty()) return;
    m_anchorOverrides.clear();
}

void WriterController::setAnchorOverrides(const QHash<int, QPointF> &h) {
    m_anchorOverrides = h;
    emit layoutInvalidated();
}

void WriterController::startRun() {
    if (m_runActive) return;
    m_runActive = true;
    emit runActiveChanged();
}

void WriterController::stopRun() {
    if (!m_runActive) return;
    m_runActive = false;
    emit runActiveChanged();
}
