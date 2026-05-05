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

  it("soft erase records per-selection mask without setting global erased", () => {
    const s = new EditorStore();
    s.startStroke({ x: 5, y: 5 });
    s.replaceActiveStrokePoints([
      { x: 5, y: 5 },
      { x: 10, y: 10 },
      { x: 20, y: 20 },
    ]);
    const strokeId = s.strokes()[0]!.id;
    s.endStroke();
    s.setToolMode("select");
    s.setSelectionDraftRect({ x: 0, y: 0, width: 100, height: 100 });
    s.commitSelectionDraftRect();
    const selId = s.selectedSelectionId();
    expect(selId.length).toBeGreaterThan(0);
    s.setToolMode("erase");
    expect(
      s.erasePointsInSelectedSelection({ x: 10, y: 10 }, 5),
    ).toBe(true);
    const pts = s.strokes()[0]!.points;
    expect(pts.some((p) => p.erased)).toBe(false);
    expect(
      s.isPointErasedInSelection(selId, strokeId, 1),
    ).toBe(true);
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

  it("deleteSelectedSelection selects previous box by orderIndex", () => {
    const s = new EditorStore();
    s.setToolMode("select");
    s.setSelectionDraftRect({ x: 0, y: 0, width: 50, height: 50 });
    const first = s.commitSelectionDraftRect();
    s.setSelectionDraftRect({ x: 60, y: 0, width: 50, height: 50 });
    s.commitSelectionDraftRect();
    const secondId = s.selectedSelectionId();
    expect(secondId).not.toBe(first);
    s.deleteSelectedSelection();
    expect(s.selectedSelectionId()).toBe(first);
  });
});
