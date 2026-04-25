#pragma once

#include <QObject>
#include <QString>

#include "../core/EditorStore.h"

class ProjectService : public QObject {
    Q_OBJECT
public:
    explicit ProjectService(QObject *parent = nullptr);

    bool saveProject(const QString &path, const EditorStore &store, QString *errorMessage = nullptr) const;
    bool loadProject(const QString &path, EditorStore *store, QString *errorMessage = nullptr) const;
};
