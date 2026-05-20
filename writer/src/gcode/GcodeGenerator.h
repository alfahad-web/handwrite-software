#pragma once

#include <QString>
#include <QVector>

#include "PathBuilder.h"
#include "app/AppSettings.h"

struct GcodeGenerateResult {
    QString gcode;
    QVector<int> pageLineStart;
    int pageCount = 0;
};

class GcodeGenerator {
public:
    static QString generate(const PathBuildResult &path, const AppSettings *settings);
    static GcodeGenerateResult generateWithPageLines(const PathBuildResult &path, const AppSettings *settings);
};
