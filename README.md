# Handwrite software

This repository contains two Qt 6 desktop applications and a browser-based font editor:

| Directory | Target | Purpose |
|-----------|--------|---------|
| [`fonter/cachy/`](fonter/cachy/) | `HandwriteQt` | Qt 6: draw glyphs in selections and export font `.txt` files |
| [`fonter/web/`](fonter/web/) | Web SPA (Vite) | Same editor in the browser (tablet-first) |
| [`writer/`](writer/) | `WriterQt` | Preview typed text as handwriting from a font folder, layout, and run simulation |

**Requirements:** CMake 3.21+, a C++17 toolchain, and Qt 6.5+ (Quick, QuickControls2, Widgets).

During configuration you may see `Could NOT find WrapVulkanHeaders` — that is normal for these apps and can be ignored unless you are building Qt Vulkan-specific features yourself.

---

## Fonter — Qt (`HandwriteQt`, `fonter/cachy/`)

### Build

```bash
cd fonter/cachy
cmake -B build
cmake --build build
```

### Run

From `fonter/cachy` after a successful build:

```bash
./build/HandwriteQt
```

From the **repository root**:

```bash
./fonter/cachy/build/HandwriteQt
```

### Build and run (one copy-paste block)

```bash
cd fonter/cachy && cmake -B build && cmake --build build && ./build/HandwriteQt
```

---

## Fonter — Web (`fonter/web/`)

**Requirements:** Node.js 20+ (or 18+ with npm).

```bash
cd fonter/web
npm install
npm run dev
```

Production build:

```bash
cd fonter/web && npm run build && npm run preview
```

---

## Writer (`WriterQt`)

### Build

```bash
cd writer
cmake -B build
cmake --build build
```

### Run

From the `writer` directory:

```bash
./build/WriterQt
```

From the **repository root**:

```bash
./writer/build/WriterQt
```

### Build and run (one copy-paste block)

```bash
cd writer && cmake -B build && cmake --build build && ./build/WriterQt
```

---

## Quick reference (from repo root)

| Action | Command |
|--------|---------|
| Build fonter (Qt) only | `cd fonter/cachy && cmake -B build && cmake --build build` |
| Run fonter (Qt) only | `./fonter/cachy/build/HandwriteQt` |
| Build + run fonter (Qt) | `cd fonter/cachy && cmake -B build && cmake --build build && ./build/HandwriteQt` |
| Dev fonter (web) | `cd fonter/web && npm install && npm run dev` |
| Build writer only | `cd writer && cmake -B build && cmake --build build` |
| Run writer only | `./writer/build/WriterQt` |
| Build + run writer | `cd writer && cmake -B build && cmake --build build && ./build/WriterQt` |

Release builds (optional):

```bash
cd fonter/cachy && cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
cd writer && cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
```
