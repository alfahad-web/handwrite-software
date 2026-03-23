import React, { useCallback, useEffect, useRef, useState } from "react";
import { useEditorStore } from "../../context/index.ts";
import { logger } from "../../utils/logger.ts";
import "./TopMenuBar.css";

async function loadImageDataFromPath(filePath: string): Promise<ImageData> {
  const res = await window.electronAPI.readImageFile(filePath);
  if (!res?.base64) {
    throw new Error("Could not read image file");
  }
  const raw = atob(res.base64);
  const bytes = new Uint8Array(raw.length);
  for (let i = 0; i < raw.length; i++) {
    bytes[i] = raw.charCodeAt(i);
  }
  const blob = new Blob([bytes], { type: "image/*" });
  const bmp = await createImageBitmap(blob);
  const canvas = document.createElement("canvas");
  canvas.width = bmp.width;
  canvas.height = bmp.height;
  const ctx = canvas.getContext("2d");
  if (!ctx) throw new Error("2D context unavailable");
  ctx.drawImage(bmp, 0, 0);
  return ctx.getImageData(0, 0, canvas.width, canvas.height);
}

const TopMenuBar: React.FC = () => {
  const [toolsOpen, setToolsOpen] = useState(false);
  const toolsRef = useRef<HTMLDivElement>(null);

  const imagePath = useEditorStore((s) => s.imagePath);
  const hasImage = Boolean(imagePath);
  const activeTool = useEditorStore((s) => s.activeTool);
  const threshold = useEditorStore((s) => s.threshold);
  const brushRadius = useEditorStore((s) => s.brushRadius);
  const zoom = useEditorStore((s) => s.zoom);

  const setImageFromImport = useEditorStore((s) => s.setImageFromImport);
  const setActiveTool = useEditorStore((s) => s.setActiveTool);
  const setThreshold = useEditorStore((s) => s.setThreshold);
  const setBrushRadius = useEditorStore((s) => s.setBrushRadius);
  const setZoom = useEditorStore((s) => s.setZoom);
  const applyBinaryConversion = useEditorStore((s) => s.applyBinaryConversion);

  useEffect(() => {
    const onDoc = (e: MouseEvent) => {
      if (!toolsRef.current?.contains(e.target as Node)) {
        setToolsOpen(false);
      }
    };
    document.addEventListener("mousedown", onDoc);
    return () => document.removeEventListener("mousedown", onDoc);
  }, []);

  const handleImport = useCallback(async () => {
    try {
      const path = await window.electronAPI.selectImageFile();
      if (!path) return;
      const data = await loadImageDataFromPath(path);
      setImageFromImport(path, data);
    } catch (e) {
      logger.error(
        "TopMenuBar.tsx",
        "handleImport",
        (e as Error).message,
        e as Error
      );
    }
  }, [setImageFromImport]);

  const toolSummary = () => {
    if (activeTool === "correction") return "Manual correction";
    if (activeTool === "binary") return "Binary";
    return "None";
  };

  const paramLabel = activeTool === "correction" ? "Radius" : "Threshold";
  const paramValue = activeTool === "correction" ? brushRadius : threshold;
  const paramSuffix = activeTool === "correction" ? "px" : "%";

  const onParamChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const n = parseFloat(e.target.value);
    if (activeTool === "correction") {
      setBrushRadius(n);
    } else {
      setThreshold(n);
    }
  };

  return (
    <div className="top-menu-bar">
      <button
        type="button"
        className="top-menu-btn primary"
        onClick={() => void handleImport()}
      >
        Import
      </button>

      <div className="tools-dropdown" ref={toolsRef}>
        <button
          type="button"
          className="top-menu-btn"
          disabled={!hasImage}
          aria-expanded={toolsOpen}
          onClick={() => hasImage && setToolsOpen((o) => !o)}
        >
          Tools ▾ <span className="tools-current">{toolSummary()}</span>
        </button>
        {toolsOpen && (
          <ul className="tools-menu" role="menu">
            <li>
              <button
                type="button"
                role="menuitem"
                onClick={() => {
                  applyBinaryConversion();
                  setToolsOpen(false);
                }}
              >
                Convert to binary
              </button>
            </li>
            <li>
              <button
                type="button"
                role="menuitem"
                onClick={() => {
                  setActiveTool("correction");
                  setToolsOpen(false);
                }}
              >
                Manual correction
              </button>
            </li>
            <li>
              <button
                type="button"
                role="menuitem"
                onClick={() => {
                  setActiveTool("none");
                  setToolsOpen(false);
                }}
              >
                None
              </button>
            </li>
          </ul>
        )}
      </div>

      <label className="top-menu-label">
        {paramLabel}
        <input
          type="number"
          className="top-menu-input"
          min={activeTool === "correction" ? 1 : 0}
          max={activeTool === "correction" ? 500 : 100}
          step={1}
          value={Number.isFinite(paramValue) ? paramValue : ""}
          onChange={onParamChange}
        />
        <span className="top-menu-unit">{paramSuffix}</span>
      </label>

      <label className="top-menu-label">
        Zoom
        <input
          type="number"
          className="top-menu-input"
          min={10}
          max={500}
          step={5}
          value={zoom}
          onChange={(e) => setZoom(parseFloat(e.target.value))}
        />
        <span className="top-menu-unit">%</span>
      </label>
    </div>
  );
};

export default TopMenuBar;
