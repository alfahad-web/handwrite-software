import { create } from "zustand";
import type { PointPx, SelectionRect, Stroke, ToolMode } from "./types.ts";

interface ResizeDragState {
  handle:
    | "move"
    | "n"
    | "s"
    | "e"
    | "w"
    | "nw"
    | "ne"
    | "sw"
    | "se";
  startRect: SelectionRect;
  startPoint: PointPx;
}

export interface EditorStore {
  strokes: Stroke[];
  currentStrokeId: string | null;
  toolMode: ToolMode;
  strokePx: number;
  captureGapUm: number;
  zoom: number;
  selectionRect: SelectionRect | null;
  selectionDraftRect: SelectionRect | null;
  selectionResizeState: ResizeDragState | null;
  openFilePath: string | null;
  openFileName: string | null;
  isDirty: boolean;

  setToolMode: (mode: ToolMode) => void;
  setStrokePx: (value: number) => void;
  setCaptureGapUm: (value: number) => void;
  setZoom: (value: number) => void;
  zoomIn: () => void;
  zoomOut: () => void;
  startStroke: (point: PointPx) => void;
  appendStrokePoint: (point: PointPx, options?: { force?: boolean }) => void;
  /** Replaces the active stroke's points in one update (used for rAF-batched drawing). */
  replaceActiveStrokePoints: (points: PointPx[]) => void;
  endStroke: () => void;
  setSelectionDraftRect: (rect: SelectionRect | null) => void;
  commitSelectionDraftRect: () => void;
  clearSelection: () => void;
  setSelectionRect: (rect: SelectionRect | null) => void;
  setSelectionResizeState: (state: ResizeDragState | null) => void;
  setOpenFile: (path: string | null) => void;
  markSaved: () => void;
  markDirty: () => void;
  closeFile: () => void;
}

const DEFAULT_STROKE_PX = 6;
const DEFAULT_CAPTURE_GAP_UM = 350;
const MIN_RECT_SIDE = 4;
/** Minimum board-space movement before recording another point (reduces React/SVG work per drag). */
export const MIN_STROKE_SAMPLE_PX = 0.45;

function clamp(n: number, min: number, max: number): number {
  return Math.min(max, Math.max(min, n));
}

function toBaseRect(rect: SelectionRect): SelectionRect {
  const x = rect.width < 0 ? rect.x + rect.width : rect.x;
  const y = rect.height < 0 ? rect.y + rect.height : rect.y;
  const width = Math.abs(rect.width);
  const height = Math.abs(rect.height);
  return { x, y, width, height };
}

function normalizeRect(rect: SelectionRect): SelectionRect | null {
  const normalized = toBaseRect(rect);
  if (normalized.width < MIN_RECT_SIDE || normalized.height < MIN_RECT_SIDE) {
    return null;
  }
  return normalized;
}

export const useEditorStore = create<EditorStore>((set, get) => ({
  strokes: [],
  currentStrokeId: null,
  toolMode: "draw",
  strokePx: DEFAULT_STROKE_PX,
  captureGapUm: DEFAULT_CAPTURE_GAP_UM,
  zoom: 100,
  selectionRect: null,
  selectionDraftRect: null,
  selectionResizeState: null,
  openFilePath: null,
  openFileName: null,
  isDirty: false,

  setToolMode: (mode) => set({ toolMode: mode }),
  setStrokePx: (value) =>
    set({
      strokePx: clamp(Math.floor(Number.isFinite(value) ? value : DEFAULT_STROKE_PX), 1, 200),
    }),
  setCaptureGapUm: (value) =>
    set({
      captureGapUm: clamp(
        Math.floor(Number.isFinite(value) ? value : DEFAULT_CAPTURE_GAP_UM),
        1,
        200000
      ),
    }),
  setZoom: (value) => set({ zoom: clamp(Number.isFinite(value) ? value : 100, 10, 800) }),
  zoomIn: () => set((state) => ({ zoom: clamp(state.zoom + 10, 10, 800) })),
  zoomOut: () => set((state) => ({ zoom: clamp(state.zoom - 10, 10, 800) })),

  startStroke: (point) => {
    const id =
      typeof crypto !== "undefined" && "randomUUID" in crypto
        ? crypto.randomUUID()
        : `stroke-${Date.now()}-${Math.random().toString(16).slice(2)}`;
    const stroke: Stroke = { id, points: [point], createdAt: Date.now() };
    set((state) => ({
      strokes: [...state.strokes, stroke],
      currentStrokeId: id,
      isDirty: true,
    }));
  },

  appendStrokePoint: (point, options) => {
    const { currentStrokeId, strokes } = get();
    if (!currentStrokeId) return;
    const active = strokes.find((s) => s.id === currentStrokeId);
    if (!active) return;
    if (active.points.length > 0) {
      const last = active.points[active.points.length - 1];
      const dist = Math.hypot(point.x - last.x, point.y - last.y);
      if (options?.force) {
        if (dist < 0.02) return;
      } else if (dist < MIN_STROKE_SAMPLE_PX) return;
    }
    set((state) => ({
      strokes: state.strokes.map((stroke) =>
        stroke.id === currentStrokeId
          ? { ...stroke, points: [...stroke.points, point] }
          : stroke
      ),
      isDirty: true,
    }));
  },

  replaceActiveStrokePoints: (points) => {
    const { currentStrokeId } = get();
    if (!currentStrokeId || points.length === 0) return;
    set((state) => ({
      strokes: state.strokes.map((stroke) =>
        stroke.id === currentStrokeId ? { ...stroke, points: [...points] } : stroke
      ),
      isDirty: true,
    }));
  },

  endStroke: () => set({ currentStrokeId: null }),

  setSelectionDraftRect: (rect) => set({ selectionDraftRect: rect }),

  commitSelectionDraftRect: () => {
    const draft = get().selectionDraftRect;
    const next = draft ? normalizeRect(draft) : null;
    set({ selectionRect: next, selectionDraftRect: null });
  },

  clearSelection: () => set({ selectionRect: null, selectionDraftRect: null, selectionResizeState: null }),

  setSelectionRect: (rect) => set({ selectionRect: rect ? normalizeRect(rect) : null }),
  setSelectionResizeState: (state) => set({ selectionResizeState: state }),

  setOpenFile: (path) =>
    set({
      openFilePath: path,
      openFileName: path ? path.split(/[/\\]/).pop() ?? path : null,
    }),

  markSaved: () => set({ isDirty: false }),
  markDirty: () => set({ isDirty: true }),

  closeFile: () => set({ openFilePath: null, openFileName: null }),
}));
