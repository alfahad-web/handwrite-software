import { create } from "zustand";
import type { ActiveTool } from "./types.ts";
import {
  cloneImageData,
  convertToBinary,
  paintWhiteCircle as paintCircleOnData,
} from "../utils/imageProcessing.ts";

export interface EditorStore {
  imagePath: string | null;
  originalImageData: ImageData | null;
  currentImageData: ImageData | null;
  imageWidth: number;
  imageHeight: number;
  activeTool: ActiveTool;
  threshold: number;
  brushRadius: number;
  zoom: number;

  setImageFromImport: (path: string, data: ImageData) => void;
  setActiveTool: (tool: ActiveTool) => void;
  setThreshold: (value: number) => void;
  setBrushRadius: (value: number) => void;
  setZoom: (value: number) => void;
  applyBinaryConversion: () => void;
  paintWhiteCircle: (cx: number, cy: number, radius: number) => void;
  clearImage: () => void;
}

function clamp(n: number, min: number, max: number): number {
  return Math.min(max, Math.max(min, n));
}

export const useEditorStore = create<EditorStore>((set, get) => ({
  imagePath: null,
  originalImageData: null,
  currentImageData: null,
  imageWidth: 0,
  imageHeight: 0,
  activeTool: "none",
  threshold: 50,
  brushRadius: 10,
  zoom: 100,

  setImageFromImport: (path, data) => {
    const orig = cloneImageData(data);
    const cur = cloneImageData(data);
    set({
      imagePath: path,
      originalImageData: orig,
      currentImageData: cur,
      imageWidth: data.width,
      imageHeight: data.height,
      activeTool: "none",
    });
  },

  setActiveTool: (tool) => set({ activeTool: tool }),

  setThreshold: (value) =>
    set({ threshold: clamp(Number.isFinite(value) ? value : 50, 0, 100) }),

  setBrushRadius: (value) =>
    set({
      brushRadius: Math.max(
        1,
        Math.floor(Number.isFinite(value) ? value : 10)
      ),
    }),

  setZoom: (value) =>
    set({
      zoom: clamp(Number.isFinite(value) ? value : 100, 10, 500),
    }),

  applyBinaryConversion: () => {
    const { currentImageData, threshold } = get();
    if (!currentImageData) return;
    const next = convertToBinary(currentImageData, threshold);
    set({ currentImageData: next, activeTool: "binary" });
  },

  paintWhiteCircle: (cx, cy, radius) => {
    const { currentImageData } = get();
    if (!currentImageData) return;
    const next = cloneImageData(currentImageData);
    paintCircleOnData(next, cx, cy, radius);
    set({ currentImageData: next });
  },

  clearImage: () =>
    set({
      imagePath: null,
      originalImageData: null,
      currentImageData: null,
      imageWidth: 0,
      imageHeight: 0,
      activeTool: "none",
    }),
}));
