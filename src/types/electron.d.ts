export interface ElectronAPI {
  selectImageFile: () => Promise<string | null>;
  readImageFile: (
    filePath: string
  ) => Promise<{ base64: string } | null>;
  minimizeWindow: () => Promise<void>;
  maximizeWindow: () => Promise<void>;
  closeWindow: () => Promise<void>;
}

declare global {
  interface Window {
    electronAPI: ElectronAPI;
  }
}

export {};
