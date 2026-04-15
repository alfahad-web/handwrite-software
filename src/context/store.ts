import { create } from "zustand";
import type { ActiveTool } from "./types.ts";
import {
  cloneImageData,
  convertToBinary,
  paintWhiteCircle as paintCircleOnData,
} from "../utils/imageProcessing.ts";

export interface EditorStore {
  // Last opened/saved .hw path (used by Save / Save As)
  hwPath: string | null;

  // Committed state = what Save persists (after `tick`)
  committedRgb: ImageData | null;
  committedBinary: ImageData | null;
  committedThreshold: number;

  // Draft state = pending changes (after Import and/or Erase, before `tick`)
  draftRgb: ImageData | null;
  draftBinary: ImageData | null;
  threshold: number; // draft-only

  // Used to lazily regenerate draftBinary when draftRgb/threshold changes.
  draftRgbVersion: number;
  draftBinarySourceVersion: number | null;

  isDirty: boolean;
  needsSave: boolean;

  imageWidth: number;
  imageHeight: number;
  activeTool: ActiveTool;
  brushRadius: number;
  zoom: number;

  setImageFromImport: (path: string, data: ImageData) => void;
  openHw: (filePath: string, payload: HwOpenPayload) => void;

  setActiveTool: (tool: ActiveTool) => void;

  tick: () => void;
  cross: () => void;

  ensureDraftBinary: () => void;

  setThreshold: (value: number) => void;
  setBrushRadius: (value: number) => void;
  setZoom: (value: number) => void;

  // Erase modifies draftBinary only.
  paintWhiteCircle: (cx: number, cy: number, radius: number) => void;
  clearImage: () => void;
  markSaved: (path: string) => void;
}

export type HwOpenPayload = {
  width: number;
  height: number;
  threshold: number;
  rgbRgbaBase64: string;
  binaryRgbaBase64: string;
};

function clamp(n: number, min: number, max: number): number {
  return Math.min(max, Math.max(min, n));
}

export const useEditorStore = create<EditorStore>((set, get) => ({
  hwPath: null,

  committedRgb: null,
  committedBinary: null,
  committedThreshold: 50,

  draftRgb: null,
  draftBinary: null,
  threshold: 50,

  draftRgbVersion: 0,
  draftBinarySourceVersion: null,

  isDirty: false,
  needsSave: false,

  imageWidth: 0,
  imageHeight: 0,
  activeTool: "none",
  brushRadius: 10,
  zoom: 100,

  setImageFromImport: (path, data) => {
    const orig = cloneImageData(data);
    set({
      // Import doesn't change currently saved .hw path.
      hwPath: get().hwPath,
      draftRgb: orig,
      draftBinary: null,
      draftRgbVersion: get().draftRgbVersion + 1,
      draftBinarySourceVersion: null,
      imageWidth: data.width,
      imageHeight: data.height,
      activeTool: "none",
      isDirty: true,
      needsSave: true,
    });
  },

  setActiveTool: (tool) => set({ activeTool: tool }),

  openHw: (filePath, payload) => {
    const { width, height, threshold, rgbRgbaBase64, binaryRgbaBase64 } = payload;

    // Decode base64 (RGBA bytes) -> ImageData.
    const decodeRgbaBase64ToImageData = (
      base64: string,
      w: number,
      h: number
    ): ImageData => {
      const raw = atob(base64);
      const bytes = new Uint8Array(raw.length);
      for (let i = 0; i < raw.length; i++) bytes[i] = raw.charCodeAt(i);
      return new ImageData(new Uint8ClampedArray(bytes), w, h);
    };

    const rgb = decodeRgbaBase64ToImageData(rgbRgbaBase64, width, height);
    const binary = decodeRgbaBase64ToImageData(binaryRgbaBase64, width, height);

    set({
      hwPath: filePath,

      committedRgb: cloneImageData(rgb),
      committedBinary: cloneImageData(binary),
      committedThreshold: threshold,

      draftRgb: cloneImageData(rgb),
      draftBinary: cloneImageData(binary),
      threshold,

      draftRgbVersion: 1,
      draftBinarySourceVersion: 1,
      isDirty: false,
      needsSave: false,

      imageWidth: width,
      imageHeight: height,
      activeTool: "none",
    });
  },

  tick: () => {
    const { draftRgb, draftBinary, threshold, draftRgbVersion } = get();
    if (!draftRgb) return;

    const nextRgb = cloneImageData(draftRgb);
    const nextThreshold = threshold;

    // If draftBinary isn't generated yet, generate it now from draftRgb.
    const nextBinary = draftBinary ?? convertToBinary(draftRgb, nextThreshold);

    set({
      committedRgb: cloneImageData(nextRgb),
      committedBinary: cloneImageData(nextBinary),
      committedThreshold: nextThreshold,

      // Keep draft aligned with committed after tick.
      draftRgb: cloneImageData(nextRgb),
      draftBinary: cloneImageData(nextBinary),
      threshold: nextThreshold,

      draftRgbVersion,
      draftBinarySourceVersion: draftRgbVersion,

      isDirty: false,
      needsSave: true,
    });
  },

  cross: () => {
    const { committedRgb, committedBinary, committedThreshold, needsSave } = get();
    if (!committedRgb || !committedBinary) return;

    const nextRgb = cloneImageData(committedRgb);
    const nextBinary = cloneImageData(committedBinary);

    const nextVersion = get().draftRgbVersion + 1;
    set({
      draftRgb: nextRgb,
      draftBinary: nextBinary,
      threshold: committedThreshold,

      draftRgbVersion: nextVersion,
      draftBinarySourceVersion: nextVersion,

      imageWidth: nextRgb.width,
      imageHeight: nextRgb.height,

      isDirty: false,
      needsSave,
    });
  },

  ensureDraftBinary: () => {
    const state = get();
    if (!state.draftRgb) return;

    const needsRegen =
      !state.draftBinary ||
      state.draftBinarySourceVersion !== state.draftRgbVersion;
    if (!needsRegen) return;

    const next = convertToBinary(state.draftRgb, state.threshold);
    set({
      draftBinary: next,
      draftBinarySourceVersion: state.draftRgbVersion,
    });
  },

  setThreshold: (value) => {
    const next = clamp(Number.isFinite(value) ? value : 50, 0, 100);
    const state = get();
    if (next === state.threshold) return;

    // Threshold tuning should be real-time: regenerate binary immediately
    // so erase view updates without unloading the image.
    const nextBinary = state.draftRgb
      ? convertToBinary(state.draftRgb, next)
      : state.draftBinary;

    set({
      threshold: next,
      draftBinary: nextBinary,
      draftBinarySourceVersion: state.draftRgb ? state.draftRgbVersion : null,
      isDirty: state.draftRgb ? true : state.isDirty,
      needsSave: state.draftRgb ? true : state.needsSave,
    });
  },

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

  paintWhiteCircle: (cx, cy, radius) => {
    const state = get();
    if (!state.draftRgb) return;

    // Ensure erase stage exists.
    if (!state.draftBinary) {
      const nextBinary = convertToBinary(state.draftRgb, state.threshold);
      set({
        draftBinary: nextBinary,
        draftBinarySourceVersion: state.draftRgbVersion,
      });
    }

    const { draftBinary } = get();
    if (!draftBinary) return;

    const next = cloneImageData(draftBinary);
    paintCircleOnData(next, cx, cy, radius);
    set({
      draftBinary: next,
      draftBinarySourceVersion: get().draftRgbVersion,
      isDirty: true,
      needsSave: true,
    });
  },

  markSaved: (path) =>
    set({
      hwPath: path,
      needsSave: false,
    }),

  clearImage: () =>
    set({
      hwPath: null,
      committedRgb: null,
      committedBinary: null,
      committedThreshold: 50,

      draftRgb: null,
      draftBinary: null,
      threshold: 50,

      draftRgbVersion: 0,
      draftBinarySourceVersion: null,
      isDirty: false,
      needsSave: false,
      imageWidth: 0,
      imageHeight: 0,
      activeTool: "none",
    }),
}));
