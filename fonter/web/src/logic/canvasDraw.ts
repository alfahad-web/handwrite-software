/**
 * Paint order mirrors fonter/cachy/src/canvas/CanvasItem.cpp paint()
 */

import {
  K_ANCHOR_RADIUS_BOARD,
  K_BOARD_HEIGHT,
  K_BOARD_WIDTH,
} from "./canvasPointer";
import { pxToUm } from "./exportService";
import type { EditorStore } from "./editorStore";
import type { Point } from "./editorTypes";

const COLOR_DOT_DEFAULT = "#eab308";
const COLOR_DOT_INSIDE_ANY = "rgb(167, 139, 250)";
const COLOR_DOT_INSIDE_SELECTED = "rgb(109, 40, 217)";

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

export interface PaintBoardOptions {
  dpi: number;
  hoverAnchorSelectionId?: string;
  anchorDragSelectionId?: string;
}

export function paintBoard(
  ctx: CanvasRenderingContext2D,
  store: EditorStore,
  livePoints: Point[],
  options: PaintBoardOptions,
): void {
  const { dpi, hoverAnchorSelectionId = "", anchorDragSelectionId = "" } =
    options;
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

  const guideGap = store.guideLineGapPx();
  if (guideGap > 0) {
    ctx.strokeStyle = "#000000";
    ctx.lineWidth = 1 / scale;
    ctx.beginPath();
    for (let y = guideGap; y < K_BOARD_HEIGHT; y += guideGap) {
      ctx.moveTo(0, y);
      ctx.lineTo(K_BOARD_WIDTH, y);
    }
    ctx.stroke();
  }

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

  const selectedSelectionId = store.selectedSelectionId();
  const highlightedIds = store.highlightedSelectionIds();

  for (const box of store.selectionBoxes()) {
    const selected =
      box.id === selectedSelectionId || highlightedIds.has(box.id);
    ctx.strokeStyle = "#2563eb";
    ctx.fillStyle = selected
      ? "rgba(59, 130, 246, 0.43)"
      : "rgba(59, 130, 246, 0.28)";
    ctx.lineWidth = selected ? 2 : 1;
    ctx.fillRect(box.rect.x, box.rect.y, box.rect.width, box.rect.height);
    ctx.strokeRect(box.rect.x, box.rect.y, box.rect.width, box.rect.height);
  }

  const dotRadius = 2.2;
  const gapPx =
    pxToUm(1, dpi) > 0 ? store.captureGapUm() / pxToUm(1, dpi) : 1;
  const boxes = store.selectionBoxes();

  for (const stroke of store.strokes()) {
    if (stroke.points.length === 0) continue;
    let last: Point | null = null;
    for (let pointIndex = 0; pointIndex < stroke.points.length; pointIndex++) {
      const pt = stroke.points[pointIndex]!;
      if (pt.erased) continue;
      if (
        !last ||
        Math.hypot(last.x - pt.pos.x, last.y - pt.pos.y) >= gapPx
      ) {
        let inAnySelection = false;
        let inSelectedSelection = false;
        for (const box of boxes) {
          const inside =
            pt.pos.x >= box.rect.x &&
            pt.pos.x <= box.rect.x + box.rect.width &&
            pt.pos.y >= box.rect.y &&
            pt.pos.y <= box.rect.y + box.rect.height;
          if (!inside) continue;
          if (
            store.isPointErasedInSelection(
              box.id,
              stroke.id,
              pointIndex,
            )
          ) {
            continue;
          }
          inAnySelection = true;
          if (
            box.id === selectedSelectionId ||
            highlightedIds.has(box.id)
          ) {
            inSelectedSelection = true;
            break;
          }
        }
        if (inSelectedSelection) {
          ctx.fillStyle = COLOR_DOT_INSIDE_SELECTED;
        } else if (inAnySelection) {
          ctx.fillStyle = COLOR_DOT_INSIDE_ANY;
        } else {
          ctx.fillStyle = COLOR_DOT_DEFAULT;
        }
        ctx.beginPath();
        ctx.arc(pt.pos.x, pt.pos.y, dotRadius, 0, Math.PI * 2);
        ctx.fill();
        last = pt.pos;
      }
    }
  }

  for (const box of store.selectionBoxes()) {
    const anchorHot =
      box.id === hoverAnchorSelectionId ||
      (box.id === anchorDragSelectionId && anchorDragSelectionId.length > 0);
    const anchorR = anchorHot
      ? K_ANCHOR_RADIUS_BOARD * 1.8
      : K_ANCHOR_RADIUS_BOARD;
    ctx.fillStyle = anchorHot
      ? "rgb(22, 163, 74)"
      : "rgb(34, 197, 94)";
    ctx.beginPath();
    ctx.arc(box.anchorX, box.anchorY, anchorR, 0, Math.PI * 2);
    ctx.fill();
  }
  if (draft && draft.width > 0 && draft.height > 0) {
    const da = store.draftAnchorBoard();
    ctx.fillStyle = "rgb(34, 197, 94)";
    ctx.beginPath();
    ctx.arc(da.x, da.y, K_ANCHOR_RADIUS_BOARD, 0, Math.PI * 2);
    ctx.fill();
  }

  ctx.restore();
}
