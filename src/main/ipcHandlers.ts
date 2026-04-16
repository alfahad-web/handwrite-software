import { ipcMain, dialog, BrowserWindow } from "electron";
import * as fs from "fs";
import * as path from "path";
import { logger } from "../utils/logger.ts";

export function registerIpcHandlers(): void {
  ipcMain.handle("select-or-create-txt-file", async () => {
    const openResult = await dialog.showOpenDialog({
      properties: ["openFile"],
      filters: [
        { name: "Text", extensions: ["txt"] },
        { name: "All Files", extensions: ["*"] },
      ],
    });

    if (!openResult.canceled && openResult.filePaths.length > 0) {
      return openResult.filePaths[0];
    }
    const saveResult = await dialog.showSaveDialog({
      filters: [{ name: "Text", extensions: ["txt"] }],
      defaultPath: "glyph.txt",
    });
    if (saveResult.canceled) return null;
    const selected = saveResult.filePath ?? null;
    if (!selected) return null;
    const normalized = selected.endsWith(".txt") ? selected : `${selected}.txt`;
    if (!fs.existsSync(normalized)) {
      fs.writeFileSync(normalized, "", "utf-8");
    }
    return normalized;
  });

  ipcMain.handle(
    "append-txt-lines",
    async (_event, filePath: string, lines: string[]): Promise<void> => {
      try {
        if (!filePath || typeof filePath !== "string") {
          throw new Error("Invalid txt file path");
        }
        if (!Array.isArray(lines)) {
          throw new Error("Invalid lines payload");
        }
        const sanitized = lines
          .map((line) => String(line ?? "").trim())
          .filter((line) => line.length > 0);
        if (sanitized.length === 0) return;
        const parentDir = path.dirname(filePath);
        if (!fs.existsSync(parentDir)) {
          fs.mkdirSync(parentDir, { recursive: true });
        }
        fs.appendFileSync(filePath, `${sanitized.join("\n")}\n`, "utf-8");
      } catch (error) {
        logger.error(
          "ipcHandlers.ts",
          "append-txt-lines",
          `Failed to append lines: ${(error as Error).message}`,
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

  ipcMain.handle("confirm-close", () => {
    const win = BrowserWindow.getFocusedWindow() ?? BrowserWindow.getAllWindows()[0];
    if (!win) return;
    (win as BrowserWindow & { __allowClose?: boolean }).__allowClose = true;
    win.close();
  });
}
