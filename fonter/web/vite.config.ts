import { defineConfig } from "vitest/config";
import react from "@vitejs/plugin-react";

export default defineConfig({
  // GitHub Pages serves project sites from `/<repo>/`.
  // Set `VITE_BASE=/<repo>/` in CI so asset URLs resolve correctly.
  base: process.env.VITE_BASE ?? "/",
  plugins: [react()],
  test: {
    environment: "node",
    include: ["src/**/*.test.ts"],
  },
});
