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
    QString joinMode;
    int variantIndex = 0;
    QVector<QVector<QPointF>> polylinesCm;
    QRectF bboxCm;
    QPointF placementAnchorCm;
    int pageIndex = 0;
    int lineIndex = 0;
};

struct LayoutResult {
    QVector<LayoutGlyph> glyphs;
    double totalHeightCm = 0;
    bool anyGlyphExceedsLineHeight = false;
};

class LayoutEngine {
public:
    static LayoutResult layout(
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
    );
};
