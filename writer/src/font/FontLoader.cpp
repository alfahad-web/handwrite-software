#include "FontLoader.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>
#include <algorithm>

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

int legacyVariantIndex(const QString &variant) {
    if (variant == QStringLiteral("_fallback")) return 1000000000;
    bool ok = false;
    const int n = variant.toInt(&ok);
    if (ok) return qMax(0, n);
    return 1000000000;
}

QPointF parsePoint(const QStringList &nums, int xIdx, int yIdx, bool *ok) {
    bool okx = false;
    bool oky = false;
    const double x = nums[xIdx].toDouble(&okx);
    const double y = nums[yIdx].toDouble(&oky);
    *ok = okx && oky;
    return QPointF(x, y);
}

struct GlyphCandidate {
    QChar ch;
    QString path;
    QString joinMode;
    int variantIndex = 0;
    int sourcePriority = 0; // 0=join, 1=legacy, 2=simple
};
}

int FontCatalog::totalVariants() const {
    int total = 0;
    for (auto it = variantsByChar.constBegin(); it != variantsByChar.constEnd(); ++it)
        total += it.value().size();
    return total;
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
    out->hasSelectionBox = false;

    while (!ts.atEnd()) {
        const QString line = ts.readLine().trimmed();
        if (line.isEmpty()) continue;
        QStringList segments = line.split(QLatin1Char(';'), Qt::SkipEmptyParts);
        if (!consumedFirstLine) {
            consumedFirstLine = true;
            if (!segments.isEmpty()) {
                const QString firstSeg = segments.first().trimmed();
                const QStringList nums = firstSeg.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                if (nums.size() == 8) {
                    bool ok = true;
                    out->selectionBottomLeftFontUnits = parsePoint(nums, 0, 1, &ok);
                    if (ok) out->selectionTopLeftFontUnits = parsePoint(nums, 2, 3, &ok);
                    if (ok) out->selectionTopRightFontUnits = parsePoint(nums, 4, 5, &ok);
                    if (ok) out->selectionBottomRightFontUnits = parsePoint(nums, 6, 7, &ok);
                    if (ok) {
                        out->hasSelectionBox = true;
                        segments.removeFirst();
                    }
                } else if (nums.size() == 2) {
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
        if (out->hasGlyphAnchor || out->hasSelectionBox) {
            out->polylinesFontUnits.clear();
            if (out->hasSelectionBox) {
                QRectF rb(out->selectionBottomLeftFontUnits, QSizeF(0, 0));
                rb.setLeft(qMin(qMin(out->selectionBottomLeftFontUnits.x(), out->selectionTopLeftFontUnits.x()),
                                qMin(out->selectionTopRightFontUnits.x(), out->selectionBottomRightFontUnits.x())));
                rb.setRight(qMax(qMax(out->selectionBottomLeftFontUnits.x(), out->selectionTopLeftFontUnits.x()),
                                 qMax(out->selectionTopRightFontUnits.x(), out->selectionBottomRightFontUnits.x())));
                rb.setTop(qMin(qMin(out->selectionBottomLeftFontUnits.y(), out->selectionTopLeftFontUnits.y()),
                               qMin(out->selectionTopRightFontUnits.y(), out->selectionBottomRightFontUnits.y())));
                rb.setBottom(qMax(qMax(out->selectionBottomLeftFontUnits.y(), out->selectionTopLeftFontUnits.y()),
                                  qMax(out->selectionTopRightFontUnits.y(), out->selectionBottomRightFontUnits.y())));
                out->bboxFontUnits = rb;
            } else {
                out->bboxFontUnits = QRectF(out->glyphAnchorFontUnits, QSizeF(0, 0));
            }
            return true;
        }
        if (errorMessage) *errorMessage = QStringLiteral("No polylines in file.");
        return false;
    }
    out->polylinesFontUnits = std::move(all);
    out->bboxFontUnits = bbox;
    return true;
}

FontCatalog FontLoader::loadDirectory(const QString &dirPath, QString *errorMessage) {
    FontCatalog catalog;
    QDir dir(dirPath);
    if (!dir.exists()) {
        if (errorMessage) *errorMessage = QStringLiteral("Directory does not exist.");
        return catalog;
    }
    static const QRegularExpression kJoinStemRe(QStringLiteral(R"(^([^.]+)\.(L|R|LR|N)\.(\d+)\.txt$)"));
    static const QRegularExpression kLegacyRe(QStringLiteral(R"(^([^.]+)\.([^./]+)\.txt$)"));
    static const QRegularExpression kSimpleRe(QStringLiteral(R"(^([^./]+)\.txt$)"));

    const QFileInfoList files = dir.entryInfoList(QStringList() << QStringLiteral("*.txt"), QDir::Files);
    QVector<GlyphCandidate> candidates;

    for (const QFileInfo &fi : files) {
        const QString name = fi.fileName();
        const QRegularExpressionMatch mj = kJoinStemRe.match(name);
        if (!mj.hasMatch()) continue;
        const QString stem = mj.captured(1);
        const QString join = mj.captured(2);
        const int idx = mj.captured(3).toInt();
        const QChar ch = stemToChar(stem);
        if (ch.isNull()) continue;
        GlyphCandidate c;
        c.ch = ch;
        c.path = fi.absoluteFilePath();
        c.joinMode = join;
        c.variantIndex = qMax(0, idx);
        c.sourcePriority = 0;
        candidates.push_back(c);
    }

    for (const QFileInfo &fi : files) {
        const QString name = fi.fileName();
        if (kJoinStemRe.match(name).hasMatch()) continue;
        const QRegularExpressionMatch m = kLegacyRe.match(name);
        if (!m.hasMatch()) continue;
        const QString stem = m.captured(1);
        const QString variant = m.captured(2);
        const QChar ch = stemToChar(stem);
        if (ch.isNull()) continue;
        GlyphCandidate c;
        c.ch = ch;
        c.path = fi.absoluteFilePath();
        c.joinMode = QStringLiteral("N");
        c.variantIndex = legacyVariantIndex(variant);
        c.sourcePriority = 1;
        candidates.push_back(c);
    }

    for (const QFileInfo &fi : files) {
        const QString name = fi.fileName();
        if (kJoinStemRe.match(name).hasMatch()) continue;
        if (kLegacyRe.match(name).hasMatch()) continue;
        const QRegularExpressionMatch sm = kSimpleRe.match(name);
        if (!sm.hasMatch()) continue;
        const QString stem = sm.captured(1);
        const QChar ch = stemToChar(stem);
        if (ch.isNull()) continue;
        GlyphCandidate c;
        c.ch = ch;
        c.path = fi.absoluteFilePath();
        c.joinMode = QStringLiteral("N");
        c.variantIndex = 0;
        c.sourcePriority = 2;
        candidates.push_back(c);
    }

    std::sort(candidates.begin(), candidates.end(), [](const GlyphCandidate &a, const GlyphCandidate &b) {
        if (a.ch != b.ch) return a.ch.unicode() < b.ch.unicode();
        if (a.sourcePriority != b.sourcePriority) return a.sourcePriority < b.sourcePriority;
        if (a.joinMode != b.joinMode) return joinRank(a.joinMode) < joinRank(b.joinMode);
        if (a.variantIndex != b.variantIndex) return a.variantIndex < b.variantIndex;
        return a.path < b.path;
    });

    for (const GlyphCandidate &c : candidates) {
        GlyphData g;
        g.sourceFile = c.path;
        g.joinMode = c.joinMode;
        g.variantIndex = c.variantIndex;
        g.sourcePriority = c.sourcePriority;
        QString err;
        if (!parseFile(c.path, &g, &err)) {
            qWarning() << "[font] skip" << c.path << err;
            continue;
        }
        catalog.variantsByChar[c.ch].push_back(g);
    }
    return catalog;
}
