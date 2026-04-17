import React, { useCallback, useEffect } from "react";
import TitleBar from "./components/TitleBar/TitleBar.tsx";
import TopMenuBar from "./components/TopMenuBar/TopMenuBar.tsx";
import ImageCanvas from "./components/ImageCanvas/ImageCanvas.tsx";
import { useEditorStore } from "./context/index.ts";
import "./App.css";

const LINUX_ELECTRON_ATTR = "data-handwrite-linux-electron";

/** True when running under Electron on Linux (preload platform or UA fallback if preload glitches). */
function isLinuxElectronClient(): boolean {
  const p = window.electronAPI?.platform;
  if (p === "linux") return true;
  if (p === "win32" || p === "darwin") return false;
  if (typeof navigator === "undefined") return false;
  return /linux/i.test(navigator.userAgent) && /electron/i.test(navigator.userAgent);
}

const App: React.FC = () => {
  const hideCustomTitleBar = isLinuxElectronClient();

  useEffect(() => {
    if (!isLinuxElectronClient()) return;
    document.documentElement.setAttribute(LINUX_ELECTRON_ATTR, "1");
    return () => document.documentElement.removeAttribute(LINUX_ELECTRON_ATTR);
  }, []);

  const handleAppCloseRequested = useCallback(async () => {
    const api = window.electronAPI;
    if (!api?.confirmClose) return;

    const stateBefore = useEditorStore.getState();
    const mustPrompt = stateBefore.isDirty;

    if (!mustPrompt) {
      await api.confirmClose();
      return;
    }

    const shouldSave = window.confirm(
      "Unsaved whiteboard changes detected. Press OK to close anyway, or Cancel to continue editing."
    );
    if (!shouldSave) return;
    await api.confirmClose();
  }, []);

  useEffect(() => {
    const api = window.electronAPI;
    if (!api?.onAppCloseRequested) return;
    return api.onAppCloseRequested(() => {
      void handleAppCloseRequested();
    });
  }, [handleAppCloseRequested]);

  return (
    <div className="app">
      {!hideCustomTitleBar && <TitleBar />}
      <TopMenuBar />
      <main className="main-content">
        <ImageCanvas />
      </main>
    </div>
  );
};

export default App;
