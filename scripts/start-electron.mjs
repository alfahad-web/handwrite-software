import { spawn } from "node:child_process";
import { existsSync } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");
const electronCli = path.join(root, "node_modules", "electron", "cli.js");

if (!existsSync(electronCli)) {
  console.error("Missing Electron. Run: npm install");
  process.exit(1);
}

const child = spawn(process.execPath, [electronCli, "."], {
  stdio: "inherit",
  cwd: root,
  env: { ...process.env, NODE_ENV: "development" },
});

child.on("exit", (code) => process.exit(code ?? 0));
