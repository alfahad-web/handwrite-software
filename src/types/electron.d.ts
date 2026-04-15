export interface ElectronAPI {
  selectImageFile: () => Promise<string | null>;
  readImageFile: (
    filePath: string
  ) => Promise<{ base64: string } | null>;

  selectHwFile: () => Promise<string | null>;
  readHwFile: (filePath: string) => Promise<any | null>;
  selectSaveHwFile: () => Promise<string | null>;
  writeHwFile: (filePath: string, payload: any) => Promise<void>;

  minimizeWindow: () => Promise<void>;
  maximizeWindow: () => Promise<void>;
  closeWindow: () => Promise<void>;
  confirmClose: () => Promise<void>;
  onAppCloseRequested: (callback: () => void) => () => void;
}

declare global {
  interface Window {
    electronAPI: ElectronAPI;
  }
}

export {};
