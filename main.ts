import { app, BrowserWindow } from "electron";
import { createWindow } from "./src/main/windowManager.ts";
import { registerIpcHandlers } from "./src/main/ipcHandlers.ts";

registerIpcHandlers();

app.whenReady().then(() => {
  createWindow();
});

app.on("window-all-closed", () => {
  if (process.platform !== "darwin") app.quit();
});

app.on("activate", () => {
  if (BrowserWindow.getAllWindows().length === 0) createWindow();
});
