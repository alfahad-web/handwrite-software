#include "LayoutEngine.h"

#include <QtMath>

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
    return g;
}

GlyphData spaceGlyph(double lineHeightCm, double fontUnitToCm) {
    const double wCm = qMax(0.02, lineHeightCm * 0.35);
    const double wFu = wCm / fontUnitToCm;
    const double hFu = qMax(1.0, lineHeightCm * 0.2 / fontUnitToCm);
    GlyphData g;
    g.bboxFontUnits = QRectF(0, 0, wFu, hFu);
    g.sourceFile = QStringLiteral("<space>");
    return g;
}

QVector<QVector<QPointF>> transformPolylines(const GlyphData &g, double fontUnitToCm, const QPointF &bottomLeftCm) {
    const QRectF bb = g.bboxFontUnits;
    const double blx = bb.left() * fontUnitToCm;
    const double bly = bb.bottom() * fontUnitToCm;
    QVector<QVector<QPointF>> out;
    out.reserve(g.polylinesFontUnits.size());
    for (const QVector<QPointF> &poly : g.polylinesFontUnits) {
        QVector<QPointF> t;
        t.reserve(poly.size());
        for (const QPointF &p : poly) {
            const double x = bottomLeftCm.x() + (p.x() * fontUnitToCm - blx);
            const double y = bottomLeftCm.y() + (p.y() * fontUnitToCm - bly);
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
}

LayoutResult LayoutEngine::layout(
    const QString &text,
    const QHash<QChar, GlyphData> &font,
    double fontUnitToCm,
    double pageWidthCm,
    double pageHeightCm,
    double leftMarginCm,
    double rightMarginCm,
    double verticalGapCm,
    double hxCm,
    double hyCm,
    double lineHeightCm,
    const QHash<int, QPointF> &forcedBottomLeftCmByDocIndex
) {
    LayoutResult res;
    const double contentLeft = leftMarginCm + hxCm;
    const double contentRight = pageWidthCm - rightMarginCm;
    const GlyphData missing = missingGlyph(lineHeightCm, fontUnitToCm);
    const GlyphData space = spaceGlyph(lineHeightCm, fontUnitToCm);

    auto pageTopY = [&](int p) { return p * (pageHeightCm + verticalGapCm); };

    int pageIndex = 0;
    int lineIndex = 0;
    double cursorX = contentLeft;
    double baselineY = 0;

    auto recomputeBaseline = [&]() {
        baselineY = pageTopY(pageIndex) + hyCm + (lineIndex + 1) * lineHeightCm;
    };

    auto ensurePage = [&]() {
        while (hyCm + (lineIndex + 1) * lineHeightCm > pageHeightCm + 1e-9) {
            ++pageIndex;
            lineIndex = 0;
            cursorX = contentLeft;
            recomputeBaseline();
        }
    };

    recomputeBaseline();
    ensurePage();

    for (int i = 0; i < text.size(); ++i) {
        const QChar ch = text.at(i);
        if (ch == QChar('\n')) {
            ++lineIndex;
            cursorX = contentLeft;
            recomputeBaseline();
            ensurePage();
            continue;
        }
        if (ch == QChar('\r')) continue;

        const GlyphData *gPtr = nullptr;
        auto it = font.constFind(ch);
        if (it != font.constEnd())
            gPtr = &it.value();
        else if (ch.isSpace())
            gPtr = &space;
        else
            gPtr = &missing;

        const GlyphData &g = *gPtr;
        const double gw = g.bboxFontUnits.width() * fontUnitToCm;
        const double gh = g.bboxFontUnits.height() * fontUnitToCm;

        QPointF bottomLeft;
        if (hasForced(forcedBottomLeftCmByDocIndex, i)) {
            bottomLeft = forcedBottomLeftCmByDocIndex.value(i);
            pageIndex = qMax(0, static_cast<int>(qFloor(bottomLeft.y() / (pageHeightCm + verticalGapCm))));
            const double localY = bottomLeft.y() - pageTopY(pageIndex);
            lineIndex = qMax(0, static_cast<int>(qRound((localY - hyCm) / lineHeightCm - 1.0)));
            cursorX = bottomLeft.x() + gw;
            baselineY = bottomLeft.y();
        } else {
            if (cursorX + gw > contentRight + 1e-9) {
                ++lineIndex;
                cursorX = contentLeft;
                recomputeBaseline();
                ensurePage();
            }
            bottomLeft = QPointF(cursorX, baselineY);
            cursorX += gw;
        }

        const auto polys = transformPolylines(g, fontUnitToCm, bottomLeft);
        QRectF bbox = bboxOfPolys(polys);
        if (polys.isEmpty())
            bbox = QRectF(bottomLeft.x(), bottomLeft.y() - gh, gw, gh);

        LayoutGlyph lg;
        lg.docIndex = i;
        lg.ch = ch;
        lg.polylinesCm = polys;
        lg.bboxCm = bbox;
        lg.pageIndex = pageIndex;
        lg.lineIndex = lineIndex;
        res.glyphs.push_back(lg);
    }

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
