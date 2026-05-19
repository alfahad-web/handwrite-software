#pragma once

#include <QString>

class WriterController;

class WriterProjectService {
public:
    static bool saveProject(const QString &path, WriterController *ctrl, QString *errorMessage);
    static bool loadProject(const QString &path, WriterController *ctrl, QString *errorMessage);
};
