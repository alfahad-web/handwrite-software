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
}

PathBuildResult PathBuilder::build(const LayoutResult &layout) {
    PathBuildResult result;
    QPointF prevEnd;
    bool hasPrev = false;

    for (const LayoutGlyph &lg : layout.glyphs) {
        if (lg.isMissing) {
            hasPrev = false;
            continue;
        }
        for (const QVector<QPointF> &poly : lg.polylinesCm) {
            if (poly.size() < 2) continue;
            if (hasPrev) {
                const double len = dist(prevEnd, poly.first());
                if (len > 1e-9) {
                    PathSegment seg;
                    seg.travel = true;
                    seg.pointsCm = {prevEnd, poly.first()};
                    result.segments.push_back(seg);
                    result.totalLengthCm += len;
                }
            }
            const double len = polylineLength(poly);
            if (len > 1e-9) {
                PathSegment seg;
                seg.travel = false;
                seg.pointsCm = poly;
                result.segments.push_back(seg);
                result.totalLengthCm += len;
            }
            prevEnd = poly.last();
            hasPrev = true;
        }
    }
    return result;
}

PathBuildResult PathBuilder::buildFromController(WriterController *ctrl) {
    PathBuildResult empty;
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
    return build(layout);
}
