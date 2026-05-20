#pragma once

#include <QString>
#include <QVector>

#include "PathBuilder.h"
#include "app/AppSettings.h"

struct GcodeGenerateResult {
    QString gcode;
    QVector<int> pageLineStart;
    QVector<int> pageLineCount;
    int pageCount = 0;
};

class GcodeGenerator {
public:
    static QString generate(const PathBuildResult &path, const AppSettings *settings);
    static GcodeGenerateResult generateWithPageLines(const PathBuildResult &path, const AppSettings *settings);
    /** One page in page-local coords (0,0 = plot origin after vertical gap); ends pen-up at X0 Y0. */
    static QString generateSinglePage(const PathBuildResult &path, int pageIndex, const AppSettings *settings);
};
