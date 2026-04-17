import { app, BrowserWindow } from "electron";
import { createWindow } from "./src/main/windowManager.ts";
import { registerIpcHandlers } from "./src/main/ipcHandlers.ts";
import { logger } from "./src/utils/logger.ts";

// Linux: your logs showed x11_software_bitmap_presenter + GPU 139 → SIGTRAP; that path is XWayland/Ozone-X11
// with a dying GPU helper. On real Wayland sessions we force native Wayland Ozone unless overridden.
if (process.platform === "linux") {
  const ozEnv = process.env.HANDWRITE_OZONE_PLATFORM?.trim().toLowerCase();
  if (ozEnv === "x11" || ozEnv === "wayland") {
    app.commandLine.appendSwitch("ozone-platform", ozEnv);
    console.info(`[handwrite] Linux: ozone-platform=${ozEnv} (HANDWRITE_OZONE_PLATFORM)`);
  } else if (process.env.WAYLAND_DISPLAY) {
    app.commandLine.appendSwitch("ozone-platform", "wayland");
    console.info(
      `[handwrite] Linux: Wayland (${process.env.WAYLAND_DISPLAY}) → ozone-platform=wayland. For broken pointer hit-testing only, try HANDWRITE_OZONE_PLATFORM=x11.`
    );
  } else {
    console.info("[handwrite] Linux: no WAYLAND_DISPLAY — Chromium default Ozone (typically X11).");
  }

  const gpuWant = process.env.HANDWRITE_GPU?.trim().toLowerCase();
  if (gpuWant === "full" || gpuWant === "on" || gpuWant === "1") {
    console.info("[handwrite] Linux GPU: full (HANDWRITE_GPU override, no extra switches).");
  } else if (gpuWant === "safe" || gpuWant === "software") {
    app.commandLine.appendSwitch("disable-gpu-sandbox");
    app.commandLine.appendSwitch("disable-gpu");
    console.info("[handwrite] Linux GPU: safe (--disable-gpu; window has 4s show fallback if needed).");
  } else if (gpuWant === "no-hw" || gpuWant === "noaccel") {
    app.commandLine.appendSwitch("disable-gpu-sandbox");
    app.disableHardwareAcceleration();
    console.info("[handwrite] Linux GPU: no-hw (disableHardwareAcceleration — opt-in; can SIGTRAP on some stacks).");
  } else {
    app.commandLine.appendSwitch("disable-gpu-sandbox");
    // Avoids separate GPU process repeatedly dying with 139 on several Linux GL stacks.
    app.commandLine.appendSwitch("in-process-gpu");
    console.info(
      "[handwrite] Linux GPU: disable-gpu-sandbox + in-process-gpu (HANDWRITE_GPU=safe|full|no-hw as needed)."
    );
  }
}

registerIpcHandlers();

process.on("uncaughtException", (error) => {
  logger.error("main.ts", "uncaughtException", error.message, error, "BG");
});

process.on("unhandledRejection", (reason) => {
  const err =
    reason instanceof Error ? reason : new Error(typeof reason === "string" ? reason : JSON.stringify(reason));
  logger.error("main.ts", "unhandledRejection", err.message, err, "BG");
});

app.whenReady().then(() => {
  // Chromium/Electron persistent data (Crashpad minidumps, caches, etc. often live under here).
  console.info("[handwrite] Electron userData directory:", app.getPath("userData"));
  createWindow();
});

app.on("window-all-closed", () => {
  if (process.platform !== "darwin") app.quit();
});

app.on("activate", () => {
  if (BrowserWindow.getAllWindows().length === 0) createWindow();
});
