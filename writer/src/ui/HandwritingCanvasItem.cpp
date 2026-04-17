#include "HandwritingCanvasItem.h"

#include "app/WriterController.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>

namespace {
double polylineLength(const QVector<QPointF> &pts) {
    double len = 0;
    for (int i = 1; i < pts.size(); ++i)
        len += QLineF(pts[i - 1], pts[i]).length();
    return len;
}

double dist(const QPointF &a, const QPointF &b) {
    return QLineF(a, b).length();
}
}

void HandwritingCanvasItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) {
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
    if (!qFuzzyCompare(newGeometry.width(), oldGeometry.width())) {
        m_layoutDirty = true;
        update();
    }
}

HandwritingCanvasItem::HandwritingCanvasItem(QQuickItem *parent) : QQuickPaintedItem(parent) {
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
    m_runTimer.setInterval(16);
    connect(&m_runTimer, &QTimer::timeout, this, &HandwritingCanvasItem::onRunTick);
}

void HandwritingCanvasItem::setController(WriterController *c) {
    if (m_ctrl == c) return;
    if (m_ctrl) {
        disconnect(m_ctrl, nullptr, this, nullptr);
    }
    m_ctrl = c;
    if (m_ctrl) {
        connect(m_ctrl, &WriterController::layoutInvalidated, this, &HandwritingCanvasItem::onInvalidated);
        connect(m_ctrl, &WriterController::runActiveChanged, this, [this]() {
            if (!m_ctrl) return;
            if (m_ctrl->runActive()) {
                m_layoutDirty = true;
                rebuildLayout();
                m_layoutDirty = false;
                rebuildRunPath();
                m_runDistance = 0;
                if (m_runTotalCm <= 1e-9) {
                    m_ctrl->stopRun();
                    return;
                }
                m_runTimer.start();
            } else {
                m_runTimer.stop();
                m_runDistance = 0;
            }
            update();
        });
    }
    m_layoutDirty = true;
    emit controllerChanged();
    update();
}

double HandwritingCanvasItem::pxPerCm() const {
    if (!m_ctrl || !m_ctrl->settings()) return 40.0;
    const double w = width();
    if (w < 4) return 40.0;
    return w / m_ctrl->settings()->pageWidthCm();
}

QPointF HandwritingCanvasItem::cmFromPixel(const QPointF &px) const {
    const double s = pxPerCm();
    return QPointF(px.x() / s, px.y() / s);
}

QPointF HandwritingCanvasItem::pixelFromCm(const QPointF &cm) const {
    const double s = pxPerCm();
    return QPointF(cm.x() * s, cm.y() * s);
}

QPointF HandwritingCanvasItem::glyphBottomLeft(const LayoutGlyph &g) const {
    return QPointF(g.bboxCm.left(), g.bboxCm.bottom());
}

void HandwritingCanvasItem::rebuildLayout() {
    if (!m_ctrl || !m_ctrl->settings()) return;
    const AppSettings *st = m_ctrl->settings();
    m_layout = LayoutEngine::layout(
        m_ctrl->document()->text(),
        m_ctrl->fontMap(),
        st->fontUnitToCm(),
        st->pageWidthCm(),
        st->pageHeightCm(),
        st->leftMarginCm(),
        st->rightMarginCm(),
        st->verticalGapCm(),
        st->hxCm(),
        st->hyCm(),
        st->lineHeightCm(),
        m_ctrl->anchorOverrides()
    );
    const double hPx = qMax(100.0, m_layout.totalHeightCm * pxPerCm());
    setImplicitHeight(hPx);
}

void HandwritingCanvasItem::onInvalidated() {
    m_layoutDirty = true;
    update();
}

void HandwritingCanvasItem::rebuildRunPath() {
    m_runSegments.clear();
    m_runTotalCm = 0;
    QPointF prevEnd;
    bool hasPrev = false;

    for (const LayoutGlyph &lg : m_layout.glyphs) {
        for (const QVector<QPointF> &poly : lg.polylinesCm) {
            if (poly.size() < 2) continue;
            if (hasPrev) {
                m_runSegments.push_back(qMakePair(true, QVector<QPointF>{prevEnd, poly.first()}));
                m_runTotalCm += dist(prevEnd, poly.first());
            }
            m_runSegments.push_back(qMakePair(false, poly));
            m_runTotalCm += polylineLength(poly);
            prevEnd = poly.last();
            hasPrev = true;
        }
    }
}

void HandwritingCanvasItem::onRunTick() {
    if (!m_ctrl || !m_ctrl->settings() || !m_ctrl->runActive()) return;
    if (m_runTotalCm <= 1e-9) {
        m_ctrl->stopRun();
        return;
    }
    const double v = m_ctrl->settings()->feedRateCmPerS();
    m_runDistance += v * (m_runTimer.interval() / 1000.0);
    if (m_runDistance >= m_runTotalCm) {
        m_runDistance = m_runTotalCm;
        m_ctrl->stopRun();
    }
    update();
}

int HandwritingCanvasItem::hitTestGlyph(const QPointF &px) {
    const QPointF cm = cmFromPixel(px);
    const double tol = 0.15;
    for (int i = m_layout.glyphs.size() - 1; i >= 0; --i) {
        const QRectF r = m_layout.glyphs[i].bboxCm.adjusted(-tol, -tol, tol, tol);
        if (r.contains(cm)) return m_layout.glyphs[i].docIndex;
    }
    return -1;
}

void HandwritingCanvasItem::mousePressEvent(QMouseEvent *event) {
    if (!m_ctrl || event->button() != Qt::LeftButton) {
        QQuickPaintedItem::mousePressEvent(event);
        return;
    }
    rebuildLayout();
    m_layoutDirty = false;
    const QPointF local = event->position();
    m_dragDocIndex = hitTestGlyph(local);
    if (m_dragDocIndex < 0) {
        QQuickPaintedItem::mousePressEvent(event);
        return;
    }
    m_pressCm = cmFromPixel(local);
    for (const LayoutGlyph &lg : m_layout.glyphs) {
        if (lg.docIndex < m_dragDocIndex) m_frozenDuringDrag.insert(lg.docIndex, glyphBottomLeft(lg));
    }
    m_dragGlyphStartCm = QPointF();
    for (const LayoutGlyph &lg : m_layout.glyphs) {
        if (lg.docIndex == m_dragDocIndex) {
            m_dragGlyphStartCm = glyphBottomLeft(lg);
            break;
        }
    }
    QHash<int, QPointF> merged = m_frozenDuringDrag;
    merged.insert(m_dragDocIndex, m_dragGlyphStartCm);
    m_ctrl->setAnchorOverrides(merged);
    event->accept();
}

void HandwritingCanvasItem::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragDocIndex < 0 || !m_ctrl) {
        QQuickPaintedItem::mouseMoveEvent(event);
        return;
    }
    const QPointF cm = cmFromPixel(event->position());
    const QPointF delta = cm - m_pressCm;
    const QPointF newBl = m_dragGlyphStartCm + delta;
    QHash<int, QPointF> merged = m_frozenDuringDrag;
    merged.insert(m_dragDocIndex, newBl);
    m_ctrl->setAnchorOverrides(merged);
    event->accept();
}

void HandwritingCanvasItem::mouseReleaseEvent(QMouseEvent *event) {
    if (m_dragDocIndex < 0 || !m_ctrl) {
        QQuickPaintedItem::mouseReleaseEvent(event);
        return;
    }
    const QPointF cm = cmFromPixel(event->position());
    const QPointF newBl = m_dragGlyphStartCm + (cm - m_pressCm);
    QHash<int, QPointF> merged = m_frozenDuringDrag;
    merged.insert(m_dragDocIndex, newBl);
    m_ctrl->setAnchorOverrides(merged);
    m_dragDocIndex = -1;
    m_frozenDuringDrag.clear();
    event->accept();
}

void HandwritingCanvasItem::paint(QPainter *painter) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->fillRect(boundingRect(), QColor("#f4f4f5"));

    if (!m_ctrl || !m_ctrl->settings()) return;
    if (m_layoutDirty) {
        rebuildLayout();
        m_layoutDirty = false;
    }

    const AppSettings *st = m_ctrl->settings();
    const double s = pxPerCm();
    const double pageW = st->pageWidthCm();
    const double pageH = st->pageHeightCm();
    const double gap = st->verticalGapCm();
    const double lm = st->leftMarginCm();
    const double rm = st->rightMarginCm();
    const QColor green(72, 180, 96, 55);

    auto pageTop = [&](int p) { return p * (pageH + gap); };

    int lastPage = 0;
    for (const LayoutGlyph &lg : m_layout.glyphs)
        lastPage = qMax(lastPage, lg.pageIndex);

    for (int p = 0; p <= lastPage; ++p) {
        const double top = pageTop(p);
        const QRectF pageCm(0, top, pageW, pageH);
        const QRectF pagePx(pageCm.topLeft() * s, pageCm.size() * s);

        painter->setPen(QPen(QColor("#d4d4d8"), 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(pagePx);

        const QRectF leftCm(0, top, lm, pageH);
        painter->fillRect(QRectF(leftCm.topLeft() * s, leftCm.size() * s), green);

        const QRectF rightCm(pageW - rm, top, rm, pageH);
        painter->fillRect(QRectF(rightCm.topLeft() * s, rightCm.size() * s), green);

        if (p < lastPage) {
            const QRectF gapCm(0, top + pageH, pageW, gap);
            painter->fillRect(QRectF(gapCm.topLeft() * s, gapCm.size() * s), green);
        }
    }

    painter->setPen(QPen(Qt::black, 1.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    for (const LayoutGlyph &lg : m_layout.glyphs) {
        for (const QVector<QPointF> &poly : lg.polylinesCm) {
            if (poly.size() < 2) continue;
            QPainterPath path;
            path.moveTo(poly[0] * s);
            for (int i = 1; i < poly.size(); ++i) path.lineTo(poly[i] * s);
            painter->drawPath(path);
        }
    }

    if (!m_ctrl->runActive()) return;

    const double drawDist = m_runDistance;
    if (drawDist <= 0) return;

    double remaining = drawDist;
    painter->setPen(QPen(QColor("#dc2626"), 1.2));
    painter->setBrush(QColor("#dc2626"));

    for (int si = 0; si < m_runSegments.size(); ++si) {
        const bool travel = m_runSegments[si].first;
        const QVector<QPointF> &pts = m_runSegments[si].second;
        if (travel) {
            if (pts.size() < 2) continue;
            const double segLen = dist(pts[0], pts[1]);
            if (remaining >= segLen) {
                painter->drawLine(pts[0] * s, pts[1] * s);
                painter->drawEllipse(pts[0] * s, 2.5, 2.5);
                painter->drawEllipse(pts[1] * s, 2.5, 2.5);
                remaining -= segLen;
            } else if (remaining > 1e-9) {
                const QPointF dir = pts[1] - pts[0];
                const double t = remaining / segLen;
                const QPointF end = pts[0] + dir * t;
                painter->drawLine(pts[0] * s, end * s);
                painter->drawEllipse(pts[0] * s, 2.5, 2.5);
                remaining = 0;
                break;
            } else break;
        } else {
            if (pts.size() < 2) continue;
            double segConsumed = 0;
            for (int i = 0; i < pts.size(); ++i) {
                if (i == 0) {
                    painter->drawEllipse(pts[0] * s, 2.5, 2.5);
                    continue;
                }
                const double d = dist(pts[i - 1], pts[i]);
                if (remaining >= d) {
                    painter->drawLine(pts[i - 1] * s, pts[i] * s);
                    painter->drawEllipse(pts[i] * s, 2.5, 2.5);
                    remaining -= d;
                } else if (remaining > 1e-9) {
                    const QPointF dir = pts[i] - pts[i - 1];
                    const double t = remaining / d;
                    const QPointF end = pts[i - 1] + dir * t;
                    painter->drawLine(pts[i - 1] * s, end * s);
                    remaining = 0;
                    break;
                } else {
                    break;
                }
                if (remaining <= 0) break;
            }
            if (remaining <= 0) break;
        }
    }
}
