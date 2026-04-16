import React, { useCallback } from "react";
import { useEditorStore } from "../../context/index.ts";
import { buildExportStrokes, serializeExportLines } from "../../utils/exportSelection.ts";
import { logger } from "../../utils/logger.ts";
import { resolveScreenDpi } from "../../utils/units.ts";
import "./TopMenuBar.css";

const TopMenuBar: React.FC = () => {
  const strokes = useEditorStore((s) => s.strokes);
  const selectionRect = useEditorStore((s) => s.selectionRect);
  const openFilePath = useEditorStore((s) => s.openFilePath);
  const openFileName = useEditorStore((s) => s.openFileName);
  const strokePx = useEditorStore((s) => s.strokePx);
  const captureGapUm = useEditorStore((s) => s.captureGapUm);
  const zoom = useEditorStore((s) => s.zoom);
  const toolMode = useEditorStore((s) => s.toolMode);
  const isDirty = useEditorStore((s) => s.isDirty);
  const setToolMode = useEditorStore((s) => s.setToolMode);
  const setStrokePx = useEditorStore((s) => s.setStrokePx);
  const setCaptureGapUm = useEditorStore((s) => s.setCaptureGapUm);
  const setZoom = useEditorStore((s) => s.setZoom);
  const zoomIn = useEditorStore((s) => s.zoomIn);
  const zoomOut = useEditorStore((s) => s.zoomOut);
  const setOpenFile = useEditorStore((s) => s.setOpenFile);
  const markSaved = useEditorStore((s) => s.markSaved);
  const closeFile = useEditorStore((s) => s.closeFile);

  const canWriteSelection = Boolean(openFilePath && selectionRect && strokes.length > 0);

  const handleFileDialog = useCallback(async () => {
    try {
      const api = window.electronAPI;
      if (!api?.selectOrCreateTxtFile) {
        throw new Error("Electron preload bridge unavailable");
      }
      const targetPath = await api.selectOrCreateTxtFile();
      if (!targetPath) return;
      setOpenFile(targetPath);
    } catch (e) {
      logger.error("TopMenuBar.tsx", "handleFileDialog", (e as Error).message, e as Error);
    }
  }, [setOpenFile]);

  const writeSelection = useCallback(async (): Promise<boolean> => {
    if (!openFilePath || !selectionRect) return false;
    try {
      const api = window.electronAPI;
      if (!api?.appendTxtLines) {
        throw new Error("Electron preload bridge unavailable");
      }
      const dpi = resolveScreenDpi();
      const exported = buildExportStrokes(strokes, selectionRect, captureGapUm, dpi);
      const lines = serializeExportLines(exported);
      if (lines.length === 0) return false;
      await api.appendTxtLines(openFilePath, lines);
      markSaved();
      return true;
    } catch (e) {
      logger.error("TopMenuBar.tsx", "writeSelection", (e as Error).message, e as Error);
      return false;
    }
  }, [openFilePath, selectionRect, strokes, captureGapUm, markSaved]);

  const handleTick = useCallback(async () => {
    await writeSelection();
  }, [writeSelection]);

  const handleCross = useCallback(async () => {
    if (!openFilePath) return;
    await writeSelection();
    closeFile();
  }, [openFilePath, writeSelection, closeFile]);

  return (
    <div className="top-menu-bar">
      <label className="top-menu-label">
        Stroke
        <input
          type="number"
          className="top-menu-input"
          min={1}
          max={200}
          step={1}
          value={Number.isFinite(strokePx) ? strokePx : ""}
          onChange={(e) => setStrokePx(parseFloat(e.target.value))}
        />
        <span className="top-menu-unit">px</span>
      </label>

      <label className="top-menu-label">
        Point capture gap
        <input
          type="number"
          className="top-menu-input"
          min={1}
          max={200000}
          step={10}
          value={Number.isFinite(captureGapUm) ? captureGapUm : ""}
          onChange={(e) => setCaptureGapUm(parseFloat(e.target.value))}
        />
        <span className="top-menu-unit">um</span>
      </label>

      <button type="button" className="top-menu-btn file-btn" onClick={() => void handleFileDialog()}>
        {openFileName ?? "Open/Create .txt"}
      </button>

      <button
        type="button"
        className="top-menu-btn primary"
        disabled={!canWriteSelection}
        onClick={() => void handleTick()}
        title="Save selected points to open file"
      >
        ✓
      </button>
      <button
        type="button"
        className="top-menu-btn"
        disabled={!openFilePath}
        onClick={() => void handleCross()}
        title="Save and close file"
      >
        ✕
      </button>

      <div className="zoom-controls">
        <button type="button" className="top-menu-btn" onClick={() => zoomOut()}>
          -
        </button>
        <input
          type="number"
          className="top-menu-input zoom-input"
          min={10}
          max={800}
          step={10}
          value={zoom}
          onChange={(e) => setZoom(parseFloat(e.target.value))}
        />
        <button type="button" className="top-menu-btn" onClick={() => zoomIn()}>
          +
        </button>
      </div>

      <button
        type="button"
        className={`top-menu-btn ${toolMode === "select" ? "primary" : ""}`}
        onClick={() => setToolMode(toolMode === "select" ? "draw" : "select")}
      >
        Selection
      </button>
      <span className="status-pill">{isDirty ? "Unsaved board" : "Saved board"}</span>
    </div>
  );
};

export default TopMenuBar;
