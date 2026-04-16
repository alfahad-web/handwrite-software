export interface ElectronAPI {
  platform: string;
  selectOrCreateTxtFile: () => Promise<string | null>;
  appendTxtLines: (filePath: string, lines: string[]) => Promise<void>;

  minimizeWindow: () => Promise<void>;
  maximizeWindow: () => Promise<void>;
  closeWindow: () => Promise<void>;
  confirmClose: () => Promise<void>;
  onAppCloseRequested: (callback: () => void) => () => void;
}

declare global {
  interface Window {
    /** Present when running inside Electron with preload; absent in a plain browser (e.g. Vite-only tab). */
    electronAPI?: ElectronAPI;
  }
}

export {};
