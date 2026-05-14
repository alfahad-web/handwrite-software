#include "HandwritingCanvasItem.h"

#include "app/AppSettings.h"
#include "app/WriterController.h"

#include <QPainter>
#include <QPainterPath>
#include <QQuickWindow>
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

void drawPolylinePortionCm(
    QPainter *painter,
    const QVector<QPointF> &poly,
    double fromAlong,
    double toAlong,
    double s
) {
    if (poly.size() < 2 || toAlong <= fromAlong + 1e-12) return;

    double pos = 0;
    for (int i = 1; i < poly.size(); ++i) {
        const double segLen = dist(poly[i - 1], poly[i]);
        if (segLen < 1e-12) continue;
        const double segEnd = pos + segLen;
        const double u0 = qMax(fromAlong, pos);
        const double u1 = qMin(toAlong, segEnd);
        if (u1 > u0 + 1e-12) {
            const double t0 = (u0 - pos) / segLen;
            const double t1 = (u1 - pos) / segLen;
            const QPointF a = poly[i - 1];
            const QPointF b = poly[i];
            const QPointF p0 = a + (b - a) * t0;
            const QPointF p1 = a + (b - a) * t1;
            painter->drawEllipse(p0 * s, 2.5, 2.5);
            painter->drawLine(p0 * s, p1 * s);
            painter->drawEllipse(p1 * s, 2.5, 2.5);
        }
        pos = segEnd;
        if (toAlong <= segEnd + 1e-12) break;
    }
}
}

void HandwritingCanvasItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) {
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
    if (!qFuzzyCompare(newGeometry.width(), oldGeometry.width())) {
        if (m_ctrl && m_ctrl->runActive()) m_ctrl->stopRun();
        m_layoutDirty = true;
        clearRunCaches();
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
                emit runPreparationStarted();
                QTimer::singleShot(0, this, &HandwritingCanvasItem::prepareRunSimulationAfterUi);
            } else {
                m_runTimer.stop();
                m_runDistance = 0;
                m_runLastPaintedDist = 0;
                clearRunCaches();
                update();
            }
        });
    }
    m_layoutDirty = true;
    emit controllerChanged();
    update();
}

void HandwritingCanvasItem::clearRunCaches() {
    m_runStaticValid = false;
    m_runStaticPixmap = QPixmap();
    m_runRedPixmap = QPixmap();
    m_runSegCumStartCm.clear();
    m_runSegLenCm.clear();
    m_runSegments.clear();
    m_runTotalCm = 0;
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

QPointF HandwritingCanvasItem::glyphBottomLeft(const LayoutGlyph &g) const {
    return g.placementAnchorCm;
}

QHash<int, QPointF> HandwritingCanvasItem::forcedAnchorsForLayout() const {
    if (!m_ctrl) return {};
    QHash<int, QPointF> f = m_ctrl->manualAnchors();
    if (m_dragDocIndex >= 0) {
        f.insert(m_dragDocIndex, m_dragGlyphStartCm + (m_currentDragCm - m_pressCm));
    }
    return f;
}

void HandwritingCanvasItem::rebuildLayout() {
    if (!m_ctrl || !m_ctrl->settings()) return;
    const AppSettings *st = m_ctrl->settings();
    m_layout = LayoutEngine::layout(
        m_ctrl->document()->text(),
        m_ctrl->fontCatalog(),
        st->fontUnitToCm(),
        st->pageWidthCm(),
        st->pageHeightCm(),
        st->leftMarginCm(),
        st->rightMarginCm(),
        st->verticalGapCm(),
        st->hxCm(),
        st->hyCm(),
        st->lineHeightCm(),
        st->joinDistMm(),
        forcedAnchorsForLayout()
    );
    m_ctrl->notifyLineHeightCollision(m_layout.anyGlyphExceedsLineHeight);
    const double hPx = qMax(100.0, m_layout.totalHeightCm * pxPerCm());
    setImplicitHeight(hPx);
}

void HandwritingCanvasItem::onInvalidated() {
    if (m_ctrl && m_selectedDocIndex >= m_ctrl->document()->text().size()) m_selectedDocIndex = -1;
    if (m_ctrl && m_ctrl->runActive()) m_ctrl->stopRun();
    m_layoutDirty = true;
    clearRunCaches();
    update();
}

double HandwritingCanvasItem::segmentLengthCm(const QPair<bool, QVector<QPointF>> &seg) const {
    const QVector<QPointF> &pts = seg.second;
    if (pts.size() < 2) return 0;
    if (seg.first) return dist(pts[0], pts[1]);
    return polylineLength(pts);
}

void HandwritingCanvasItem::rebuildRunPath() {
    m_runSegments.clear();
    m_runSegCumStartCm.clear();
    m_runSegLenCm.clear();
    m_runTotalCm = 0;
    QPointF prevEnd;
    bool hasPrev = false;

    for (const LayoutGlyph &lg : m_layout.glyphs) {
        for (const QVector<QPointF> &poly : lg.polylinesCm) {
            if (poly.size() < 2) continue;
            if (hasPrev) {
                QVector<QPointF> bridge = {prevEnd, poly.first()};
                const double len = dist(prevEnd, poly.first());
                m_runSegCumStartCm.push_back(m_runTotalCm);
                m_runSegLenCm.push_back(len);
                m_runSegments.push_back(qMakePair(true, bridge));
                m_runTotalCm += len;
            }
            const double len = polylineLength(poly);
            m_runSegCumStartCm.push_back(m_runTotalCm);
            m_runSegLenCm.push_back(len);
            m_runSegments.push_back(qMakePair(false, poly));
            m_runTotalCm += len;
            prevEnd = poly.last();
            hasPrev = true;
        }
    }
}

void HandwritingCanvasItem::drawRunProgressAlongPath(QPainter *painter, double pathFrom, double pathTo, double s) const {
    if (pathTo <= pathFrom + 1e-12 || m_runSegments.isEmpty()) return;

    painter->setPen(QPen(QColor("#dc2626"), 1.2));
    painter->setBrush(QColor("#dc2626"));

    for (int si = 0; si < m_runSegments.size(); ++si) {
        const double segStart = m_runSegCumStartCm.at(si);
        const double segLen = m_runSegLenCm.at(si);
        const double segEnd = segStart + segLen;
        if (segEnd <= pathFrom + 1e-12) continue;
        const double u0 = qMax(0.0, pathFrom - segStart);
        const double u1 = qMin(segLen, pathTo - segStart);
        if (u1 <= u0 + 1e-12) {
            if (pathTo <= segEnd + 1e-12) break;
            continue;
        }

        const bool travel = m_runSegments[si].first;
        const QVector<QPointF> &pts = m_runSegments[si].second;
        if (travel && pts.size() >= 2) {
            const QPointF a = pts[0];
            const QPointF b = pts[1];
            const QPointF p0 = a + (b - a) * (u0 / segLen);
            const QPointF p1 = a + (b - a) * (u1 / segLen);
            painter->drawLine(p0 * s, p1 * s);
            painter->drawEllipse(p0 * s, 2.5, 2.5);
            painter->drawEllipse(p1 * s, 2.5, 2.5);
        } else if (!travel && pts.size() >= 2) {
            drawPolylinePortionCm(painter, pts, u0, u1, s);
        }

        if (pathTo <= segEnd + 1e-12) break;
    }
}

void HandwritingCanvasItem::paintStaticContent(QPainter *painter, const AppSettings *st, double s) const {
    const double pageW = st->pageWidthCm();
    const double pageH = st->pageHeightCm();
    const double gap = st->verticalGapCm();
    const double lm = st->leftMarginCm();
    const double rm = st->rightMarginCm();
    const QColor green(72, 180, 96, 55);

    auto pageTop = [&](int p) { return p * pageH; };

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

        if (gap > 1e-9) {
            const QRectF topGapCm(0, top, pageW, gap);
            painter->fillRect(QRectF(topGapCm.topLeft() * s, topGapCm.size() * s), green);
        }

        const QRectF leftCm(0, top, lm, pageH);
        painter->fillRect(QRectF(leftCm.topLeft() * s, leftCm.size() * s), green);

        const QRectF rightCm(pageW - rm, top, rm, pageH);
        painter->fillRect(QRectF(rightCm.topLeft() * s, rightCm.size() * s), green);
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

    if (m_selectedDocIndex >= 0) {
        painter->setPen(QPen(QColor("#2563eb"), 1));
        painter->setBrush(QColor("#2563eb"));
        const double r = 3.0;
        for (const LayoutGlyph &lg : m_layout.glyphs) {
            if (lg.docIndex != m_selectedDocIndex) continue;
            for (const QVector<QPointF> &poly : lg.polylinesCm) {
                for (const QPointF &pt : poly) {
                    const QPointF px = pt * s;
                    painter->drawEllipse(px, r, r);
                }
            }
            break;
        }
    }
}

void HandwritingCanvasItem::prepareRunSimulationAfterUi() {
    if (!m_ctrl || !m_ctrl->runActive()) {
        emit runPreparationFinished();
        return;
    }

    m_layoutDirty = true;
    rebuildLayout();
    m_layoutDirty = false;
    rebuildRunPath();

    if (m_runTotalCm <= 1e-9) {
        clearRunCaches();
        emit runPreparationFinished();
        m_ctrl->stopRun();
        update();
        return;
    }

    const int w = qMax(1, int(qCeil(width())));
    const int h = qMax(1, int(qCeil(height())));
    const QSize sz(w, h);
    const qreal dpr = window() ? window()->devicePixelRatio() : 1.0;
    const QSize phys(qCeil(sz.width() * dpr), qCeil(sz.height() * dpr));

    m_runStaticPixmap = QPixmap(phys);
    m_runStaticPixmap.setDevicePixelRatio(dpr);
    m_runStaticPixmap.fill(QColor("#f4f4f5"));

    {
        QPainter p(&m_runStaticPixmap);
        p.setRenderHint(QPainter::Antialiasing, true);
        const AppSettings *st = m_ctrl->settings();
        const double s = pxPerCm();
        paintStaticContent(&p, st, s);
    }

    if (!m_ctrl || !m_ctrl->runActive()) {
        clearRunCaches();
        emit runPreparationFinished();
        update();
        return;
    }

    m_runRedPixmap = QPixmap(phys);
    m_runRedPixmap.setDevicePixelRatio(dpr);
    m_runRedPixmap.fill(Qt::transparent);

    m_runStaticValid = true;
    m_runDistance = 0;
    m_runLastPaintedDist = 0;
    m_runElapsed.restart();

    emit runPreparationFinished();
    m_runTimer.start();
    update();
}

void HandwritingCanvasItem::onRunTick() {
    if (!m_ctrl || !m_ctrl->settings() || !m_ctrl->runActive()) return;
    if (m_runTotalCm <= 1e-9) {
        m_ctrl->stopRun();
        return;
    }
    const qint64 ns = m_runElapsed.nsecsElapsed();
    m_runElapsed.restart();
    const double dt = qBound(0.0, ns / 1e9, 0.25);
    const double v = m_ctrl->settings()->feedRateCmPerS();
    m_runDistance += v * dt;
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

void HandwritingCanvasItem::mouseDoubleClickEvent(QMouseEvent *event) {
    if (!m_ctrl || event->button() != Qt::LeftButton) {
        QQuickPaintedItem::mouseDoubleClickEvent(event);
        return;
    }
    rebuildLayout();
    m_layoutDirty = false;
    const int hit = hitTestGlyph(event->position());
    m_selectedDocIndex = hit;
    m_dragDocIndex = -1;
    event->accept();
    update();
}

void HandwritingCanvasItem::mousePressEvent(QMouseEvent *event) {
    if (!m_ctrl || event->button() != Qt::LeftButton) {
        QQuickPaintedItem::mousePressEvent(event);
        return;
    }
    rebuildLayout();
    m_layoutDirty = false;
    const QPointF local = event->position();
    const int hit = hitTestGlyph(local);
    if (hit < 0) {
        m_selectedDocIndex = -1;
        m_dragDocIndex = -1;
        update();
        event->accept();
        return;
    }
    if (hit != m_selectedDocIndex) {
        event->accept();
        return;
    }
    m_dragDocIndex = hit;
    m_pressCm = cmFromPixel(local);
    m_currentDragCm = m_pressCm;
    m_dragGlyphStartCm = QPointF();
    for (const LayoutGlyph &lg : m_layout.glyphs) {
        if (lg.docIndex == m_dragDocIndex) {
            m_dragGlyphStartCm = glyphBottomLeft(lg);
            break;
        }
    }
    event->accept();
}

void HandwritingCanvasItem::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragDocIndex < 0 || !m_ctrl) {
        QQuickPaintedItem::mouseMoveEvent(event);
        return;
    }
    m_currentDragCm = cmFromPixel(event->position());
    rebuildLayout();
    event->accept();
    update();
}

void HandwritingCanvasItem::mouseReleaseEvent(QMouseEvent *event) {
    if (m_dragDocIndex < 0 || !m_ctrl) {
        QQuickPaintedItem::mouseReleaseEvent(event);
        return;
    }
    m_currentDragCm = cmFromPixel(event->position());
    const QPointF newBl = m_dragGlyphStartCm + (m_currentDragCm - m_pressCm);
    m_ctrl->setManualAnchor(m_dragDocIndex, newBl);
    m_dragDocIndex = -1;
    m_layoutDirty = true;
    event->accept();
    update();
}

void HandwritingCanvasItem::paint(QPainter *painter) {
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (!m_ctrl || !m_ctrl->settings()) {
        painter->fillRect(boundingRect(), QColor("#f4f4f5"));
        return;
    }

    const AppSettings *st = m_ctrl->settings();
    const double s = pxPerCm();

    if (m_ctrl->runActive() && m_runStaticValid && !m_runStaticPixmap.isNull()) {
        painter->fillRect(boundingRect(), QColor("#f4f4f5"));
        painter->drawPixmap(0, 0, m_runStaticPixmap);

        if (m_runDistance > m_runLastPaintedDist + 1e-9 && !m_runRedPixmap.isNull()) {
            QPainter rp(&m_runRedPixmap);
            rp.setRenderHint(QPainter::Antialiasing, true);
            drawRunProgressAlongPath(&rp, m_runLastPaintedDist, m_runDistance, s);
            rp.end();
            m_runLastPaintedDist = m_runDistance;
        }
        painter->drawPixmap(0, 0, m_runRedPixmap);
        return;
    }

    painter->fillRect(boundingRect(), QColor("#f4f4f5"));

    if (m_layoutDirty) {
        rebuildLayout();
        m_layoutDirty = false;
    }

    paintStaticContent(painter, st, s);

    if (!m_ctrl->runActive()) return;
    if (m_runSegCumStartCm.isEmpty()) return;

    const double drawDist = m_runDistance;
    if (drawDist <= 0) return;

    painter->setPen(QPen(QColor("#dc2626"), 1.2));
    painter->setBrush(QColor("#dc2626"));
    drawRunProgressAlongPath(painter, 0, drawDist, s);
}
