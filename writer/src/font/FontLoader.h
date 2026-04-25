#pragma once

#include <QChar>
#include <QHash>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QVector>

struct GlyphData {
    QVector<QVector<QPointF>> polylinesFontUnits;
    QRectF bboxFontUnits;
    QString sourceFile;
    QString joinMode = QStringLiteral("N");
    int variantIndex = 0;
    int sourcePriority = 0;
    bool hasGlyphAnchor = false;
    QPointF glyphAnchorFontUnits;
    bool hasSelectionBox = false;
    QPointF selectionBottomLeftFontUnits;
    QPointF selectionTopLeftFontUnits;
    QPointF selectionTopRightFontUnits;
    QPointF selectionBottomRightFontUnits;
};

struct FontCatalog {
    QHash<QChar, QVector<GlyphData>> variantsByChar;

    bool isEmpty() const { return variantsByChar.isEmpty(); }
    int size() const { return variantsByChar.size(); }
    int totalVariants() const;
};

class FontLoader {
public:
    static FontCatalog loadDirectory(const QString &dirPath, QString *errorMessage = nullptr);

private:
    static bool parseFile(const QString &path, GlyphData *out, QString *errorMessage);
};
