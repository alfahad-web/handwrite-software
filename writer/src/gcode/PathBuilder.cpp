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

double pointToSegmentDistance(const QPointF &p, const QPointF &a, const QPointF &b) {
    const double dx = b.x() - a.x();
    const double dy = b.y() - a.y();
    const double len2 = dx * dx + dy * dy;
    if (len2 <= 1e-12) return dist(p, a);
    const double t = qBound(0.0, ((p.x() - a.x()) * dx + (p.y() - a.y()) * dy) / len2, 1.0);
    const QPointF proj(a.x() + dx * t, a.y() + dy * t);
    return dist(p, proj);
}

QVector<QPointF> shortSegmentCull(const QVector<QPointF> &pts, double minSegmentCm) {
    if (pts.size() < 2 || minSegmentCm <= 1e-12) return pts;
    QVector<QPointF> out;
    out.reserve(pts.size());
    out.push_back(pts.first());
    for (int i = 1; i < pts.size(); ++i) {
        if (dist(out.last(), pts[i]) >= minSegmentCm)
            out.push_back(pts[i]);
    }
    if (out.size() == 1 && pts.size() > 1)
        out.push_back(pts.last());
    return out;
}

QVector<QPointF> mergeCollinear(const QVector<QPointF> &pts, double toleranceCm) {
    if (pts.size() < 3 || toleranceCm <= 1e-12) return pts;
    QVector<QPointF> out;
    out.reserve(pts.size());
    out.push_back(pts[0]);
    out.push_back(pts[1]);
    for (int i = 2; i < pts.size(); ++i) {
        const QPointF c = pts[i];
        while (out.size() >= 2) {
            const QPointF a = out[out.size() - 2];
            const QPointF b = out[out.size() - 1];
            if (pointToSegmentDistance(b, a, c) <= toleranceCm)
                out.removeLast();
            else
                break;
        }
        out.push_back(c);
    }
    return out;
}

void douglasPeuckerRec(const QVector<QPointF> &pts, int start, int end, double toleranceCm,
                       QVector<bool> *keep) {
    if (!keep || end <= start + 1) return;
    double maxDist = -1.0;
    int maxIdx = -1;
    for (int i = start + 1; i < end; ++i) {
        const double d = pointToSegmentDistance(pts[i], pts[start], pts[end]);
        if (d > maxDist) {
            maxDist = d;
            maxIdx = i;
        }
    }
    if (maxIdx > 0 && maxDist > toleranceCm) {
        keep->operator[](maxIdx) = true;
        douglasPeuckerRec(pts, start, maxIdx, toleranceCm, keep);
        douglasPeuckerRec(pts, maxIdx, end, toleranceCm, keep);
    }
}

QVector<QPointF> douglasPeucker(const QVector<QPointF> &pts, double toleranceCm) {
    if (pts.size() < 3 || toleranceCm <= 1e-12) return pts;
    QVector<bool> keep(pts.size(), false);
    keep[0] = true;
    keep[pts.size() - 1] = true;
    douglasPeuckerRec(pts, 0, pts.size() - 1, toleranceCm, &keep);
    QVector<QPointF> out;
    out.reserve(pts.size());
    for (int i = 0; i < pts.size(); ++i) {
        if (keep[i]) out.push_back(pts[i]);
    }
    return out;
}

QVector<QPointF> simplifyPolyline(const QVector<QPointF> &pts, const AppSettings *settings) {
    if (!settings || pts.size() < 2) return pts;
    const double minSegCm = qMax(0.0, settings->minSegmentMm()) / 10.0;
    const double colCm = qMax(0.0, settings->collinearToleranceMm()) / 10.0;
    const double dpCm = qMax(0.0, settings->simplifyToleranceMm()) / 10.0;
    QVector<QPointF> out = shortSegmentCull(pts, minSegCm);
    out = mergeCollinear(out, colCm);
    out = douglasPeucker(out, dpCm);
    out = shortSegmentCull(out, minSegCm);
    return out;
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

PathBuildResult PathBuilder::build(const LayoutResult &layout, const AppSettings *settings) {
    return buildWithPageMap(layout, settings).path;
}

PathBuildWithPageMap PathBuilder::buildWithPageMap(const LayoutResult &layout, const AppSettings *settings) {
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
            const QVector<QPointF> reduced = simplifyPolyline(poly, settings);
            if (reduced.size() < 2) continue;
            if (hasPrev) {
                const double len = dist(prevEnd, reduced.first());
                if (len > 1e-9) {
                    PathSegment seg;
                    seg.travel = true;
                    seg.pageIndex = glyphPage;
                    seg.pointsCm = {prevEnd, reduced.first()};
                    result.segments.push_back(seg);
                    result.totalLengthCm += len;
                }
            }
            const double len = polylineLength(reduced);
            if (len > 1e-9) {
                PathSegment seg;
                seg.travel = false;
                seg.pageIndex = glyphPage;
                seg.pointsCm = reduced;
                result.segments.push_back(seg);
                result.totalLengthCm += len;
            }
            prevEnd = reduced.last();
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
    return buildWithPageMap(layout, st);
}
