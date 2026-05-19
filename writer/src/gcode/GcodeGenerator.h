#pragma once

#include <QString>

#include "PathBuilder.h"
#include "app/AppSettings.h"

class GcodeGenerator {
public:
    static QString generate(const PathBuildResult &path, const AppSettings *settings);
};
