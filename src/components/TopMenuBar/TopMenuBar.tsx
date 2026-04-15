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

  const [fileMenuOpen, setFileMenuOpen] = useState(false);
  const fileMenuRef = useRef<HTMLDivElement>(null);

  const draftRgb = useEditorStore((s) => s.draftRgb);
  const committedRgb = useEditorStore((s) => s.committedRgb);
  const isDirty = useEditorStore((s) => s.isDirty);

  const hasDraftImage = Boolean(draftRgb);
  const canSave = Boolean(committedRgb) && !isDirty;

  const activeTool = useEditorStore((s) => s.activeTool);
  const threshold = useEditorStore((s) => s.threshold);
  const brushRadius = useEditorStore((s) => s.brushRadius);
  const zoom = useEditorStore((s) => s.zoom);

  const hwPath = useEditorStore((s) => s.hwPath);
  const setImageFromImport = useEditorStore((s) => s.setImageFromImport);
  const openHw = useEditorStore((s) => s.openHw);
  const setActiveTool = useEditorStore((s) => s.setActiveTool);
  const tick = useEditorStore((s) => s.tick);
  const cross = useEditorStore((s) => s.cross);
  const setThreshold = useEditorStore((s) => s.setThreshold);
  const setBrushRadius = useEditorStore((s) => s.setBrushRadius);
  const setZoom = useEditorStore((s) => s.setZoom);
  const markSaved = useEditorStore((s) => s.markSaved);

  const committedBinary = useEditorStore((s) => s.committedBinary);
  const committedThreshold = useEditorStore((s) => s.committedThreshold);

  // ImageData -> base64 (RGBA bytes). Chunking avoids argument length limits.
  const encodeRgbaImageDataToBase64 = (img: ImageData): string => {
    const bytes = img.data;
    let binary = "";
    // Keep chunk small to avoid `String.fromCharCode(...chunk)` argument limits.
    const chunkSize = 0x1000;
    for (let i = 0; i < bytes.length; i += chunkSize) {
      const chunk = bytes.subarray(i, i + chunkSize);
      binary += String.fromCharCode(...chunk);
    }
    return btoa(binary);
  };

  useEffect(() => {
    const onDoc = (e: MouseEvent) => {
      if (!toolsRef.current?.contains(e.target as Node)) {
        setToolsOpen(false);
      }
      if (!fileMenuRef.current?.contains(e.target as Node)) {
        setFileMenuOpen(false);
      }
    };
    document.addEventListener("mousedown", onDoc);
    return () => document.removeEventListener("mousedown", onDoc);
  }, []);

  const handleOpenHw = useCallback(async () => {
    try {
      const filePath = await window.electronAPI.selectHwFile();
      if (!filePath) return;
      const payload = await window.electronAPI.readHwFile(filePath);
      if (!payload) return;
      openHw(filePath, payload);
      setFileMenuOpen(false);
      setToolsOpen(false);
    } catch (e) {
      logger.error(
        "TopMenuBar.tsx",
        "handleOpenHw",
        (e as Error).message,
        e as Error
      );
    }
  }, [openHw]);

  const handleImport = useCallback(async () => {
    try {
      const path = await window.electronAPI.selectImageFile();
      if (!path) return;
      const data = await loadImageDataFromPath(path);
      setImageFromImport(path, data);
      setFileMenuOpen(false);
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
    if (activeTool === "erase") return "Eraser";
    return "None";
  };

  const writeHwToPath = useCallback(
    async (targetPath: string) => {
      const rgb = committedRgb;
      const binary = committedBinary;
      if (!rgb || !binary) return;

      const payload = {
        width: rgb.width,
        height: rgb.height,
        threshold: committedThreshold,
        rgbRgbaBase64: encodeRgbaImageDataToBase64(rgb),
        binaryRgbaBase64: encodeRgbaImageDataToBase64(binary),
      };

      await window.electronAPI.writeHwFile(targetPath, payload);
      markSaved(targetPath);
    },
    [committedRgb, committedBinary, committedThreshold, markSaved]
  );

  const handleSave = useCallback(async () => {
    if (!canSave) return;
    try {
      if (hwPath) {
        await writeHwToPath(hwPath);
        return;
      }
      const targetPath = await window.electronAPI.selectSaveHwFile();
      if (!targetPath) return;
      await writeHwToPath(targetPath);
    } catch (e) {
      logger.error("TopMenuBar.tsx", "handleSave", (e as Error).message, e as Error);
    } finally {
      setFileMenuOpen(false);
    }
  }, [canSave, hwPath, writeHwToPath]);

  const handleSaveAs = useCallback(async () => {
    if (!canSave) return;
    try {
      const targetPath = await window.electronAPI.selectSaveHwFile();
      if (!targetPath) return;
      await writeHwToPath(targetPath);
    } catch (e) {
      logger.error(
        "TopMenuBar.tsx",
        "handleSaveAs",
        (e as Error).message,
        e as Error
      );
    } finally {
      setFileMenuOpen(false);
    }
  }, [canSave, writeHwToPath]);

  return (
    <div className="top-menu-bar">
      <div className="tools-dropdown" ref={fileMenuRef}>
        <button
          type="button"
          className="top-menu-btn primary"
          aria-expanded={fileMenuOpen}
          onClick={() => setFileMenuOpen((o) => !o)}
        >
          File ▾
        </button>
        {fileMenuOpen && (
          <ul className="tools-menu" role="menu">
            <li>
              <button type="button" role="menuitem" onClick={() => void handleOpenHw()}>
                Open (.hw)
              </button>
            </li>
            <li>
              <button type="button" role="menuitem" onClick={() => void handleImport()}>
                Import image...
              </button>
            </li>
            <li>
              <button type="button" role="menuitem" disabled={!canSave} onClick={() => void handleSave()}>
                Save
              </button>
            </li>
            <li>
              <button type="button" role="menuitem" disabled={!canSave} onClick={() => void handleSaveAs()}>
                Save As...
              </button>
            </li>
          </ul>
        )}
      </div>

      <div className="tools-dropdown" ref={toolsRef}>
        <button
          type="button"
          className="top-menu-btn"
          disabled={!hasDraftImage}
          aria-expanded={toolsOpen}
          onClick={() => hasDraftImage && setToolsOpen((o) => !o)}
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
                  setActiveTool("none");
                  setToolsOpen(false);
                }}
              >
                None
              </button>
            </li>
            <li>
              <button
                type="button"
                role="menuitem"
                onClick={() => {
                  setActiveTool("erase");
                  setToolsOpen(false);
                }}
              >
                Erase (circle tool)
              </button>
            </li>
          </ul>
        )}
      </div>

      <label className="top-menu-label">
        Radius
        <input
          type="number"
          className="top-menu-input"
          min={1}
          max={500}
          step={1}
          value={Number.isFinite(brushRadius) ? brushRadius : ""}
          disabled={activeTool !== "erase"}
          onChange={(e) => setBrushRadius(parseFloat(e.target.value))}
        />
        <span className="top-menu-unit">px</span>
      </label>

      <label className="top-menu-label">
        Threshold
        <input
          type="number"
          className="top-menu-input"
          min={0}
          max={100}
          step={1}
          value={Number.isFinite(threshold) ? threshold : ""}
          onChange={(e) => setThreshold(parseFloat(e.target.value))}
        />
        <span className="top-menu-unit">%</span>
      </label>

      {hasDraftImage && (
        <>
          <button
            type="button"
            className="top-menu-btn primary"
            disabled={!isDirty}
            onClick={() => tick()}
          >
            Tick
          </button>
          <button
            type="button"
            className="top-menu-btn"
            disabled={!isDirty}
            onClick={() => cross()}
          >
            Cross
          </button>
        </>
      )}

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
