// preload.js
const { contextBridge, ipcRenderer } = require("electron");

contextBridge.exposeInMainWorld("electronAPI", {
  platform: process.platform,
  selectOrCreateTxtFile: () => ipcRenderer.invoke("select-or-create-txt-file"),
  appendTxtLines: (filePath, lines) =>
    ipcRenderer.invoke("append-txt-lines", filePath, lines),

  minimizeWindow: () => ipcRenderer.invoke("minimize-window"),
  maximizeWindow: () => ipcRenderer.invoke("maximize-window"),
  closeWindow: () => ipcRenderer.invoke("close-window"),
  confirmClose: () => ipcRenderer.invoke("confirm-close"),
  onAppCloseRequested: (callback) => {
    const listener = () => callback();
    ipcRenderer.on("app-close-requested", listener);
    return () => ipcRenderer.removeListener("app-close-requested", listener);
  },
});
