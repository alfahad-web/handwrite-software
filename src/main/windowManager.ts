/**
 * Window Management
 */

import { BrowserWindow, session } from "electron";
import path from "path";
import { fileURLToPath } from "url";
import { logger } from "../utils/logger.ts";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const isDev: boolean = process.env.NODE_ENV === "development";
const shouldOpenDevTools =
  isDev && process.env.ELECTRON_NO_DEVTOOLS !== "1";

const isLinux = process.platform === "linux";
const useNativeLinuxFrame = isLinux;
const framelessTitleChrome = useNativeLinuxFrame
  ? {}
  : ({
      titleBarStyle: "hidden" as const,
      titleBarOverlay: {
        color: "#101419",
        symbolColor: "#4ade80",
      },
    } satisfies Partial<ConstructorParameters<typeof BrowserWindow>[0]>);

let sessionCspListenerAttached = false;

function attachProductionCspOnce(): void {
  if (isDev || sessionCspListenerAttached) return;
  sessionCspListenerAttached = true;
  // Only for packaged file:// loads. In dev, injecting CSP on every localhost response
  // breaks Vite (WebSocket HMR, dynamic imports) and has caused repeated renderer crashes on Linux.
  session.defaultSession.webRequest.onHeadersReceived((details, callback) => {
    callback({
      responseHeaders: {
        ...details.responseHeaders,
        "Content-Security-Policy": [
          "default-src 'self' data: blob:; connect-src 'self' http://localhost:* https://localhost:* ws://localhost:* wss://localhost:*; script-src 'self' 'unsafe-inline' 'unsafe-eval' blob:; worker-src 'self' blob:; style-src 'self' 'unsafe-inline'; img-src 'self' data: blob:;",
        ],
      },
    });
  });
}

export function createWindow(): BrowserWindow {
  const win = new BrowserWindow({
    width: 1200,
    height: 800,
    minWidth: 640,
    minHeight: 480,
    resizable: true,
    title: "Handwrite Software",
    frame: useNativeLinuxFrame ? true : false,
    ...framelessTitleChrome,
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

  const showWhenReady = () => {
    if (win.isDestroyed()) return;
    if (!win.isVisible()) win.show();
  };

  const fallbackMs = 4000;
  const fallbackTimer = setTimeout(() => {
    if (win.isDestroyed()) return;
    if (!win.isVisible()) {
      logger.warn(
        "windowManager.ts",
        "show-fallback",
        `Window still hidden after ${fallbackMs}ms; forcing show (ready-to-show may not have fired).`,
        "BG"
      );
      showWhenReady();
    }
  }, fallbackMs);

  win.once("ready-to-show", () => {
    clearTimeout(fallbackTimer);
    showWhenReady();
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

  // Open DevTools only after navigation succeeds, in a detached window so it
  // does not dock inside the frameless app (which can feel broken or hang the UI).
  if (shouldOpenDevTools) {
    win.webContents.once("did-finish-load", () => {
      try {
        win.webContents.openDevTools({ mode: "detach" });
      } catch (e) {
        logger.error(
          "windowManager.ts",
          "openDevTools",
          `Failed to open DevTools: ${(e as Error).message}`,
          e as Error,
          "BG"
        );
      }
    });
  }

  win.webContents.on("did-fail-load", (_event, code, desc, url) => {
    logger.error(
      "windowManager.ts",
      "did-fail-load",
      `code=${code} url=${url} ${desc}`,
      undefined,
      "BG"
    );
  });

  win.webContents.on("render-process-gone", (_event, details) => {
    logger.error(
      "windowManager.ts",
      "render-process-gone",
      `reason=${details.reason} exitCode=${details.exitCode}`,
      undefined,
      "BG"
    );
  });

  win.on("unresponsive", () => {
    logger.warn("windowManager.ts", "unresponsive", "BrowserWindow became unresponsive", "BG");
  });

  attachProductionCspOnce();

  return win;
}
