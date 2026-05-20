#include "WriterController.h"

#include "cnc/GrblConnection.h"
#include "gcode/GcodeController.h"
#include "gcode/PathBuilder.h"
#include "services/WriterProjectService.h"

#include <QDir>
#include <QFileDialog>

namespace {
constexpr int kMaxUndoSteps = 100;

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
    m_grbl->setStreamingPreset(m_settings->streamingPreset());
    m_lastDocumentText = m_document->text();
    connect(m_document, &DocumentModel::textAboutToChange, this, [this]() {
        pushUndoState();
    });
    connect(m_document, &DocumentModel::textChanged, this, &WriterController::onDocumentTextChanged);
    connect(m_settings, &AppSettings::aboutToChange, this, [this]() {
        pushUndoState();
    });
    connect(m_settings, &AppSettings::anyChanged, this, [this]() {
        emit layoutInvalidated();
        bumpDirty();
    });
    connect(m_settings, &AppSettings::streamingPresetChanged, this, [this]() {
        if (m_grbl && m_settings)
            m_grbl->setStreamingPreset(m_settings->streamingPreset());
    });
    connect(m_grbl, &GrblConnection::streamFinished, this, [this](bool success) {
        if (!m_runActive) return;
        if (!m_expectPageStreamComplete) {
            if (!success) stopRunInternal(true, false);
            return;
        }
        if (!success) {
            m_waitingMachineIdleAfterPage = false;
            stopRunInternal(true, false);
            return;
        }
        if (m_grbl->connected()) {
            m_waitingMachineIdleAfterPage = true;
            if (m_grbl->machineState() == QLatin1String("Idle"))
                onMachineIdleAfterPageStream();
        } else {
            finishPageRun();
        }
    });
    connect(m_grbl, &GrblConnection::machineStateChanged, this, [this]() {
        if (m_waitingMachineIdleAfterPage && m_grbl->machineState() == QLatin1String("Idle"))
            onMachineIdleAfterPageStream();
    });
    connect(this, &WriterController::layoutInvalidated, this, [this]() {
        clearRunArm();
        refreshPageMap();
    });
    refreshPageMap();
}

void WriterController::setViewMode(const QString &m) {
    if (m_viewMode == m) return;
    pushUndoState();
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
    m_suppressUndo = true;
}

void WriterController::endProjectLoad() {
    m_lastDocumentText = m_document->text();
    m_blockAnchorRemap = false;
    m_suppressDirty = false;
    m_suppressUndo = false;
    clearHistory();
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

void WriterController::refreshPageMap() {
    const PathBuildWithPageMap built = PathBuilder::buildWithPageMapFromController(this);
    const int oldCount = m_pathPageMap.pageCount;
    m_pathPageMap = built.pageMap;
    if (oldCount != m_pathPageMap.pageCount)
        emit pageCountChanged();
    emit runPathChanged();
}

double WriterController::pageStartDistance(int page) const {
    if (page < 0 || page >= m_pathPageMap.pageStartDistanceCm.size())
        return 0;
    return m_pathPageMap.pageStartDistanceCm.at(page);
}

double WriterController::pageEndDistance(int page) const {
    if (page + 1 < m_pathPageMap.pageCount)
        return pageStartDistance(page + 1);
    return m_pathPageMap.totalLengthCm;
}

QVariantMap WriterController::pageBenchmark(int page) const {
    QVariantMap out;
    if (!m_settings || !m_gcode) return out;
    const int clampedPage = qMax(0, page);
    const double distanceCm = qMax(0.0, pageEndDistance(clampedPage) - pageStartDistance(clampedPage));
    const double feedCmPerS = qMax(1e-6, m_settings->feedRateCmPerS());
    const double estimatedSec = distanceCm / feedCmPerS;
    const QString pageGcode = m_gcode->gcodeForPageRange(clampedPage, clampedPage + 1);
    int lineCount = 0;
    const QStringList raw = pageGcode.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : raw) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith(QLatin1Char(';'))) continue;
        ++lineCount;
    }
    out.insert(QStringLiteral("page"), clampedPage);
    out.insert(QStringLiteral("distanceCm"), distanceCm);
    out.insert(QStringLiteral("estimatedSec"), estimatedSec);
    out.insert(QStringLiteral("lineCount"), lineCount);
    return out;
}

void WriterController::setRunArmed(bool armed) {
    if (m_runArmed == armed) return;
    m_runArmed = armed;
    emit runArmedChanged();
    if (armed) emit runArmVisualsChanged();
}

void WriterController::setRunStartPage(int page) {
    if (m_runStartPage == page) return;
    m_runStartPage = page;
    emit runStartPageChanged();
}

void WriterController::clearRunArm() {
    if (!m_runArmed && m_runStartPage == 0 && m_runStartDistanceCm <= 1e-9) return;
    setRunArmed(false);
    m_runStartPage = 0;
    m_runStartDistanceCm = 0;
    m_runEndDistanceCm = 0;
    emit runStartPageChanged();
    emit runPathChanged();
    emit runArmVisualsChanged();
}

void WriterController::advanceRunToPage(int page) {
    refreshPageMap();
    if (m_pathPageMap.pageCount <= 0) return;
    const int clamped = qBound(0, page, m_pathPageMap.pageCount - 1);
    m_runStartPage = clamped;
    m_runStartDistanceCm = pageStartDistance(clamped);
    m_runEndDistanceCm = pageEndDistance(clamped);
    setRunArmed(true);
    emit runStartPageChanged();
    emit runPathChanged();
    emit runArmVisualsChanged();

    if (m_gcode)
        m_gcode->regeneratePage(clamped);
}

void WriterController::startRun() {
    if (m_runActive) return;
    refreshPageMap();
    if (m_pathPageMap.pageCount <= 0 || m_pathPageMap.totalLengthCm <= 1e-9)
        return;

    m_executingPage = m_runArmed ? m_runStartPage : 0;
    m_runStartDistanceCm = pageStartDistance(m_executingPage);
    m_runEndDistanceCm = pageEndDistance(m_executingPage);
    emit runPathChanged();

    m_deferGrblStream = false;
    m_pageLocalGrblStream = false;
    m_pendingGrblSlice.clear();
    m_currentPageBenchmarkLineCount = 0;
    if (m_grbl->connected()) {
        if (!m_gcode->regeneratePage(m_executingPage)) {
            m_grbl->logMessage(QStringLiteral("No strokes on selected page."));
            return;
        }
        m_currentPageBenchmarkLineCount = m_gcode ? m_gcode->pageProgramLineCount(0) : 0;
        m_pendingGrblSlice = m_gcode->generatedGcode();
        m_deferGrblStream = true;
        m_pageLocalGrblStream = true;
        m_expectPageStreamComplete = true;
    } else {
        if (m_gcode)
            m_gcode->regeneratePage(m_executingPage);
        m_expectPageStreamComplete = false;
        m_grbl->logMessage(QStringLiteral("Not connected — preview only (no CNC stream)."));
    }

    setRunArmed(false);
    m_runPaused = false;
    m_runActive = true;
    m_pageRunTimer.start();
    emit runActiveChanged();
}

void WriterController::onMachineIdleAfterPageStream() {
    if (!m_waitingMachineIdleAfterPage || !m_runActive) return;
    m_waitingMachineIdleAfterPage = false;
    finishPageRun();
}

void WriterController::onRunApproachComplete() {
    if (!m_runActive || !m_deferGrblStream) return;
    m_deferGrblStream = false;
    if (m_grbl->connected() && !m_pendingGrblSlice.isEmpty()) {
        m_pageLocalGrblStream = true;
        m_grbl->streamProgram(m_pendingGrblSlice);
    }
    m_pendingGrblSlice.clear();
}

void WriterController::finishPageRun() {
    if (!m_runActive) return;
    m_waitingMachineIdleAfterPage = false;
    m_deferGrblStream = false;
    m_pageLocalGrblStream = false;
    m_pendingGrblSlice.clear();
    const int nextPage = m_executingPage + 1;
    m_expectPageStreamComplete = false;

    const bool hadPause = m_runPaused;
    const bool wasActive = m_runActive;
    const qint64 elapsedMs = m_pageRunTimer.isValid() ? m_pageRunTimer.elapsed() : -1;
    m_runActive = false;
    m_runPaused = false;
    if (hadPause) emit runPausedChanged();
    if (wasActive) emit runActiveChanged();

    if (m_grbl->connected()) {
        const double distanceCm = qMax(0.0, m_runEndDistanceCm - m_runStartDistanceCm);
        const double estSec = m_settings ? distanceCm / qMax(1e-6, m_settings->feedRateCmPerS()) : 0.0;
        const double actualSec = elapsedMs >= 0 ? double(elapsedMs) / 1000.0 : 0.0;
        m_grbl->logMessage(
            QStringLiteral("Page %1 benchmark — lines=%2 distance=%3cm est=%4s actual=%5s")
                .arg(m_executingPage)
                .arg(m_currentPageBenchmarkLineCount)
                .arg(distanceCm, 0, 'f', 3)
                .arg(estSec, 0, 'f', 1)
                .arg(actualSec, 0, 'f', 1));
    }

    if (nextPage < m_pathPageMap.pageCount) {
        m_runStartPage = nextPage;
        m_runStartDistanceCm = pageStartDistance(nextPage);
        m_runEndDistanceCm = pageEndDistance(nextPage);
        setRunArmed(true);
        emit runStartPageChanged();
        emit runPathChanged();
        emit runArmVisualsChanged();
        if (m_gcode)
            m_gcode->regeneratePage(nextPage);
        if (m_grbl->connected())
            m_grbl->logMessage(
                QStringLiteral("Page %1 done — pen up, at home. Advance/Run for page %2.")
                    .arg(m_executingPage)
                    .arg(nextPage));
    } else {
        clearRunArm();
        if (m_grbl->connected())
            m_grbl->logMessage(QStringLiteral("All pages complete — pen up, at home."));
    }
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
    stopRunInternal(true, true);
}

void WriterController::stopRunPreserveCnc() {
    stopRunInternal(true, false);
}

void WriterController::stopRunInternal(bool clearArm, bool abortGrbl) {
    if (!m_runActive && !m_grbl->streaming()) {
        if (clearArm) clearRunArm();
        return;
    }

    m_expectPageStreamComplete = false;
    m_waitingMachineIdleAfterPage = false;
    m_deferGrblStream = false;
    m_pageLocalGrblStream = false;
    m_pendingGrblSlice.clear();
    if (m_grbl->streaming() || m_grbl->connected()) {
        if (abortGrbl)
            m_grbl->abortStreamAndRecover();
        else if (m_grbl->streaming())
            m_grbl->cancelStream();
    }

    const bool hadPause = m_runPaused;
    const bool wasActive = m_runActive;
    m_runActive = false;
    m_runPaused = false;
    if (hadPause) emit runPausedChanged();
    if (wasActive) emit runActiveChanged();
    if (clearArm) clearRunArm();
}

void WriterController::notifyLineHeightCollision(bool exceeds) {
    if (exceeds && !m_wasLineHeightExceeding) emit lineHeightCollisionWarning();
    m_wasLineHeightExceeding = exceeds;
}

bool WriterController::generateGcode() {
    if (m_fontFolderPath.isEmpty() || !QDir(m_fontFolderPath).exists()) {
        emit fontFolderMissing(m_fontFolderPath);
        return false;
    }
    if (m_fontCatalog.isEmpty()) {
        emit fontFolderMissing(m_fontFolderPath);
        return false;
    }
    if (m_gcode) m_gcode->regenerate();
    return true;
}

void WriterController::markSaved() {
    setDocumentDirty(false);
}

void WriterController::resetToEmptyProject(bool resetSettingsToDefaults) {
    m_suppressDirty = true;
    m_suppressUndo = true;
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
        m_settings->setBacklashYStartMm(0.0);
        m_settings->setBacklashYEndMm(297.0);
        m_settings->setXErrorNearMm(0.0);
        m_settings->setXErrorMm(0.0);
        m_settings->setYErrorNearMm(0.0);
        m_settings->setYErrorMm(0.0);
        m_settings->setSimplifyToleranceMm(0.0);
        m_settings->setMinSegmentMm(0.05);
        m_settings->setCollinearToleranceMm(0.02);
        m_settings->setStreamingPreset(QStringLiteral("balanced"));
        m_settings->setArcFitEnabled(false);
        m_settings->setArcFitToleranceMm(0.05);
        m_settings->setGrblJunctionDeviation(0.03);
        m_settings->setGrblAccelX(300.0);
        m_settings->setGrblAccelY(300.0);
        m_settings->setServoSnapMode(false);
        m_settings->setServoUpS(0.0);
        m_settings->setServoDownS(1000.0);
        m_settings->setPenUpZ(30.0);
        m_settings->setPenDownZ(-5.0);
        m_settings->setPreviewDisplayScale(1.0);
    }
    m_fontFolderPath.clear();
    emit fontFolderPathChanged();
    m_fontCatalog = FontCatalog();
    m_fontStatus = QStringLiteral("No font folder selected.");
    emit fontStatusChanged();
    setProjectFilePath(QString());
    m_suppressDirty = false;
    m_suppressUndo = false;
    clearHistory();
    setDocumentDirty(false);
    emit layoutInvalidated();
}

WriterController::HistoryState WriterController::makeHistoryState() const {
    HistoryState s;
    s.documentText = m_document ? m_document->text() : QString();
    s.manualAnchors = m_manualAnchors;
    s.viewMode = m_viewMode;
    if (m_settings) {
        s.feedRateCmPerS = m_settings->feedRateCmPerS();
        s.pageWidthCm = m_settings->pageWidthCm();
        s.pageHeightCm = m_settings->pageHeightCm();
        s.leftMarginCm = m_settings->leftMarginCm();
        s.rightMarginCm = m_settings->rightMarginCm();
        s.verticalGapCm = m_settings->verticalGapCm();
        s.hxCm = m_settings->hxCm();
        s.hyCm = m_settings->hyCm();
        s.lineHeightCm = m_settings->lineHeightCm();
        s.fontUnitToCm = m_settings->fontUnitToCm();
        s.joinDistMm = m_settings->joinDistMm();
        s.backlashYStartMm = m_settings->backlashYStartMm();
        s.backlashYEndMm = m_settings->backlashYEndMm();
        s.xErrorNearMm = m_settings->xErrorNearMm();
        s.xErrorMm = m_settings->xErrorMm();
        s.yErrorNearMm = m_settings->yErrorNearMm();
        s.yErrorMm = m_settings->yErrorMm();
        s.simplifyToleranceMm = m_settings->simplifyToleranceMm();
        s.minSegmentMm = m_settings->minSegmentMm();
        s.collinearToleranceMm = m_settings->collinearToleranceMm();
        s.streamingPreset = m_settings->streamingPreset();
        s.arcFitEnabled = m_settings->arcFitEnabled();
        s.arcFitToleranceMm = m_settings->arcFitToleranceMm();
        s.grblJunctionDeviation = m_settings->grblJunctionDeviation();
        s.grblAccelX = m_settings->grblAccelX();
        s.grblAccelY = m_settings->grblAccelY();
        s.servoSnapMode = m_settings->servoSnapMode();
        s.servoUpS = m_settings->servoUpS();
        s.servoDownS = m_settings->servoDownS();
        s.penUpZ = m_settings->penUpZ();
        s.penDownZ = m_settings->penDownZ();
        s.previewDisplayScale = m_settings->previewDisplayScale();
    }
    return s;
}

void WriterController::applySettingsSnapshot(const HistoryState &state) {
    if (!m_settings) return;
    m_settings->setFeedRateCmPerS(state.feedRateCmPerS);
    m_settings->setPageWidthCm(state.pageWidthCm);
    m_settings->setPageHeightCm(state.pageHeightCm);
    m_settings->setLeftMarginCm(state.leftMarginCm);
    m_settings->setRightMarginCm(state.rightMarginCm);
    m_settings->setVerticalGapCm(state.verticalGapCm);
    m_settings->setHxCm(state.hxCm);
    m_settings->setHyCm(state.hyCm);
    m_settings->setLineHeightCm(state.lineHeightCm);
    m_settings->setFontUnitToCm(state.fontUnitToCm);
    m_settings->setJoinDistMm(state.joinDistMm);
    m_settings->setBacklashYStartMm(state.backlashYStartMm);
    m_settings->setBacklashYEndMm(state.backlashYEndMm);
    m_settings->setXErrorNearMm(state.xErrorNearMm);
    m_settings->setXErrorMm(state.xErrorMm);
    m_settings->setYErrorNearMm(state.yErrorNearMm);
    m_settings->setYErrorMm(state.yErrorMm);
    m_settings->setSimplifyToleranceMm(state.simplifyToleranceMm);
    m_settings->setMinSegmentMm(state.minSegmentMm);
    m_settings->setCollinearToleranceMm(state.collinearToleranceMm);
    m_settings->setStreamingPreset(state.streamingPreset);
    m_settings->setArcFitEnabled(state.arcFitEnabled);
    m_settings->setArcFitToleranceMm(state.arcFitToleranceMm);
    m_settings->setGrblJunctionDeviation(state.grblJunctionDeviation);
    m_settings->setGrblAccelX(state.grblAccelX);
    m_settings->setGrblAccelY(state.grblAccelY);
    m_settings->setServoSnapMode(state.servoSnapMode);
    m_settings->setServoUpS(state.servoUpS);
    m_settings->setServoDownS(state.servoDownS);
    m_settings->setPenUpZ(state.penUpZ);
    m_settings->setPenDownZ(state.penDownZ);
    m_settings->setPreviewDisplayScale(state.previewDisplayScale);
}

void WriterController::restoreHistoryState(const HistoryState &state) {
    m_restoringHistory = true;
    m_blockAnchorRemap = true;
    stopRun();

    if (m_document) m_document->setText(state.documentText);
    m_lastDocumentText = state.documentText;
    m_manualAnchors = state.manualAnchors;

    if (m_viewMode != state.viewMode) {
        m_viewMode = state.viewMode;
        emit viewModeChanged();
    }

    applySettingsSnapshot(state);
    bumpDirty();

    m_blockAnchorRemap = false;
    m_restoringHistory = false;
    emit layoutInvalidated();
}

void WriterController::pushUndoState() {
    if (m_restoringHistory || m_suppressUndo) return;
    const bool wasCanUndo = canUndo();
    const bool wasCanRedo = canRedo();
    m_undoStack.push_back(makeHistoryState());
    if (m_undoStack.size() > kMaxUndoSteps) m_undoStack.removeFirst();
    m_redoStack.clear();
    emitHistoryChangedIfNeeded(wasCanUndo, wasCanRedo);
}

void WriterController::pushUndoSnapshot() {
    pushUndoState();
}

void WriterController::clearHistory() {
    const bool wasCanUndo = canUndo();
    const bool wasCanRedo = canRedo();
    m_undoStack.clear();
    m_redoStack.clear();
    emitHistoryChangedIfNeeded(wasCanUndo, wasCanRedo);
}

void WriterController::emitHistoryChangedIfNeeded(bool wasCanUndo, bool wasCanRedo) {
    if (wasCanUndo != canUndo() || wasCanRedo != canRedo()) emit historyChanged();
}

bool WriterController::undo() {
    if (m_undoStack.isEmpty()) return false;
    const bool wasCanUndo = canUndo();
    const bool wasCanRedo = canRedo();
    m_redoStack.push_back(makeHistoryState());
    restoreHistoryState(m_undoStack.takeLast());
    emitHistoryChangedIfNeeded(wasCanUndo, wasCanRedo);
    return true;
}

bool WriterController::redo() {
    if (m_redoStack.isEmpty()) return false;
    const bool wasCanUndo = canUndo();
    const bool wasCanRedo = canRedo();
    m_undoStack.push_back(makeHistoryState());
    restoreHistoryState(m_redoStack.takeLast());
    emitHistoryChangedIfNeeded(wasCanUndo, wasCanRedo);
    return true;
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
