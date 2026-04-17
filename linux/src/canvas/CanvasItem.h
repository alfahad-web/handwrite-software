#pragma once

#include <QPointer>
#include <QKeyEvent>
#include <QQuickPaintedItem>

#include "../core/EditorStore.h"

class CanvasItem : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(QObject *editorStore READ editorStore WRITE setEditorStore NOTIFY editorStoreChanged)
public:
    explicit CanvasItem(QQuickItem *parent = nullptr);

    QObject *editorStore() const;
    void setEditorStore(QObject *store);

    void paint(QPainter *painter) override;
    Q_INVOKABLE void pointerDown(qreal x, qreal y, int button);
    Q_INVOKABLE void pointerMove(qreal x, qreal y);
    Q_INVOKABLE void pointerUp(qreal x, qreal y, int button);
    Q_INVOKABLE bool pointerDoubleClick(qreal x, qreal y, int button);
    Q_INVOKABLE bool hasSelectionAt(qreal x, qreal y) const;

signals:
    void editorStoreChanged();
    void selectionDoubleClicked(const QString &selectionId);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;

private:
    QPointF toBoard(const QPointF &itemPos, bool clamp = false) const;
    ResizeHandle hitHandle(const QPointF &point, const SelectionRect &rect) const;
    QString hitSelectionId(const QPointF &point) const;
    void applyResize(const QPointF &current);
    void updateCursorForSelection(const QPointF &point);
    void finalizeInteraction(const QPointF &point);
    void updateToolCursor();

    QPointer<EditorStore> m_store;
    bool m_isDrawing = false;
    bool m_isSelecting = false;
    bool m_isResizing = false;
    bool m_isErasing = false;
    QPointF m_selectStart;
    QVector<QPointF> m_livePoints;
    QString m_activeSelectionId;

    static constexpr qreal kBoardWidth = 3000.0;
    static constexpr qreal kBoardHeight = 2000.0;
    static constexpr qreal kMinStrokeSamplePx = 0.45;
};
