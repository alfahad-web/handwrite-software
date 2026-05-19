#pragma once

#include <QHash>
#include <QObject>
#include <QPointF>
#include <QString>

#include "AppSettings.h"
#include "DocumentModel.h"
#include "cnc/GrblConnection.h"
#include "font/FontLoader.h"
#include "gcode/GcodeController.h"

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

    Q_INVOKABLE void notifyLineHeightCollision(bool exceeds);

    Q_INVOKABLE void newWriterProject();
    Q_INVOKABLE void openWriterProject();
    Q_INVOKABLE void saveWriterProject();
    Q_INVOKABLE void saveWriterProjectAs();
    Q_INVOKABLE bool loadWriterProjectFromPath(const QString &path);

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
    void lineHeightCollisionWarning();
    void fontFolderMissing(const QString &path);
    void projectIoError(const QString &message);

private:
    void loadFontsFromPath(const QString &path, bool emitMissingIfNotFound);
    void onDocumentTextChanged();
    void setDocumentDirty(bool dirty);
    void setProjectFilePath(const QString &path);
    void bumpDirty();

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
    bool m_wasLineHeightExceeding = false;
    bool m_documentDirty = false;
    bool m_suppressDirty = false;
    bool m_blockAnchorRemap = false;
};
