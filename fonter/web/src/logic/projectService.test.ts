import { describe, expect, it } from "vitest";
import { EditorStore } from "./editorStore";
import { loadProjectFromJson, saveProjectToJson } from "./projectService";

describe("projectService", () => {
  it("round-trips minimal project", () => {
    const a = new EditorStore();
    a.setProjectFilePath("/tmp/test.hw");
    a.setStrokePx(8);
    a.startStroke({ x: 10, y: 10 });
    a.replaceActiveStrokePoints([
      { x: 10, y: 10 },
      { x: 50, y: 50 },
    ]);
    a.endStroke();
    const json = saveProjectToJson(a);

    const b = new EditorStore();
    const r = loadProjectFromJson(b, json, "/tmp/test.hw");
    expect(r).toEqual({ ok: true });
    expect(b.strokePx()).toBe(8);
    expect(b.strokes().length).toBe(1);
    expect(b.strokes()[0]!.points.length).toBe(2);
    expect(b.projectFilePath()).toBe("/tmp/test.hw");
    expect(b.isDirty()).toBe(false);
  });
});
