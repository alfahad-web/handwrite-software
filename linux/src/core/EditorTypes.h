#pragma once

#include <QDateTime>
#include <QPointF>
#include <QString>
#include <QVector>

enum class ToolMode {
    Draw,
    Select
};

enum class ResizeHandle {
    None,
    Move,
    N,
    S,
    E,
    W,
    NW,
    NE,
    SW,
    SE
};

struct SelectionRect {
    qreal x = 0;
    qreal y = 0;
    qreal width = 0;
    qreal height = 0;
};

struct Stroke {
    QString id;
    QVector<QPointF> points;
    qint64 createdAt = QDateTime::currentMSecsSinceEpoch();
};

struct ResizeDragState {
    ResizeHandle handle = ResizeHandle::None;
    SelectionRect startRect;
    QPointF startPoint;
};
