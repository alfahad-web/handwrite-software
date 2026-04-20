#include "EditorStore.h"

#include <QFileInfo>
#include <QHash>
#include <QLineF>
#include <QRandomGenerator>
#include <QRectF>

namespace {
constexpr int kDefaultStrokePx = 6;
constexpr int kDefaultCaptureGapUm = 350;
constexpr int kDefaultEraseRadiusPx = 20;
constexpr int kMinRectSide = 4;
constexpr char kStemAlphabet[] = "abcdefghijklmnopqrstuvwxyz0123456789";

bool rectsIntersect(const SelectionRect &a, const SelectionRect &b) {
    const QRectF ra(a.x, a.y, a.width, a.height);
    const QRectF rb(b.x, b.y, b.width, b.height);
    return ra.normalized().intersects(rb.normalized());
}

bool pointInsideRect(const QPointF &p, const SelectionRect &r) {
    return p.x() >= r.x && p.x() <= (r.x + r.width) && p.y() >= r.y && p.y() <= (r.y + r.height);
}

QPointF anchorBoardForBox(const SelectionBox &self, const QVector<SelectionBox> &allCommitted) {
    const SelectionBox *bestPred = nullptr;
    int bestOrder = -1;
    for (const SelectionBox &b : allCommitted) {
        if (b.id == self.id) continue;
        if (b.orderIndex >= self.orderIndex) continue;
        if (!rectsIntersect(b.rect, self.rect)) continue;
        if (b.orderIndex > bestOrder) {
            bestOrder = b.orderIndex;
            bestPred = &b;
        }
    }
    const QPointF fallback(self.rect.x, self.rect.y + self.rect.height);
    if (!bestPred) return fallback;

    const QPointF predBottomRight(bestPred->rect.x + bestPred->rect.width, bestPred->rect.y + bestPred->rect.height);
    if (!pointInsideRect(predBottomRight, self.rect)) return fallback;
    return predBottomRight;
}
}

EditorStore::EditorStore(QObject *parent)
    : QObject(parent),
      m_toolMode(ToolMode::Draw),
      m_strokePx(kDefaultStrokePx),
      m_captureGapUm(kDefaultCaptureGapUm),
      m_zoom(100),
      m_eraseRadiusPx(kDefaultEraseRadiusPx),
      m_hasSelectionDraftRect(false),
      m_hasResizeState(false),
      m_nextSelectionOrder(1),
      m_isDirty(false) {}

int EditorStore::strokePx() const { return m_strokePx; }
int EditorStore::captureGapUm() const { return m_captureGapUm; }
int EditorStore::zoom() const { return m_zoom; }
QString EditorStore::toolMode() const {
    if (m_toolMode == ToolMode::Select) return "select";
    if (m_toolMode == ToolMode::Erase) return "erase";
    return "draw";
}
bool EditorStore::hasSelection() const { return !m_selectionBoxes.isEmpty() || m_hasSelectionDraftRect; }
bool EditorStore::hasSelectedSelection() const { return !m_selectedSelectionId.isEmpty(); }
int EditorStore::eraseRadiusPx() const { return m_eraseRadiusPx; }
bool EditorStore::drawStrokeEraseActive() const { return m_drawStrokeEraseActive; }
bool EditorStore::isDirty() const { return m_isDirty; }
QString EditorStore::projectFilePath() const { return m_projectFilePath; }
QString EditorStore::projectFileName() const { return m_projectFileName; }
QString EditorStore::selectedSelectionId() const { return m_selectedSelectionId; }
const QVector<Stroke> &EditorStore::strokes() const { return m_strokes; }
QString EditorStore::currentStrokeId() const { return m_currentStrokeId; }
ToolMode EditorStore::toolModeValue() const { return m_toolMode; }
const QVector<SelectionBox> &EditorStore::selectionBoxes() const { return m_selectionBoxes; }
const SelectionRect *EditorStore::selectionDraftRect() const { return m_hasSelectionDraftRect ? &m_selectionDraftRect : nullptr; }
const ResizeDragState *EditorStore::selectionResizeState() const { return m_hasResizeState ? &m_resizeState : nullptr; }

const SelectionBox *EditorStore::selectedSelection() const {
    if (m_selectedSelectionId.isEmpty()) return nullptr;
    return selectionById(m_selectedSelectionId);
}

const SelectionBox *EditorStore::selectionById(const QString &id) const {
    const int idx = findSelectionIndexById(id);
    if (idx < 0) return nullptr;
    return &m_selectionBoxes[idx];
}

SelectionBox *EditorStore::selectionByIdMutable(const QString &id) {
    const int idx = findSelectionIndexById(id);
    if (idx < 0) return nullptr;
    return &m_selectionBoxes[idx];
}

void EditorStore::setStrokePx(int value) {
    const int next = clampInt(value, 1, 200);
    if (next == m_strokePx) return;
    m_strokePx = next;
    emit strokePxChanged();
    emit strokesChanged();
}

void EditorStore::setCaptureGapUm(int value) {
    const int next = clampInt(value, 1, 200000);
    if (next == m_captureGapUm) return;
    m_captureGapUm = next;
    emit captureGapUmChanged();
    emit strokesChanged();
}

void EditorStore::setZoom(int value) {
    const int next = clampInt(value, 10, 800);
    if (next == m_zoom) return;
    m_zoom = next;
    emit zoomChanged();
}

void EditorStore::zoomIn() { setZoom(m_zoom + 10); }
void EditorStore::zoomOut() { setZoom(m_zoom - 10); }

void EditorStore::setToolMode(const QString &mode) {
    ToolMode next = ToolMode::Draw;
    if (mode == "select") next = ToolMode::Select;
    if (mode == "erase") next = ToolMode::Erase;
    if (next == m_toolMode) {
        if (next == ToolMode::Draw && m_drawStrokeEraseActive) {
            m_drawStrokeEraseActive = false;
            emit drawStrokeEraseActiveChanged();
        }
        return;
    }
    if (m_drawStrokeEraseActive) {
        m_drawStrokeEraseActive = false;
        emit drawStrokeEraseActiveChanged();
    }
    m_toolMode = next;
    emit toolModeChanged();
}

void EditorStore::toggleToolMode() {
    setToolMode(m_toolMode == ToolMode::Draw ? "select" : "draw");
}

void EditorStore::setEraseRadiusPx(int value) {
    const int next = clampInt(value, 1, 500);
    if (next == m_eraseRadiusPx) return;
    m_eraseRadiusPx = next;
    emit eraseRadiusPxChanged();
}

void EditorStore::setDrawStrokeEraseActive(bool active) {
    if (active && m_toolMode != ToolMode::Draw)
        return;
    if (m_drawStrokeEraseActive == active) return;
    m_drawStrokeEraseActive = active;
    emit drawStrokeEraseActiveChanged();
}

bool EditorStore::deleteSelectedSelection() {
    if (m_selectedSelectionId.isEmpty()) return false;
    const int idx = findSelectionIndexById(m_selectedSelectionId);
    if (idx < 0) return false;
    m_selectionBoxes.removeAt(idx);
    m_selectedSelectionId.clear();
    m_hasResizeState = false;
    recomputeSelectionAnchors();
    markDirty();
    return true;
}

void EditorStore::startStroke(const QPointF &point) {
    Stroke stroke;
    stroke.id = makeStrokeId();
    stroke.points.push_back(Stroke::StrokePoint{point, false});
    m_currentStrokeId = stroke.id;
    m_strokes.push_back(stroke);
    markDirty();
    emit strokesChanged();
}

void EditorStore::replaceActiveStrokePoints(const QVector<QPointF> &points) {
    if (m_currentStrokeId.isEmpty() || points.isEmpty()) return;
    const int index = findStrokeIndexById(m_currentStrokeId);
    if (index < 0) return;
    QVector<Stroke::StrokePoint> normalized;
    normalized.reserve(points.size());
    for (const QPointF &p : points) normalized.push_back(Stroke::StrokePoint{p, false});
    m_strokes[index].points = normalized;
    markDirty();
    emit strokesChanged();
}

void EditorStore::endStroke() { m_currentStrokeId.clear(); }

void EditorStore::setSelectionDraftRect(const SelectionRect *rect) {
    if (!rect) {
        if (!m_hasSelectionDraftRect) return;
        m_hasSelectionDraftRect = false;
        emit selectionChanged();
        return;
    }
    m_selectionDraftRect = *rect;
    m_hasSelectionDraftRect = true;
    emit selectionChanged();
}

QString EditorStore::commitSelectionDraftRect() {
    if (!m_hasSelectionDraftRect) return QString();
    bool ok = false;
    const SelectionRect normalized = normalizeRect(m_selectionDraftRect, &ok);
    m_hasSelectionDraftRect = false;
    if (!ok) {
        emit selectionChanged();
        return QString();
    }
    SelectionBox box;
    box.id = makeSelectionId();
    box.orderIndex = m_nextSelectionOrder++;
    box.rect = normalized;
    m_selectionBoxes.push_back(box);
    m_selectedSelectionId = box.id;
    recomputeSelectionAnchors();
    markDirty();
    return box.id;
}

void EditorStore::clearSelectionDraft() {
    if (!m_hasSelectionDraftRect) return;
    m_hasSelectionDraftRect = false;
    emit selectionChanged();
}

void EditorStore::setSelectionRect(const QString &selectionId, const SelectionRect *rect) {
    const int idx = findSelectionIndexById(selectionId);
    if (idx < 0) return;
    if (!rect) return;
    bool ok = false;
    const SelectionRect normalized = normalizeRect(*rect, &ok);
    if (!ok) return;
    m_selectionBoxes[idx].rect = normalized;
    recomputeSelectionAnchors();
    markDirty();
}

void EditorStore::setSelectedSelectionId(const QString &selectionId) {
    if (selectionId == m_selectedSelectionId) return;
    if (!selectionId.isEmpty() && findSelectionIndexById(selectionId) < 0) return;
    m_selectedSelectionId = selectionId;
    emit selectionChanged();
}

void EditorStore::setSelectionResizeState(const ResizeDragState *state) {
    m_hasResizeState = state != nullptr;
    if (state) m_resizeState = *state;
}

bool EditorStore::erasePointsInSelectedSelection(const QPointF &center, qreal radiusPx) {
    const SelectionBox *box = selectedSelection();
    if (!box) return false;
    const qreal rsq = radiusPx * radiusPx;
    const qreal minX = box->rect.x;
    const qreal minY = box->rect.y;
    const qreal maxX = box->rect.x + box->rect.width;
    const qreal maxY = box->rect.y + box->rect.height;
    bool changed = false;
    for (Stroke &stroke : m_strokes) {
        for (Stroke::StrokePoint &pt : stroke.points) {
            if (pt.erased) continue;
            if (pt.pos.x() < minX || pt.pos.x() > maxX || pt.pos.y() < minY || pt.pos.y() > maxY) continue;
            const qreal dx = pt.pos.x() - center.x();
            const qreal dy = pt.pos.y() - center.y();
            if ((dx * dx + dy * dy) <= rsq) {
                pt.erased = true;
                changed = true;
            }
        }
    }
    if (changed) {
        markDirty();
        emit strokesChanged();
    }
    return changed;
}

bool EditorStore::removeStrokePointsNear(const QPointF &center, qreal radiusPx) {
    const qreal rsq = radiusPx * radiusPx;
    const qreal splitDistThresh = qMax(radiusPx * 3.0, static_cast<qreal>(m_strokePx) * 5.0);
    QVector<Stroke> next;
    next.reserve(m_strokes.size() + 4);
    bool changed = false;
    const QString priorCurrentId = m_currentStrokeId;

    for (const Stroke &stroke : m_strokes) {
        QVector<Stroke::StrokePoint> kept;
        kept.reserve(stroke.points.size());
        for (const Stroke::StrokePoint &pt : stroke.points) {
            const qreal dx = pt.pos.x() - center.x();
            const qreal dy = pt.pos.y() - center.y();
            if (dx * dx + dy * dy <= rsq) {
                changed = true;
                continue;
            }
            kept.push_back(pt);
        }
        if (kept.isEmpty()) continue;

        QVector<QVector<Stroke::StrokePoint>> runs;
        QVector<Stroke::StrokePoint> run;
        for (const Stroke::StrokePoint &pt : kept) {
            if (run.isEmpty()) {
                run.push_back(pt);
                continue;
            }
            if (QLineF(run.last().pos, pt.pos).length() > splitDistThresh) {
                runs.push_back(run);
                run = {pt};
            } else {
                run.push_back(pt);
            }
        }
        if (!run.isEmpty()) runs.push_back(run);

        for (const QVector<Stroke::StrokePoint> &r : runs) {
            if (r.isEmpty()) continue;
            Stroke s;
            s.id = makeStrokeId();
            s.createdAt = stroke.createdAt;
            s.points = r;
            next.push_back(std::move(s));
        }
    }

    if (!changed) return false;

    m_strokes = std::move(next);
    if (!priorCurrentId.isEmpty() && findStrokeIndexById(priorCurrentId) < 0)
        m_currentStrokeId.clear();
    markDirty();
    emit strokesChanged();
    return true;
}

void EditorStore::setProjectFilePath(const QString &path) {
    m_projectFilePath = path;
    m_projectFileName = path.isEmpty() ? QString() : QFileInfo(path).fileName();
    emit projectFilePathChanged();
    emit projectFileNameChanged();
}

void EditorStore::clearProjectFilePath() {
    if (m_projectFilePath.isEmpty() && m_projectFileName.isEmpty()) return;
    m_projectFilePath.clear();
    m_projectFileName.clear();
    emit projectFilePathChanged();
    emit projectFileNameChanged();
}

void EditorStore::clearAll() {
    m_strokes.clear();
    m_currentStrokeId.clear();
    m_selectionBoxes.clear();
    m_selectedSelectionId.clear();
    m_hasSelectionDraftRect = false;
    m_hasResizeState = false;
    m_nextSelectionOrder = 1;
    m_specialCharStemMap.clear();
    m_isDirty = false;
    if (m_drawStrokeEraseActive) {
        m_drawStrokeEraseActive = false;
        emit drawStrokeEraseActiveChanged();
    }
    emit strokesChanged();
    emit selectionChanged();
    emit isDirtyChanged();
}

QString EditorStore::fileStemForAscii(int asciiCode) {
    if ((asciiCode >= 'a' && asciiCode <= 'z') || (asciiCode >= 'A' && asciiCode <= 'Z')) {
        return QString(QChar(static_cast<char>(asciiCode)));
    }
    if (m_specialCharStemMap.contains(asciiCode)) return m_specialCharStemMap.value(asciiCode);
    QString stem = makeSpecialStem();
    QSet<QString> used;
    for (auto it = m_specialCharStemMap.cbegin(); it != m_specialCharStemMap.cend(); ++it) used.insert(it.value());
    while (used.contains(stem)) stem = makeSpecialStem();
    m_specialCharStemMap.insert(asciiCode, stem);
    markDirty();
    return stem;
}

QVariantList EditorStore::selectionBoxesModel() const {
    QVariantList out;
    for (const SelectionBox &box : m_selectionBoxes) {
        QVariantMap item;
        item.insert("id", box.id);
        item.insert("x", box.rect.x);
        item.insert("y", box.rect.y);
        item.insert("width", box.rect.width);
        item.insert("height", box.rect.height);
        item.insert("orderIndex", box.orderIndex);
        item.insert("assigned", box.assigned);
        item.insert("assignedAscii", box.assignedAscii);
        item.insert("fileStem", box.fileStem);
        item.insert("joinMode", joinModeToString(box.joinMode));
        item.insert("anchorX", box.anchorX);
        item.insert("anchorY", box.anchorY);
        item.insert("selected", box.id == m_selectedSelectionId);
        out.push_back(item);
    }
    return out;
}

const QHash<int, QString> &EditorStore::specialCharStemMap() const { return m_specialCharStemMap; }

void EditorStore::setSpecialCharStemMap(const QHash<int, QString> &map) { m_specialCharStemMap = map; }

void EditorStore::setStrokes(const QVector<Stroke> &strokes) {
    m_strokes = strokes;
    m_currentStrokeId.clear();
    emit strokesChanged();
}

void EditorStore::setSelectionBoxes(const QVector<SelectionBox> &boxes, const QString &selectedId) {
    m_selectionBoxes = boxes;
    int maxOrder = 0;
    for (const SelectionBox &box : m_selectionBoxes) {
        if (box.orderIndex > maxOrder) maxOrder = box.orderIndex;
    }
    m_nextSelectionOrder = maxOrder + 1;
    m_selectedSelectionId = selectedId;
    if (!m_selectedSelectionId.isEmpty() && findSelectionIndexById(m_selectedSelectionId) < 0) {
        m_selectedSelectionId.clear();
    }
    recomputeSelectionAnchors();
}

void EditorStore::recomputeSelectionAnchors() {
    for (SelectionBox &box : m_selectionBoxes) {
        const QPointF a = anchorBoardForBox(box, m_selectionBoxes);
        box.anchorX = a.x();
        box.anchorY = a.y();
    }
    emit selectionChanged();
}

QPointF EditorStore::draftAnchorBoard() const {
    if (!m_hasSelectionDraftRect) return QPointF();
    SelectionBox self;
    self.id = QStringLiteral("_draft");
    self.orderIndex = m_nextSelectionOrder;
    self.rect = m_selectionDraftRect;
    self.joinMode = JoinMode::N;
    return anchorBoardForBox(self, m_selectionBoxes);
}

void EditorStore::markSaved() {
    if (!m_isDirty) return;
    m_isDirty = false;
    emit isDirtyChanged();
}

void EditorStore::markDirty() {
    if (m_isDirty) return;
    m_isDirty = true;
    emit isDirtyChanged();
}

SelectionRect EditorStore::normalizeRect(const SelectionRect &rect, bool *ok) {
    SelectionRect out;
    out.x = rect.width < 0 ? rect.x + rect.width : rect.x;
    out.y = rect.height < 0 ? rect.y + rect.height : rect.y;
    out.width = qAbs(rect.width);
    out.height = qAbs(rect.height);
    const bool valid = out.width >= kMinRectSide && out.height >= kMinRectSide;
    if (ok) *ok = valid;
    return out;
}

int EditorStore::clampInt(int value, int min, int max) { return qMin(max, qMax(min, value)); }

QString EditorStore::makeStrokeId() {
    const quint64 r = QRandomGenerator::global()->generate64();
    return QString("stroke-%1").arg(r, 0, 16);
}

QString EditorStore::makeSelectionId() {
    const quint64 r = QRandomGenerator::global()->generate64();
    return QString("sel-%1").arg(r, 0, 16);
}

QString EditorStore::makeSpecialStem() {
    QString out;
    out.reserve(4);
    constexpr int alphabetSize = static_cast<int>(sizeof(kStemAlphabet) - 1);
    for (int i = 0; i < 4; ++i) {
        const int idx = QRandomGenerator::global()->bounded(alphabetSize);
        out.push_back(QChar(kStemAlphabet[idx]));
    }
    return out;
}

int EditorStore::findStrokeIndexById(const QString &id) const {
    for (int i = 0; i < m_strokes.size(); ++i) {
        if (m_strokes[i].id == id) return i;
    }
    return -1;
}

int EditorStore::findSelectionIndexById(const QString &id) const {
    for (int i = 0; i < m_selectionBoxes.size(); ++i) {
        if (m_selectionBoxes[i].id == id) return i;
    }
    return -1;
}
