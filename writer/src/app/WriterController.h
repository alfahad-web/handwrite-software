#pragma once

#include <QHash>
#include <QObject>
#include <QPointF>
#include <QString>

#include "AppSettings.h"
#include "DocumentModel.h"
#include "font/FontLoader.h"

class WriterController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged)
    Q_PROPERTY(QString fontFolderPath READ fontFolderPath NOTIFY fontFolderPathChanged)
    Q_PROPERTY(QString fontStatus READ fontStatus NOTIFY fontStatusChanged)
    Q_PROPERTY(DocumentModel *document READ document CONSTANT)
    Q_PROPERTY(AppSettings *settings READ settings CONSTANT)
    Q_PROPERTY(bool settingsOpen READ settingsOpen WRITE setSettingsOpen NOTIFY settingsOpenChanged)
    Q_PROPERTY(bool runActive READ runActive NOTIFY runActiveChanged)

public:
    explicit WriterController(QObject *parent = nullptr);

    QString viewMode() const { return m_viewMode; }
    void setViewMode(const QString &m);

    QString fontFolderPath() const { return m_fontFolderPath; }
    QString fontStatus() const { return m_fontStatus; }

    DocumentModel *document() const { return m_document; }
    AppSettings *settings() const { return m_settings; }

    bool settingsOpen() const { return m_settingsOpen; }
    void setSettingsOpen(bool v);

    bool runActive() const { return m_runActive; }

    const QHash<QChar, GlyphData> &fontMap() const { return m_fontMap; }

    Q_INVOKABLE void pickFontFolder();
    Q_INVOKABLE void reloadFonts();
    Q_INVOKABLE void startRun();
    Q_INVOKABLE void stopRun();

    Q_INVOKABLE void notifyLineHeightCollision(bool exceeds);

    QHash<int, QPointF> anchorOverrides() const { return m_anchorOverrides; }
    void setAnchorOverrides(const QHash<int, QPointF> &h);
    void clearAnchorOverrides();

signals:
    void viewModeChanged();
    void fontFolderPathChanged();
    void fontStatusChanged();
    void layoutInvalidated();
    void settingsOpenChanged();
    void runActiveChanged();
    void lineHeightCollisionWarning();

private:
    void loadFontsFromPath(const QString &path);

    DocumentModel *m_document = nullptr;
    AppSettings *m_settings = nullptr;
    QString m_viewMode = QStringLiteral("typing");
    QString m_fontFolderPath;
    QString m_fontStatus;
    QHash<QChar, GlyphData> m_fontMap;
    QHash<int, QPointF> m_anchorOverrides;
    bool m_settingsOpen = false;
    bool m_runActive = false;
    bool m_wasLineHeightExceeding = false;
};
