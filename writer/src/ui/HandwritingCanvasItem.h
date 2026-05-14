#pragma once

#include <QHash>
#include <QMouseEvent>
#include <QQuickPaintedItem>
#include <QTimer>

#include "layout/LayoutEngine.h"

class WriterController;

class HandwritingCanvasItem : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(WriterController *controller READ controller WRITE setController NOTIFY controllerChanged)

public:
    explicit HandwritingCanvasItem(QQuickItem *parent = nullptr);

    WriterController *controller() const { return m_ctrl; }
    void setController(WriterController *c);

    void paint(QPainter *painter) override;

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

signals:
    void controllerChanged();

private slots:
    void onInvalidated();
    void onRunTick();

private:
    QHash<int, QPointF> forcedAnchorsForLayout() const;
    void rebuildLayout();
    double pxPerCm() const;
    QPointF cmFromPixel(const QPointF &px) const;
    int hitTestGlyph(const QPointF &px);
    void rebuildRunPath();
    QPointF glyphBottomLeft(const LayoutGlyph &g) const;

    WriterController *m_ctrl = nullptr;
    LayoutResult m_layout;
    bool m_layoutDirty = true;

    int m_selectedDocIndex = -1;
    int m_dragDocIndex = -1;
    QPointF m_pressCm;
    QPointF m_currentDragCm;
    QPointF m_dragGlyphStartCm;

    QTimer m_runTimer;
    QVector<QPair<bool, QVector<QPointF>>> m_runSegments;
    double m_runTotalCm = 0;
    double m_runDistance = 0;
};
