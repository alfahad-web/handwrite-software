/**
 * Window Management
 */

import { BrowserWindow } from "electron";
import path from "path";
import { fileURLToPath } from "url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const isDev: boolean = process.env.NODE_ENV === "development";

export function createWindow(): BrowserWindow {
  const win = new BrowserWindow({
    width: 1200,
    height: 800,
    minWidth: 640,
    minHeight: 480,
    resizable: true,
    title: "Handwrite Software",
    frame: false,
    titleBarStyle: "hidden",
    titleBarOverlay: {
      color: "#101419",
      symbolColor: "#4ade80",
    },
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true,
      preload: path.join(__dirname, "../../preload.js"),
    },
    backgroundColor: "#1a1a1a",
    show: false,
  });

  if (isDev) {
    win.loadURL("http://localhost:3000");
  } else {
    win.loadFile(path.join(__dirname, "../../dist/index.html"));
  }

  win.once("ready-to-show", () => {
    win.show();
  });

  // Intercept all close attempts (title bar, OS controls, Alt+F4).
  // Renderer decides whether to save/cancel/discard, then calls confirm-close.
  (win as BrowserWindow & { __allowClose?: boolean }).__allowClose = false;
  win.on("close", (event) => {
    const w = win as BrowserWindow & { __allowClose?: boolean };
    if (w.__allowClose) return;
    event.preventDefault();
    win.webContents.send("app-close-requested");
  });

  if (isDev) {
    win.webContents.openDevTools();
  }

  win.webContents.session.webRequest.onHeadersReceived((details, callback) => {
    callback({
      responseHeaders: {
        ...details.responseHeaders,
        "Content-Security-Policy": [
          "default-src 'self' data: blob:; connect-src 'self' http://localhost:* https://localhost:*; script-src 'self' 'unsafe-inline' 'unsafe-eval' blob:; worker-src 'self' blob:; style-src 'self' 'unsafe-inline'; img-src 'self' data: blob:;",
        ],
      },
    });
  });

  return win;
}
