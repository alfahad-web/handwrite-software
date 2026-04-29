import {
  useCallback,
  useEffect,
  useRef,
  useState,
} from "react";
import { BoardPointerController, hasSelectionAt } from "../logic/canvasPointer";
import { paintBoard } from "../logic/canvasDraw";
import type { EditorStore } from "../logic/editorStore";
import { ToolMode } from "../logic/editorTypes";

type BoardCanvasProps = {
  store: EditorStore;
  dpi: number;
  onSelectionDoubleClick: (selectionId: string) => void;
};

export function BoardCanvas({
  store,
  dpi,
  onSelectionDoubleClick,
}: BoardCanvasProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const scrollRef = useRef<HTMLDivElement>(null);
  const pointerRef = useRef(new BoardPointerController(store));
  const [, forcePaint] = useState(0);
  const [cursorPos, setCursorPos] = useState({ x: 0, y: 0 });
  const [cursorVisible, setCursorVisible] = useState(false);
  const panArmedRef = useRef(false);
  const panningRef = useRef(false);
  const panLastRef = useRef({ x: 0, y: 0 });

  const repaint = useCallback(() => {
    const c = canvasRef.current;
    if (!c) return;
    const ctx = c.getContext("2d");
    if (!ctx) return;
    paintBoard(ctx, store, pointerRef.current.livePoints, dpi);
  }, [store, dpi]);

  useEffect(() => {
    pointerRef.current = new BoardPointerController(store);
  }, [store]);

  useEffect(() => {
    return store.subscribe(() => {
      forcePaint((n) => n + 1);
    });
  }, [store]);

  useEffect(() => {
    repaint();
  }, [repaint, store, dpi]);

  const clientToCanvas = (ev: React.PointerEvent<HTMLCanvasElement>) => {
    const c = canvasRef.current;
    if (!c) return { x: 0, y: 0 };
    const r = c.getBoundingClientRect();
    return { x: ev.clientX - r.left, y: ev.clientY - r.top };
  };

  const onPointerDown = (ev: React.PointerEvent<HTMLCanvasElement>) => {
    if (panArmedRef.current && ev.button === 0) {
      panArmedRef.current = false;
      panningRef.current = true;
      panLastRef.current = { x: ev.clientX, y: ev.clientY };
      ev.currentTarget.setPointerCapture(ev.pointerId);
      return;
    }
    const { x, y } = clientToCanvas(ev);
    pointerRef.current.pointerDown(x, y, ev.button);
    repaint();
  };

  const onPointerMove = (ev: React.PointerEvent<HTMLCanvasElement>) => {
    const { x, y } = clientToCanvas(ev);
    setCursorPos({ x, y });
    if (panningRef.current && scrollRef.current) {
      const dx = ev.clientX - panLastRef.current.x;
      const dy = ev.clientY - panLastRef.current.y;
      panLastRef.current = { x: ev.clientX, y: ev.clientY };
      scrollRef.current.scrollLeft -= dx;
      scrollRef.current.scrollTop -= dy;
      return;
    }
    pointerRef.current.pointerMove(x, y);
    repaint();
  };

  const onPointerUp = (ev: React.PointerEvent<HTMLCanvasElement>) => {
    if (panningRef.current) {
      panningRef.current = false;
      try {
        ev.currentTarget.releasePointerCapture(ev.pointerId);
      } catch {
        /* ignore */
      }
      return;
    }
    const { x, y } = clientToCanvas(ev);
    pointerRef.current.pointerUp(x, y, ev.button);
    repaint();
  };

  const onDoubleClick = (ev: React.MouseEvent<HTMLCanvasElement>) => {
    if (store.toolModeValue() !== ToolMode.Select) return;
    const c = canvasRef.current;
    if (!c) return;
    const r0 = c.getBoundingClientRect();
    const x = ev.clientX - r0.left;
    const y = ev.clientY - r0.top;
    const r = pointerRef.current.pointerDoubleClick(x, y, 0);
    if (r.consumed && r.selectionId) {
      onSelectionDoubleClick(r.selectionId);
      return;
    }
    if (ev.button === 0 && !hasSelectionAt(x, y, store)) {
      panArmedRef.current = true;
    }
  };

  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Delete" || e.key === "Backspace") {
        if (store.deleteSelectedSelection()) {
          repaint();
        }
      }
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [store, repaint]);

  return (
    <div
      ref={scrollRef}
      className="board-scroll"
      style={{
        flex: 1,
        minHeight: 0,
        overflow: "auto",
        background: "#f4f4f5",
        position: "relative",
      }}
    >
      <canvas
        ref={canvasRef}
        tabIndex={0}
        style={{
          display: "block",
          touchAction: "none",
          cursor:
            store.toolModeValue() === ToolMode.Draw &&
            !store.drawStrokeEraseActive()
              ? "crosshair"
              : store.toolModeValue() === ToolMode.Erase ||
                  store.drawStrokeEraseActive()
                ? "none"
                : "default",
        }}
        onPointerDown={onPointerDown}
        onPointerEnter={() => setCursorVisible(true)}
        onPointerMove={onPointerMove}
        onPointerUp={onPointerUp}
        onPointerLeave={(ev) => {
          setCursorVisible(false);
          if (!panningRef.current) {
            const { x, y } = clientToCanvas(ev);
            pointerRef.current.pointerUp(x, y, ev.button);
            repaint();
          }
        }}
        onDoubleClick={onDoubleClick}
      />
      {(store.toolModeValue() === ToolMode.Erase ||
        store.drawStrokeEraseActive()) &&
        cursorVisible && (
          <div
            style={{
              position: "absolute",
              left: cursorPos.x,
              top: cursorPos.y,
              width: `${Math.max(2, store.eraseRadiusPx() * (store.zoom() / 100)) * 2}px`,
              height: `${Math.max(2, store.eraseRadiusPx() * (store.zoom() / 100)) * 2}px`,
              transform: "translate(-50%, -50%)",
              borderRadius: "50%",
              border: "1.5px solid rgba(161, 98, 7, 0.85)",
              background: "rgba(250, 204, 21, 0.25)",
              pointerEvents: "none",
            }}
          />
        )}
    </div>
  );
}
