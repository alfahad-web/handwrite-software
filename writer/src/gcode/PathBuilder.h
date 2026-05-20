#pragma once

#include <QPointF>
#include <QVector>

#include "layout/LayoutEngine.h"

struct PathSegment {
    bool travel = false;
    int pageIndex = 0;
    QVector<QPointF> pointsCm;
};

struct PathPageMap {
    QVector<double> pageStartDistanceCm;
    int pageCount = 0;
    double totalLengthCm = 0;
};

struct PathBuildResult {
    QVector<PathSegment> segments;
    double totalLengthCm = 0;
};

struct PathBuildWithPageMap {
    PathBuildResult path;
    PathPageMap pageMap;
};

class WriterController;

class PathBuilder {
public:
    static PathBuildResult build(const LayoutResult &layout);
    static PathBuildWithPageMap buildWithPageMap(const LayoutResult &layout);
    static PathBuildResult buildFromController(WriterController *ctrl);
    static PathBuildWithPageMap buildWithPageMapFromController(WriterController *ctrl);

    static PathPageMap pageMapFromPath(const PathBuildResult &path, int layoutPageCount);
};
