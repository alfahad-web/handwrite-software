export type ToolMode = "draw" | "select";

export interface PointPx {
  x: number;
  y: number;
}

export interface Stroke {
  id: string;
  points: PointPx[];
  createdAt: number;
}

export interface SelectionRect {
  x: number;
  y: number;
  width: number;
  height: number;
}

export interface SampledStrokeUm {
  strokeId: string;
  points: Array<{ xUm: number; yUm: number }>;
}
