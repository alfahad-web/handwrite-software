// preload.js
const { contextBridge, ipcRenderer } = require("electron");

contextBridge.exposeInMainWorld("electronAPI", {
  selectImageFile: () => ipcRenderer.invoke("select-image-file"),
  readImageFile: (filePath) => ipcRenderer.invoke("read-image-file", filePath),

  selectHwFile: () => ipcRenderer.invoke("select-hw-file"),
  readHwFile: (filePath) => ipcRenderer.invoke("read-hw-file", filePath),
  selectSaveHwFile: () => ipcRenderer.invoke("select-save-hw-file"),
  writeHwFile: (filePath, payload) =>
    ipcRenderer.invoke("write-hw-file", filePath, payload),

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
