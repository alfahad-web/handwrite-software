#include "ExportService.h"

#include <QGuiApplication>
#include <QPointF>
#include <QScreen>
#include <cmath>

namespace {
constexpr double kMicrometersPerInch = 25400.0;
constexpr double kCssPixelsPerInch = 96.0;

bool isPointInsideRect(const QPointF &p, const SelectionRect &r) {
    return p.x() >= r.x && p.x() <= (r.x + r.width) && p.y() >= r.y && p.y() <= (r.y + r.height);
}

double distance(const QPointF &a, const QPointF &b) {
    const double dx = a.x() - b.x();
    const double dy = a.y() - b.y();
    return std::hypot(dx, dy);
}
}

ExportService::ExportService(QObject *parent) : QObject(parent) {}

QVector<SampledStrokeUm> ExportService::buildExportStrokes(
    const QVector<Stroke> &strokes,
    const SelectionRect *selectionRect,
    const QPointF &anchorBoard,
    int captureGapUm,
    double dpi
) {
    QVector<SampledStrokeUm> output;
    if (!selectionRect) return output;

    for (const Stroke &stroke : strokes) {
        QVector<QPointF> inside;
        inside.reserve(stroke.points.size());
        for (const Stroke::StrokePoint &pt : stroke.points) {
            if (pt.erased) continue;
            if (isPointInsideRect(pt.pos, *selectionRect)) inside.push_back(pt.pos);
        }
        if (inside.isEmpty()) continue;

        QVector<QPointF> sampled;
        sampled.push_back(inside[0]);
        QPointF last = inside[0];
        for (int i = 1; i < inside.size(); ++i) {
            if (pxToUm(distance(last, inside[i]), dpi) >= captureGapUm) {
                sampled.push_back(inside[i]);
                last = inside[i];
            }
        }
        if (sampled.isEmpty()) continue;

        SampledStrokeUm out;
        out.strokeId = stroke.id;
        out.points.reserve(sampled.size());
        for (const QPointF &p : sampled) {
            const double localXPx = p.x() - anchorBoard.x();
            const double localYPx = anchorBoard.y() - p.y();
            out.points.push_back(SampledPointUm{
                static_cast<qint64>(std::llround(pxToUm(localXPx, dpi))),
                static_cast<qint64>(std::llround(pxToUm(localYPx, dpi)))
            });
        }
        output.push_back(out);
    }
    return output;
}

QVector<SelectionExportFile> ExportService::buildSelectionExports(
    const QVector<Stroke> &strokes,
    const QVector<SelectionBox> &selectionBoxes,
    int captureGapUm,
    double dpi
) {
    QVector<SelectionExportFile> files;
    for (const SelectionBox &box : selectionBoxes) {
        if (!box.assigned || box.fileStem.isEmpty()) continue;
        const auto sampled = buildExportStrokes(strokes, &box.rect, QPointF(box.anchorX, box.anchorY), captureGapUm, dpi);
        SelectionExportFile f;
        f.selectionId = box.id;
        f.fileName = QString("%1.txt").arg(box.fileStem);
        f.lines = serializeExportLines(sampled);
        files.push_back(f);
    }
    return files;
}

QStringList ExportService::serializeExportLines(const QVector<SampledStrokeUm> &strokes) {
    QStringList lines;
    for (const SampledStrokeUm &stroke : strokes) {
        QStringList parts;
        for (const SampledPointUm &p : stroke.points) {
            parts << QString::number(p.xUm) << QString::number(p.yUm);
        }
        if (parts.isEmpty()) continue;
        // One curve per line; trailing ';' matches WriterQt font parser (curves split by ';').
        lines << parts.join(QLatin1Char(' ')) + QLatin1Char(';');
    }
    return lines;
}

double ExportService::resolveScreenDpi() {
    const QScreen *screen = QGuiApplication::primaryScreen();
    const double ratio = screen ? screen->devicePixelRatio() : 1.0;
    return kCssPixelsPerInch * ratio;
}

double ExportService::pxToUm(double px, double dpi) {
    if (!std::isfinite(px) || !std::isfinite(dpi) || dpi <= 0.0) return 0.0;
    return (px / dpi) * kMicrometersPerInch;
}
