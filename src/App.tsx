import React, { useCallback, useEffect } from "react";
import TitleBar from "./components/TitleBar/TitleBar.tsx";
import TopMenuBar from "./components/TopMenuBar/TopMenuBar.tsx";
import ImageCanvas from "./components/ImageCanvas/ImageCanvas.tsx";
import { useEditorStore } from "./context/index.ts";
import "./App.css";

const App: React.FC = () => {
  const isLinux = window.electronAPI?.platform === "linux";

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
      {!isLinux && <TitleBar />}
      <TopMenuBar />
      <main className="main-content">
        <ImageCanvas />
      </main>
    </div>
  );
};

export default App;
