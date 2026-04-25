/**
 * Mirrors fonter/cachy/src/core/EditorStore.cpp / .h
 */

import {
  JoinMode,
  joinModeToString,
  type Point,
  type ResizeDragState,
  ResizeHandle,
  type SelectionBox,
  type SelectionBoxRow,
  type SelectionRect,
  type Stroke,
  type StrokePoint,
  ToolMode,
} from "./editorTypes";

const kDefaultStrokePx = 6;
const kDefaultCaptureGapUm = 350;
const kDefaultEraseRadiusPx = 20;
const kMinRectSide = 4;
const kStemAlphabet = "abcdefghijklmnopqrstuvwxyz0123456789";

function normRect(r: SelectionRect): { x: number; y: number; w: number; h: number } {
  const x = r.width < 0 ? r.x + r.width : r.x;
  const y = r.height < 0 ? r.y + r.height : r.y;
  return { x, y, w: Math.abs(r.width), h: Math.abs(r.height) };
}

function rectsIntersect(a: SelectionRect, b: SelectionRect): boolean {
  const ra = normRect(a);
  const rb = normRect(b);
  return !(
    ra.x + ra.w < rb.x ||
    rb.x + rb.w < ra.x ||
    ra.y + ra.h < rb.y ||
    rb.y + rb.h < ra.y
  );
}

function pointInsideRect(p: Point, r: SelectionRect): boolean {
  return (
    p.x >= r.x &&
    p.x <= r.x + r.width &&
    p.y >= r.y &&
    p.y <= r.y + r.height
  );
}

function anchorBoardForBox(
  self: SelectionBox,
  allCommitted: SelectionBox[],
): Point {
  let bestPred: SelectionBox | null = null;
  let bestOrder = -1;
  for (const b of allCommitted) {
    if (b.id === self.id) continue;
    if (b.orderIndex >= self.orderIndex) continue;
    if (!rectsIntersect(b.rect, self.rect)) continue;
    if (b.orderIndex > bestOrder) {
      bestOrder = b.orderIndex;
      bestPred = b;
    }
  }
  const fallback: Point = {
    x: self.rect.x,
    y: self.rect.y + self.rect.height,
  };
  if (!bestPred) return fallback;

  const predBottomRight: Point = {
    x: bestPred.rect.x + bestPred.rect.width,
    y: bestPred.rect.y + bestPred.rect.height,
  };
  if (!pointInsideRect(predBottomRight, self.rect)) return fallback;
  return predBottomRight;
}

function lineLength(a: Point, b: Point): number {
  return Math.hypot(b.x - a.x, b.y - a.y);
}

function makeUint64Hex(): string {
  const buf = new Uint32Array(2);
  crypto.getRandomValues(buf);
  const n = (BigInt(buf[0]!) << 32n) | BigInt(buf[1]!);
  return n.toString(16);
}

export class EditorStore {
  private listeners = new Set<() => void>();

  private mStrokes: Stroke[] = [];
  private mCurrentStrokeId = "";
  private mToolMode: ToolMode = ToolMode.Draw;
  private mStrokePx = kDefaultStrokePx;
  private mCaptureGapUm = kDefaultCaptureGapUm;
  private mZoom = 100;
  private mEraseRadiusPx = kDefaultEraseRadiusPx;
  private mDrawStrokeEraseActive = false;
  private mSelectionBoxes: SelectionBox[] = [];
  private mSelectedSelectionId = "";
  private mHasSelectionDraftRect = false;
  private mSelectionDraftRect: SelectionRect = {
    x: 0,
    y: 0,
    width: 0,
    height: 0,
  };
  private mHasResizeState = false;
  private mResizeState: ResizeDragState = {
    handle: ResizeHandle.None,
    selectionId: "",
    startRect: { x: 0, y: 0, width: 0, height: 0 },
    startPoint: { x: 0, y: 0 },
  };
  private mNextSelectionOrder = 1;
  private mProjectFilePath = "";
  private mProjectFileName = "";
  private mSpecialCharStemMap = new Map<number, string>();
  private mIsDirty = false;

  subscribe(fn: () => void): () => void {
    this.listeners.add(fn);
    return () => {
      this.listeners.delete(fn);
    };
  }

  private emit(): void {
    for (const f of this.listeners) f();
  }

  strokePx(): number {
    return this.mStrokePx;
  }
  captureGapUm(): number {
    return this.mCaptureGapUm;
  }
  zoom(): number {
    return this.mZoom;
  }
  toolMode(): string {
    if (this.mToolMode === ToolMode.Select) return "select";
    if (this.mToolMode === ToolMode.Erase) return "erase";
    return "draw";
  }
  hasSelection(): boolean {
    return this.mSelectionBoxes.length > 0 || this.mHasSelectionDraftRect;
  }
  hasSelectedSelection(): boolean {
    return this.mSelectedSelectionId.length > 0;
  }
  eraseRadiusPx(): number {
    return this.mEraseRadiusPx;
  }
  drawStrokeEraseActive(): boolean {
    return this.mDrawStrokeEraseActive;
  }
  isDirty(): boolean {
    return this.mIsDirty;
  }
  projectFilePath(): string {
    return this.mProjectFilePath;
  }
  projectFileName(): string {
    return this.mProjectFileName;
  }
  selectedSelectionId(): string {
    return this.mSelectedSelectionId;
  }

  strokes(): Stroke[] {
    return this.mStrokes;
  }
  currentStrokeId(): string {
    return this.mCurrentStrokeId;
  }
  toolModeValue(): ToolMode {
    return this.mToolMode;
  }
  selectionBoxes(): SelectionBox[] {
    return this.mSelectionBoxes;
  }
  selectedSelection(): SelectionBox | null {
    if (!this.mSelectedSelectionId) return null;
    return this.selectionById(this.mSelectedSelectionId);
  }
  selectionById(id: string): SelectionBox | null {
    const idx = this.findSelectionIndexById(id);
    if (idx < 0) return null;
    return this.mSelectionBoxes[idx]!;
  }
  selectionByIdMutable(id: string): SelectionBox | null {
    return this.selectionById(id);
  }
  selectionDraftRect(): SelectionRect | null {
    return this.mHasSelectionDraftRect ? this.mSelectionDraftRect : null;
  }
  selectionResizeState(): ResizeDragState | null {
    return this.mHasResizeState ? this.mResizeState : null;
  }

  setStrokePx(value: number): void {
    const next = EditorStore.clampInt(value, 1, 200);
    if (next === this.mStrokePx) return;
    this.mStrokePx = next;
    this.emit();
  }

  setCaptureGapUm(value: number): void {
    const next = EditorStore.clampInt(value, 1, 200000);
    if (next === this.mCaptureGapUm) return;
    this.mCaptureGapUm = next;
    this.emit();
  }

  setZoom(value: number): void {
    const next = EditorStore.clampInt(value, 10, 800);
    if (next === this.mZoom) return;
    this.mZoom = next;
    this.emit();
  }

  zoomIn(): void {
    this.setZoom(this.mZoom + 10);
  }
  zoomOut(): void {
    this.setZoom(this.mZoom - 10);
  }

  setToolMode(mode: string): void {
    let next: ToolMode = ToolMode.Draw;
    if (mode === "select") next = ToolMode.Select;
    if (mode === "erase") next = ToolMode.Erase;
    if (next === this.mToolMode) {
      if (next === ToolMode.Draw && this.mDrawStrokeEraseActive) {
        this.mDrawStrokeEraseActive = false;
        this.emit();
      }
      return;
    }
    if (this.mDrawStrokeEraseActive) {
      this.mDrawStrokeEraseActive = false;
    }
    this.mToolMode = next;
    this.emit();
  }

  setEraseRadiusPx(value: number): void {
    const next = EditorStore.clampInt(value, 1, 500);
    if (next === this.mEraseRadiusPx) return;
    this.mEraseRadiusPx = next;
    this.emit();
  }

  setDrawStrokeEraseActive(active: boolean): void {
    if (active && this.mToolMode !== ToolMode.Draw) return;
    if (this.mDrawStrokeEraseActive === active) return;
    this.mDrawStrokeEraseActive = active;
    this.emit();
  }

  deleteSelectedSelection(): boolean {
    if (!this.mSelectedSelectionId) return false;
    const idx = this.findSelectionIndexById(this.mSelectedSelectionId);
    if (idx < 0) return false;
    this.mSelectionBoxes.splice(idx, 1);
    this.mSelectedSelectionId = "";
    this.mHasResizeState = false;
    this.recomputeSelectionAnchors();
    this.markDirty();
    return true;
  }

  startStroke(point: Point): void {
    const stroke: Stroke = {
      id: EditorStore.makeStrokeId(),
      points: [{ pos: { ...point }, erased: false }],
      createdAt: Date.now(),
    };
    this.mCurrentStrokeId = stroke.id;
    this.mStrokes.push(stroke);
    this.markDirty();
    this.emit();
  }

  replaceActiveStrokePoints(points: Point[]): void {
    if (!this.mCurrentStrokeId || points.length === 0) return;
    const index = this.findStrokeIndexById(this.mCurrentStrokeId);
    if (index < 0) return;
    const normalized: StrokePoint[] = points.map((p) => ({
      pos: { ...p },
      erased: false,
    }));
    this.mStrokes[index]!.points = normalized;
    this.markDirty();
    this.emit();
  }

  endStroke(): void {
    this.mCurrentStrokeId = "";
  }

  setSelectionDraftRect(rect: SelectionRect | null): void {
    if (!rect) {
      if (!this.mHasSelectionDraftRect) return;
      this.mHasSelectionDraftRect = false;
      this.emit();
      return;
    }
    this.mSelectionDraftRect = { ...rect };
    this.mHasSelectionDraftRect = true;
    this.emit();
  }

  commitSelectionDraftRect(): string {
    if (!this.mHasSelectionDraftRect) return "";
    const normalized = EditorStore.normalizeRect(this.mSelectionDraftRect);
    this.mHasSelectionDraftRect = false;
    if (!normalized.ok) {
      this.emit();
      return "";
    }
    const box: SelectionBox = {
      id: EditorStore.makeSelectionId(),
      orderIndex: this.mNextSelectionOrder++,
      rect: normalized.rect,
      assigned: false,
      assignedAscii: -1,
      fileStem: "",
      joinMode: JoinMode.N,
      anchorX: 0,
      anchorY: 0,
    };
    this.mSelectionBoxes.push(box);
    this.mSelectedSelectionId = box.id;
    this.recomputeSelectionAnchors();
    this.markDirty();
    this.emit();
    return box.id;
  }

  clearSelectionDraft(): void {
    if (!this.mHasSelectionDraftRect) return;
    this.mHasSelectionDraftRect = false;
    this.emit();
  }

  setSelectionRect(selectionId: string, rect: SelectionRect | null): void {
    const idx = this.findSelectionIndexById(selectionId);
    if (idx < 0 || !rect) return;
    const normalized = EditorStore.normalizeRect(rect);
    if (!normalized.ok) return;
    this.mSelectionBoxes[idx]!.rect = normalized.rect;
    this.recomputeSelectionAnchors();
    this.markDirty();
    this.emit();
  }

  setSelectedSelectionId(selectionId: string): void {
    if (selectionId === this.mSelectedSelectionId) return;
    if (selectionId && this.findSelectionIndexById(selectionId) < 0)
      return;
    this.mSelectedSelectionId = selectionId;
    this.emit();
  }

  setSelectionResizeState(state: ResizeDragState | null): void {
    this.mHasResizeState = state !== null;
    if (state) this.mResizeState = { ...state };
    this.emit();
  }

  erasePointsInSelectedSelection(center: Point, radiusPx: number): boolean {
    const box = this.selectedSelection();
    if (!box) return false;
    const rsq = radiusPx * radiusPx;
    const minX = box.rect.x;
    const minY = box.rect.y;
    const maxX = box.rect.x + box.rect.width;
    const maxY = box.rect.y + box.rect.height;
    let changed = false;
    for (const stroke of this.mStrokes) {
      for (const pt of stroke.points) {
        if (pt.erased) continue;
        if (
          pt.pos.x < minX ||
          pt.pos.x > maxX ||
          pt.pos.y < minY ||
          pt.pos.y > maxY
        )
          continue;
        const dx = pt.pos.x - center.x;
        const dy = pt.pos.y - center.y;
        if (dx * dx + dy * dy <= rsq) {
          pt.erased = true;
          changed = true;
        }
      }
    }
    if (changed) {
      this.markDirty();
      this.emit();
    }
    return changed;
  }

  removeStrokePointsNear(center: Point, radiusPx: number): boolean {
    const rsq = radiusPx * radiusPx;
    const splitDistThresh = Math.max(
      radiusPx * 3.0,
      this.mStrokePx * 5.0,
    );
    const next: Stroke[] = [];
    let changed = false;
    const priorCurrentId = this.mCurrentStrokeId;

    for (const stroke of this.mStrokes) {
      const kept: StrokePoint[] = [];
      for (const pt of stroke.points) {
        const dx = pt.pos.x - center.x;
        const dy = pt.pos.y - center.y;
        if (dx * dx + dy * dy <= rsq) {
          changed = true;
          continue;
        }
        kept.push(pt);
      }
      if (kept.length === 0) continue;

      const runs: StrokePoint[][] = [];
      let run: StrokePoint[] = [];
      for (const pt of kept) {
        if (run.length === 0) {
          run.push(pt);
          continue;
        }
        if (lineLength(run[run.length - 1]!.pos, pt.pos) > splitDistThresh) {
          runs.push(run);
          run = [pt];
        } else {
          run.push(pt);
        }
      }
      if (run.length) runs.push(run);

      for (const r of runs) {
        if (r.length === 0) continue;
        next.push({
          id: EditorStore.makeStrokeId(),
          createdAt: stroke.createdAt,
          points: r,
        });
      }
    }

    if (!changed) return false;

    this.mStrokes = next;
    if (
      priorCurrentId &&
      this.findStrokeIndexById(priorCurrentId) < 0
    ) {
      this.mCurrentStrokeId = "";
    }
    this.markDirty();
    this.emit();
    return true;
  }

  setProjectFilePath(path: string): void {
    this.mProjectFilePath = path.trim();
    this.mProjectFileName = path
      ? EditorStore.fileNameFromPath(path)
      : "";
    this.emit();
  }

  clearProjectFilePath(): void {
    if (!this.mProjectFilePath && !this.mProjectFileName) return;
    this.mProjectFilePath = "";
    this.mProjectFileName = "";
    this.emit();
  }

  clearAll(): void {
    this.mStrokes = [];
    this.mCurrentStrokeId = "";
    this.mSelectionBoxes = [];
    this.mSelectedSelectionId = "";
    this.mHasSelectionDraftRect = false;
    this.mHasResizeState = false;
    this.mNextSelectionOrder = 1;
    this.mSpecialCharStemMap.clear();
    this.mIsDirty = false;
    if (this.mDrawStrokeEraseActive) {
      this.mDrawStrokeEraseActive = false;
    }
    this.emit();
  }

  fileStemForAscii(asciiCode: number): string {
    if (
      (asciiCode >= "a".charCodeAt(0) &&
        asciiCode <= "z".charCodeAt(0)) ||
      (asciiCode >= "A".charCodeAt(0) && asciiCode <= "Z".charCodeAt(0))
    ) {
      return String.fromCharCode(asciiCode);
    }
    const existing = this.mSpecialCharStemMap.get(asciiCode);
    if (existing !== undefined) return existing;
    let stem = EditorStore.makeSpecialStem();
    const used = new Set<string>();
    for (const v of this.mSpecialCharStemMap.values()) used.add(v);
    while (used.has(stem)) stem = EditorStore.makeSpecialStem();
    this.mSpecialCharStemMap.set(asciiCode, stem);
    this.markDirty();
    return stem;
  }

  selectionBoxesModel(): SelectionBoxRow[] {
    return this.mSelectionBoxes.map((box) => ({
      id: box.id,
      x: box.rect.x,
      y: box.rect.y,
      width: box.rect.width,
      height: box.rect.height,
      orderIndex: box.orderIndex,
      assigned: box.assigned,
      assignedAscii: box.assignedAscii,
      fileStem: box.fileStem,
      joinMode: joinModeToString(box.joinMode),
      anchorX: box.anchorX,
      anchorY: box.anchorY,
      selected: box.id === this.mSelectedSelectionId,
    }));
  }

  getSpecialCharStemMap(): Map<number, string> {
    return this.mSpecialCharStemMap;
  }

  setSpecialCharStemMap(map: Map<number, string>): void {
    this.mSpecialCharStemMap = new Map(map);
  }

  setStrokes(strokes: Stroke[]): void {
    this.mStrokes = strokes.map((s) => ({
      ...s,
      points: s.points.map((p) => ({
        pos: { ...p.pos },
        erased: p.erased,
      })),
    }));
    this.mCurrentStrokeId = "";
    this.emit();
  }

  setSelectionBoxes(boxes: SelectionBox[], selectedId: string): void {
    this.mSelectionBoxes = boxes.map((b) => ({
      ...b,
      rect: { ...b.rect },
    }));
    let maxOrder = 0;
    for (const box of this.mSelectionBoxes) {
      if (box.orderIndex > maxOrder) maxOrder = box.orderIndex;
    }
    this.mNextSelectionOrder = maxOrder + 1;
    this.mSelectedSelectionId = selectedId;
    if (
      this.mSelectedSelectionId &&
      this.findSelectionIndexById(this.mSelectedSelectionId) < 0
    ) {
      this.mSelectedSelectionId = "";
    }
    this.recomputeSelectionAnchors();
  }

  recomputeSelectionAnchors(): void {
    for (const box of this.mSelectionBoxes) {
      const a = anchorBoardForBox(box, this.mSelectionBoxes);
      box.anchorX = a.x;
      box.anchorY = a.y;
    }
    this.emit();
  }

  draftAnchorBoard(): Point {
    if (!this.mHasSelectionDraftRect) return { x: 0, y: 0 };
    const self: SelectionBox = {
      id: "_draft",
      orderIndex: this.mNextSelectionOrder,
      rect: this.mSelectionDraftRect,
      assigned: false,
      assignedAscii: -1,
      fileStem: "",
      joinMode: JoinMode.N,
      anchorX: 0,
      anchorY: 0,
    };
    return anchorBoardForBox(self, this.mSelectionBoxes);
  }

  markSaved(): void {
    if (!this.mIsDirty) return;
    this.mIsDirty = false;
    this.emit();
  }

  markDirty(): void {
    if (this.mIsDirty) return;
    this.mIsDirty = true;
    this.emit();
  }

  private static normalizeRect(
    rect: SelectionRect,
  ): { rect: SelectionRect; ok: boolean } {
    const out: SelectionRect = {
      x: rect.width < 0 ? rect.x + rect.width : rect.x,
      y: rect.height < 0 ? rect.y + rect.height : rect.y,
      width: Math.abs(rect.width),
      height: Math.abs(rect.height),
    };
    const valid = out.width >= kMinRectSide && out.height >= kMinRectSide;
    return { rect: out, ok: valid };
  }

  private static clampInt(value: number, min: number, max: number): number {
    return Math.min(max, Math.max(min, Math.trunc(value)));
  }

  private static makeStrokeId(): string {
    return `stroke-${makeUint64Hex()}`;
  }

  private static makeSelectionId(): string {
    return `sel-${makeUint64Hex()}`;
  }

  private static makeSpecialStem(): string {
    let out = "";
    const alphabetSize = kStemAlphabet.length;
    const buf = new Uint32Array(4);
    crypto.getRandomValues(buf);
    for (let i = 0; i < 4; i++) {
      const idx = buf[i]! % alphabetSize;
      out += kStemAlphabet[idx]!;
    }
    return out;
  }

  private findStrokeIndexById(id: string): number {
    for (let i = 0; i < this.mStrokes.length; i++) {
      if (this.mStrokes[i]!.id === id) return i;
    }
    return -1;
  }

  private findSelectionIndexById(id: string): number {
    for (let i = 0; i < this.mSelectionBoxes.length; i++) {
      if (this.mSelectionBoxes[i]!.id === id) return i;
    }
    return -1;
  }

  private static fileNameFromPath(path: string): string {
    const parts = path.split(/[/\\]/);
    return parts[parts.length - 1] ?? path;
  }
}
