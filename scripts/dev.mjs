/**
 * Starts Vite and launches Electron only after THIS Vite process is ready.
 * This avoids relying on concurrently/wait-on in fragile installs.
 */
import { spawn } from "node:child_process";
import { existsSync } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");
const viteJs = path.join(root, "node_modules", "vite", "bin", "vite.js");
const electronCli = path.join(root, "node_modules", "electron", "cli.js");

if (!existsSync(viteJs)) {
  console.error("Missing Vite. Run: npm install");
  process.exit(1);
}
if (!existsSync(electronCli)) {
  console.error("Missing Electron. Run: npm install");
  process.exit(1);
}

const childEnv = { ...process.env, NODE_ENV: "development" };
const vite = spawn(process.execPath, [viteJs, "--port", "3000", "--strictPort"], {
  stdio: ["ignore", "pipe", "pipe"],
  cwd: root,
  env: childEnv,
});

let electronProc = null;
let electronStarted = false;

function maybeStartElectron(line) {
  if (electronStarted) return;
  const isReadyLine = line.includes("Local:") || line.includes("ready in");
  if (!isReadyLine) return;
  electronStarted = true;
  console.log("Vite is ready, launching Electron...");
  electronProc = spawn(process.execPath, [electronCli, "."], {
    stdio: "inherit",
    cwd: root,
    env: childEnv,
  });
  electronProc.on("exit", (code) => shutdown(code ?? 0));
}

function shutdown(code = 0) {
  try {
    if (electronProc && !electronProc.killed) electronProc.kill("SIGTERM");
  } catch {}
  try {
    if (vite && !vite.killed) vite.kill("SIGTERM");
  } catch {}
  process.exit(code);
}

process.on("SIGINT", () => shutdown(0));
process.on("SIGTERM", () => shutdown(0));

vite.stdout?.on("data", (chunk) => {
  const text = chunk.toString();
  process.stdout.write(text);
  const lines = text.split(/\r?\n/);
  for (const line of lines) {
    maybeStartElectron(line);
  }
});

vite.stderr?.on("data", (chunk) => {
  const text = chunk.toString();
  process.stderr.write(text);
});

vite.on("exit", (code) => {
  if (electronProc && !electronProc.killed) {
    try {
      electronProc.kill("SIGTERM");
    } catch {}
  }
  if (!electronStarted) {
    // Vite failed before app could start.
    process.exit(code ?? 1);
  }
  if (code !== 0 && code !== null) process.exit(code);
});
