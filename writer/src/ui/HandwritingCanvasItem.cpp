#include "HandwritingCanvasItem.h"

#include "app/AppSettings.h"
#include "app/WriterController.h"
#include "cnc/GrblConnection.h"
#include "gcode/PathBuilder.h"

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
        if (m_ctrl && m_ctrl->runActive()) {
            if (m_ctrl->grbl()->connected())
                m_ctrl->stopRunPreserveCnc();
            else
                m_ctrl->stopRun();
        }
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
        if (m_ctrl->settings()) {
            connect(m_ctrl->settings(), &AppSettings::previewDisplayScaleChanged, this, [this]() { update(); });
        }
        connect(m_ctrl, &WriterController::runActiveChanged, this, [this]() {
            if (!m_ctrl) return;
            if (m_ctrl->runActive()) {
                emit runPreparationStarted();
                QTimer::singleShot(0, this, &HandwritingCanvasItem::prepareRunSimulationAfterUi);
            } else {
                m_runTimer.stop();
                if (!m_ctrl->runArmed()) {
                    m_runDistance = 0;
                    m_runLastPaintedDist = 0;
                    clearRunCaches();
                } else {
                    m_runDistance = m_ctrl->runStartDistanceCm();
                    m_runLastPaintedDist = 0;
                    if (!m_runStaticValid)
                        QTimer::singleShot(0, this, &HandwritingCanvasItem::onRunArmVisualsChanged);
                    else
                        update();
                }
                update();
            }
        });
        connect(m_ctrl, &WriterController::runArmVisualsChanged, this, &HandwritingCanvasItem::onRunArmVisualsChanged);
        connect(m_ctrl->grbl(), &GrblConnection::positionChanged, this,
                &HandwritingCanvasItem::onLiveCncPositionChanged);
        connect(m_ctrl, &WriterController::runPausedChanged, this, [this]() {
            if (!m_ctrl || !m_ctrl->runActive()) return;
            if (m_ctrl->runPaused()) {
                m_runTimer.stop();
            } else {
                const bool needsTimer = m_approachActive
                        || (!m_ctrl->grbl()->connected() && m_runStaticValid
                            && m_runDistance < m_runEndDistanceCm - 1e-9);
                if (needsTimer) {
                    m_runElapsed.restart();
                    m_runTimer.start();
                }
            }
            update();
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
    m_approachActive = false;
    m_approachTraveledCm = 0;
}

void HandwritingCanvasItem::setDragDocIndex(int idx) {
    if (m_dragDocIndex == idx) return;
    const bool before = (m_dragDocIndex >= 0);
    m_dragDocIndex = idx;
    const bool after = (m_dragDocIndex >= 0);
    if (before != after) emit glyphDragActiveChanged();
}

double HandwritingCanvasItem::pxPerCm() const {
    if (!m_ctrl || !m_ctrl->settings()) return 40.0;
    const double w = width();
    if (w < 4) return 40.0;
    return (w / m_ctrl->settings()->pageWidthCm()) * m_ctrl->settings()->previewDisplayScale();
}

double HandwritingCanvasItem::contentOffsetX() const {
    if (!m_ctrl || !m_ctrl->settings()) return 0.0;
    const double pagePx = m_ctrl->settings()->pageWidthCm() * pxPerCm();
    return qMax(0.0, (width() - pagePx) * 0.5);
}

QPointF HandwritingCanvasItem::cmFromPixel(const QPointF &px) const {
    const double s = pxPerCm();
    return QPointF((px.x() - contentOffsetX()) / s, px.y() / s);
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
    if (m_ctrl && m_ctrl->runActive()) {
        if (m_ctrl->grbl()->connected())
            m_ctrl->stopRunPreserveCnc();
        else
            m_ctrl->stopRun();
    }
    m_layoutDirty = true;
    clearRunCaches();
    update();
}

double HandwritingCanvasItem::segmentLengthCm(const PathSegment &seg) const {
    const QVector<QPointF> &pts = seg.pointsCm;
    if (pts.size() < 2) return 0;
    if (seg.travel) return dist(pts[0], pts[1]);
    return polylineLength(pts);
}

void HandwritingCanvasItem::rebuildRunPath() {
    m_runSegments.clear();
    m_runSegCumStartCm.clear();
    m_runSegLenCm.clear();
    m_runTotalCm = 0;

    const PathBuildResult built = PathBuilder::build(m_layout);
    for (const PathSegment &seg : built.segments) {
        const double len = segmentLengthCm(seg);
        if (len <= 1e-9) continue;
        m_runSegCumStartCm.push_back(m_runTotalCm);
        m_runSegLenCm.push_back(len);
        m_runSegments.push_back(seg);
        m_runTotalCm += len;
    }
}

void HandwritingCanvasItem::drawRunProgressAlongPath(QPainter *painter, double pathFrom, double pathTo, double s) const {
    if (pathTo <= pathFrom + 1e-12 || m_runSegments.isEmpty()) return;

    painter->save();
    painter->translate(contentOffsetX(), 0);
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

        const bool travel = m_runSegments[si].travel;
        const QVector<QPointF> &pts = m_runSegments[si].pointsCm;
        if (travel && pts.size() >= 2) {
            const QPointF a = pts[0];
            const QPointF b = pts[1];
            const QPointF p0 = a + (b - a) * (u0 / segLen);
            const QPointF p1 = a + (b - a) * (u1 / segLen);
            painter->setBrush(Qt::NoBrush);
            painter->drawLine(p0 * s, p1 * s);
        } else if (!travel && pts.size() >= 2) {
            painter->setBrush(QColor("#dc2626"));
            drawPolylinePortionCm(painter, pts, u0, u1, s);
        }

        if (pathTo <= segEnd + 1e-12) break;
    }
    painter->restore();
}

QPointF HandwritingCanvasItem::pagePlotOriginCm(int page) const {
    if (!m_ctrl || !m_ctrl->settings()) return QPointF();
    const AppSettings *st = m_ctrl->settings();
    return QPointF(0, page * st->pageHeightCm() + st->verticalGapCm());
}

QPointF HandwritingCanvasItem::layoutCmFromMachineMm(double mmX, double mmY) const {
    // Inverse of GcodeGenerator layoutCmToMachineMm: GRBL X = layout Y, GRBL Y = layout X.
    return QPointF(mmY / 10.0, mmX / 10.0);
}

bool HandwritingCanvasItem::useLiveCncTip() const {
    return m_ctrl && m_ctrl->grbl()->connected() && m_ctrl->grbl()->positionKnown()
           && m_ctrl->viewMode() == QLatin1String("handwriting");
}

double HandwritingCanvasItem::pathDistanceForLayoutPoint(const QPointF &layoutCm) const {
    if (m_runSegments.isEmpty()) return 0;

    double bestDist2 = 1e300;
    double bestAlong = 0;

    for (int si = 0; si < m_runSegments.size(); ++si) {
        const double segStart = m_runSegCumStartCm.at(si);
        const double segLen = m_runSegLenCm.at(si);
        const QVector<QPointF> &pts = m_runSegments.at(si).pointsCm;
        if (pts.size() < 2 || segLen <= 1e-12) continue;

        if (m_runSegments.at(si).travel) {
            const QPointF a = pts.at(0);
            const QPointF b = pts.at(1);
            const QPointF ab = b - a;
            const double abLen2 = QPointF::dotProduct(ab, ab);
            double t = 0;
            if (abLen2 > 1e-12)
                t = qBound(0.0, QPointF::dotProduct(layoutCm - a, ab) / abLen2, 1.0);
            const QPointF closest = a + ab * t;
            const double d = dist(layoutCm, closest);
            const double d2 = d * d;
            if (d2 < bestDist2) {
                bestDist2 = d2;
                bestAlong = segStart + t * segLen;
            }
            continue;
        }

        double pos = 0;
        for (int i = 1; i < pts.size(); ++i) {
            const QPointF a = pts.at(i - 1);
            const QPointF b = pts.at(i);
            const QPointF ab = b - a;
            const double sl = dist(a, b);
            if (sl < 1e-12) continue;
            const double abLen2 = sl * sl;
            const double t = qBound(0.0, QPointF::dotProduct(layoutCm - a, ab) / abLen2, 1.0);
            const QPointF closest = a + ab * t;
            const double d = dist(layoutCm, closest);
            const double d2 = d * d;
            if (d2 < bestDist2) {
                bestDist2 = d2;
                bestAlong = segStart + pos + t * sl;
            }
            pos += sl;
        }
    }

    return bestAlong;
}

void HandwritingCanvasItem::syncRedTrailFromLivePosition() {
    if (!m_ctrl || !m_runStaticValid || m_runRedPixmap.isNull() || !useLiveCncTip()) return;
    if (!m_ctrl->runActive() || m_approachActive) return;

    const QPointF tipCm = layoutCmFromMachineMm(m_ctrl->grbl()->posX(), m_ctrl->grbl()->posY());
    double along = pathDistanceForLayoutPoint(tipCm);
    along = qBound(m_runStartDistanceCm, along, m_runEndDistanceCm);

    constexpr double kCatchUpCm = 0.35;
    const double gap = along - m_runLastPaintedDist;

    if (along + 0.08 < m_runLastPaintedDist) {
        m_runLastPaintedDist = qMax(m_runStartDistanceCm, along - 0.02);
        m_runDistance = along;
        return;
    }

    if (gap <= 0.02) return;

    // Preview behind CNC: draw the whole gap in one step (no frame-by-frame catch-up lag).
    if (gap < kCatchUpCm) {
        if (m_redTrailThrottle.isValid() && m_redTrailThrottle.elapsed() < 50)
            return;
        m_redTrailThrottle.restart();
    }

    m_runDistance = along;
    QPainter rp(&m_runRedPixmap);
    rp.setRenderHint(QPainter::Antialiasing, true);
    drawRunProgressAlongPath(&rp, m_runLastPaintedDist, m_runDistance, pxPerCm());
    rp.end();
    m_runLastPaintedDist = m_runDistance;
}

void HandwritingCanvasItem::onLiveCncPositionChanged() {
    if (!useLiveCncTip()) return;
    syncRedTrailFromLivePosition();
    update();
}

QPointF HandwritingCanvasItem::pointAtPathDistance(double distCm) const {
    if (m_runSegments.isEmpty()) return QPointF();
    if (distCm <= 1e-12) {
        const QVector<QPointF> &pts = m_runSegments.first().pointsCm;
        return pts.isEmpty() ? QPointF() : pts.first();
    }

    for (int si = 0; si < m_runSegments.size(); ++si) {
        const double segStart = m_runSegCumStartCm.at(si);
        const double segLen = m_runSegLenCm.at(si);
        const double segEnd = segStart + segLen;
        if (distCm > segEnd + 1e-12) continue;

        const double u = qBound(0.0, distCm - segStart, segLen);
        const QVector<QPointF> &pts = m_runSegments.at(si).pointsCm;
        if (pts.size() < 2) return pts.isEmpty() ? QPointF() : pts.first();

        if (m_runSegments.at(si).travel) {
            const double t = segLen > 1e-12 ? u / segLen : 0;
            return pts.at(0) + (pts.at(1) - pts.at(0)) * t;
        }

        double pos = 0;
        for (int i = 1; i < pts.size(); ++i) {
            const double sl = dist(pts.at(i - 1), pts.at(i));
            if (sl < 1e-12) continue;
            if (pos + sl >= u - 1e-12) {
                const double t = (u - pos) / sl;
                return pts.at(i - 1) + (pts.at(i) - pts.at(i - 1)) * t;
            }
            pos += sl;
        }
        return pts.last();
    }

    const QVector<QPointF> &pts = m_runSegments.last().pointsCm;
    return pts.isEmpty() ? QPointF() : pts.last();
}

void HandwritingCanvasItem::drawPurpleTip(QPainter *painter, const QPointF &cm, double s) const {
    painter->save();
    painter->translate(contentOffsetX(), 0);
    painter->setPen(QPen(QColor("#6d28d9"), 2));
    painter->setBrush(QColor("#a855f7"));
    const QPointF px = cm * s;
    painter->drawEllipse(px, 7, 7);
    painter->restore();
}

void HandwritingCanvasItem::drawApproachTravel(QPainter *painter, const QPointF &fromCm,
                                               const QPointF &toCm, double s) const {
    painter->save();
    painter->translate(contentOffsetX(), 0);
    QPen pen(QColor("#a855f7"), 1.2, Qt::DashLine);
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(fromCm * s, toCm * s);
    painter->restore();
}

void HandwritingCanvasItem::startApproachToPathStart(int page) {
    m_tipPage = page;
    m_approachFromCm = pagePlotOriginCm(page);
    m_approachToCm = pointAtPathDistance(m_runStartDistanceCm);
    const double len = dist(m_approachFromCm, m_approachToCm);
    m_approachActive = len > 1e-6;
    m_approachTraveledCm = 0;
}

QPointF HandwritingCanvasItem::currentTipPositionCm() const {
    if (!m_ctrl) return QPointF();
    if (m_ctrl->runArmed())
        return pagePlotOriginCm(m_ctrl->runStartPage());
    if (m_approachActive) {
        const double len = dist(m_approachFromCm, m_approachToCm);
        const double t = len > 1e-9 ? qMin(1.0, m_approachTraveledCm / len) : 1.0;
        return m_approachFromCm + (m_approachToCm - m_approachFromCm) * t;
    }
    if (useLiveCncTip())
        return layoutCmFromMachineMm(m_ctrl->grbl()->posX(), m_ctrl->grbl()->posY());
    if (m_ctrl->runActive())
        return pointAtPathDistance(m_runDistance);
    return QPointF();
}

void HandwritingCanvasItem::paintStaticContent(QPainter *painter, const AppSettings *st, double s) const {
    painter->save();
    painter->translate(contentOffsetX(), 0);
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

    for (const LayoutGlyph &lg : m_layout.glyphs) {
        if (lg.isMissing) {
            painter->setPen(QPen(QColor("#dc2626"), 1.5));
            painter->setBrush(Qt::NoBrush);
            const QRectF boxPx(lg.bboxCm.topLeft() * s, lg.bboxCm.size() * s);
            painter->drawRect(boxPx);
            continue;
        }
        painter->setPen(QPen(Qt::black, 1.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::NoBrush);
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
    painter->restore();
}

void HandwritingCanvasItem::onRunArmVisualsChanged() {
    if (!m_ctrl || !m_ctrl->runArmed()) return;
    m_tipPage = m_ctrl->runStartPage();
    m_approachActive = false;
    m_approachTraveledCm = 0;
    prepareRunOverlayAt(m_ctrl->runStartDistanceCm());
}

void HandwritingCanvasItem::prepareRunOverlayAt(double progressDistanceCm) {
    if (!m_ctrl || !m_ctrl->settings()) return;

    m_layoutDirty = true;
    rebuildLayout();
    m_layoutDirty = false;
    rebuildRunPath();

    if (m_runTotalCm <= 1e-9) {
        clearRunCaches();
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
        paintStaticContent(&p, st, pxPerCm());
    }

    m_runRedPixmap = QPixmap(phys);
    m_runRedPixmap.setDevicePixelRatio(dpr);
    m_runRedPixmap.fill(Qt::transparent);

    m_runStaticValid = true;
    m_runDistance = qBound(0.0, progressDistanceCm, m_runTotalCm);
    m_runLastPaintedDist = 0;
    m_approachActive = false;

    if (m_runDistance > 1e-9) {
        QPainter rp(&m_runRedPixmap);
        rp.setRenderHint(QPainter::Antialiasing, true);
        drawRunProgressAlongPath(&rp, 0, m_runDistance, pxPerCm());
        rp.end();
        m_runLastPaintedDist = m_runDistance;
    }

    update();
}

void HandwritingCanvasItem::prepareRunSimulationAfterUi() {
    if (!m_ctrl || !m_ctrl->runActive()) {
        emit runPreparationFinished();
        return;
    }

    m_runStartDistanceCm = m_ctrl->runStartDistanceCm();
    m_runEndDistanceCm = m_ctrl->runEndDistanceCm();
    if (m_runEndDistanceCm <= m_runStartDistanceCm + 1e-9) {
        emit runPreparationFinished();
        m_ctrl->stopRun();
        update();
        return;
    }

    prepareRunOverlayAt(m_runStartDistanceCm);

    if (!m_ctrl || !m_ctrl->runActive() || !m_runStaticValid) {
        emit runPreparationFinished();
        update();
        return;
    }

    m_runDistance = m_runStartDistanceCm;
    m_runLastPaintedDist = m_runStartDistanceCm;
    startApproachToPathStart(m_ctrl->executingPage());

    emit runPreparationFinished();
    if (!m_ctrl || !m_ctrl->runActive()) {
        update();
        return;
    }
    if (m_ctrl->runPaused()) {
        update();
        return;
    }

    if (!m_approachActive) {
        m_ctrl->onRunApproachComplete();
        if (!m_ctrl->grbl()->connected()) {
            m_runElapsed.restart();
            m_runTimer.start();
        }
    } else {
        m_runElapsed.restart();
        m_runTimer.start();
    }
    update();
}

void HandwritingCanvasItem::onRunTick() {
    if (!m_ctrl || !m_ctrl->settings() || !m_ctrl->runActive() || m_ctrl->runPaused()) return;

    const qint64 ns = m_runElapsed.nsecsElapsed();
    m_runElapsed.restart();
    const double dt = qBound(0.0, ns / 1e9, 0.25);
    const double v = m_ctrl->settings()->feedRateCmPerS();

    if (m_ctrl->grbl()->connected()) {
        if (m_approachActive) {
            const double len = dist(m_approachFromCm, m_approachToCm);
            m_approachTraveledCm += v * dt;
            if (m_approachTraveledCm >= len - 1e-9) {
                m_approachTraveledCm = len;
                m_approachActive = false;
                m_ctrl->onRunApproachComplete();
                m_runTimer.stop();
            }
            update();
        }
        return;
    }

    if (m_approachActive) {
        const double len = dist(m_approachFromCm, m_approachToCm);
        m_approachTraveledCm += v * dt;
        if (m_approachTraveledCm >= len - 1e-9) {
            m_approachTraveledCm = len;
            m_approachActive = false;
            m_ctrl->onRunApproachComplete();
            if (m_ctrl->grbl()->connected())
                m_runTimer.stop();
            else
                m_runElapsed.restart();
        }
        update();
        return;
    }

    if (m_runEndDistanceCm <= m_runStartDistanceCm + 1e-9) {
        m_ctrl->finishPageRun();
        return;
    }

    m_runDistance += v * dt;
    if (m_runDistance >= m_runEndDistanceCm - 1e-9) {
        m_runDistance = m_runEndDistanceCm;
        m_ctrl->finishPageRun();
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
    setDragDocIndex(-1);
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
        setDragDocIndex(-1);
        update();
        event->accept();
        return;
    }
    if (hit != m_selectedDocIndex) {
        event->accept();
        return;
    }
    setDragDocIndex(hit);
    m_ctrl->pushUndoSnapshot();
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
    const int committed = m_dragDocIndex;
    m_currentDragCm = cmFromPixel(event->position());
    const QPointF newBl = m_dragGlyphStartCm + (m_currentDragCm - m_pressCm);
    m_ctrl->setManualAnchor(committed, newBl);
    setDragDocIndex(-1);
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

    if ((m_ctrl->runActive() || m_ctrl->runArmed()) && m_runStaticValid && !m_runStaticPixmap.isNull()) {
        painter->fillRect(boundingRect(), QColor("#f4f4f5"));
        painter->drawPixmap(0, 0, m_runStaticPixmap);

        if (useLiveCncTip())
            syncRedTrailFromLivePosition();
        else if (m_runDistance > m_runLastPaintedDist + 1e-9 && !m_runRedPixmap.isNull()) {
            QPainter rp(&m_runRedPixmap);
            rp.setRenderHint(QPainter::Antialiasing, true);
            drawRunProgressAlongPath(&rp, m_runLastPaintedDist, m_runDistance, s);
            rp.end();
            m_runLastPaintedDist = m_runDistance;
        }
        painter->drawPixmap(0, 0, m_runRedPixmap);

        const QPointF tipCm = currentTipPositionCm();
        if (!tipCm.isNull()) {
            if (m_approachActive)
                drawApproachTravel(painter, m_approachFromCm, tipCm, s);
            drawPurpleTip(painter, tipCm, s);
        }
        return;
    }

    painter->fillRect(boundingRect(), QColor("#f4f4f5"));

    if (m_layoutDirty) {
        rebuildLayout();
        m_layoutDirty = false;
    }

    paintStaticContent(painter, st, s);

    const QPointF liveTip = currentTipPositionCm();
    if (!liveTip.isNull()) {
        drawPurpleTip(painter, liveTip, s);
    }

    if (!m_ctrl->runActive()) return;
    if (m_runSegCumStartCm.isEmpty()) return;

    const double drawDist = m_runDistance;
    if (drawDist <= 0) return;

    painter->setPen(QPen(QColor("#dc2626"), 1.2));
    painter->setBrush(QColor("#dc2626"));
    drawRunProgressAlongPath(painter, 0, drawDist, s);
}
