/**
 * Interaction state machine from fonter/cachy/src/canvas/CanvasItem.cpp
 * (paint remains in React/canvas component).
 */

import type { EditorStore } from "./editorStore";
import {
  ResizeHandle,
  type Point,
  type SelectionRect,
  ToolMode,
} from "./editorTypes";

const K_BASE_BOARD_WIDTH = 3000;
const K_BASE_BOARD_HEIGHT = 2000;
const K_VIEWPORT_MULTIPLIER = 3;
const K_FALLBACK_VIEWPORT_WIDTH = 1280;
const K_FALLBACK_VIEWPORT_HEIGHT = 720;

const viewportWidth =
  typeof window !== "undefined"
    ? window.innerWidth
    : K_FALLBACK_VIEWPORT_WIDTH;
const viewportHeight =
  typeof window !== "undefined"
    ? window.innerHeight
    : K_FALLBACK_VIEWPORT_HEIGHT;

export const K_BOARD_WIDTH = Math.max(
  K_BASE_BOARD_WIDTH,
  Math.ceil(viewportWidth * K_VIEWPORT_MULTIPLIER),
);
export const K_BOARD_HEIGHT = Math.max(
  K_BASE_BOARD_HEIGHT,
  Math.ceil(viewportHeight * K_VIEWPORT_MULTIPLIER),
);
export const K_MIN_STROKE_SAMPLE_PX = 0.45;
export const K_ANCHOR_RADIUS_BOARD = 3;

export function toBoard(
  itemX: number,
  itemY: number,
  zoomPercent: number,
  clamp: boolean,
): Point {
  const scale = zoomPercent / 100;
  const x = itemX / scale;
  const y = itemY / scale;
  const inside =
    x >= 0 &&
    x <= K_BOARD_WIDTH &&
    y >= 0 &&
    y <= K_BOARD_HEIGHT;
  if (!inside && !clamp) return { x: -1, y: -1 };
  return {
    x: Math.min(K_BOARD_WIDTH, Math.max(0, x)),
    y: Math.min(K_BOARD_HEIGHT, Math.max(0, y)),
  };
}

export function hitHandle(
  point: Point,
  rect: SelectionRect,
  zoomPercent: number,
): ResizeHandle {
  const scale = zoomPercent / 100;
  const hs = 10.0 / scale;
  const left = rect.x;
  const right = rect.x + rect.width;
  const top = rect.y;
  const bottom = rect.y + rect.height;
  const near = (v: number, t: number) => Math.abs(v - t) <= hs;
  const withinX = point.x >= left - hs && point.x <= right + hs;
  const withinY = point.y >= top - hs && point.y <= bottom + hs;
  if (near(point.x, left) && near(point.y, top)) return ResizeHandle.NW;
  if (near(point.x, right) && near(point.y, top)) return ResizeHandle.NE;
  if (near(point.x, left) && near(point.y, bottom)) return ResizeHandle.SW;
  if (near(point.x, right) && near(point.y, bottom)) return ResizeHandle.SE;
  if (near(point.x, left) && withinY) return ResizeHandle.W;
  if (near(point.x, right) && withinY) return ResizeHandle.E;
  if (near(point.y, top) && withinX) return ResizeHandle.N;
  if (near(point.y, bottom) && withinX) return ResizeHandle.S;
  if (
    point.x >= left + hs &&
    point.x <= right - hs &&
    point.y >= top + hs &&
    point.y <= bottom - hs
  ) {
    return ResizeHandle.Move;
  }
  return ResizeHandle.None;
}

export function hitSelectionId(point: Point, store: EditorStore): string {
  const boxes = store.selectionBoxes();
  for (let i = boxes.length - 1; i >= 0; i--) {
    const b = boxes[i]!;
    const r = b.rect;
    if (
      point.x >= r.x &&
      point.x <= r.x + r.width &&
      point.y >= r.y &&
      point.y <= r.y + r.height
    ) {
      return b.id;
    }
  }
  return "";
}

export function hitAnchorSelectionId(
  point: Point,
  store: EditorStore,
  zoomPercent: number,
): string {
  const scale = zoomPercent / 100;
  if (scale <= 0) return "";
  const hitRadiusBoard = (K_ANCHOR_RADIUS_BOARD * 2.4) / scale;
  const hitRadiusSq = hitRadiusBoard * hitRadiusBoard;
  let bestId = "";
  let bestDistSq = Number.POSITIVE_INFINITY;
  for (const box of store.selectionBoxes()) {
    const dx = point.x - box.anchorX;
    const dy = point.y - box.anchorY;
    const distSq = dx * dx + dy * dy;
    if (distSq > hitRadiusSq) continue;
    if (distSq < bestDistSq) {
      bestDistSq = distSq;
      bestId = box.id;
    }
  }
  return bestId;
}

export function hasSelectionAt(
  itemX: number,
  itemY: number,
  store: EditorStore,
): boolean {
  const p = toBoard(itemX, itemY, store.zoom(), false);
  if (p.x < 0 || p.y < 0) return false;
  return hitSelectionId(p, store).length > 0;
}

function lineLen(a: Point, b: Point): number {
  return Math.hypot(b.x - a.x, b.y - a.y);
}

function applyResize(store: EditorStore, current: Point): void {
  const state = store.selectionResizeState();
  if (!state) return;
  const base = state.startRect;
  const next: SelectionRect = { ...base };
  const dx = current.x - state.startPoint.x;
  const dy = current.y - state.startPoint.y;
  const h = state.handle;
  const hasW =
    h === ResizeHandle.W ||
    h === ResizeHandle.NW ||
    h === ResizeHandle.SW;
  const hasE =
    h === ResizeHandle.E ||
    h === ResizeHandle.NE ||
    h === ResizeHandle.SE;
  const hasN =
    h === ResizeHandle.N ||
    h === ResizeHandle.NE ||
    h === ResizeHandle.NW;
  const hasS =
    h === ResizeHandle.S ||
    h === ResizeHandle.SE ||
    h === ResizeHandle.SW;
  if (h === ResizeHandle.Move) {
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
  store.setSelectionRect(state.selectionId, next);
}

/** Mirrors CanvasItem interaction fields and pointer* methods */
export class BoardPointerController {
  private readonly store: EditorStore;
  private mIsDrawing = false;
  private mIsSelecting = false;
  private mIsResizing = false;
  private mIsErasing = false;
  private mIsStrokeHardErasing = false;
  private mIsAnchorDragging = false;
  private mSelectStart: Point = { x: 0, y: 0 };
  private mLivePoints: Point[] = [];
  private mEraseTracePoints: Point[] = [];
  private mAnchorDragSelectionId = "";
  private mHoverAnchorSelectionId = "";

  constructor(store: EditorStore) {
    this.store = store;
  }

  get livePoints(): Point[] {
    return this.mLivePoints;
  }

  hoverAnchorSelectionId(): string {
    return this.mHoverAnchorSelectionId;
  }

  anchorDragSelectionId(): string {
    return this.mAnchorDragSelectionId;
  }

  isAnchorDragging(): boolean {
    return this.mIsAnchorDragging;
  }

  /** Hover highlight only; interaction end uses pointerUp. */
  clearHoverAnchor(): void {
    this.mHoverAnchorSelectionId = "";
  }

  pointerDown(x: number, y: number, button: number): void {
    if (button !== 0) return;
    const store = this.store;
    const p = toBoard(x, y, store.zoom(), false);
    if (p.x < 0 || p.y < 0) return;

    if (store.toolModeValue() === ToolMode.Draw) {
      if (store.drawStrokeEraseActive()) {
        this.mIsStrokeHardErasing = true;
        this.mEraseTracePoints = [p];
        return;
      }
      this.mIsDrawing = true;
      this.mLivePoints = [p];
      store.startStroke(p);
    } else if (store.toolModeValue() === ToolMode.Select) {
      const anchorSelectionId = hitAnchorSelectionId(p, store, store.zoom());
      if (anchorSelectionId) {
        store.setSelectedSelectionId(anchorSelectionId);
        this.mIsAnchorDragging = true;
        this.mAnchorDragSelectionId = anchorSelectionId;
        this.mHoverAnchorSelectionId = anchorSelectionId;
        store.setSelectionAnchorPoint(anchorSelectionId, p);
        return;
      }
      const hitId = hitSelectionId(p, store);
      if (hitId) store.setSelectedSelectionId(hitId);
      const sel = store.selectedSelection();
      if (sel) {
        const handle = hitHandle(p, sel.rect, store.zoom());
        if (handle !== ResizeHandle.None) {
          store.setSelectionResizeState({
            handle,
            selectionId: sel.id,
            startRect: { ...sel.rect },
            startPoint: p,
          });
          this.mIsResizing = true;
          return;
        }
      }
      this.mIsSelecting = true;
      this.mSelectStart = p;
      store.setSelectionDraftRect({ x: p.x, y: p.y, width: 0, height: 0 });
    } else {
      this.mIsErasing = true;
      this.mEraseTracePoints = [p];
    }
  }

  pointerMove(x: number, y: number): void {
    const store = this.store;
    const p = toBoard(
      x,
      y,
      store.zoom(),
      this.mIsDrawing ||
        this.mIsSelecting ||
        this.mIsResizing ||
        this.mIsStrokeHardErasing ||
        this.mIsAnchorDragging ||
        this.mIsErasing,
    );
    if (p.x < 0 || p.y < 0) return;

    if (this.mIsDrawing) {
      const last = this.mLivePoints[this.mLivePoints.length - 1];
      if (
        !last ||
        lineLen(last, p) >= K_MIN_STROKE_SAMPLE_PX
      ) {
        this.mLivePoints.push(p);
        store.replaceActiveStrokePoints(this.mLivePoints);
      }
      return;
    }
    if (this.mIsSelecting) {
      store.setSelectionDraftRect({
        x: this.mSelectStart.x,
        y: this.mSelectStart.y,
        width: p.x - this.mSelectStart.x,
        height: p.y - this.mSelectStart.y,
      });
      return;
    }
    if (this.mIsResizing) {
      applyResize(store, p);
      return;
    }
    if (this.mIsAnchorDragging) {
      if (this.mAnchorDragSelectionId) {
        store.setSelectionAnchorPoint(this.mAnchorDragSelectionId, p);
      }
      this.mHoverAnchorSelectionId = this.mAnchorDragSelectionId;
      return;
    }
    if (this.mIsErasing) {
      const last = this.mEraseTracePoints[this.mEraseTracePoints.length - 1];
      if (
        !last ||
        lineLen(last, p) >= K_MIN_STROKE_SAMPLE_PX
      ) {
        this.mEraseTracePoints.push(p);
      }
      return;
    }
    if (this.mIsStrokeHardErasing) {
      const last = this.mEraseTracePoints[this.mEraseTracePoints.length - 1];
      if (
        !last ||
        lineLen(last, p) >= K_MIN_STROKE_SAMPLE_PX
      ) {
        this.mEraseTracePoints.push(p);
      }
      return;
    }
    if (store.toolModeValue() === ToolMode.Select) {
      const hoverAnchorId = hitAnchorSelectionId(p, store, store.zoom());
      if (hoverAnchorId !== this.mHoverAnchorSelectionId) {
        this.mHoverAnchorSelectionId = hoverAnchorId;
      }
    }
  }

  pointerUp(x: number, y: number, button: number): void {
    if (button !== 0) return;
    const store = this.store;
    this.finalizeInteraction(
      toBoard(x, y, store.zoom(), true),
    );
  }

  /** Returns selection id if handled (assign dialog); false if should start pan */
  pointerDoubleClick(
    x: number,
    y: number,
    button: number,
  ): { consumed: boolean; selectionId?: string } {
    if (
      this.store.toolModeValue() !== ToolMode.Select ||
      button !== 0
    ) {
      return { consumed: false };
    }
    const p = toBoard(x, y, this.store.zoom(), false);
    if (p.x < 0 || p.y < 0) return { consumed: false };
    const selId = hitSelectionId(p, this.store);
    if (!selId) return { consumed: false };
    this.store.setSelectedSelectionId(selId);
    return { consumed: true, selectionId: selId };
  }

  private finalizeInteraction(point: Point): void {
    const store = this.store;
    if (this.mIsDrawing) {
      if (point.x >= 0 && point.y >= 0) {
        const last = this.mLivePoints[this.mLivePoints.length - 1];
        if (
          !last ||
          lineLen(last, point) >= 0.02
        ) {
          this.mLivePoints.push(point);
        }
      }
      store.replaceActiveStrokePoints(this.mLivePoints);
      store.endStroke();
      this.mLivePoints = [];
      this.mIsDrawing = false;
    }
    if (this.mIsSelecting) {
      store.commitSelectionDraftRect();
      this.mIsSelecting = false;
    }
    if (this.mIsResizing) {
      store.setSelectionResizeState(null);
      this.mIsResizing = false;
    }
    if (this.mIsAnchorDragging) {
      if (this.mAnchorDragSelectionId) {
        store.setSelectionAnchorPoint(this.mAnchorDragSelectionId, point);
      }
      this.mIsAnchorDragging = false;
      this.mAnchorDragSelectionId = "";
    }
    if (this.mIsErasing) {
      if (this.mEraseTracePoints.length === 0) {
        this.mEraseTracePoints.push(point);
      }
      store.erasePointsInSelectedSelectionPath(
        this.mEraseTracePoints,
        store.eraseRadiusPx(),
      );
      this.mEraseTracePoints = [];
      this.mIsErasing = false;
    }
    if (this.mIsStrokeHardErasing) {
      if (this.mEraseTracePoints.length === 0) {
        this.mEraseTracePoints.push(point);
      }
      store.removeStrokePointsNearPath(
        this.mEraseTracePoints,
        store.eraseRadiusPx(),
      );
      this.mEraseTracePoints = [];
      this.mIsStrokeHardErasing = false;
    }

    if (
      store.toolModeValue() === ToolMode.Select &&
      point.x >= 0 &&
      point.y >= 0
    ) {
      this.mHoverAnchorSelectionId = hitAnchorSelectionId(
        point,
        store,
        store.zoom(),
      );
    }
  }

  resetGestureState(): void {
    this.mIsDrawing = false;
    this.mIsSelecting = false;
    this.mIsResizing = false;
    this.mIsErasing = false;
    this.mIsStrokeHardErasing = false;
    this.mIsAnchorDragging = false;
    this.mLivePoints = [];
    this.mEraseTracePoints = [];
    this.mAnchorDragSelectionId = "";
  }
}
