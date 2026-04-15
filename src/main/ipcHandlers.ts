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

  ipcMain.handle("select-hw-file", async () => {
    const result = await dialog.showOpenDialog({
      properties: ["openFile"],
      filters: [{ name: "Handwrite Project", extensions: ["hw"] }],
    });

    if (!result.canceled && result.filePaths.length > 0) {
      return result.filePaths[0];
    }
    return null;
  });

  ipcMain.handle("select-save-hw-file", async () => {
    const result = await dialog.showSaveDialog({
      filters: [{ name: "Handwrite Project", extensions: ["hw"] }],
      defaultPath: "project.hw",
    });

    if (result.canceled) return null;
    return result.filePath ?? null;
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

  ipcMain.handle(
    "read-hw-file",
    async (_event, filePath: string): Promise<any | null> => {
      try {
        if (!filePath || typeof filePath !== "string") return null;
        const txt = fs.readFileSync(filePath, "utf-8");
        const parsed = JSON.parse(txt) as any;
        return parsed?.version ? parsed : parsed; // tolerate missing version
      } catch (error) {
        logger.error(
          "ipcHandlers.ts",
          "read-hw-file",
          `Failed to read .hw file: ${(error as Error).message}`,
          error as Error
        );
        throw error;
      }
    }
  );

  ipcMain.handle(
    "write-hw-file",
    async (_event, filePath: string, payload: any): Promise<void> => {
      if (!filePath || typeof filePath !== "string") {
        throw new Error("Invalid .hw file path");
      }

      const toWrite = {
        version: 1,
        ...payload,
      };

      fs.writeFileSync(filePath, JSON.stringify(toWrite), "utf-8");
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

  ipcMain.handle("confirm-close", () => {
    const win = BrowserWindow.getFocusedWindow() ?? BrowserWindow.getAllWindows()[0];
    if (!win) return;
    (win as BrowserWindow & { __allowClose?: boolean }).__allowClose = true;
    win.close();
  });
}
