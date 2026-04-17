#pragma once

#include <QObject>
#include <QStringList>
#include <QVector>

#include "../core/EditorTypes.h"

struct SampledPointUm {
    qint64 xUm = 0;
    qint64 yUm = 0;
};

struct SampledStrokeUm {
    QString strokeId;
    QVector<SampledPointUm> points;
};

class ExportService : public QObject {
    Q_OBJECT
public:
    explicit ExportService(QObject *parent = nullptr);

    static QVector<SampledStrokeUm> buildExportStrokes(
        const QVector<Stroke> &strokes,
        const SelectionRect *selectionRect,
        int captureGapUm,
        double dpi
    );
    static QStringList serializeExportLines(const QVector<SampledStrokeUm> &strokes);
    static double resolveScreenDpi();
    static double pxToUm(double px, double dpi);
};
