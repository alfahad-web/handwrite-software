/**
 * Paint order mirrors fonter/cachy/src/canvas/CanvasItem.cpp paint()
 */

import { K_BOARD_HEIGHT, K_BOARD_WIDTH } from "./canvasPointer";
import { pxToUm } from "./exportService";
import type { EditorStore } from "./editorStore";
import type { Point } from "./editorTypes";

function drawStrokePath(
  ctx: CanvasRenderingContext2D,
  points: { x: number; y: number }[],
  strokePx: number,
): void {
  if (points.length === 1) {
    ctx.beginPath();
    ctx.arc(
      points[0]!.x,
      points[0]!.y,
      strokePx * 0.5,
      0,
      Math.PI * 2,
    );
    ctx.fill();
    return;
  }
  if (points.length < 2) return;
  ctx.beginPath();
  ctx.moveTo(points[0]!.x, points[0]!.y);
  for (let i = 1; i < points.length; i++) {
    ctx.lineTo(points[i]!.x, points[i]!.y);
  }
  ctx.stroke();
}

export function paintBoard(
  ctx: CanvasRenderingContext2D,
  store: EditorStore,
  livePoints: Point[],
  dpi: number,
): void {
  const scale = store.zoom() / 100;
  const cw = K_BOARD_WIDTH * scale;
  const ch = K_BOARD_HEIGHT * scale;
  if (ctx.canvas.width !== cw || ctx.canvas.height !== ch) {
    ctx.canvas.width = cw;
    ctx.canvas.height = ch;
  }

  ctx.setTransform(1, 0, 0, 1, 0, 0);
  ctx.fillStyle = "#f4f4f5";
  ctx.fillRect(0, 0, cw, ch);

  const boardRectW = K_BOARD_WIDTH * scale;
  const boardRectH = K_BOARD_HEIGHT * scale;
  ctx.fillStyle = "#ffffff";
  ctx.fillRect(0, 0, boardRectW, boardRectH);
  ctx.strokeStyle = "#e4e4e7";
  ctx.lineWidth = 1;
  ctx.strokeRect(0, 0, boardRectW, boardRectH);

  ctx.save();
  ctx.scale(scale, scale);

  ctx.strokeStyle = "#000000";
  ctx.lineWidth = store.strokePx();
  ctx.lineCap = "round";
  ctx.lineJoin = "round";
  ctx.fillStyle = "#000000";

  for (const stroke of store.strokes()) {
    const pts = stroke.points.map((p) => p.pos);
    drawStrokePath(ctx, pts, store.strokePx());
  }

  if (livePoints.length > 1) {
    ctx.strokeStyle = "#000000";
    ctx.lineWidth = store.strokePx();
    drawStrokePath(ctx, livePoints, store.strokePx());
  }

  const draft = store.selectionDraftRect();
  if (draft && draft.width > 0 && draft.height > 0) {
    ctx.strokeStyle = "#2563eb";
    ctx.fillStyle = "rgba(59, 130, 246, 0.2)";
    ctx.lineWidth = 1;
    ctx.fillRect(draft.x, draft.y, draft.width, draft.height);
    ctx.strokeRect(draft.x, draft.y, draft.width, draft.height);
  }

  for (const box of store.selectionBoxes()) {
    const selected = box.id === store.selectedSelectionId();
    ctx.strokeStyle = "#2563eb";
    ctx.fillStyle = selected
      ? "rgba(59, 130, 246, 0.43)"
      : "rgba(59, 130, 246, 0.28)";
    ctx.lineWidth = selected ? 2 : 1;
    ctx.fillRect(box.rect.x, box.rect.y, box.rect.width, box.rect.height);
    ctx.strokeRect(box.rect.x, box.rect.y, box.rect.width, box.rect.height);
  }

  const dotRadius = 2.2;
  ctx.fillStyle = "#eab308";
  const gapPx =
    pxToUm(1, dpi) > 0 ? store.captureGapUm() / pxToUm(1, dpi) : 1;

  for (const stroke of store.strokes()) {
    if (stroke.points.length === 0) continue;
    let last: Point | null = null;
    for (const pt of stroke.points) {
      if (pt.erased) continue;
      if (
        !last ||
        Math.hypot(last.x - pt.pos.x, last.y - pt.pos.y) >= gapPx
      ) {
        ctx.beginPath();
        ctx.arc(pt.pos.x, pt.pos.y, dotRadius, 0, Math.PI * 2);
        ctx.fill();
        last = pt.pos;
      }
    }
  }

  ctx.fillStyle = "#22c55e";
  const anchorR = 3;
  for (const box of store.selectionBoxes()) {
    ctx.beginPath();
    ctx.arc(box.anchorX, box.anchorY, anchorR, 0, Math.PI * 2);
    ctx.fill();
  }
  if (draft && draft.width > 0 && draft.height > 0) {
    const da = store.draftAnchorBoard();
    ctx.beginPath();
    ctx.arc(da.x, da.y, anchorR, 0, Math.PI * 2);
    ctx.fill();
  }

  ctx.restore();
}
