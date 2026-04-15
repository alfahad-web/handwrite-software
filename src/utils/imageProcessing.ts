/**
 * RGB → grayscale → binary threshold, image cloning, white brush
 */

export function cloneImageData(source: ImageData): ImageData {
  const copy = new ImageData(source.width, source.height);
  copy.data.set(source.data);
  return copy;
}

/** luminance weights (ITU-R BT.601) */
const R = 0.299;
const G = 0.587;
const B = 0.114;

/**
 * If gray < (threshold/100)*255 → black (0), else white (255).
 * Mutates a new ImageData; does not modify `source`.
 */
export function convertToBinary(source: ImageData, thresholdPercent: number): ImageData {
  const out = new ImageData(source.width, source.height);
  const d = source.data;
  const o = out.data;
  const cutoff = (clamp(thresholdPercent, 0, 100) / 100) * 255;

  for (let i = 0; i < d.length; i += 4) {
    const gray = R * d[i] + G * d[i + 1] + B * d[i + 2];
    const v = gray < cutoff ? 0 : 255;
    o[i] = v;
    o[i + 1] = v;
    o[i + 2] = v;
    o[i + 3] = d[i + 3];
  }
  return out;
}

function clamp(n: number, min: number, max: number): number {
  return Math.min(max, Math.max(min, n));
}

/**
 * Set all pixels inside circle (cx, cy, r) to white. Mutates `target` in place.
 */
export function paintWhiteCircle(
  target: ImageData,
  cx: number,
  cy: number,
  radius: number
): void {
  const w = target.width;
  const h = target.height;
  const r = Math.max(0, radius);
  const r2 = r * r;
  const x0 = Math.max(0, Math.floor(cx - r - 1));
  const x1 = Math.min(w - 1, Math.ceil(cx + r + 1));
  const y0 = Math.max(0, Math.floor(cy - r - 1));
  const y1 = Math.min(h - 1, Math.ceil(cy + r + 1));

  for (let y = y0; y <= y1; y++) {
    const dy = y - cy;
    for (let x = x0; x <= x1; x++) {
      const dx = x - cx;
      if (dx * dx + dy * dy <= r2) {
        const i = (y * w + x) * 4;
        target.data[i] = 255;
        target.data[i + 1] = 255;
        target.data[i + 2] = 255;
        target.data[i + 3] = 255;
      }
    }
  }
}
