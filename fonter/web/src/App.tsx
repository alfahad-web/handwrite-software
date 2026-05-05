import {
  useCallback,
  useEffect,
  useMemo,
  useReducer,
  useRef,
  useState,
} from "react";
import "./App.css";
import { BoardCanvas } from "./components/BoardCanvas";
import {
  assignSelectionCharacter,
  deleteSelectedSelectionWithMessage,
} from "./logic/appActions";
import { EditorStore } from "./logic/editorStore";
import { generateFontsZip } from "./logic/fontGenerator";
import {
  loadProjectFromJson,
  saveProjectToJson,
} from "./logic/projectService";
import { resolveScreenDpi } from "./logic/exportService";

function useEditorStore(): EditorStore {
  return useMemo(() => new EditorStore(), []);
}

export default function App() {
  const store = useEditorStore();
  const [, rerender] = useReducer((n: number) => n + 1, 0);
  const [statusMessage, setStatusMessage] = useState("Ready");
  const [cachedFontZip, setCachedFontZip] = useState<Blob | null>(null);
  const [fileMenuOpen, setFileMenuOpen] = useState(false);
  const [assignOpen, setAssignOpen] = useState(false);
  const [assignChar, setAssignChar] = useState("");
  const [assignJoin, setAssignJoin] = useState("N");
  const pendingAssignIdRef = useRef("");
  const openInputRef = useRef<HTMLInputElement>(null);
  const uploadInputRef = useRef<HTMLInputElement>(null);

  useEffect(() => {
    return store.subscribe(() => rerender());
  }, [store]);

  const dpi = useMemo(
    () =>
      typeof window !== "undefined"
        ? resolveScreenDpi(window.devicePixelRatio || 1)
        : 96,
    [],
  );

  const setStatus = useCallback((msg: string) => {
    setStatusMessage(msg);
  }, []);

  const downloadBlob = useCallback((blob: Blob, fileName: string) => {
    const a = document.createElement("a");
    a.href = URL.createObjectURL(blob);
    a.download = fileName;
    a.click();
    URL.revokeObjectURL(a.href);
  }, []);

  const handleNew = () => {
    store.clearAll();
    store.clearProjectFilePath();
    setCachedFontZip(null);
    setFileMenuOpen(false);
    setStatus("New board (unsaved).");
  };

  const handleDownloadProject = () => {
    const name =
      store.projectFileName().trim() || "project.hw";
    const json = saveProjectToJson(store);
    const blob = new Blob([json], { type: "application/json" });
    downloadBlob(blob, name.endsWith(".hw") ? name : `${name}.hw`);
    if (!store.projectFilePath()) {
      store.setProjectFilePath(name.endsWith(".hw") ? name : `${name}.hw`);
    }
    store.markSaved();
    setFileMenuOpen(false);
    setStatus("Project downloaded.");
  };

  const readHwFile = (file: File) => {
    const reader = new FileReader();
    reader.onload = () => {
      const text = String(reader.result ?? "");
      const path = file.name;
      const r = loadProjectFromJson(store, text, path);
      if (!r.ok) {
        setStatus(`Open failed: ${r.error}`);
        return;
      }
      setCachedFontZip(null);
      setStatus("Project loaded.");
    };
    reader.readAsText(file);
  };

  const handleGenerate = async () => {
    const r = await generateFontsZip(store, dpi);
    setStatus(r.message);
    if (r.ok && r.blob) {
      setCachedFontZip(r.blob);
      setStatus("Fonts generated and cached. Use Save Fonts to download.");
    }
  };

  const handleSaveFonts = () => {
    if (!cachedFontZip) {
      setStatus("Generate fonts first.");
      setFileMenuOpen(false);
      return;
    }
    downloadBlob(cachedFontZip, "font.zip");
    setFileMenuOpen(false);
    setStatus("Font folder downloaded as font.zip.");
  };

  const openAssignFor = (selectionId: string) => {
    pendingAssignIdRef.current = selectionId;
    setAssignJoin("N");
    setAssignChar("");
    const rows = store.selectionBoxesModel();
    const row = rows.find((x) => x.id === selectionId);
    setAssignJoin(row?.joinMode ?? "N");
    if (row?.assigned && row.assignedAscii >= 0) {
      setAssignChar(String.fromCharCode(row.assignedAscii));
    } else {
      setAssignChar("");
    }
    setAssignOpen(true);
  };

  const submitAssign = () => {
    const msg = assignSelectionCharacter(
      store,
      pendingAssignIdRef.current,
      assignChar,
      assignJoin,
    );
    setStatus(msg);
    setAssignOpen(false);
  };

  const fileLabel =
    store.projectFileName().length > 0
      ? store.projectFileName()
      : "File";

  return (
    <div className="app-root">
      <div className="narrow-banner">
        Minimum width 768px — resize for the full toolbar.
      </div>
      <header className="app-header">
        <div className="header-inner">
          <label className="field">
            <span>Stroke</span>
            <input
              type="number"
              min={1}
              max={200}
              value={store.strokePx()}
              onChange={(e) =>
                store.setStrokePx(Number(e.target.value) || 1)
              }
            />
          </label>
          <label className="field">
            <span>Gap (um)</span>
            <input
              type="number"
              min={1}
              max={200000}
              value={store.captureGapUm()}
              onChange={(e) =>
                store.setCaptureGapUm(Number(e.target.value) || 1)
              }
            />
          </label>
          <label className="field">
            <span>Guide gap (px)</span>
            <input
              type="number"
              min={10}
              max={1000}
              value={store.guideLineGapPx()}
              onChange={(e) =>
                store.setGuideLineGapPx(Number(e.target.value) || 10)
              }
            />
          </label>
          <div className="file-wrap">
            <button
              type="button"
              className="btn"
              onClick={(e) => {
                e.stopPropagation();
                setFileMenuOpen((o) => !o);
              }}
            >
              {fileLabel}
            </button>
            {fileMenuOpen && (
              <div
                className="file-menu"
                onClick={(e) => e.stopPropagation()}
              >
                <button type="button" onClick={handleNew}>
                  New
                </button>
                <button
                  type="button"
                  onClick={() => {
                    setFileMenuOpen(false);
                    queueMicrotask(() => openInputRef.current?.click());
                  }}
                >
                  Open
                </button>
                <button
                  type="button"
                  onClick={() => {
                    setFileMenuOpen(false);
                    queueMicrotask(() => uploadInputRef.current?.click());
                  }}
                >
                  Upload
                </button>
                <button type="button" onClick={handleSaveFonts}>
                  Save Fonts
                </button>
                <button type="button" onClick={handleDownloadProject}>
                  Download
                </button>
              </div>
            )}
          </div>
          <input
            ref={openInputRef}
            type="file"
            accept=".hw,application/json,.json"
            className="hidden-input"
            onChange={(e) => {
              const f = e.target.files?.[0];
              e.target.value = "";
              if (f) readHwFile(f);
            }}
          />
          <input
            ref={uploadInputRef}
            type="file"
            accept=".hw,application/json,.json"
            className="hidden-input"
            onChange={(e) => {
              const f = e.target.files?.[0];
              e.target.value = "";
              if (f) readHwFile(f);
            }}
          />
          <div className="zoom">
            <button
              type="button"
              className="btn"
              onClick={() => store.zoomOut()}
            >
              −
            </button>
            <input
              type="number"
              min={10}
              max={800}
              step={10}
              value={store.zoom()}
              onChange={(e) =>
                store.setZoom(Number(e.target.value) || 100)
              }
            />
            <button
              type="button"
              className="btn"
              onClick={() => store.zoomIn()}
            >
              +
            </button>
          </div>
          <button
            type="button"
            className={`btn ${store.toolMode() === "select" ? "active" : ""}`}
            onClick={() => store.setToolMode("select")}
          >
            {store.toolMode() === "select" ? "Selection On" : "Selection Off"}
          </button>
          <button
            type="button"
            className={`btn ${store.toolMode() === "draw" ? "active" : ""}`}
            onClick={() => store.setToolMode("draw")}
          >
            {store.toolMode() === "draw" ? "Draw On" : "Draw"}
          </button>
          <button
            type="button"
            className={`btn ${
              store.toolMode() === "draw" && store.drawStrokeEraseActive()
                ? "active"
                : ""
            }`}
            onClick={() => {
              if (store.toolMode() === "draw") {
                store.setDrawStrokeEraseActive(!store.drawStrokeEraseActive());
              } else {
                store.setToolMode("erase");
              }
            }}
          >
            {store.toolMode() === "draw" && store.drawStrokeEraseActive()
              ? "Erase On"
              : "Erase"}
          </button>
          <label className="field">
            <span>Erase r(px)</span>
            <input
              type="number"
              min={1}
              max={500}
              value={store.eraseRadiusPx()}
              onChange={(e) =>
                store.setEraseRadiusPx(Number(e.target.value) || 1)
              }
            />
          </label>
          <button
            type="button"
            className="btn"
            disabled={!store.hasSelectedSelection()}
            onClick={() =>
              setStatus(deleteSelectedSelectionWithMessage(store))
            }
          >
            Delete Sel
          </button>
          <button type="button" className="btn" onClick={handleGenerate}>
            Generate Fonts
          </button>
          <span className="dirty-label">
            {store.isDirty() ? "Unsaved board" : "Saved board"}
          </span>
        </div>
      </header>

      <main className="app-main">
        <BoardCanvas
          store={store}
          dpi={dpi}
          onSelectionDoubleClick={openAssignFor}
        />
      </main>

      <footer className="app-footer">
        {statusMessage || "Ready"}
      </footer>

      {assignOpen && (
        <div
          className="modal-backdrop"
          role="presentation"
          onClick={() => setAssignOpen(false)}
        >
          <div
            className="modal"
            role="dialog"
            aria-labelledby="assign-title"
            onClick={(e) => e.stopPropagation()}
          >
            <h2 id="assign-title">Assign Character</h2>
            <p>Enter exactly one ASCII character:</p>
            <input
              className="text-input"
              value={assignChar}
              maxLength={1}
              onChange={(e) => setAssignChar(e.target.value)}
              placeholder="Example: A or #"
            />
            <p>Join mode (export filename token):</p>
            <div className="radio-row">
              {(["L", "R", "LR", "N"] as const).map((j) => (
                <label key={j} className="radio">
                  <input
                    type="radio"
                    name="join"
                    checked={assignJoin === j}
                    onChange={() => setAssignJoin(j)}
                  />
                  {j}
                </label>
              ))}
            </div>
            <div className="modal-actions">
              <button type="button" onClick={() => setAssignOpen(false)}>
                Cancel
              </button>
              <button type="button" onClick={submitAssign}>
                OK
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
