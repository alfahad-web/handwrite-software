#pragma once

#include <QObject>

#include "../core/EditorStore.h"
#include "../services/ExportService.h"
#include "../services/FileService.h"

class AppController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
public:
    explicit AppController(EditorStore *store, QObject *parent = nullptr);

    QString statusMessage() const;

    Q_INVOKABLE void openOrCreateFile();
    Q_INVOKABLE void appendSelection();
    Q_INVOKABLE void appendSelectionAndClose();
    Q_INVOKABLE bool canWriteSelection() const;
    Q_INVOKABLE void setBoardCursorActive(bool active);

signals:
    void statusMessageChanged();

private:
    bool writeSelectionInternal();

    EditorStore *m_store;
    ExportService m_exportService;
    FileService m_fileService;
    QString m_statusMessage;
    bool m_boardCursorActive = false;
};
