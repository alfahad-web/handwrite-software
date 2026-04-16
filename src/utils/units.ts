const MICROMETERS_PER_INCH = 25400;
const CSS_PIXELS_PER_INCH = 96;

export function resolveScreenDpi(): number {
  const ratio = typeof window !== "undefined" ? window.devicePixelRatio || 1 : 1;
  return CSS_PIXELS_PER_INCH * ratio;
}

export function pxToUm(px: number, dpi: number): number {
  if (!Number.isFinite(px) || !Number.isFinite(dpi) || dpi <= 0) return 0;
  return (px / dpi) * MICROMETERS_PER_INCH;
}

export function umToPx(um: number, dpi: number): number {
  if (!Number.isFinite(um) || !Number.isFinite(dpi) || dpi <= 0) return 0;
  return (um / MICROMETERS_PER_INCH) * dpi;
}
