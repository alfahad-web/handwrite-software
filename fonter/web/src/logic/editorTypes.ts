/** Mirrors fonter/cachy/src/core/EditorTypes.h (const objects instead of enum for TS erasableSyntaxOnly) */

export const ToolMode = {
  Draw: 0,
  Select: 1,
  Erase: 2,
} as const;
export type ToolMode = (typeof ToolMode)[keyof typeof ToolMode];

export const JoinMode = {
  L: 0,
  R: 1,
  LR: 2,
  N: 3,
} as const;
export type JoinMode = (typeof JoinMode)[keyof typeof JoinMode];

export function joinModeToString(m: JoinMode): string {
  switch (m) {
    case JoinMode.L:
      return "L";
    case JoinMode.R:
      return "R";
    case JoinMode.LR:
      return "LR";
    case JoinMode.N:
      return "N";
    default:
      return "N";
  }
}

export function joinModeFromString(s: string): JoinMode {
  if (s === "LR") return JoinMode.LR;
  if (s === "L") return JoinMode.L;
  if (s === "R") return JoinMode.R;
  return JoinMode.N;
}

export const ResizeHandle = {
  None: 0,
  Move: 1,
  N: 2,
  S: 3,
  E: 4,
  W: 5,
  NW: 6,
  NE: 7,
  SW: 8,
  SE: 9,
} as const;
export type ResizeHandle = (typeof ResizeHandle)[keyof typeof ResizeHandle];

export interface Point {
  x: number;
  y: number;
}

export interface SelectionRect {
  x: number;
  y: number;
  width: number;
  height: number;
}

export interface StrokePoint {
  pos: Point;
  erased: boolean;
}

export interface Stroke {
  id: string;
  points: StrokePoint[];
  createdAt: number;
}

export interface ResizeDragState {
  handle: ResizeHandle;
  selectionId: string;
  startRect: SelectionRect;
  startPoint: Point;
}

export interface SelectionBox {
  id: string;
  orderIndex: number;
  rect: SelectionRect;
  assigned: boolean;
  assignedAscii: number;
  fileStem: string;
  joinMode: JoinMode;
  anchorX: number;
  anchorY: number;
}

export interface SelectionBoxRow {
  id: string;
  x: number;
  y: number;
  width: number;
  height: number;
  orderIndex: number;
  assigned: boolean;
  assignedAscii: number;
  fileStem: string;
  joinMode: string;
  anchorX: number;
  anchorY: number;
  selected: boolean;
}
