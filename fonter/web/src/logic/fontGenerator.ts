/**
 * Mirrors fonter/cachy/src/app/AppController.cpp generateFonts()
 * Output: ZIP blob with a top-level `font/` folder containing .txt files.
 */

import JSZip from "jszip";
import type { EditorStore } from "./editorStore";
import { buildSelectionExports } from "./exportService";
import {
  joinModeToString,
  type SelectionBox,
  type Stroke,
} from "./editorTypes";

export interface GenerateFontsResult {
  ok: boolean;
  message: string;
  blob?: Blob;
}

function writeTextFileLines(lines: string[]): string {
  const parts: string[] = [];
  for (const line of lines) {
    const clean = line.trim();
    if (clean.length === 0) continue;
    parts.push(clean);
  }
  return parts.join("\n");
}

function cloneStrokesWithSelectionMask(
  store: EditorStore,
  selectionId: string,
): Stroke[] {
  return store.strokes().map((stroke) => ({
    ...stroke,
    points: stroke.points.map((pt, idx) => ({
      ...pt,
      pos: { ...pt.pos },
      erased:
        pt.erased ||
        store.isPointErasedInSelection(selectionId, stroke.id, idx),
    })),
  }));
}

export async function generateFontsZip(
  store: EditorStore,
  dpi: number,
): Promise<GenerateFontsResult> {
  if (!store.projectFilePath().trim()) {
    return { ok: false, message: "Save project first." };
  }

  store.recomputeSelectionAnchors();
  const boxes = [...store.selectionBoxes()].sort(
    (a, b) => a.orderIndex - b.orderIndex,
  );
  const invalidSelectionIds = new Set<string>();
  let firstInvalidSelectionId = "";

  for (const box of boxes) {
    const join = joinModeToString(box.joinMode);
    const joinValid =
      join === "L" || join === "R" || join === "LR" || join === "N";
    const valid =
      box.assigned &&
      box.assignedAscii >= 0 &&
      box.assignedAscii <= 127 &&
      joinValid;
    if (!valid) {
      invalidSelectionIds.add(box.id);
      if (!firstInvalidSelectionId) {
        firstInvalidSelectionId = box.id;
      }
    }
  }

  if (invalidSelectionIds.size > 0) {
    store.setHighlightedSelectionIds(invalidSelectionIds);
    if (firstInvalidSelectionId) {
      store.setSelectedSelectionId(firstInvalidSelectionId);
    }
    const count = invalidSelectionIds.size;
    return {
      ok: false,
      message: `${count} selection boxes have incompatible details.`,
    };
  }

  store.clearHighlightedSelectionIds();

  const stemCounter = new Map<string, number>();
  let generated = 0;
  let skipped = 0;
  const zip = new JSZip();
  const fontFolder = zip.folder("font");
  if (!fontFolder) {
    return { ok: false, message: "Generate failed: could not create zip folder." };
  }

  for (const box of boxes) {
    if (
      !box.assigned ||
      box.assignedAscii < 0 ||
      box.assignedAscii > 127
    ) {
      skipped++;
      continue;
    }
    const deterministicStem = store.fileStemForAscii(box.assignedAscii);
    const stemJoin = `${deterministicStem}.${joinModeToString(box.joinMode)}`;
    const seq = (stemCounter.get(stemJoin) ?? 0) + 1;
    stemCounter.set(stemJoin, seq);
    const exportBox: SelectionBox = {
      ...box,
      fileStem: `${deterministicStem}.${joinModeToString(box.joinMode)}.${seq}`,
    };
    const maskedStrokes = cloneStrokesWithSelectionMask(store, box.id);
    const files = buildSelectionExports(
      maskedStrokes,
      [exportBox],
      store.captureGapUm(),
      dpi,
    );
    for (const f of files) {
      if (f.lines.length === 0) continue;
      const body = writeTextFileLines(f.lines);
      if (!body) continue;
      fontFolder.file(f.fileName, body + "\n");
      generated++;
    }
  }

  const blob = await zip.generateAsync({ type: "blob" });
  return {
    ok: true,
    blob,
    message: `Generated ${generated} files (${skipped} skipped) in font.zip`,
  };
}
