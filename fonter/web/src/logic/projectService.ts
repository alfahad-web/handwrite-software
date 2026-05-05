/**
 * Mirrors fonter/cachy/src/services/ProjectService.cpp
 * (file I/O replaced by string / object in/out for the web).
 */

import type { EditorStore } from "./editorStore";
import {
  joinModeFromString,
  joinModeToString,
  type SelectionBox,
  type Stroke,
  type StrokePoint,
} from "./editorTypes";

export interface HwProjectJson {
  formatVersion: number;
  strokePx: number;
  captureGapUm: number;
  guideLineGapPx?: number;
  zoom: number;
  eraseRadiusPx: number;
  strokes: Array<{
    id: string;
    createdAt: string;
    points: Array<{ x: number; y: number; erased?: boolean }>;
  }>;
  selectionBoxes: Array<{
    id: string;
    orderIndex: number;
    x: number;
    y: number;
    width: number;
    height: number;
    assigned: boolean;
    assignedAscii: number;
    fileStem: string;
    joinMode: string;
    hasManualAnchor?: boolean;
    manualAnchorRx?: number;
    manualAnchorRy?: number;
    anchorX: number;
    anchorY: number;
  }>;
  selectedSelectionId: string;
  /** selectionId -> strokeId#pointIndex keys */
  selectionErasedPoints?: Record<string, string[]>;
  specialCharStemMap: Record<string, string>;
}

export function saveProjectToJson(store: EditorStore): string {
  const erased = store.getSelectionErasedPointKeys();
  const selectionErasedPoints: Record<string, string[]> = {};
  for (const [selId, keys] of erased) {
    selectionErasedPoints[selId] = [...keys];
  }

  const root: HwProjectJson = {
    formatVersion: 2,
    strokePx: store.strokePx(),
    captureGapUm: store.captureGapUm(),
    guideLineGapPx: store.guideLineGapPx(),
    zoom: store.zoom(),
    eraseRadiusPx: store.eraseRadiusPx(),
    strokes: [],
    selectionBoxes: [],
    selectedSelectionId: store.selectedSelectionId(),
    selectionErasedPoints,
    specialCharStemMap: {},
  };

  for (const stroke of store.strokes()) {
    const points = stroke.points.map((pt) => ({
      x: pt.pos.x,
      y: pt.pos.y,
      erased: pt.erased,
    }));
    root.strokes.push({
      id: stroke.id,
      createdAt: String(stroke.createdAt),
      points,
    });
  }

  for (const box of store.selectionBoxes()) {
    root.selectionBoxes.push({
      id: box.id,
      orderIndex: box.orderIndex,
      x: box.rect.x,
      y: box.rect.y,
      width: box.rect.width,
      height: box.rect.height,
      assigned: box.assigned,
      assignedAscii: box.assignedAscii,
      fileStem: box.fileStem,
      joinMode: joinModeToString(box.joinMode),
      hasManualAnchor: box.hasManualAnchor,
      manualAnchorRx: box.manualAnchorRx,
      manualAnchorRy: box.manualAnchorRy,
      anchorX: box.anchorX,
      anchorY: box.anchorY,
    });
  }

  const stems = store.getSpecialCharStemMap();
  for (const [k, v] of stems) {
    root.specialCharStemMap[String(k)] = v;
  }

  return JSON.stringify(root, null, 2);
}

export function loadProjectFromJson(
  store: EditorStore,
  jsonText: string,
  projectPathForName: string,
): { ok: true } | { ok: false; error: string } {
  let root: HwProjectJson;
  try {
    root = JSON.parse(jsonText) as HwProjectJson;
  } catch {
    return { ok: false, error: "Invalid project file." };
  }

  if (!root || typeof root !== "object") {
    return { ok: false, error: "Invalid project file." };
  }

  const fmt = root.formatVersion ?? 0;
  if (fmt !== 1 && fmt !== 2) {
    return { ok: false, error: "Unsupported project version." };
  }

  store.clearAll();
  store.setStrokePx(root.strokePx ?? store.strokePx());
  store.setCaptureGapUm(root.captureGapUm ?? store.captureGapUm());
  store.setGuideLineGapPx(root.guideLineGapPx ?? store.guideLineGapPx());
  store.setZoom(root.zoom ?? store.zoom());
  store.setEraseRadiusPx(root.eraseRadiusPx ?? store.eraseRadiusPx());

  const loadedStrokes: Stroke[] = [];
  for (const s of root.strokes ?? []) {
    const pointsArr = s.points ?? [];
    if (pointsArr.length === 0) continue;
    const points: StrokePoint[] = [];
    for (const p of pointsArr) {
      points.push({
        pos: { x: p.x, y: p.y },
        erased: p.erased ?? false,
      });
    }
    loadedStrokes.push({
      id: s.id,
      createdAt: Number(s.createdAt),
      points,
    });
  }
  store.setStrokes(loadedStrokes);

  const loadedBoxes: SelectionBox[] = [];
  for (const b of root.selectionBoxes ?? []) {
    loadedBoxes.push({
      id: b.id,
      orderIndex: b.orderIndex ?? 0,
      rect: {
        x: b.x,
        y: b.y,
        width: b.width,
        height: b.height,
      },
      assigned: b.assigned ?? false,
      assignedAscii: b.assignedAscii ?? -1,
      fileStem: b.fileStem ?? "",
      joinMode: joinModeFromString(b.joinMode ?? "N"),
      hasManualAnchor: b.hasManualAnchor ?? false,
      manualAnchorRx: b.manualAnchorRx ?? 0,
      manualAnchorRy: b.manualAnchorRy ?? 0,
      anchorX: b.anchorX ?? 0,
      anchorY: b.anchorY ?? 0,
    });
  }
  store.setSelectionBoxes(loadedBoxes, root.selectedSelectionId ?? "");

  const erasedObj = root.selectionErasedPoints ?? {};
  const erasedMap = new Map<string, Set<string>>();
  for (const key of Object.keys(erasedObj)) {
    const arr = erasedObj[key];
    if (Array.isArray(arr) && arr.length > 0) {
      erasedMap.set(key, new Set(arr.filter((x) => typeof x === "string")));
    }
  }
  store.setSelectionErasedPointKeys(erasedMap);

  const stemMap = new Map<number, string>();
  const stemObj = root.specialCharStemMap ?? {};
  for (const key of Object.keys(stemObj)) {
    stemMap.set(Number(key), stemObj[key]!);
  }
  store.setSpecialCharStemMap(stemMap);

  store.setProjectFilePath(projectPathForName);
  store.markSaved();
  return { ok: true };
}
