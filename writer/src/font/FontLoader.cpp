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

int joinRank(const QString &join) {
    if (join == QStringLiteral("L")) return 0;
    if (join == QStringLiteral("R")) return 1;
    if (join == QStringLiteral("LR")) return 2;
    return 3;
}

QString sortKeyJoin(const QString &join, int index) {
    return QLatin1String("j") + QString::number(index).rightJustified(8, QLatin1Char('0'))
        + QString::number(joinRank(join));
}

QString sortKeyLegacyVariant(const QString &variant) {
    if (variant == QStringLiteral("_fallback")) return QLatin1String("y_fallback");
    bool ok = false;
    const int n = variant.toInt(&ok);
    if (ok) return QLatin1String("l") + QString::number(n).rightJustified(8, QLatin1Char('0'));
    return QLatin1String("z") + variant;
}

QString sortKeySimpleStem() { return QLatin1String("s00000000"); }
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
    bool consumedFirstLine = false;
    out->hasGlyphAnchor = false;

    while (!ts.atEnd()) {
        const QString line = ts.readLine().trimmed();
        if (line.isEmpty()) continue;
        QStringList segments = line.split(QLatin1Char(';'), Qt::SkipEmptyParts);
        if (!consumedFirstLine) {
            consumedFirstLine = true;
            if (!segments.isEmpty()) {
                const QString firstSeg = segments.first().trimmed();
                const QStringList nums = firstSeg.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                if (nums.size() == 2) {
                    bool okx = false, oky = false;
                    const double ax = nums[0].toDouble(&okx);
                    const double ay = nums[1].toDouble(&oky);
                    if (okx && oky) {
                        out->hasGlyphAnchor = true;
                        out->glyphAnchorFontUnits = QPointF(ax, ay);
                        segments.removeFirst();
                    }
                }
            }
        }
        for (const QString &segRaw : segments) {
            const QString seg = segRaw.trimmed();
            if (seg.isEmpty()) continue;
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
        if (out->hasGlyphAnchor) {
            out->polylinesFontUnits.clear();
            out->bboxFontUnits = QRectF(out->glyphAnchorFontUnits, QSizeF(0, 0));
            return true;
        }
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
    static const QRegularExpression kJoinStemRe(QStringLiteral(R"(^([^.]+)\.(L|R|LR|N)\.(\d+)\.txt$)"));
    static const QRegularExpression kLegacyRe(QStringLiteral(R"(^([^.]+)\.([^./]+)\.txt$)"));
    static const QRegularExpression kSimpleRe(QStringLiteral(R"(^([^./]+)\.txt$)"));

    const QFileInfoList files = dir.entryInfoList(QStringList() << QStringLiteral("*.txt"), QDir::Files);
    QHash<QChar, QPair<QString, QString>> bestPathSortKey;

    auto consider = [&](QChar ch, const QString &absPath, const QString &sortKey) {
        if (ch.isNull()) return;
        if (!bestPathSortKey.contains(ch) || sortKey < bestPathSortKey[ch].second)
            bestPathSortKey.insert(ch, qMakePair(absPath, sortKey));
    };

    for (const QFileInfo &fi : files) {
        const QString name = fi.fileName();
        const QRegularExpressionMatch mj = kJoinStemRe.match(name);
        if (!mj.hasMatch()) continue;
        const QString stem = mj.captured(1);
        const QString join = mj.captured(2);
        const int idx = mj.captured(3).toInt();
        const QChar ch = stemToChar(stem);
        consider(ch, fi.absoluteFilePath(), sortKeyJoin(join, idx));
    }

    for (const QFileInfo &fi : files) {
        const QString name = fi.fileName();
        if (kJoinStemRe.match(name).hasMatch()) continue;
        const QRegularExpressionMatch m = kLegacyRe.match(name);
        if (!m.hasMatch()) continue;
        const QString stem = m.captured(1);
        const QString variant = m.captured(2);
        const QChar ch = stemToChar(stem);
        consider(ch, fi.absoluteFilePath(), sortKeyLegacyVariant(variant));
    }

    for (const QFileInfo &fi : files) {
        const QString name = fi.fileName();
        if (kJoinStemRe.match(name).hasMatch()) continue;
        if (kLegacyRe.match(name).hasMatch()) continue;
        const QRegularExpressionMatch sm = kSimpleRe.match(name);
        if (!sm.hasMatch()) continue;
        const QString stem = sm.captured(1);
        const QChar ch = stemToChar(stem);
        consider(ch, fi.absoluteFilePath(), sortKeySimpleStem());
    }

    for (auto it = bestPathSortKey.constBegin(); it != bestPathSortKey.constEnd(); ++it) {
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
