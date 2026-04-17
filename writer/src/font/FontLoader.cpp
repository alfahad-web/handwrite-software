#include "FontLoader.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

namespace {
QChar stemToChar(const QString &stem) {
    if (stem.isEmpty()) return QChar();
    if (stem == QStringLiteral("space")) return QLatin1Char(' ');
    if (stem == QStringLiteral("tab")) return QLatin1Char('\t');
    return stem.at(0);
}

bool variantLessThan(const QString &a, const QString &b) {
    if (a == QStringLiteral("_fallback")) return false;
    if (b == QStringLiteral("_fallback")) return true;
    bool okA = false, okB = false;
    const int na = a.toInt(&okA);
    const int nb = b.toInt(&okB);
    if (okA && okB) return na < nb;
    return a < b;
}
}

bool FontLoader::parseFile(const QString &path, GlyphData *out, QString *errorMessage) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = f.errorString();
        return false;
    }
    QTextStream ts(&f);
    QVector<QVector<QPointF>> all;
    QRectF bbox;
    bool any = false;
    while (!ts.atEnd()) {
        const QString line = ts.readLine().trimmed();
        if (line.isEmpty()) continue;
        const QStringList segments = line.split(QLatin1Char(';'), Qt::SkipEmptyParts);
        for (const QString &seg : segments) {
            const QStringList nums = seg.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (nums.size() < 4 || (nums.size() % 2) != 0) continue;
            QVector<QPointF> poly;
            for (int i = 0; i + 1 < nums.size(); i += 2) {
                bool okx = false, oky = false;
                const double x = nums[i].toDouble(&okx);
                const double y = nums[i + 1].toDouble(&oky);
                if (!okx || !oky) continue;
                poly.push_back(QPointF(x, y));
            }
            if (poly.size() < 2) continue;
            all.push_back(poly);
            for (const QPointF &p : poly) {
                if (!any) {
                    bbox = QRectF(p, QSizeF(0, 0));
                    any = true;
                } else {
                    bbox.setLeft(qMin(bbox.left(), p.x()));
                    bbox.setRight(qMax(bbox.right(), p.x()));
                    bbox.setTop(qMin(bbox.top(), p.y()));
                    bbox.setBottom(qMax(bbox.bottom(), p.y()));
                }
            }
        }
    }
    if (all.isEmpty()) {
        if (errorMessage) *errorMessage = QStringLiteral("No polylines in file.");
        return false;
    }
    out->polylinesFontUnits = std::move(all);
    out->bboxFontUnits = bbox;
    return true;
}

QHash<QChar, GlyphData> FontLoader::loadDirectory(const QString &dirPath, QString *errorMessage) {
    QHash<QChar, GlyphData> map;
    QDir dir(dirPath);
    if (!dir.exists()) {
        if (errorMessage) *errorMessage = QStringLiteral("Directory does not exist.");
        return map;
    }
    static const QRegularExpression kNameRe(QStringLiteral("^(.+)\\.([^./]+)\\.txt$"));
    const QFileInfoList files = dir.entryInfoList(QStringList() << QStringLiteral("*.txt"), QDir::Files);
    QHash<QChar, QPair<QString, QString>> bestStemVariant;
    for (const QFileInfo &fi : files) {
        const QRegularExpressionMatch m = kNameRe.match(fi.fileName());
        if (!m.hasMatch()) continue;
        const QString stem = m.captured(1);
        const QString variant = m.captured(2);
        const QChar ch = stemToChar(stem);
        if (ch.isNull()) continue;
        if (!bestStemVariant.contains(ch) || variantLessThan(variant, bestStemVariant[ch].second)) {
            bestStemVariant.insert(ch, qMakePair(fi.absoluteFilePath(), variant));
        }
    }
    static const QRegularExpression kSimpleRe(QStringLiteral("^([^./]+)\\.txt$"));
    for (const QFileInfo &fi : files) {
        const QString name = fi.fileName();
        if (kNameRe.match(name).hasMatch()) continue;
        const QRegularExpressionMatch sm = kSimpleRe.match(name);
        if (!sm.hasMatch()) continue;
        const QString stem = sm.captured(1);
        const QChar ch = stemToChar(stem);
        if (ch.isNull()) continue;
        const QString variant = QStringLiteral("_fallback");
        if (!bestStemVariant.contains(ch) || variantLessThan(variant, bestStemVariant[ch].second)) {
            bestStemVariant.insert(ch, qMakePair(fi.absoluteFilePath(), variant));
        }
    }
    for (auto it = bestStemVariant.constBegin(); it != bestStemVariant.constEnd(); ++it) {
        GlyphData g;
        g.sourceFile = it.value().first;
        QString err;
        if (!parseFile(it.value().first, &g, &err)) {
            qWarning() << "[font] skip" << it.value().first << err;
            continue;
        }
        map.insert(it.key(), g);
    }
    return map;
}
