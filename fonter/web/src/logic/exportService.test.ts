import { describe, expect, it } from "vitest";
import {
  buildExportStrokes,
  pxToUm,
  serializeExportLines,
} from "./exportService";
import type { Stroke } from "./editorTypes";

describe("exportService", () => {
  it("pxToUm matches 96dpi inch formula", () => {
    const um = pxToUm(96, 96);
    expect(um).toBeCloseTo(25400, 0);
  });

  it("serializeExportLines ends lines with semicolon", () => {
    const rect = { x: 0, y: 0, width: 100, height: 100 };
    const anchor = { x: 0, y: 100 };
    const dpi = 96;
    const strokes = [
      {
        strokeId: "s1",
        points: [
          { xUm: 0, yUm: 0 },
          { xUm: 1000, yUm: 0 },
        ],
      },
    ];
    const lines = serializeExportLines(strokes, rect, anchor, dpi);
    expect(lines.length).toBeGreaterThanOrEqual(2);
    expect(lines[0]!.endsWith(";")).toBe(true);
    expect(lines[1]!.endsWith(";")).toBe(true);
  });

  it("buildExportStrokes samples by capture gap", () => {
    const strokes: Stroke[] = [
      {
        id: "a",
        createdAt: 1,
        points: [
          { pos: { x: 10, y: 10 }, erased: false },
          { pos: { x: 11, y: 10 }, erased: false },
          { pos: { x: 500, y: 10 }, erased: false },
        ],
      },
    ];
    const rect = { x: 0, y: 0, width: 600, height: 100 };
    const out = buildExportStrokes(
      strokes,
      rect,
      { x: 0, y: 100 },
      350,
      96,
    );
    expect(out.length).toBeGreaterThanOrEqual(1);
  });
});
