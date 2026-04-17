#pragma once

#include <QChar>
#include <QHash>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QVector>

#include "font/FontLoader.h"

struct LayoutGlyph {
    int docIndex = -1;
    QChar ch;
    QVector<QVector<QPointF>> polylinesCm;
    QRectF bboxCm;
    int pageIndex = 0;
    int lineIndex = 0;
};

struct LayoutResult {
    QVector<LayoutGlyph> glyphs;
    double totalHeightCm = 0;
};

class LayoutEngine {
public:
    static LayoutResult layout(
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
    );
};
