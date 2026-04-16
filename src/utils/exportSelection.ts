import type { PointPx, SampledStrokeUm, SelectionRect, Stroke } from "../context/types.ts";
import { pxToUm } from "./units.ts";

function isPointInsideRect(point: PointPx, rect: SelectionRect): boolean {
  return (
    point.x >= rect.x &&
    point.x <= rect.x + rect.width &&
    point.y >= rect.y &&
    point.y <= rect.y + rect.height
  );
}

function distance(a: PointPx, b: PointPx): number {
  const dx = a.x - b.x;
  const dy = a.y - b.y;
  return Math.hypot(dx, dy);
}

function sampleStrokePointsByGapUm(
  points: PointPx[],
  captureGapUm: number,
  dpi: number
): Array<{ xUm: number; yUm: number }> {
  if (points.length === 0) return [];
  const sampled: PointPx[] = [points[0]];
  let lastKept = points[0];
  for (let i = 1; i < points.length; i++) {
    const p = points[i];
    if (pxToUm(distance(lastKept, p), dpi) >= captureGapUm) {
      sampled.push(p);
      lastKept = p;
    }
  }
  return sampled.map((p) => ({
    xUm: Math.round(pxToUm(p.x, dpi)),
    yUm: Math.round(pxToUm(p.y, dpi)),
  }));
}

export function buildExportStrokes(
  strokes: Stroke[],
  selectionRect: SelectionRect | null,
  captureGapUm: number,
  dpi: number
): SampledStrokeUm[] {
  if (!selectionRect) return [];
  const output: SampledStrokeUm[] = [];
  for (const stroke of strokes) {
    const insidePoints = stroke.points.filter((point) =>
      isPointInsideRect(point, selectionRect)
    );
    if (insidePoints.length === 0) continue;
    const sampled = sampleStrokePointsByGapUm(insidePoints, captureGapUm, dpi);
    if (sampled.length === 0) continue;
    output.push({ strokeId: stroke.id, points: sampled });
  }
  return output;
}

export function serializeExportLines(strokes: SampledStrokeUm[]): string[] {
  return strokes.map((stroke) =>
    stroke.points.flatMap((point) => [String(point.xUm), String(point.yUm)]).join(" ")
  );
}
