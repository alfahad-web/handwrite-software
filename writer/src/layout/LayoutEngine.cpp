#include "LayoutEngine.h"

#include <QLineF>
#include <QtMath>
#include <algorithm>
#include <limits>

namespace {
bool hasForced(const QHash<int, QPointF> &forced, int i) {
    return forced.contains(i) && std::isfinite(forced[i].x()) && std::isfinite(forced[i].y());
}

GlyphData missingGlyph(double lineHeightCm, double fontUnitToCm) {
    const double wCm = qMax(0.05, lineHeightCm * 0.25);
    const double hCm = lineHeightCm * 0.85;
    const double wFu = wCm / fontUnitToCm;
    const double hFu = hCm / fontUnitToCm;
    GlyphData g;
    QVector<QPointF> box = {
        QPointF(0, 0),
        QPointF(wFu, 0),
        QPointF(wFu, hFu),
        QPointF(0, hFu),
        QPointF(0, 0)
    };
    g.polylinesFontUnits.push_back(box);
    g.bboxFontUnits = QRectF(0, 0, wFu, hFu);
    g.sourceFile = QStringLiteral("<missing>");
    g.joinMode = QStringLiteral("N");
    return g;
}

GlyphData spaceGlyph(double lineHeightCm, double fontUnitToCm) {
    const double wCm = qMax(0.02, lineHeightCm * 0.35);
    const double wFu = wCm / fontUnitToCm;
    const double hFu = qMax(1.0, lineHeightCm * 0.2 / fontUnitToCm);
    GlyphData g;
    g.bboxFontUnits = QRectF(0, 0, wFu, hFu);
    g.sourceFile = QStringLiteral("<space>");
    g.joinMode = QStringLiteral("N");
    return g;
}

// Font .txt coords are y-up (see fonter/cachy ExportService); map to Qt y-down cm space.
QVector<QVector<QPointF>> transformPolylines(const GlyphData &g, double fontUnitToCm, const QPointF &anchorCm) {
    const QPointF anchorFu = g.hasGlyphAnchor ? g.glyphAnchorFontUnits : QPointF(0, 0);
    QVector<QVector<QPointF>> out;
    out.reserve(g.polylinesFontUnits.size());
    for (const QVector<QPointF> &poly : g.polylinesFontUnits) {
        QVector<QPointF> t;
        t.reserve(poly.size());
        for (const QPointF &p : poly) {
            const double x = anchorCm.x() + (p.x() - anchorFu.x()) * fontUnitToCm;
            const double y = anchorCm.y() - (p.y() - anchorFu.y()) * fontUnitToCm;
            t.push_back(QPointF(x, y));
        }
        if (t.size() >= 2) out.push_back(t);
    }
    return out;
}

QRectF bboxOfPolys(const QVector<QVector<QPointF>> &polys) {
    QRectF r;
    bool any = false;
    for (const QVector<QPointF> &poly : polys) {
        for (const QPointF &p : poly) {
            if (!any) {
                r = QRectF(p, QSizeF(0, 0));
                any = true;
            } else {
                r.setLeft(qMin(r.left(), p.x()));
                r.setRight(qMax(r.right(), p.x()));
                r.setTop(qMin(r.top(), p.y()));
                r.setBottom(qMax(r.bottom(), p.y()));
            }
        }
    }
    return r;
}

QPointF toVectorCm(const QPointF &vFu, double fontUnitToCm) {
    return QPointF(vFu.x() * fontUnitToCm, -vFu.y() * fontUnitToCm);
}

int previousNonCR(const QString &text, int i, bool *exists) {
    int j = i - 1;
    while (j >= 0 && text.at(j) == QChar('\r')) --j;
    if (j < 0) {
        *exists = false;
        return -1;
    }
    *exists = true;
    return j;
}

int nextNonCR(const QString &text, int i, bool *exists) {
    int j = i + 1;
    while (j < text.size() && text.at(j) == QChar('\r')) ++j;
    if (j >= text.size()) {
        *exists = false;
        return -1;
    }
    *exists = true;
    return j;
}

QVector<QString> joinPriorityForContext(
    bool afterLeftBoundary,
    const QString &prevJoinMode
) {
    // After a space/start: R > N > LR > L
    if (afterLeftBoundary) {
        return {QStringLiteral("R"), QStringLiteral("N"), QStringLiteral("LR"), QStringLiteral("L")};
    }
    // Before a space OR middle:
    // - if prev N or L: R > N > LR > L
    // - if prev LR or R: LR > L > N > R
    if (prevJoinMode == QStringLiteral("LR") || prevJoinMode == QStringLiteral("R")) {
        return {QStringLiteral("LR"), QStringLiteral("L"), QStringLiteral("N"), QStringLiteral("R")};
    }
    return {QStringLiteral("R"), QStringLiteral("N"), QStringLiteral("LR"), QStringLiteral("L")};
}

const GlyphData *pickByJoin(const QVector<GlyphData> &variants, const QString &joinMode) {
    const GlyphData *best = nullptr;
    for (const GlyphData &g : variants) {
        if (g.joinMode != joinMode) continue;
        if (!best || g.variantIndex < best->variantIndex
            || (g.variantIndex == best->variantIndex && g.sourcePriority < best->sourcePriority)
            || (g.variantIndex == best->variantIndex && g.sourcePriority == best->sourcePriority
                && g.sourceFile < best->sourceFile)) {
            best = &g;
        }
    }
    return best;
}

const GlyphData *pickBestAny(const QVector<GlyphData> &variants) {
    const GlyphData *best = nullptr;
    for (const GlyphData &g : variants) {
        if (!best || g.sourcePriority < best->sourcePriority
            || (g.sourcePriority == best->sourcePriority && g.variantIndex < best->variantIndex)
            || (g.sourcePriority == best->sourcePriority && g.variantIndex == best->variantIndex
                && g.sourceFile < best->sourceFile)) {
            best = &g;
        }
    }
    return best;
}

const GlyphData *resolveGlyph(
    const FontCatalog &catalog,
    const QChar ch,
    const QVector<QString> &priorities
) {
    auto it = catalog.variantsByChar.constFind(ch);
    if (it == catalog.variantsByChar.constEnd()) return nullptr;
    const QVector<GlyphData> &variants = it.value();
    for (const QString &join : priorities) {
        if (const GlyphData *picked = pickByJoin(variants, join)) return picked;
    }
    return pickBestAny(variants);
}

bool isJoinLeft(const QString &joinMode) {
    return joinMode == QStringLiteral("R") || joinMode == QStringLiteral("LR");
}

bool isJoinRight(const QString &joinMode) {
    return joinMode == QStringLiteral("L") || joinMode == QStringLiteral("LR");
}

int pickLeftExitPolyline(const LayoutGlyph &g) {
    int best = -1;
    double bestX = -std::numeric_limits<double>::infinity();
    for (int i = 0; i < g.polylinesCm.size(); ++i) {
        const QVector<QPointF> &poly = g.polylinesCm[i];
        if (poly.size() < 2) continue;
        const double x = poly.last().x();
        if (x > bestX) {
            bestX = x;
            best = i;
        }
    }
    return best;
}

int pickRightEntryPolyline(const LayoutGlyph &g) {
    int best = -1;
    double bestX = std::numeric_limits<double>::infinity();
    for (int i = 0; i < g.polylinesCm.size(); ++i) {
        const QVector<QPointF> &poly = g.polylinesCm[i];
        if (poly.size() < 2) continue;
        const double x = poly.first().x();
        if (x < bestX) {
            bestX = x;
            best = i;
        }
    }
    return best;
}

bool trimFromEnd(const QVector<QPointF> &poly, double trimCm, QVector<QPointF> *out, QPointF *trimmedEnd) {
    if (poly.size() < 2) return false;
    if (trimCm <= 1e-12) {
        *out = poly;
        *trimmedEnd = poly.last();
        return true;
    }
    double total = 0;
    for (int i = 1; i < poly.size(); ++i) total += QLineF(poly[i - 1], poly[i]).length();
    if (trimCm >= total - 1e-9) return false;

    double rem = trimCm;
    for (int i = poly.size() - 1; i > 0; --i) {
        const QPointF a = poly[i - 1];
        const QPointF b = poly[i];
        const double seg = QLineF(a, b).length();
        if (rem < seg) {
            const double t = (seg - rem) / seg;
            const QPointF p = a + (b - a) * t;
            QVector<QPointF> kept;
            kept.reserve(i + 1);
            for (int k = 0; k < i; ++k) kept.push_back(poly[k]);
            kept.push_back(p);
            if (kept.size() < 2) return false;
            *out = kept;
            *trimmedEnd = p;
            return true;
        }
        rem -= seg;
    }
    return false;
}

bool trimFromStart(const QVector<QPointF> &poly, double trimCm, QVector<QPointF> *out, QPointF *trimmedStart) {
    if (poly.size() < 2) return false;
    if (trimCm <= 1e-12) {
        *out = poly;
        *trimmedStart = poly.first();
        return true;
    }
    double total = 0;
    for (int i = 1; i < poly.size(); ++i) total += QLineF(poly[i - 1], poly[i]).length();
    if (trimCm >= total - 1e-9) return false;

    double rem = trimCm;
    for (int i = 1; i < poly.size(); ++i) {
        const QPointF a = poly[i - 1];
        const QPointF b = poly[i];
        const double seg = QLineF(a, b).length();
        if (rem < seg) {
            const double t = rem / seg;
            const QPointF p = a + (b - a) * t;
            QVector<QPointF> kept;
            kept.reserve(poly.size() - i + 1);
            kept.push_back(p);
            for (int k = i; k < poly.size(); ++k) kept.push_back(poly[k]);
            if (kept.size() < 2) return false;
            *out = kept;
            *trimmedStart = p;
            return true;
        }
        rem -= seg;
    }
    return false;
}

QVector<QPointF> sampleCubicBezier(
    const QPointF &p0,
    const QPointF &p1,
    const QPointF &p2,
    const QPointF &p3,
    int steps
) {
    QVector<QPointF> pts;
    pts.reserve(steps + 1);
    for (int i = 0; i <= steps; ++i) {
        const double t = static_cast<double>(i) / static_cast<double>(steps);
        const double u = 1.0 - t;
        const QPointF p =
            p0 * (u * u * u) + p1 * (3.0 * u * u * t) + p2 * (3.0 * u * t * t) + p3 * (t * t * t);
        pts.push_back(p);
    }
    return pts;
}

void applyStrokeJoins(QVector<LayoutGlyph> *glyphs, double joinDistMm) {
    if (joinDistMm < 0) joinDistMm = 0;
    const double halfCm = (joinDistMm / 10.0) / 2.0;
    for (int i = 0; i + 1 < glyphs->size(); ++i) {
        LayoutGlyph &left = (*glyphs)[i];
        LayoutGlyph &right = (*glyphs)[i + 1];
        if (right.docIndex != left.docIndex + 1) continue;
        if (left.pageIndex != right.pageIndex || left.lineIndex != right.lineIndex) continue;
        if (left.ch.isSpace() || right.ch.isSpace()) continue;
        if (!isJoinLeft(left.joinMode) || !isJoinRight(right.joinMode)) continue;

        const int li = pickLeftExitPolyline(left);
        const int ri = pickRightEntryPolyline(right);
        if (li < 0 || ri < 0) continue;

        const QVector<QPointF> leftPoly = left.polylinesCm[li];
        const QVector<QPointF> rightPoly = right.polylinesCm[ri];
        if (leftPoly.size() < 2 || rightPoly.size() < 2) continue;

        QVector<QPointF> leftTrimmed;
        QVector<QPointF> rightTrimmed;
        QPointF t0;
        QPointF t1;
        if (!trimFromEnd(leftPoly, halfCm, &leftTrimmed, &t0)) continue;
        if (!trimFromStart(rightPoly, halfCm, &rightTrimmed, &t1)) continue;

        const QPointF e0 = leftPoly.last();
        const QPointF e1 = rightPoly.first();
        const QVector<QPointF> bridge = sampleCubicBezier(t0, e0, e1, t1, 20);

        QVector<QPointF> merged = leftTrimmed;
        for (int b = 1; b < bridge.size(); ++b) merged.push_back(bridge[b]);
        left.polylinesCm[li] = merged;
        right.polylinesCm[ri] = rightTrimmed;
    }
}
}

LayoutResult LayoutEngine::layout(
    const QString &text,
    const FontCatalog &fontCatalog,
    double fontUnitToCm,
    double pageWidthCm,
    double pageHeightCm,
    double leftMarginCm,
    double rightMarginCm,
    double verticalGapCm,
    double hxCm,
    double hyCm,
    double lineHeightCm,
    double joinDistMm,
    const QHash<int, QPointF> &forcedBottomLeftCmByDocIndex
) {
    LayoutResult res;
    const double contentLeft = leftMarginCm + hxCm;
    const double contentRight = pageWidthCm - rightMarginCm;
    const GlyphData missing = missingGlyph(lineHeightCm, fontUnitToCm);
    const GlyphData space = spaceGlyph(lineHeightCm, fontUnitToCm);

    // verticalGapCm is a top band inside each page (included in pageHeightCm); pages stack by full height only.
    auto pageTopY = [&](int p) { return p * pageHeightCm; };

    int pageIndex = 0;
    int lineIndex = 0;
    double cursorX = contentLeft;
    double baselineY = 0;
    bool hasChain = false;
    QPointF prevAnchorCm;
    QPointF prevAdvanceVectorCm;
    int prevPage = 0;
    int prevLine = 0;
    QHash<int, QString> chosenJoinModeByDocIndex;

    auto recomputeBaseline = [&]() {
        baselineY = pageTopY(pageIndex) + verticalGapCm + hyCm + (lineIndex + 1) * lineHeightCm;
    };

    auto ensurePage = [&]() {
        while (verticalGapCm + hyCm + (lineIndex + 1) * lineHeightCm > pageHeightCm + 1e-9) {
            ++pageIndex;
            lineIndex = 0;
            cursorX = contentLeft;
            recomputeBaseline();
        }
    };

    recomputeBaseline();
    ensurePage();

    auto advanceXForGlyph = [&](const GlyphData &g) -> double {
        const QPointF anchorFu = g.hasGlyphAnchor ? g.glyphAnchorFontUnits : QPointF(0, 0);
        const double fallbackAdvanceCm = qMax(0.02, g.bboxFontUnits.width() * fontUnitToCm);
        if (!g.hasSelectionBox) return fallbackAdvanceCm;
        const QPointF brVectorFu = g.selectionBottomRightFontUnits - anchorFu;
        const QPointF brCm = toVectorCm(brVectorFu, fontUnitToCm);
        if (!std::isfinite(brCm.x())) return fallbackAdvanceCm;
        return qMax(0.0, brCm.x());
    };

    auto estimateWordWidthCm = [&](int startIndex, const QString &initialPrevJoinMode) -> double {
        double widthCm = 0.0;
        QString simulatedPrevJoin = initialPrevJoinMode;
        for (int j = startIndex; j < text.size(); ++j) {
            const QChar cj = text.at(j);
            if (cj == QChar('\r')) continue;
            if (cj == QChar('\n') || cj.isSpace()) break;
            bool hasPrevJ = false;
            const int prevJ = previousNonCR(text, j, &hasPrevJ);
            const bool afterLeftBoundary = !hasPrevJ || text.at(prevJ).isSpace();
            const QVector<QString> priorities = joinPriorityForContext(afterLeftBoundary, simulatedPrevJoin);
            const GlyphData *gj = resolveGlyph(fontCatalog, cj, priorities);
            if (!gj) gj = &missing;
            widthCm += advanceXForGlyph(*gj);
            simulatedPrevJoin = gj->joinMode;
        }
        return widthCm;
    };

    for (int i = 0; i < text.size(); ++i) {
        const QChar ch = text.at(i);
        if (ch == QChar('\n')) {
            ++lineIndex;
            cursorX = contentLeft;
            recomputeBaseline();
            ensurePage();
            hasChain = false;
            continue;
        }
        if (ch == QChar('\r')) continue;

        const GlyphData *gPtr = nullptr;
        if (ch.isSpace()) {
            gPtr = &space;
        } else {
            bool hasPrev = false;
            bool hasNext = false;
            const int prevIdx = previousNonCR(text, i, &hasPrev);
            const int nextIdx = nextNonCR(text, i, &hasNext);
            const bool afterLeftBoundary = !hasPrev || text.at(prevIdx).isSpace();
            Q_UNUSED(nextIdx);
            Q_UNUSED(hasNext);
            const QString prevJoinMode = chosenJoinModeByDocIndex.value(prevIdx, QStringLiteral("N"));
            const QVector<QString> priorities = joinPriorityForContext(
                afterLeftBoundary,
                prevJoinMode
            );
            gPtr = resolveGlyph(fontCatalog, ch, priorities);
        }
        if (!gPtr)
            gPtr = &missing;

        const GlyphData &g = *gPtr;
        const QPointF anchorFu = g.hasGlyphAnchor ? g.glyphAnchorFontUnits : QPointF(0, 0);
        const double fallbackAdvanceCm = qMax(0.02, g.bboxFontUnits.width() * fontUnitToCm);
        QPointF advanceVectorCm(fallbackAdvanceCm, 0);
        if (g.hasSelectionBox) {
            const QPointF brVectorFu = g.selectionBottomRightFontUnits - anchorFu;
            const QPointF brCm = toVectorCm(brVectorFu, fontUnitToCm);
            // Keep anchors aligned on one baseline: advance only in X.
            if (std::isfinite(brCm.x()))
                advanceVectorCm = QPointF(brCm.x(), 0);
        }
        const double gw = qMax(0.0, advanceVectorCm.x());
        const double gh = g.bboxFontUnits.height() * fontUnitToCm;

        QPointF placementAnchor;
        if (hasForced(forcedBottomLeftCmByDocIndex, i)) {
            placementAnchor = forcedBottomLeftCmByDocIndex.value(i);
            pageIndex = qMax(0, static_cast<int>(qFloor(placementAnchor.y() / pageHeightCm)));
            const double localY = placementAnchor.y() - pageTopY(pageIndex);
            lineIndex = qMax(0, static_cast<int>(qRound((localY - verticalGapCm - hyCm) / lineHeightCm - 1.0)));
            cursorX = placementAnchor.x() + gw;
            baselineY = placementAnchor.y();
            hasChain = false;
        } else {
            bool hasPrevVisible = false;
            const int prevVisibleIdx = previousNonCR(text, i, &hasPrevVisible);
            const bool startsWord = !ch.isSpace() && (!hasPrevVisible || text.at(prevVisibleIdx).isSpace() || text.at(prevVisibleIdx) == QChar('\n'));
            if (startsWord) {
                const QString prevJoinMode = chosenJoinModeByDocIndex.value(prevVisibleIdx, QStringLiteral("N"));
                const double wordWidthCm = estimateWordWidthCm(i, prevJoinMode);
                const double lineWidthCm = contentRight - contentLeft;
                if (cursorX > contentLeft + 1e-9
                    && wordWidthCm <= lineWidthCm + 1e-9
                    && cursorX + wordWidthCm > contentRight + 1e-9) {
                    ++lineIndex;
                    cursorX = contentLeft;
                    recomputeBaseline();
                    ensurePage();
                    hasChain = false;
                }
            }
            bool usedChain = false;
            if (hasChain && !ch.isSpace() && prevPage == pageIndex && prevLine == lineIndex) {
                const QPointF chainedAnchor(
                    prevAnchorCm.x() + prevAdvanceVectorCm.x(),
                    baselineY
                );
                const double expectedRight = chainedAnchor.x() + qMax(0.02, fallbackAdvanceCm);
                if (expectedRight <= contentRight + 1e-9) {
                    placementAnchor = chainedAnchor;
                    cursorX = placementAnchor.x() + qMax(0.0, advanceVectorCm.x());
                    usedChain = true;
                }
            }
            if (!usedChain && cursorX + qMax(0.02, fallbackAdvanceCm) > contentRight + 1e-9) {
                ++lineIndex;
                cursorX = contentLeft;
                recomputeBaseline();
                ensurePage();
                hasChain = false;
            }
            if (!usedChain) {
                placementAnchor = QPointF(cursorX, baselineY);
                cursorX += qMax(0.0, advanceVectorCm.x());
            }
        }

        const auto polys = transformPolylines(g, fontUnitToCm, placementAnchor);
        QRectF bbox = bboxOfPolys(polys);
        if (polys.isEmpty())
            bbox = QRectF(placementAnchor.x(), placementAnchor.y() - gh, fallbackAdvanceCm, gh);

        LayoutGlyph lg;
        lg.docIndex = i;
        lg.ch = ch;
        lg.joinMode = g.joinMode;
        lg.variantIndex = g.variantIndex;
        lg.polylinesCm = polys;
        lg.bboxCm = bbox;
        lg.placementAnchorCm = placementAnchor;
        lg.pageIndex = pageIndex;
        lg.lineIndex = lineIndex;
        res.glyphs.push_back(lg);
        if (!ch.isSpace()) {
            chosenJoinModeByDocIndex.insert(i, g.joinMode);
        }

        if (bbox.height() > lineHeightCm + 1e-6)
            res.anyGlyphExceedsLineHeight = true;

        if (ch.isSpace()) {
            hasChain = false;
        } else {
            hasChain = true;
            prevAnchorCm = placementAnchor;
            prevAdvanceVectorCm = advanceVectorCm;
            prevPage = pageIndex;
            prevLine = lineIndex;
        }
    }

    applyStrokeJoins(&res.glyphs, joinDistMm);

    if (text.isEmpty()) {
        res.totalHeightCm = pageHeightCm;
    } else {
        int lastPage = 0;
        for (const LayoutGlyph &lg : res.glyphs)
            lastPage = qMax(lastPage, lg.pageIndex);
        res.totalHeightCm = pageTopY(lastPage) + pageHeightCm;
    }
    return res;
}
