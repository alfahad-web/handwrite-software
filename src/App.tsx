import React, { useCallback, useEffect } from "react";
import TitleBar from "./components/TitleBar/TitleBar.tsx";
import TopMenuBar from "./components/TopMenuBar/TopMenuBar.tsx";
import ImageCanvas from "./components/ImageCanvas/ImageCanvas.tsx";
import { useEditorStore } from "./context/index.ts";
import "./App.css";

const App: React.FC = () => {
  const handleAppCloseRequested = useCallback(async () => {
    const stateBefore = useEditorStore.getState();
    const mustPrompt = stateBefore.isDirty || stateBefore.needsSave;

    if (!mustPrompt) {
      await window.electronAPI.confirmClose();
      return;
    }

    const shouldSave = window.confirm(
      "Unsaved changes detected. Press OK to Save As before exit, or Cancel to keep the app open."
    );
    if (!shouldSave) return;

    if (stateBefore.isDirty) {
      useEditorStore.getState().tick();
    }

    const state = useEditorStore.getState();
    const rgb = state.committedRgb;
    const binary = state.committedBinary;
    if (!rgb || !binary) return;

    const encodeRgbaImageDataToBase64 = (img: ImageData): string => {
      const bytes = img.data;
      let binaryStr = "";
      const chunkSize = 0x1000;
      for (let i = 0; i < bytes.length; i += chunkSize) {
        const chunk = bytes.subarray(i, i + chunkSize);
        binaryStr += String.fromCharCode(...chunk);
      }
      return btoa(binaryStr);
    };

    const targetPath = await window.electronAPI.selectSaveHwFile();
    if (!targetPath) return;

    const payload = {
      width: rgb.width,
      height: rgb.height,
      threshold: state.committedThreshold,
      rgbRgbaBase64: encodeRgbaImageDataToBase64(rgb),
      binaryRgbaBase64: encodeRgbaImageDataToBase64(binary),
    };

    await window.electronAPI.writeHwFile(targetPath, payload);
    useEditorStore.getState().markSaved(targetPath);
    await window.electronAPI.confirmClose();
  }, []);

  useEffect(() => {
    return window.electronAPI.onAppCloseRequested(() => {
      void handleAppCloseRequested();
    });
  }, [handleAppCloseRequested]);

  return (
    <div className="app">
      <TitleBar />
      <TopMenuBar />
      <main className="main-content">
        <ImageCanvas />
      </main>
    </div>
  );
};

export default App;
