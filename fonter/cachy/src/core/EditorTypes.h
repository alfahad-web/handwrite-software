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

enum class JoinMode {
    L,
    R,
    LR,
    N
};

inline QString joinModeToString(JoinMode m) {
    switch (m) {
    case JoinMode::L: return QStringLiteral("L");
    case JoinMode::R: return QStringLiteral("R");
    case JoinMode::LR: return QStringLiteral("LR");
    case JoinMode::N: return QStringLiteral("N");
    }
    return QStringLiteral("N");
}

inline JoinMode joinModeFromString(const QString &s) {
    if (s == QStringLiteral("LR")) return JoinMode::LR;
    if (s == QStringLiteral("L")) return JoinMode::L;
    if (s == QStringLiteral("R")) return JoinMode::R;
    return JoinMode::N;
}

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
    JoinMode joinMode = JoinMode::N;
    double anchorX = 0;
    double anchorY = 0;
};
