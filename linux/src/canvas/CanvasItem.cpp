#include "CanvasItem.h"

#include <QCursor>
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

CanvasItem::CanvasItem(QQuickItem *parent) : QQuickPaintedItem(parent) {
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
    setFlag(QQuickItem::ItemHasContents, true);
}

QObject *CanvasItem::editorStore() const { return m_store; }

void CanvasItem::setEditorStore(QObject *storeObj) {
    EditorStore *store = qobject_cast<EditorStore *>(storeObj);
    qInfo() << "[canvas] setEditorStore called with" << storeObj << "cast=" << store;
    if (m_store == store) return;
    if (m_store) {
        disconnect(m_store, nullptr, this, nullptr);
    }
    m_store = store;
    if (m_store) {
        connect(m_store, &EditorStore::strokesChanged, this, [this]() { update(); });
        connect(m_store, &EditorStore::selectionChanged, this, [this]() { update(); });
        connect(m_store, &EditorStore::zoomChanged, this, [this]() { update(); });
        connect(m_store, &EditorStore::strokePxChanged, this, [this]() { update(); });
        connect(m_store, &EditorStore::toolModeChanged, this, [this]() {
            if (!m_store) return;
            if (m_store->toolModeValue() == ToolMode::Draw) {
                setCursor(Qt::CrossCursor);
            } else {
                unsetCursor();
            }
            update();
        });
        if (m_store->toolModeValue() == ToolMode::Draw) {
            setCursor(Qt::CrossCursor);
        } else {
            unsetCursor();
        }
    }
    emit editorStoreChanged();
    update();
}

void CanvasItem::paint(QPainter *painter) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->fillRect(boundingRect(), QColor("#f4f4f5"));

    if (!m_store) return;

    const qreal scale = m_store->zoom() / 100.0;
    const QRectF boardRect(0, 0, kBoardWidth * scale, kBoardHeight * scale);
    painter->fillRect(boardRect, Qt::white);
    painter->setPen(QPen(QColor("#e4e4e7"), 1));
    painter->drawRect(boardRect);

    painter->save();
    painter->scale(scale, scale);

    QPen strokePen(Qt::black, m_store->strokePx(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(strokePen);
    painter->setBrush(Qt::NoBrush);

    for (const Stroke &stroke : m_store->strokes()) {
        if (stroke.points.size() == 1) {
            painter->drawEllipse(stroke.points[0], m_store->strokePx() * 0.5, m_store->strokePx() * 0.5);
            continue;
        }
        if (stroke.points.size() < 2) continue;
        QPainterPath path(stroke.points[0]);
        for (int i = 1; i < stroke.points.size(); ++i) path.lineTo(stroke.points[i]);
        painter->drawPath(path);
    }

    if (m_livePoints.size() > 1) {
        QPainterPath live(m_livePoints[0]);
        for (int i = 1; i < m_livePoints.size(); ++i) live.lineTo(m_livePoints[i]);
        painter->drawPath(live);
    }

    const SelectionRect viewRect = currentSelectionForView();
    if (viewRect.width > 0 && viewRect.height > 0) {
        painter->setPen(QPen(QColor("#2563eb"), 1));
        painter->setBrush(QColor(59, 130, 246, 72));
        painter->drawRect(QRectF(viewRect.x, viewRect.y, viewRect.width, viewRect.height));
    }
    painter->restore();
}

void CanvasItem::mousePressEvent(QMouseEvent *event) {
    pointerDown(event->position().x(), event->position().y(), static_cast<int>(event->button()));
}

void CanvasItem::pointerDown(qreal x, qreal y, int button) {
    qInfo() << "[canvas] pointerDown x=" << x << "y=" << y << "button=" << button << "store=" << m_store;
    if (!m_store || button != Qt::LeftButton) return;
    const QPointF p = toBoard(QPointF(x, y), false);
    if (p.x() < 0 || p.y() < 0) return;

    if (m_store->toolModeValue() == ToolMode::Draw) {
        m_isDrawing = true;
        m_livePoints = {p};
        m_store->startStroke(p);
        m_activeStrokeIdAtDown = m_store->currentStrokeId();
        update();
    } else {
        const SelectionRect *sel = m_store->selectionRect();
        if (sel) {
            const ResizeHandle handle = hitHandle(p, *sel);
            if (handle != ResizeHandle::None) {
                ResizeDragState state;
                state.handle = handle;
                state.startRect = *sel;
                state.startPoint = p;
                m_store->setSelectionResizeState(&state);
                m_isResizing = true;
                return;
            }
        }
        m_isSelecting = true;
        m_selectStart = p;
        SelectionRect draft{p.x(), p.y(), 0, 0};
        m_store->setSelectionDraftRect(&draft);
        update();
    }
}

void CanvasItem::mouseMoveEvent(QMouseEvent *event) {
    pointerMove(event->position().x(), event->position().y());
}

void CanvasItem::pointerMove(qreal x, qreal y) {
    if (!m_store) return;
    static int moveLogCounter = 0;
    moveLogCounter++;
    if ((moveLogCounter % 25) == 0) {
        qInfo() << "[canvas] pointerMove#" << moveLogCounter << "x=" << x << "y=" << y
                << "drawing=" << m_isDrawing << "selecting=" << m_isSelecting << "resizing=" << m_isResizing;
    }
    if (m_store->toolModeValue() == ToolMode::Draw) setCursor(Qt::CrossCursor);
    const QPointF p = toBoard(QPointF(x, y), m_isDrawing || m_isSelecting || m_isResizing);
    if (p.x() < 0 || p.y() < 0) return;

    if (m_isDrawing) {
        if (m_livePoints.isEmpty() || QLineF(m_livePoints.last(), p).length() >= kMinStrokeSamplePx) {
            m_livePoints.push_back(p);
            m_store->replaceActiveStrokePoints(m_livePoints);
            update();
        }
        return;
    }
    if (m_isSelecting) {
        SelectionRect draft{m_selectStart.x(), m_selectStart.y(), p.x() - m_selectStart.x(), p.y() - m_selectStart.y()};
        m_store->setSelectionDraftRect(&draft);
        update();
        return;
    }
    if (m_isResizing) {
        applyResize(p);
        return;
    }
    if (m_store->toolModeValue() == ToolMode::Select) {
        updateCursorForSelection(p);
    }
}

void CanvasItem::mouseReleaseEvent(QMouseEvent *event) {
    pointerUp(event->position().x(), event->position().y(), static_cast<int>(event->button()));
}

void CanvasItem::pointerUp(qreal x, qreal y, int button) {
    qInfo() << "[canvas] pointerUp x=" << x << "y=" << y << "button=" << button;
    if (!m_store || button != Qt::LeftButton) return;
    finalizeInteraction(toBoard(QPointF(x, y), true));
}

void CanvasItem::hoverMoveEvent(QHoverEvent *event) {
    if (!m_store || m_store->toolModeValue() != ToolMode::Select) return;
    if (m_isDrawing || m_isSelecting || m_isResizing) return;
    const QPointF p = toBoard(event->position(), false);
    if (p.x() < 0 || p.y() < 0) return;
    updateCursorForSelection(p);
}

void CanvasItem::hoverLeaveEvent(QHoverEvent *event) {
    Q_UNUSED(event);
    unsetCursor();
}

QPointF CanvasItem::toBoard(const QPointF &itemPos, bool clamp) const {
    if (!m_store) return QPointF(-1, -1);
    const qreal scale = m_store->zoom() / 100.0;
    const qreal x = itemPos.x() / scale;
    const qreal y = itemPos.y() / scale;
    const bool inside = x >= 0 && x <= kBoardWidth && y >= 0 && y <= kBoardHeight;
    if (!inside && !clamp) return QPointF(-1, -1);
    return QPointF(qBound(0.0, x, kBoardWidth), qBound(0.0, y, kBoardHeight));
}

ResizeHandle CanvasItem::hitHandle(const QPointF &point, const SelectionRect &rect) const {
    const qreal hs = 10.0 / (m_store ? (m_store->zoom() / 100.0) : 1.0);
    const qreal left = rect.x;
    const qreal right = rect.x + rect.width;
    const qreal top = rect.y;
    const qreal bottom = rect.y + rect.height;
    const auto near = [hs](qreal v, qreal t) { return qAbs(v - t) <= hs; };
    const bool withinX = point.x() >= left - hs && point.x() <= right + hs;
    const bool withinY = point.y() >= top - hs && point.y() <= bottom + hs;
    if (near(point.x(), left) && near(point.y(), top)) return ResizeHandle::NW;
    if (near(point.x(), right) && near(point.y(), top)) return ResizeHandle::NE;
    if (near(point.x(), left) && near(point.y(), bottom)) return ResizeHandle::SW;
    if (near(point.x(), right) && near(point.y(), bottom)) return ResizeHandle::SE;
    if (near(point.x(), left) && withinY) return ResizeHandle::W;
    if (near(point.x(), right) && withinY) return ResizeHandle::E;
    if (near(point.y(), top) && withinX) return ResizeHandle::N;
    if (near(point.y(), bottom) && withinX) return ResizeHandle::S;
    if (point.x() >= left + hs && point.x() <= right - hs && point.y() >= top + hs && point.y() <= bottom - hs) {
        return ResizeHandle::Move;
    }
    return ResizeHandle::None;
}

SelectionRect CanvasItem::currentSelectionForView() const {
    if (!m_store) return {};
    const SelectionRect *draft = m_store->selectionDraftRect();
    if (draft) return *draft;
    const SelectionRect *rect = m_store->selectionRect();
    if (rect) return *rect;
    return {};
}

void CanvasItem::applyResize(const QPointF &current) {
    const ResizeDragState *state = m_store->selectionResizeState();
    if (!state) return;
    const SelectionRect base = state->startRect;
    SelectionRect next = base;
    const qreal dx = current.x() - state->startPoint.x();
    const qreal dy = current.y() - state->startPoint.y();
    const ResizeHandle h = state->handle;
    auto hasW = h == ResizeHandle::W || h == ResizeHandle::NW || h == ResizeHandle::SW;
    auto hasE = h == ResizeHandle::E || h == ResizeHandle::NE || h == ResizeHandle::SE;
    auto hasN = h == ResizeHandle::N || h == ResizeHandle::NE || h == ResizeHandle::NW;
    auto hasS = h == ResizeHandle::S || h == ResizeHandle::SE || h == ResizeHandle::SW;
    if (h == ResizeHandle::Move) {
        next.x = base.x + dx;
        next.y = base.y + dy;
    } else {
        if (hasW) {
            next.x = base.x + dx;
            next.width = base.width - dx;
        }
        if (hasE) next.width = base.width + dx;
        if (hasN) {
            next.y = base.y + dy;
            next.height = base.height - dy;
        }
        if (hasS) next.height = base.height + dy;
    }
    m_store->setSelectionRect(&next);
    update();
}

void CanvasItem::updateCursorForSelection(const QPointF &point) {
    const SelectionRect *sel = m_store->selectionRect();
    if (!sel) {
        unsetCursor();
        return;
    }
    switch (hitHandle(point, *sel)) {
    case ResizeHandle::Move: setCursor(Qt::SizeAllCursor); break;
    case ResizeHandle::N:
    case ResizeHandle::S: setCursor(Qt::SizeVerCursor); break;
    case ResizeHandle::E:
    case ResizeHandle::W: setCursor(Qt::SizeHorCursor); break;
    case ResizeHandle::NE:
    case ResizeHandle::SW: setCursor(Qt::SizeBDiagCursor); break;
    case ResizeHandle::NW:
    case ResizeHandle::SE: setCursor(Qt::SizeFDiagCursor); break;
    default: unsetCursor(); break;
    }
}

void CanvasItem::finalizeInteraction(const QPointF &point) {
    if (!m_store) return;
    if (m_isDrawing) {
        if (point.x() >= 0 && point.y() >= 0) {
            if (m_livePoints.isEmpty() || QLineF(m_livePoints.last(), point).length() >= 0.02) {
                m_livePoints.push_back(point);
            }
        }
        m_store->replaceActiveStrokePoints(m_livePoints);
        m_store->endStroke();
        m_livePoints.clear();
        m_activeStrokeIdAtDown.clear();
        m_isDrawing = false;
    }
    if (m_isSelecting) {
        m_store->commitSelectionDraftRect();
        m_isSelecting = false;
    }
    if (m_isResizing) {
        m_store->setSelectionResizeState(nullptr);
        m_isResizing = false;
    }
    update();
}
