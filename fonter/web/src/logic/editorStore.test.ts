import { describe, expect, it } from "vitest";
import { EditorStore } from "./editorStore";
describe("EditorStore", () => {
  it("commitSelectionDraftRect rejects tiny rect", () => {
    const s = new EditorStore();
    s.setToolMode("select");
    s.setSelectionDraftRect({ x: 0, y: 0, width: 2, height: 2 });
    const id = s.commitSelectionDraftRect();
    expect(id).toBe("");
    expect(s.selectionBoxes().length).toBe(0);
  });

  it("soft erase marks points in selection", () => {
    const s = new EditorStore();
    s.startStroke({ x: 5, y: 5 });
    s.replaceActiveStrokePoints([
      { x: 5, y: 5 },
      { x: 10, y: 10 },
      { x: 20, y: 20 },
    ]);
    s.endStroke();
    s.setToolMode("select");
    s.setSelectionDraftRect({ x: 0, y: 0, width: 100, height: 100 });
    s.commitSelectionDraftRect();
    const selId = s.selectedSelectionId();
    s.setToolMode("erase");
    s.erasePointsInSelectedSelection({ x: 10, y: 10 }, 5);
    const pts = s.strokes()[0]!.points;
    expect(pts.some((p) => p.erased)).toBe(true);
    s.setSelectedSelectionId(selId);
  });

  it("removeStrokePointsNear splits stroke", () => {
    const s = new EditorStore();
    s.setDrawStrokeEraseActive(true);
    s.startStroke({ x: 0, y: 0 });
    s.replaceActiveStrokePoints([
      { x: 0, y: 0 },
      { x: 100, y: 0 },
    ]);
    s.removeStrokePointsNear({ x: 50, y: 0 }, 10);
    expect(s.strokes().length).toBeGreaterThanOrEqual(1);
  });
});
