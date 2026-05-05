/**
 * Mirrors fonter/cachy/src/app/AppController.cpp assignment / delete messages
 */

import { joinModeFromString } from "./editorTypes";
import type { EditorStore } from "./editorStore";

export function assignSelectionCharacter(
  store: EditorStore,
  selectionId: string,
  text: string,
  joinMode: string,
): string {
  if (text.length !== 1) {
    return "Assignment must be one ASCII character.";
  }
  const code = text.charCodeAt(0);
  if (code > 127) {
    return "Only ASCII characters are allowed.";
  }
  if (
    joinMode !== "L" &&
    joinMode !== "R" &&
    joinMode !== "LR" &&
    joinMode !== "N"
  ) {
    return "Join mode must be L, R, LR, or N.";
  }
  const box = store.selectionByIdMutable(selectionId);
  if (!box) return "Selection not found.";
  box.assigned = true;
  box.assignedAscii = code;
  box.fileStem = text;
  box.joinMode = joinModeFromString(joinMode);
  store.recomputeSelectionAnchors();
  store.markDirty();
  return "Selection assigned.";
}

export function deleteSelectedSelectionWithMessage(store: EditorStore): string {
  if (store.deleteSelectedSelection()) {
    return "Selection deleted.";
  }
  return "No active selection.";
}
