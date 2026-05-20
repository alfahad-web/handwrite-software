#include "PathBuilder.h"

#include "app/WriterController.h"
#include "app/AppSettings.h"
#include "app/DocumentModel.h"

#include <QLineF>
#include <QtMath>

namespace {
double polylineLength(const QVector<QPointF> &pts) {
    double len = 0;
    for (int i = 1; i < pts.size(); ++i)
        len += QLineF(pts[i - 1], pts[i]).length();
    return len;
}

double dist(const QPointF &a, const QPointF &b) {
    return QLineF(a, b).length();
}

int maxPageIndex(const LayoutResult &layout) {
    int lastPage = 0;
    for (const LayoutGlyph &lg : layout.glyphs)
        lastPage = qMax(lastPage, lg.pageIndex);
    return lastPage;
}
}

PathPageMap PathBuilder::pageMapFromPath(const PathBuildResult &path, int layoutPageCount) {
    PathPageMap map;
    map.totalLengthCm = path.totalLengthCm;
    map.pageCount = qMax(1, layoutPageCount + 1);

    map.pageStartDistanceCm.resize(map.pageCount);
    for (int i = 0; i < map.pageCount; ++i)
        map.pageStartDistanceCm[i] = -1.0;

    double cumulative = 0;
    for (const PathSegment &seg : path.segments) {
        const double len = seg.travel ? (seg.pointsCm.size() >= 2 ? dist(seg.pointsCm[0], seg.pointsCm[1]) : 0)
                                       : polylineLength(seg.pointsCm);
        if (len <= 1e-9) continue;

        if (!seg.travel && seg.pageIndex >= 0 && seg.pageIndex < map.pageCount
            && map.pageStartDistanceCm[seg.pageIndex] < 0) {
            map.pageStartDistanceCm[seg.pageIndex] = cumulative;
        }
        cumulative += len;
    }

    if (map.pageStartDistanceCm[0] < 0)
        map.pageStartDistanceCm[0] = 0;

    double lastKnown = 0;
    for (int p = 0; p < map.pageCount; ++p) {
        if (map.pageStartDistanceCm[p] < 0)
            map.pageStartDistanceCm[p] = lastKnown;
        else
            lastKnown = map.pageStartDistanceCm[p];
    }

    return map;
}

PathBuildResult PathBuilder::build(const LayoutResult &layout) {
    return buildWithPageMap(layout).path;
}

PathBuildWithPageMap PathBuilder::buildWithPageMap(const LayoutResult &layout) {
    PathBuildWithPageMap out;
    PathBuildResult &result = out.path;
    QPointF prevEnd;
    bool hasPrev = false;
    int prevPage = 0;

    for (const LayoutGlyph &lg : layout.glyphs) {
        if (lg.isMissing) {
            hasPrev = false;
            continue;
        }
        const int glyphPage = lg.pageIndex;
        for (const QVector<QPointF> &poly : lg.polylinesCm) {
            if (poly.size() < 2) continue;
            if (hasPrev) {
                const double len = dist(prevEnd, poly.first());
                if (len > 1e-9) {
                    PathSegment seg;
                    seg.travel = true;
                    seg.pageIndex = glyphPage;
                    seg.pointsCm = {prevEnd, poly.first()};
                    result.segments.push_back(seg);
                    result.totalLengthCm += len;
                }
            }
            const double len = polylineLength(poly);
            if (len > 1e-9) {
                PathSegment seg;
                seg.travel = false;
                seg.pageIndex = glyphPage;
                seg.pointsCm = poly;
                result.segments.push_back(seg);
                result.totalLengthCm += len;
            }
            prevEnd = poly.last();
            prevPage = glyphPage;
            hasPrev = true;
        }
    }

    out.pageMap = pageMapFromPath(result, maxPageIndex(layout));
    return out;
}

PathBuildResult PathBuilder::buildFromController(WriterController *ctrl) {
    return buildWithPageMapFromController(ctrl).path;
}

PathBuildWithPageMap PathBuilder::buildWithPageMapFromController(WriterController *ctrl) {
    PathBuildWithPageMap empty;
    if (!ctrl || !ctrl->settings()) return empty;

    const AppSettings *st = ctrl->settings();
    const LayoutResult layout = LayoutEngine::layout(
        ctrl->document()->text(),
        ctrl->fontCatalog(),
        st->fontUnitToCm(),
        st->pageWidthCm(),
        st->pageHeightCm(),
        st->leftMarginCm(),
        st->rightMarginCm(),
        st->verticalGapCm(),
        st->hxCm(),
        st->hyCm(),
        st->lineHeightCm(),
        st->joinDistMm(),
        ctrl->manualAnchors()
    );
    return buildWithPageMap(layout);
}
