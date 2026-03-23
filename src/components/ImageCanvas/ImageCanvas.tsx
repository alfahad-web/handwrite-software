import React, {
  useCallback,
  useEffect,
  useLayoutEffect,
  useRef,
  useState,
} from "react";
import { useEditorStore } from "../../context/index.ts";
import "./ImageCanvas.css";

const ImageCanvas: React.FC = () => {
  const viewportRef = useRef<HTMLDivElement>(null);
  const innerRef = useRef<HTMLDivElement>(null);
  const canvasRef = useRef<HTMLCanvasElement>(null);

  const currentImageData = useEditorStore((s) => s.currentImageData);
  const imageWidth = useEditorStore((s) => s.imageWidth);
  const imageHeight = useEditorStore((s) => s.imageHeight);
  const zoom = useEditorStore((s) => s.zoom);
  const activeTool = useEditorStore((s) => s.activeTool);
  const brushRadius = useEditorStore((s) => s.brushRadius);
  const paintWhiteCircle = useEditorStore((s) => s.paintWhiteCircle);

  const [viewSize, setViewSize] = useState({ w: 0, h: 0 });
  const [pointerInner, setPointerInner] = useState<{
    x: number;
    y: number;
  } | null>(null);
  const isDrawingRef = useRef(false);

  useEffect(() => {
    const el = viewportRef.current;
    if (!el) return;
    const ro = new ResizeObserver(() => {
      setViewSize({ w: el.clientWidth, h: el.clientHeight });
    });
    ro.observe(el);
    setViewSize({ w: el.clientWidth, h: el.clientHeight });
    return () => ro.disconnect();
  }, []);

  const iw = imageWidth || 0;
  const ih = imageHeight || 0;
  const vw = viewSize.w;
  const vh = viewSize.h;

  const baseFit =
    iw > 0 && ih > 0 && vw > 0 && vh > 0
      ? Math.min(vw / iw, vh / ih)
      : 1;
  const scale = baseFit * (zoom / 100);
  const displayW = iw * scale;
  const displayH = ih * scale;

  useLayoutEffect(() => {
    const canvas = canvasRef.current;
    const ctx = canvas?.getContext("2d");
    if (!canvas || !ctx) return;

    if (!currentImageData) {
      canvas.width = 0;
      canvas.height = 0;
      return;
    }

    canvas.width = currentImageData.width;
    canvas.height = currentImageData.height;
    ctx.putImageData(currentImageData, 0, 0);
  }, [currentImageData]);

  const clientToImage = useCallback(
    (clientX: number, clientY: number) => {
      const canvas = canvasRef.current;
      if (!canvas || canvas.width === 0) return null;
      const rect = canvas.getBoundingClientRect();
      const sx = canvas.width / rect.width;
      const sy = canvas.height / rect.height;
      const x = (clientX - rect.left) * sx;
      const y = (clientY - rect.top) * sy;
      return { x, y };
    },
    []
  );

  const updatePointerFromEvent = useCallback(
    (e: React.PointerEvent | PointerEvent) => {
      const inner = innerRef.current;
      if (!inner) return;
      const r = inner.getBoundingClientRect();
      setPointerInner({
        x: e.clientX - r.left,
        y: e.clientY - r.top,
      });
    },
    []
  );

  const handlePointerDown = (e: React.PointerEvent<HTMLCanvasElement>) => {
    if (activeTool !== "correction") return;
    if (e.button !== 0) return;
    e.currentTarget.setPointerCapture(e.pointerId);
    isDrawingRef.current = true;
    updatePointerFromEvent(e);
    const img = clientToImage(e.clientX, e.clientY);
    if (img) paintWhiteCircle(img.x, img.y, brushRadius);
  };

  const handlePointerMove = (e: React.PointerEvent<HTMLCanvasElement>) => {
    if (activeTool === "correction") {
      updatePointerFromEvent(e);
      if (isDrawingRef.current) {
        const img = clientToImage(e.clientX, e.clientY);
        if (img) paintWhiteCircle(img.x, img.y, brushRadius);
      }
    }
  };

  const handlePointerUp = (e: React.PointerEvent<HTMLCanvasElement>) => {
    try {
      e.currentTarget.releasePointerCapture(e.pointerId);
    } catch {
      /* ignore */
    }
    isDrawingRef.current = false;
  };

  const handlePointerLeave = () => {
    setPointerInner(null);
  };

  const screenBrushRadius =
    iw > 0 && displayW > 0 ? brushRadius * (displayW / iw) : 0;

  const showBrush =
    activeTool === "correction" &&
    pointerInner !== null &&
    currentImageData &&
    screenBrushRadius > 0;

  return (
    <div className="image-canvas-viewport" ref={viewportRef}>
      {!currentImageData && (
        <div className="image-canvas-empty">
          <p>No image loaded</p>
          <p className="image-canvas-hint">Use Import to open a JPEG or PNG</p>
        </div>
      )}

      {currentImageData && (
        <div className="image-canvas-scroll">
          <div
            ref={innerRef}
            className="image-canvas-inner"
            style={{
              width: displayW,
              height: displayH,
              minWidth: displayW,
              minHeight: displayH,
            }}
          >
            <canvas
              ref={canvasRef}
              className={
                activeTool === "correction"
                  ? "image-canvas correction-mode"
                  : "image-canvas"
              }
              style={{
                width: displayW,
                height: displayH,
                display: "block",
                verticalAlign: "top",
              }}
              onPointerDown={handlePointerDown}
              onPointerMove={handlePointerMove}
              onPointerUp={handlePointerUp}
              onPointerCancel={handlePointerUp}
              onPointerLeave={handlePointerLeave}
            />
            {showBrush && pointerInner && (
              <div
                className="brush-preview"
                style={{
                  left: pointerInner.x - screenBrushRadius,
                  top: pointerInner.y - screenBrushRadius,
                  width: screenBrushRadius * 2,
                  height: screenBrushRadius * 2,
                }}
              />
            )}
          </div>
        </div>
      )}
    </div>
  );
};

export default ImageCanvas;
