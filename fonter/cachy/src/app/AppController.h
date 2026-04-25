#pragma once

#include <QObject>

#include "../core/EditorStore.h"
#include "../services/ExportService.h"
#include "../services/FileService.h"
#include "../services/ProjectService.h"

class AppController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
public:
    explicit AppController(EditorStore *store, QObject *parent = nullptr);

    QString statusMessage() const;

    Q_INVOKABLE void newProject();
    Q_INVOKABLE void openProject();
    Q_INVOKABLE void saveProject();
    Q_INVOKABLE void generateFonts();
    Q_INVOKABLE void deleteSelectedSelection();
    Q_INVOKABLE void assignSelectionCharacter(const QString &selectionId, const QString &text, const QString &joinMode);
    Q_INVOKABLE void setBoardCursorActive(bool active);
    Q_INVOKABLE void setEraseCursorActive(bool active, int radiusPx, int zoomPercent);

signals:
    void statusMessageChanged();

private:
    EditorStore *m_store;
    FileService m_fileService;
    ProjectService m_projectService;
    QString m_statusMessage;
    bool m_boardCursorActive = false;
    bool m_eraseCursorActive = false;
};
