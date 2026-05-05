import { describe, expect, it } from "vitest";
import { JoinMode } from "./editorTypes";
import { EditorStore } from "./editorStore";
import { loadProjectFromJson, saveProjectToJson } from "./projectService";

describe("projectService", () => {
  it("round-trips guide line gap, manual anchor, selection erased keys", () => {
    const s = new EditorStore();
    s.setGuideLineGapPx(200);
    s.startStroke({ x: 5, y: 5 });
    s.replaceActiveStrokePoints([{ x: 5, y: 5 }, { x: 20, y: 20 }]);
    s.endStroke();
    const strokeId = s.strokes()[0]!.id;
    s.setToolMode("select");
    s.setSelectionDraftRect({ x: 0, y: 0, width: 100, height: 100 });
    const selId = s.commitSelectionDraftRect();
    expect(selId.length).toBeGreaterThan(0);
    s.setToolMode("erase");
    expect(s.erasePointsInSelectedSelection({ x: 15, y: 15 }, 50)).toBe(true);
    s.setSelectionAnchorPoint(selId, { x: 10, y: 10 });

    const json = saveProjectToJson(s);
    const s2 = new EditorStore();
    const r = loadProjectFromJson(s2, json, "test.hw");
    expect(r).toEqual({ ok: true });
    expect(s2.guideLineGapPx()).toBe(200);
    const box = s2.selectionById(selId);
    expect(box).not.toBeNull();
    expect(box!.hasManualAnchor).toBe(true);
    expect(
      s2.isPointErasedInSelection(selId, strokeId, 1),
    ).toBe(true);
    expect(loadProjectFromJson(new EditorStore(), "", "x.hw")).toMatchObject({
      ok: false,
    });
  });

  it("defaults missing SelectionBox anchor fields", () => {
    const raw = JSON.stringify({
      formatVersion: 2,
      strokePx: 6,
      captureGapUm: 350,
      zoom: 100,
      eraseRadiusPx: 20,
      strokes: [],
      selectionBoxes: [
        {
          id: "s1",
          orderIndex: 1,
          x: 0,
          y: 0,
          width: 40,
          height: 40,
          assigned: false,
          assignedAscii: -1,
          fileStem: "",
          joinMode: "N",
          anchorX: 0,
          anchorY: 40,
        },
      ],
      selectedSelectionId: "s1",
      specialCharStemMap: {},
    });
    const s = new EditorStore();
    expect(loadProjectFromJson(s, raw, "legacy.hw")).toEqual({ ok: true });
    expect(s.selectionById("s1")!.hasManualAnchor).toBe(false);
    expect(s.selectionById("s1")!.joinMode).toBe(JoinMode.N);
  });
});
