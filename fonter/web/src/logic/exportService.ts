/**
 * Mirrors fonter/cachy/src/services/ExportService.cpp
 */

import type { Point, SelectionRect, Stroke } from "./editorTypes";

export const kMicrometersPerInch = 25400.0;
export const kCssPixelsPerInch = 96.0;

export interface SampledPointUm {
  xUm: number;
  yUm: number;
}

export interface SampledStrokeUm {
  strokeId: string;
  points: SampledPointUm[];
}

export interface SelectionExportFile {
  selectionId: string;
  fileName: string;
  lines: string[];
}

function isPointInsideRect(p: Point, r: SelectionRect): boolean {
  return (
    p.x >= r.x &&
    p.x <= r.x + r.width &&
    p.y >= r.y &&
    p.y <= r.y + r.height
  );
}

function distance(a: Point, b: Point): number {
  const dx = a.x - b.x;
  const dy = a.y - b.y;
  return Math.hypot(dx, dy);
}

export function pxToUm(px: number, dpi: number): number {
  if (!Number.isFinite(px) || !Number.isFinite(dpi) || dpi <= 0) return 0;
  return (px / dpi) * kMicrometersPerInch;
}

function toRelativeUm(
  pBoard: Point,
  anchorBoard: Point,
  dpi: number,
): SampledPointUm {
  const localXPx = pBoard.x - anchorBoard.x;
  const localYPx = anchorBoard.y - pBoard.y;
  return {
    xUm: Math.round(pxToUm(localXPx, dpi)),
    yUm: Math.round(pxToUm(localYPx, dpi)),
  };
}

export function resolveScreenDpi(devicePixelRatio: number): number {
  const ratio = Number.isFinite(devicePixelRatio) ? devicePixelRatio : 1;
  return kCssPixelsPerInch * ratio;
}

export function buildExportStrokes(
  strokes: Stroke[],
  selectionRect: SelectionRect | null,
  anchorBoard: Point,
  captureGapUm: number,
  dpi: number,
): SampledStrokeUm[] {
  const output: SampledStrokeUm[] = [];
  if (!selectionRect) return output;

  for (const stroke of strokes) {
    const inside: Point[] = [];
    for (const pt of stroke.points) {
      if (pt.erased) continue;
      if (isPointInsideRect(pt.pos, selectionRect)) inside.push(pt.pos);
    }
    if (inside.length === 0) continue;

    const sampled: Point[] = [];
    sampled.push(inside[0]!);
    let last = inside[0]!;
    for (let i = 1; i < inside.length; i++) {
      if (pxToUm(distance(last, inside[i]!), dpi) >= captureGapUm) {
        sampled.push(inside[i]!);
        last = inside[i]!;
      }
    }
    if (sampled.length === 0) continue;

    const out: SampledStrokeUm = {
      strokeId: stroke.id,
      points: [],
    };
    for (const p of sampled) {
      out.points.push(toRelativeUm(p, anchorBoard, dpi));
    }
    output.push(out);
  }
  return output;
}

export function serializeExportLines(
  strokes: SampledStrokeUm[],
  selectionRect: SelectionRect,
  anchorBoard: Point,
  dpi: number,
): string[] {
  const lines: string[] = [];
  const bl: Point = {
    x: selectionRect.x,
    y: selectionRect.y + selectionRect.height,
  };
  const tl: Point = { x: selectionRect.x, y: selectionRect.y };
  const tr: Point = {
    x: selectionRect.x + selectionRect.width,
    y: selectionRect.y,
  };
  const br: Point = {
    x: selectionRect.x + selectionRect.width,
    y: selectionRect.y + selectionRect.height,
  };
  const blUm = toRelativeUm(bl, anchorBoard, dpi);
  const tlUm = toRelativeUm(tl, anchorBoard, dpi);
  const trUm = toRelativeUm(tr, anchorBoard, dpi);
  const brUm = toRelativeUm(br, anchorBoard, dpi);
  const headerParts = [
    String(blUm.xUm),
    String(blUm.yUm),
    String(tlUm.xUm),
    String(tlUm.yUm),
    String(trUm.xUm),
    String(trUm.yUm),
    String(brUm.xUm),
    String(brUm.yUm),
  ];
  lines.push(headerParts.join(" ") + ";");

  for (const stroke of strokes) {
    const parts: string[] = [];
    for (const p of stroke.points) {
      parts.push(String(p.xUm), String(p.yUm));
    }
    if (parts.length === 0) continue;
    lines.push(parts.join(" ") + ";");
  }
  return lines;
}

export function buildSelectionExports(
  strokes: Stroke[],
  selectionBoxes: Array<{
    id: string;
    rect: SelectionRect;
    assigned: boolean;
    fileStem: string;
    anchorX: number;
    anchorY: number;
  }>,
  captureGapUm: number,
  dpi: number,
): SelectionExportFile[] {
  const files: SelectionExportFile[] = [];
  for (const box of selectionBoxes) {
    if (!box.assigned || !box.fileStem) continue;
    const anchorBoard: Point = { x: box.anchorX, y: box.anchorY };
    const sampled = buildExportStrokes(
      strokes,
      box.rect,
      anchorBoard,
      captureGapUm,
      dpi,
    );
    const f: SelectionExportFile = {
      selectionId: box.id,
      fileName: `${box.fileStem}.txt`,
      lines: serializeExportLines(
        sampled,
        box.rect,
        anchorBoard,
        dpi,
      ),
    };
    files.push(f);
  }
  return files;
}
