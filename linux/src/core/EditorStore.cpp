#include "EditorStore.h"

#include <QFileInfo>
#include <QRandomGenerator>

namespace {
constexpr int kDefaultStrokePx = 6;
constexpr int kDefaultCaptureGapUm = 350;
constexpr int kMinRectSide = 4;
}

EditorStore::EditorStore(QObject *parent)
    : QObject(parent),
      m_toolMode(ToolMode::Draw),
      m_strokePx(kDefaultStrokePx),
      m_captureGapUm(kDefaultCaptureGapUm),
      m_zoom(100),
      m_hasSelectionRect(false),
      m_hasSelectionDraftRect(false),
      m_hasResizeState(false),
      m_isDirty(false) {}

int EditorStore::strokePx() const { return m_strokePx; }
int EditorStore::captureGapUm() const { return m_captureGapUm; }
int EditorStore::zoom() const { return m_zoom; }
QString EditorStore::toolMode() const { return m_toolMode == ToolMode::Draw ? "draw" : "select"; }
bool EditorStore::hasSelection() const { return m_hasSelectionRect || m_hasSelectionDraftRect; }
bool EditorStore::isDirty() const { return m_isDirty; }
QString EditorStore::openFilePath() const { return m_openFilePath; }
QString EditorStore::openFileName() const { return m_openFileName; }
const QVector<Stroke> &EditorStore::strokes() const { return m_strokes; }
QString EditorStore::currentStrokeId() const { return m_currentStrokeId; }
ToolMode EditorStore::toolModeValue() const { return m_toolMode; }
const SelectionRect *EditorStore::selectionRect() const { return m_hasSelectionRect ? &m_selectionRect : nullptr; }
const SelectionRect *EditorStore::selectionDraftRect() const { return m_hasSelectionDraftRect ? &m_selectionDraftRect : nullptr; }
const ResizeDragState *EditorStore::selectionResizeState() const { return m_hasResizeState ? &m_resizeState : nullptr; }

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
    const ToolMode next = mode == "select" ? ToolMode::Select : ToolMode::Draw;
    if (next == m_toolMode) return;
    m_toolMode = next;
    emit toolModeChanged();
}

void EditorStore::toggleToolMode() {
    setToolMode(m_toolMode == ToolMode::Draw ? "select" : "draw");
}

void EditorStore::startStroke(const QPointF &point) {
    Stroke stroke;
    stroke.id = makeStrokeId();
    stroke.points.push_back(point);
    m_currentStrokeId = stroke.id;
    m_strokes.push_back(stroke);
    markDirty();
    emit strokesChanged();
}

void EditorStore::replaceActiveStrokePoints(const QVector<QPointF> &points) {
    if (m_currentStrokeId.isEmpty() || points.isEmpty()) return;
    const int index = findStrokeIndexById(m_currentStrokeId);
    if (index < 0) return;
    m_strokes[index].points = points;
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

void EditorStore::commitSelectionDraftRect() {
    if (!m_hasSelectionDraftRect) return;
    bool ok = false;
    const SelectionRect normalized = normalizeRect(m_selectionDraftRect, &ok);
    m_hasSelectionDraftRect = false;
    m_hasSelectionRect = ok;
    if (ok) m_selectionRect = normalized;
    emit selectionChanged();
}

void EditorStore::clearSelection() {
    if (!m_hasSelectionRect && !m_hasSelectionDraftRect && !m_hasResizeState) return;
    m_hasSelectionRect = false;
    m_hasSelectionDraftRect = false;
    m_hasResizeState = false;
    emit selectionChanged();
}

void EditorStore::setSelectionRect(const SelectionRect *rect) {
    if (!rect) {
        if (!m_hasSelectionRect) return;
        m_hasSelectionRect = false;
        emit selectionChanged();
        return;
    }
    bool ok = false;
    const SelectionRect normalized = normalizeRect(*rect, &ok);
    m_hasSelectionRect = ok;
    if (ok) m_selectionRect = normalized;
    emit selectionChanged();
}

void EditorStore::setSelectionResizeState(const ResizeDragState *state) {
    m_hasResizeState = state != nullptr;
    if (state) m_resizeState = *state;
}

void EditorStore::setOpenFile(const QString &path) {
    m_openFilePath = path;
    m_openFileName = path.isEmpty() ? QString() : QFileInfo(path).fileName();
    emit openFilePathChanged();
    emit openFileNameChanged();
}

void EditorStore::closeFile() {
    if (m_openFilePath.isEmpty()) return;
    m_openFilePath.clear();
    m_openFileName.clear();
    emit openFilePathChanged();
    emit openFileNameChanged();
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

int EditorStore::findStrokeIndexById(const QString &id) const {
    for (int i = 0; i < m_strokes.size(); ++i) {
        if (m_strokes[i].id == id) return i;
    }
    return -1;
}
