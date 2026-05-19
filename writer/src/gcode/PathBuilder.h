#pragma once

#include <QPointF>
#include <QVector>

#include "layout/LayoutEngine.h"

struct PathSegment {
    bool travel = false;
    QVector<QPointF> pointsCm;
};

struct PathBuildResult {
    QVector<PathSegment> segments;
    double totalLengthCm = 0;
};

class WriterController;

class PathBuilder {
public:
    static PathBuildResult build(const LayoutResult &layout);
    static PathBuildResult buildFromController(WriterController *ctrl);
};
