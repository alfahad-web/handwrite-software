import React from "react";
import ReactDOM from "react-dom/client";
import App from "./App.tsx";
import { logger } from "./utils/logger.ts";
import "./index.css";

window.addEventListener("error", (event) => {
  logger.error(
    "main.tsx",
    "window.error",
    event.message,
    event.error instanceof Error ? event.error : undefined,
    "RENDER"
  );
});

window.addEventListener("unhandledrejection", (event) => {
  const reason = event.reason;
  const err =
    reason instanceof Error ? reason : new Error(typeof reason === "string" ? reason : String(reason));
  logger.error("main.tsx", "unhandledrejection", err.message, err, "RENDER");
});

const rootElement = document.getElementById("root");
if (!rootElement) throw new Error("Failed to find the root element");

ReactDOM.createRoot(rootElement).render(
  <React.StrictMode>
    <App />
  </React.StrictMode>
);
