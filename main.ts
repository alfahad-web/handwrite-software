import { app, BrowserWindow } from "electron";
import { createWindow } from "./src/main/windowManager.ts";
import { registerIpcHandlers } from "./src/main/ipcHandlers.ts";
import { logger } from "./src/utils/logger.ts";

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
  createWindow();
});

app.on("window-all-closed", () => {
  if (process.platform !== "darwin") app.quit();
});

app.on("activate", () => {
  if (BrowserWindow.getAllWindows().length === 0) createWindow();
});
