import React, { useCallback, useEffect, useMemo, useRef, useState } from "react";
import { useEditorStore } from "../../context/index.ts";
import { MIN_STROKE_SAMPLE_PX } from "../../context/store.ts";
import type { PointPx, SelectionRect } from "../../context/types.ts";
import { buildExportStrokes } from "../../utils/exportSelection.ts";
import { resolveScreenDpi } from "../../utils/units.ts";
import "./ImageCanvas.css";

const ImageCanvas: React.FC = () => {
  const viewportRef = useRef<HTMLDivElement>(null);
  const boardRef = useRef<HTMLDivElement>(null);

  const strokes = useEditorStore((s) => s.strokes);
  const zoom = useEditorStore((s) => s.zoom);
  const toolMode = useEditorStore((s) => s.toolMode);
  const strokePx = useEditorStore((s) => s.strokePx);
  const captureGapUm = useEditorStore((s) => s.captureGapUm);
  const selectionRect = useEditorStore((s) => s.selectionRect);
  const selectionDraftRect = useEditorStore((s) => s.selectionDraftRect);
  const startStroke = useEditorStore((s) => s.startStroke);
  const endStroke = useEditorStore((s) => s.endStroke);
  const setSelectionDraftRect = useEditorStore((s) => s.setSelectionDraftRect);
  const commitSelectionDraftRect = useEditorStore((s) => s.commitSelectionDraftRect);
  const setSelectionResizeState = useEditorStore((s) => s.setSelectionResizeState);
  const setSelectionRect = useEditorStore((s) => s.setSelectionRect);

  const [penStrokeActive, setPenStrokeActive] = useState(false);

  const drawCrossRef = useRef<HTMLDivElement>(null);
  const lastCrossBoardRef = useRef<PointPx | null>(null);

  const updateDrawCross = useCallback((p: PointPx | null) => {
    const el = drawCrossRef.current;
    if (!el) return;
    if (!p) {
      lastCrossBoardRef.current = null;
      el.style.display = "none";
      return;
    }
    lastCrossBoardRef.current = p;
    const sc = zoom / 100;
    el.style.display = "block";
    el.style.left = `${p.x * sc}px`;
    el.style.top = `${p.y * sc}px`;
  }, [zoom]);

  const liveStrokePointsRef = useRef<PointPx[]>([]);
  const strokeSyncRafRef = useRef<number | null>(null);
  const liveStrokePathRef = useRef<SVGPathElement | null>(null);

  const currentStrokeId = useEditorStore((s) => s.currentStrokeId);

  const syncLiveStrokePathDom = useCallback(() => {
    const el = liveStrokePathRef.current;
    if (!el) return;
    const pts = liveStrokePointsRef.current;
    const px = useEditorStore.getState().strokePx;
    if (pts.length === 0) {
      el.setAttribute("d", "");
      el.style.display = "none";
      return;
    }
    el.style.display = "";
    el.setAttribute("stroke-width", String(px));
    let d: string;
    if (pts.length === 1) {
      const p = pts[0];
      d = `M ${p.x} ${p.y} L ${p.x + 0.35} ${p.y + 0.35}`;
    } else {
      d = pts.map((pt, idx) => `${idx === 0 ? "M" : "L"} ${pt.x} ${pt.y}`).join(" ");
    }
    el.setAttribute("d", d);
  }, []);

  const scheduleStrokeSync = useCallback(() => {
    if (strokeSyncRafRef.current != null) return;
    strokeSyncRafRef.current = requestAnimationFrame(() => {
      strokeSyncRafRef.current = null;
      syncLiveStrokePathDom();
    });
  }, [syncLiveStrokePathDom]);

  const tryAppendLiveStrokePoint = useCallback(
    (p: PointPx) => {
      const arr = liveStrokePointsRef.current;
      const last = arr[arr.length - 1];
      if (last && Math.hypot(p.x - last.x, p.y - last.y) < MIN_STROKE_SAMPLE_PX) return;
      arr.push(p);
      scheduleStrokeSync();
    },
    [scheduleStrokeSync]
  );

  const draftRectPendingRef = useRef<SelectionRect | null>(null);
  const draftRectRafRef = useRef<number | null>(null);

  const scheduleSelectionDraftRect = useCallback(
    (rect: SelectionRect) => {
      draftRectPendingRef.current = rect;
      if (draftRectRafRef.current != null) return;
      draftRectRafRef.current = requestAnimationFrame(() => {
        draftRectRafRef.current = null;
        const r = draftRectPendingRef.current;
        if (r) setSelectionDraftRect(r);
      });
    },
    [setSelectionDraftRect]
  );

  const resizePointPendingRef = useRef<PointPx | null>(null);
  const resizeRafRef = useRef<number | null>(null);

  const applyResize = useCallback((current: PointPx) => {
    const state = useEditorStore.getState().selectionResizeState;
    if (!state) return;
    const dx = current.x - state.startPoint.x;
    const dy = current.y - state.startPoint.y;
    const base = state.startRect;
    let next = { ...base };
    if (state.handle.includes("w")) {
      next.x = base.x + dx;
      next.width = base.width - dx;
    }
    if (state.handle.includes("e")) {
      next.width = base.width + dx;
    }
    if (state.handle.includes("n")) {
      next.y = base.y + dy;
      next.height = base.height - dy;
    }
    if (state.handle.includes("s")) {
      next.height = base.height + dy;
    }
    if (state.handle === "move") {
      next.x = base.x + dx;
      next.y = base.y + dy;
    }
    if (next.width < 0) {
      next.x += next.width;
      next.width = Math.abs(next.width);
    }
    if (next.height < 0) {
      next.y += next.height;
      next.height = Math.abs(next.height);
    }
    setSelectionRect(next);
  }, [setSelectionRect]);

  const scheduleResizeApply = useCallback(
    (point: PointPx) => {
      resizePointPendingRef.current = point;
      if (resizeRafRef.current != null) return;
      resizeRafRef.current = requestAnimationFrame(() => {
        resizeRafRef.current = null;
        const p = resizePointPendingRef.current;
        if (p) applyResize(p);
      });
    },
    [applyResize]
  );

  const isDrawingRef = useRef(false);
  const isSelectingRef = useRef(false);
  const isResizingRef = useRef(false);
  const selectStartRef = useRef<PointPx | null>(null);

  const [selectionCursor, setSelectionCursor] = useState("default");
  const dpi = useMemo(() => resolveScreenDpi(), []);
  const boardWidth = 3000;
  const boardHeight = 2000;

  const scale = zoom / 100;
  const boardScreenWidth = boardWidth * scale;
  const boardScreenHeight = boardHeight * scale;

  const clientToBoard = useCallback(
    (clientX: number, clientY: number, clampToBoard = false) => {
      const board = boardRef.current;
      if (!board) return null;
      const rect = board.getBoundingClientRect();
      const sc = zoom / 100;
      const rawX = (clientX - rect.left) / sc;
      const rawY = (clientY - rect.top) / sc;
      const inside =
        rawX >= 0 && rawX <= boardWidth && rawY >= 0 && rawY <= boardHeight;
      if (!inside && !clampToBoard) return null;
      return {
        x: Math.max(0, Math.min(boardWidth, rawX)),
        y: Math.max(0, Math.min(boardHeight, rawY)),
      };
    },
    [zoom]
  );

  const hitHandle = useCallback(
    (point: PointPx, rect: SelectionRect | null) => {
      if (!rect) return null;
      const sc = zoom / 100;
      const hs = 10 / sc;
      const left = rect.x;
      const right = rect.x + rect.width;
      const top = rect.y;
      const bottom = rect.y + rect.height;
      const near = (v: number, t: number) => Math.abs(v - t) <= hs;
      const withinX = point.x >= left - hs && point.x <= right + hs;
      const withinY = point.y >= top - hs && point.y <= bottom + hs;
      if (near(point.x, left) && near(point.y, top)) return "nw";
      if (near(point.x, right) && near(point.y, top)) return "ne";
      if (near(point.x, left) && near(point.y, bottom)) return "sw";
      if (near(point.x, right) && near(point.y, bottom)) return "se";
      if (near(point.x, left) && withinY) return "w";
      if (near(point.x, right) && withinY) return "e";
      if (near(point.y, top) && withinX) return "n";
      if (near(point.y, bottom) && withinX) return "s";
      if (
        point.x >= left + hs &&
        point.x <= right - hs &&
        point.y >= top + hs &&
        point.y <= bottom - hs
      ) {
        return "move";
      }
      return null;
    },
    [zoom]
  );

  const selectionForView = selectionDraftRect ?? selectionRect;

  useEffect(() => {
    return () => {
      if (strokeSyncRafRef.current != null) cancelAnimationFrame(strokeSyncRafRef.current);
      if (draftRectRafRef.current != null) cancelAnimationFrame(draftRectRafRef.current);
      if (resizeRafRef.current != null) cancelAnimationFrame(resizeRafRef.current);
    };
  }, []);

  useEffect(() => {
    if (toolMode !== "draw") {
      updateDrawCross(null);
    } else {
      const last = lastCrossBoardRef.current;
      if (last) updateDrawCross(last);
    }
  }, [toolMode, updateDrawCross]);

  useEffect(() => {
    const last = lastCrossBoardRef.current;
    if (toolMode === "draw" && last) {
      updateDrawCross(last);
    }
  }, [zoom, toolMode, updateDrawCross]);

  const windowDragListenersRef = useRef<{
    onMove: (e: PointerEvent) => void;
    onUp: (e: PointerEvent) => void;
  } | null>(null);

  const detachWindowDragListeners = useCallback(() => {
    const cur = windowDragListenersRef.current;
    if (!cur) return;
    window.removeEventListener("pointermove", cur.onMove);
    window.removeEventListener("pointerup", cur.onUp);
    window.removeEventListener("pointercancel", cur.onUp);
    windowDragListenersRef.current = null;
  }, []);

  useEffect(() => {
    return () => detachWindowDragListeners();
  }, [detachWindowDragListeners]);

  const finalizeMouseInteraction = useCallback(
    (clientX: number, clientY: number) => {
      if (isDrawingRef.current) {
        if (strokeSyncRafRef.current != null) {
          cancelAnimationFrame(strokeSyncRafRef.current);
          strokeSyncRafRef.current = null;
        }
        const p = clientToBoard(clientX, clientY, true);
        if (p) {
          const arr = liveStrokePointsRef.current;
          const last = arr[arr.length - 1];
          if (!last || Math.hypot(p.x - last.x, p.y - last.y) >= 0.02) {
            arr.push(p);
          }
        }
        const final = liveStrokePointsRef.current.slice();
        if (final.length > 0) {
          useEditorStore.getState().replaceActiveStrokePoints(final);
        }
        liveStrokePointsRef.current = [];
        isDrawingRef.current = false;
        syncLiveStrokePathDom();
        setPenStrokeActive(false);
        endStroke();
      }
      if (isSelectingRef.current) {
        if (draftRectRafRef.current != null) {
          cancelAnimationFrame(draftRectRafRef.current);
          draftRectRafRef.current = null;
        }
        const r = draftRectPendingRef.current;
        if (r) setSelectionDraftRect(r);
        draftRectPendingRef.current = null;
        isSelectingRef.current = false;
        selectStartRef.current = null;
        commitSelectionDraftRect();
      }
      if (isResizingRef.current) {
        if (resizeRafRef.current != null) {
          cancelAnimationFrame(resizeRafRef.current);
          resizeRafRef.current = null;
        }
        const rp = resizePointPendingRef.current;
        if (rp) applyResize(rp);
        resizePointPendingRef.current = null;
        isResizingRef.current = false;
        setSelectionResizeState(null);
      }
    },
    [
      applyResize,
      clientToBoard,
      commitSelectionDraftRect,
      endStroke,
      setSelectionDraftRect,
      setSelectionResizeState,
      syncLiveStrokePathDom,
    ]
  );

  const handlePointerDown = (e: React.PointerEvent<HTMLDivElement>) => {
    if (e.button !== 0) return;
    const point = clientToBoard(e.clientX, e.clientY);
    if (!point) return;

    detachWindowDragListeners();

    const modeAtDown = toolMode;

    if (toolMode === "draw") {
      updateDrawCross(point);
      setPenStrokeActive(true);
      isDrawingRef.current = true;
      liveStrokePointsRef.current = [point];
      startStroke(point);
      scheduleStrokeSync();
    } else {
      const handle = hitHandle(point, selectionRect);
      if (handle && selectionRect) {
        isResizingRef.current = true;
        setSelectionResizeState({
          handle,
          startRect: selectionRect,
          startPoint: point,
        });
      } else {
        isSelectingRef.current = true;
        selectStartRef.current = point;
        setSelectionDraftRect({ x: point.x, y: point.y, width: 0, height: 0 });
      }
    }

    const pointerId = e.pointerId;
    const onWinMove = (ev: PointerEvent) => {
      if (ev.pointerId !== pointerId) return;
      const dragging =
        isDrawingRef.current || isSelectingRef.current || isResizingRef.current;
      if (!dragging) return;
      const pt = clientToBoard(ev.clientX, ev.clientY, true);
      if (modeAtDown === "draw") {
        updateDrawCross(pt);
      }
      if (!pt) return;
      if (modeAtDown === "draw" && isDrawingRef.current) {
        tryAppendLiveStrokePoint(pt);
        return;
      }
      if (modeAtDown === "select" && isSelectingRef.current) {
        const start = selectStartRef.current;
        if (start) {
          scheduleSelectionDraftRect({
            x: start.x,
            y: start.y,
            width: pt.x - start.x,
            height: pt.y - start.y,
          });
        }
        return;
      }
      if (modeAtDown === "select" && isResizingRef.current) {
        scheduleResizeApply(pt);
      }
    };

    const onWinUp = (ev: PointerEvent) => {
      if (ev.pointerId !== pointerId) return;
      try {
        finalizeMouseInteraction(ev.clientX, ev.clientY);
      } finally {
        detachWindowDragListeners();
      }
    };

    windowDragListenersRef.current = { onMove: onWinMove, onUp: onWinUp };
    window.addEventListener("pointermove", onWinMove);
    window.addEventListener("pointerup", onWinUp);
    window.addEventListener("pointercancel", onWinUp);
  };

  const handlePointerMove = (e: React.PointerEvent<HTMLDivElement>) => {
    const activeDrag =
      isDrawingRef.current || isSelectingRef.current || isResizingRef.current;
    if (activeDrag) {
      return;
    }
    const point = clientToBoard(e.clientX, e.clientY, false);

    if (toolMode === "draw") {
      updateDrawCross(point);
    }

    if (!point) return;
    if (toolMode === "select") {
      const h = hitHandle(point, selectionRect);
      const nextCursorByHandle: Record<string, string> = {
        move: "move",
        n: "ns-resize",
        s: "ns-resize",
        e: "ew-resize",
        w: "ew-resize",
        nw: "nwse-resize",
        se: "nwse-resize",
        ne: "nesw-resize",
        sw: "nesw-resize",
      };
      setSelectionCursor(h ? nextCursorByHandle[h] : "default");
    }
  };

  const handlePointerUp = (e: React.PointerEvent<HTMLDivElement>) => {
    if (!isDrawingRef.current && !isSelectingRef.current && !isResizingRef.current) {
      return;
    }
    try {
      finalizeMouseInteraction(e.clientX, e.clientY);
    } finally {
      detachWindowDragListeners();
    }
  };

  const handleMouseLeave = () => {
    if (!isDrawingRef.current && toolMode === "draw") {
      updateDrawCross(null);
    }
    setSelectionCursor("default");
  };

  const sampled = useMemo(() => {
    if (penStrokeActive) return [];
    return buildExportStrokes(strokes, selectionRect, captureGapUm, dpi);
  }, [penStrokeActive, strokes, selectionRect, captureGapUm, dpi]);

  return (
    <div
      ref={viewportRef}
      className={`image-canvas-viewport${toolMode === "draw" ? " image-canvas-draw-mode" : ""}`}
    >
      <div className="image-canvas-scroll">
        <div
          ref={boardRef}
          className="whiteboard"
          style={{
            width: boardScreenWidth,
            height: boardScreenHeight,
            minWidth: boardScreenWidth,
            minHeight: boardScreenHeight,
            cursor: toolMode === "draw" ? "crosshair" : selectionCursor,
          }}
          onPointerDown={handlePointerDown}
          onPointerMove={handlePointerMove}
          onPointerUp={handlePointerUp}
          onPointerCancel={handlePointerUp}
          onMouseLeave={handleMouseLeave}
        >
          <svg
            className="whiteboard-svg"
            width={boardScreenWidth}
            height={boardScreenHeight}
            viewBox={`0 0 ${boardWidth} ${boardHeight}`}
          >
            {(penStrokeActive && currentStrokeId
              ? strokes.filter((s) => s.id !== currentStrokeId)
              : strokes
            ).map((stroke) => {
              if (stroke.points.length === 1) {
                const pt = stroke.points[0];
                return (
                  <circle
                    key={stroke.id}
                    cx={pt.x}
                    cy={pt.y}
                    r={Math.max(1, strokePx / 2)}
                    fill="#000000"
                  />
                );
              }
              if (stroke.points.length < 2) return null;
              const d = stroke.points
                .map((pt, idx) => `${idx === 0 ? "M" : "L"} ${pt.x} ${pt.y}`)
                .join(" ");
              return (
                <path
                  key={stroke.id}
                  d={d}
                  stroke="#000000"
                  strokeWidth={strokePx}
                  strokeLinecap="round"
                  strokeLinejoin="round"
                  fill="none"
                />
              );
            })}
            <path
              ref={liveStrokePathRef}
              fill="none"
              stroke="#000000"
              strokeLinecap="round"
              strokeLinejoin="round"
              strokeWidth={strokePx}
              style={{ display: "none" }}
              d=""
            />
            {sampled.map((stroke) =>
              stroke.points.map((pt, idx) => (
                <circle
                  key={`${stroke.strokeId}-${idx}`}
                  cx={(pt.xUm * dpi) / 25400}
                  cy={(pt.yUm * dpi) / 25400}
                  r={Math.max(1.5, strokePx * 0.22)}
                  fill="#eab308"
                />
              ))
            )}
            {selectionForView && (
              <rect
                x={selectionForView.x}
                y={selectionForView.y}
                width={selectionForView.width}
                height={selectionForView.height}
                fill="rgba(59,130,246,0.28)"
                stroke="#2563eb"
                strokeWidth={1}
              />
            )}
          </svg>
          {toolMode === "draw" && (
            <div ref={drawCrossRef} className="cross-cursor" style={{ display: "none" }} aria-hidden>
              +
            </div>
          )}
        </div>
      </div>
    </div>
  );
};

export default ImageCanvas;
