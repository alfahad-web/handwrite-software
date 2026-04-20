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
    bool hasGlyphAnchor = false;
    QPointF glyphAnchorFontUnits;
};

class FontLoader {
public:
    static QHash<QChar, GlyphData> loadDirectory(const QString &dirPath, QString *errorMessage = nullptr);

private:
    static bool parseFile(const QString &path, GlyphData *out, QString *errorMessage);
};
