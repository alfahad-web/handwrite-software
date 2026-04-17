#pragma once

#include <QDateTime>
#include <QPointF>
#include <QString>
#include <QVector>

enum class ToolMode {
    Draw,
    Select,
    Erase
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
    struct StrokePoint {
        QPointF pos;
        bool erased = false;
    };
    QVector<StrokePoint> points;
    qint64 createdAt = QDateTime::currentMSecsSinceEpoch();
};

struct ResizeDragState {
    ResizeHandle handle = ResizeHandle::None;
    QString selectionId;
    SelectionRect startRect;
    QPointF startPoint;
};

struct SelectionBox {
    QString id;
    int orderIndex = 0;
    SelectionRect rect;
    bool assigned = false;
    int assignedAscii = -1;
    QString fileStem;
};
