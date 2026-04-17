#include "ExportService.h"

#include <QGuiApplication>
#include <QScreen>

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
    int captureGapUm,
    double dpi
) {
    QVector<SampledStrokeUm> output;
    if (!selectionRect) return output;

    for (const Stroke &stroke : strokes) {
        QVector<QPointF> inside;
        inside.reserve(stroke.points.size());
        for (const QPointF &p : stroke.points) {
            if (isPointInsideRect(p, *selectionRect)) inside.push_back(p);
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
            out.points.push_back(SampledPointUm{
                static_cast<qint64>(std::llround(pxToUm(p.x(), dpi))),
                static_cast<qint64>(std::llround(pxToUm(p.y(), dpi)))
            });
        }
        output.push_back(out);
    }
    return output;
}

QStringList ExportService::serializeExportLines(const QVector<SampledStrokeUm> &strokes) {
    QStringList lines;
    for (const SampledStrokeUm &stroke : strokes) {
        QStringList parts;
        for (const SampledPointUm &p : stroke.points) {
            parts << QString::number(p.xUm) << QString::number(p.yUm);
        }
        lines << parts.join(" ");
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
