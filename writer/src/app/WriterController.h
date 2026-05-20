#pragma once

#include <QHash>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVector>
#include <QElapsedTimer>
#include <QVariantMap>

#include "AppSettings.h"
#include "DocumentModel.h"
#include "cnc/GrblConnection.h"
#include "font/FontLoader.h"
#include "gcode/GcodeController.h"
#include "gcode/PathBuilder.h"

class WriterController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged)
    Q_PROPERTY(QString fontFolderPath READ fontFolderPath NOTIFY fontFolderPathChanged)
    Q_PROPERTY(QString fontStatus READ fontStatus NOTIFY fontStatusChanged)
    Q_PROPERTY(QString projectFilePath READ projectFilePath NOTIFY projectFilePathChanged)
    Q_PROPERTY(bool documentDirty READ documentDirty NOTIFY documentDirtyChanged)
    Q_PROPERTY(DocumentModel *document READ document CONSTANT)
    Q_PROPERTY(AppSettings *settings READ settings CONSTANT)
    Q_PROPERTY(GcodeController *gcode READ gcode CONSTANT)
    Q_PROPERTY(GrblConnection *grbl READ grbl CONSTANT)
    Q_PROPERTY(bool settingsOpen READ settingsOpen WRITE setSettingsOpen NOTIFY settingsOpenChanged)
    Q_PROPERTY(bool runActive READ runActive NOTIFY runActiveChanged)
    Q_PROPERTY(bool runPaused READ runPaused NOTIFY runPausedChanged)
    Q_PROPERTY(bool runArmed READ runArmed NOTIFY runArmedChanged)
    Q_PROPERTY(int runStartPage READ runStartPage NOTIFY runStartPageChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY pageCountChanged)
    Q_PROPERTY(double runStartDistanceCm READ runStartDistanceCm NOTIFY runPathChanged)
    Q_PROPERTY(double runEndDistanceCm READ runEndDistanceCm NOTIFY runPathChanged)
    Q_PROPERTY(int executingPage READ executingPage NOTIFY runPathChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY historyChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY historyChanged)

public:
    explicit WriterController(QObject *parent = nullptr);

    QString viewMode() const { return m_viewMode; }
    void setViewMode(const QString &m);

    QString fontFolderPath() const { return m_fontFolderPath; }
    QString fontStatus() const { return m_fontStatus; }

    QString projectFilePath() const { return m_projectFilePath; }
    bool documentDirty() const { return m_documentDirty; }

    DocumentModel *document() const { return m_document; }
    AppSettings *settings() const { return m_settings; }
    GcodeController *gcode() const { return m_gcode; }
    GrblConnection *grbl() const { return m_grbl; }

    bool settingsOpen() const { return m_settingsOpen; }
    void setSettingsOpen(bool v);

    bool runActive() const { return m_runActive; }
    bool runPaused() const { return m_runPaused; }
    bool runArmed() const { return m_runArmed; }
    int runStartPage() const { return m_runStartPage; }
    int pageCount() const { return m_pathPageMap.pageCount; }
    double runStartDistanceCm() const { return m_runStartDistanceCm; }
    double runEndDistanceCm() const { return m_runEndDistanceCm; }
    int executingPage() const { return m_executingPage; }
    bool pageLocalMachineCoords() const { return m_pageLocalGrblStream; }
    const PathPageMap &pathPageMap() const { return m_pathPageMap; }

    bool canUndo() const { return !m_undoStack.isEmpty(); }
    bool canRedo() const { return !m_redoStack.isEmpty(); }

    const FontCatalog &fontCatalog() const { return m_fontCatalog; }

    QHash<int, QPointF> manualAnchors() const { return m_manualAnchors; }
    void setManualAnchors(const QHash<int, QPointF> &h);
    void setManualAnchor(int docIndex, const QPointF &anchorCm);
    void clearAllManualAnchors();

    Q_INVOKABLE void pickFontFolder();
    Q_INVOKABLE void reloadFonts();
    Q_INVOKABLE void startRun();
    Q_INVOKABLE void pauseRun();
    Q_INVOKABLE void resumeRun();
    Q_INVOKABLE void stopRun();
    Q_INVOKABLE void stopRunPreserveCnc();
    Q_INVOKABLE void advanceRunToPage(int page);
    Q_INVOKABLE void clearRunArm();
    Q_INVOKABLE void refreshPageMap();
    Q_INVOKABLE void finishPageRun();
    Q_INVOKABLE void onRunApproachComplete();

    Q_INVOKABLE void notifyLineHeightCollision(bool exceeds);
    Q_INVOKABLE bool generateGcode();
    Q_INVOKABLE void pushUndoSnapshot();
    Q_INVOKABLE bool undo();
    Q_INVOKABLE bool redo();

    Q_INVOKABLE void newWriterProject();
    Q_INVOKABLE void openWriterProject();
    Q_INVOKABLE void saveWriterProject();
    Q_INVOKABLE void saveWriterProjectAs();
    Q_INVOKABLE bool loadWriterProjectFromPath(const QString &path);
    Q_INVOKABLE QVariantMap pageBenchmark(int page) const;

    void markSaved();
    void resetToEmptyProject(bool resetSettingsToDefaults);

    void beginProjectLoad();
    void endProjectLoad();
    void setFontFolderPathAndLoad(const QString &path, bool emitMissingIfNotFound);

signals:
    void viewModeChanged();
    void fontFolderPathChanged();
    void fontStatusChanged();
    void projectFilePathChanged();
    void documentDirtyChanged();
    void layoutInvalidated();
    void settingsOpenChanged();
    void runActiveChanged();
    void runPausedChanged();
    void runArmedChanged();
    void runStartPageChanged();
    void pageCountChanged();
    void runPathChanged();
    void runArmVisualsChanged();
    void lineHeightCollisionWarning();
    void fontFolderMissing(const QString &path);
    void projectIoError(const QString &message);
    void historyChanged();

private:
    struct HistoryState {
        QString documentText;
        QHash<int, QPointF> manualAnchors;
        QString viewMode;
        double feedRateCmPerS = 2.0;
        double pageWidthCm = 21.0;
        double pageHeightCm = 29.7;
        double leftMarginCm = 1.5;
        double rightMarginCm = 1.5;
        double verticalGapCm = 0.5;
        double hxCm = 0.2;
        double hyCm = 0.5;
        double lineHeightCm = 0.45;
        double fontUnitToCm = 0.0001;
        double joinDistMm = 0.0;
        double backlashYStartMm = 0.0;
        double backlashYEndMm = 297.0;
        double xErrorNearMm = 0.0;
        double xErrorMm = 0.0;
        double yErrorNearMm = 0.0;
        double yErrorMm = 0.0;
        double simplifyToleranceMm = 0.0;
        double minSegmentMm = 0.05;
        double collinearToleranceMm = 0.02;
        QString streamingPreset = QStringLiteral("balanced");
        bool arcFitEnabled = false;
        double arcFitToleranceMm = 0.05;
        double grblJunctionDeviation = 0.03;
        double grblAccelX = 300.0;
        double grblAccelY = 300.0;
        bool servoSnapMode = false;
        double servoUpS = 0.0;
        double servoDownS = 1000.0;
        double penUpZ = 30.0;
        double penDownZ = -5.0;
        double previewDisplayScale = 1.0;
    };

    HistoryState makeHistoryState() const;
    void restoreHistoryState(const HistoryState &state);
    void pushUndoState();
    void clearHistory();
    void emitHistoryChangedIfNeeded(bool wasCanUndo, bool wasCanRedo);
    void applySettingsSnapshot(const HistoryState &state);
    void loadFontsFromPath(const QString &path, bool emitMissingIfNotFound);
    void onDocumentTextChanged();
    void setDocumentDirty(bool dirty);
    void setProjectFilePath(const QString &path);
    void bumpDirty();
    void setRunArmed(bool armed);
    void setRunStartPage(int page);
    double pageStartDistance(int page) const;
    double pageEndDistance(int page) const;
    void stopRunInternal(bool clearArm, bool abortGrbl);
    void onMachineIdleAfterPageStream();

    DocumentModel *m_document = nullptr;
    AppSettings *m_settings = nullptr;
    GcodeController *m_gcode = nullptr;
    GrblConnection *m_grbl = nullptr;
    QString m_viewMode = QStringLiteral("typing");
    QString m_fontFolderPath;
    QString m_fontStatus;
    QString m_projectFilePath;
    QString m_lastDocumentText;
    FontCatalog m_fontCatalog;
    QHash<int, QPointF> m_manualAnchors;
    bool m_settingsOpen = false;
    bool m_runActive = false;
    bool m_runPaused = false;
    bool m_runArmed = false;
    int m_runStartPage = 0;
    int m_executingPage = 0;
    double m_runStartDistanceCm = 0;
    double m_runEndDistanceCm = 0;
    bool m_expectPageStreamComplete = false;
    bool m_deferGrblStream = false;
    bool m_pageLocalGrblStream = false;
    bool m_waitingMachineIdleAfterPage = false;
    QElapsedTimer m_pageRunTimer;
    int m_currentPageBenchmarkLineCount = 0;
    QString m_pendingGrblSlice;
    PathPageMap m_pathPageMap;
    bool m_wasLineHeightExceeding = false;
    bool m_documentDirty = false;
    bool m_suppressDirty = false;
    bool m_blockAnchorRemap = false;
    bool m_restoringHistory = false;
    bool m_suppressUndo = false;
    QVector<HistoryState> m_undoStack;
    QVector<HistoryState> m_redoStack;
};
