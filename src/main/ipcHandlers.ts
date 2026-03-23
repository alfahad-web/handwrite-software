/**
 * IPC Handlers — image file selection, read, window controls
 */

import { ipcMain, dialog, BrowserWindow } from "electron";
import * as fs from "fs";
import { logger } from "../utils/logger.ts";

export function registerIpcHandlers(): void {
  ipcMain.handle("select-image-file", async () => {
    const result = await dialog.showOpenDialog({
      properties: ["openFile"],
      filters: [
        {
          name: "Images",
          extensions: ["jpg", "jpeg", "png"],
        },
        { name: "All Files", extensions: ["*"] },
      ],
    });

    if (!result.canceled && result.filePaths.length > 0) {
      return result.filePaths[0];
    }
    return null;
  });

  ipcMain.handle(
    "read-image-file",
    async (_event, filePath: string): Promise<{ base64: string } | null> => {
      try {
        if (!filePath || typeof filePath !== "string") {
          return null;
        }
        const buf = fs.readFileSync(filePath);
        return { base64: buf.toString("base64") };
      } catch (error) {
        logger.error(
          "ipcHandlers.ts",
          "read-image-file",
          `Failed to read image: ${(error as Error).message}`,
          error as Error
        );
        throw error;
      }
    }
  );

  ipcMain.handle("minimize-window", () => {
    const win = BrowserWindow.getFocusedWindow() ?? BrowserWindow.getAllWindows()[0];
    win?.minimize();
  });

  ipcMain.handle("maximize-window", () => {
    const win = BrowserWindow.getFocusedWindow() ?? BrowserWindow.getAllWindows()[0];
    if (!win) return;
    if (win.isMaximized()) {
      win.unmaximize();
    } else {
      win.maximize();
    }
  });

  ipcMain.handle("close-window", () => {
    const win = BrowserWindow.getFocusedWindow() ?? BrowserWindow.getAllWindows()[0];
    win?.close();
  });
}
