# Handwrite software

This repository contains two Qt 6 desktop applications and a browser-based font editor:


| Directory                        | Target         | Purpose                                                                          |
| -------------------------------- | -------------- | -------------------------------------------------------------------------------- |
| `[fonter/cachy/](fonter/cachy/)` | `HandwriteQt`  | Qt 6: draw glyphs in selections and export font `.txt` files                     |
| `[fonter/web/](fonter/web/)`     | Web SPA (Vite) | Same editor in the browser (tablet-first)                                        |
| `[writer/](writer/)`             | `WriterQt`     | Preview typed text as handwriting from a font folder, layout, and run simulation |


**Requirements:** CMake 3.21+, a C++17 toolchain, and Qt 6.5+ (Quick, QuickControls2, Widgets). For CNC serial control in Writer, install **qt6-serialport** (e.g. `pacman -S qt6-serialport`). **qt6-base and qt6-serialport must be the same minor version** (e.g. both 6.11.1); a mismatch causes link errors or Writer will build with a stubbed console. On Arch/CachyOS: `sudo pacman -Syu qt6-base qt6-serialport`.

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

### Writer performance/tuning protocol

For handwriting jobs with many short moves, use this repeatable tuning flow:

1. In Writer settings, enable simplification and set conservative values first.
2. Set streaming preset to `balanced` (or `fast` after stability check).
3. Open Writer settings > machine tuning:
   - `Junction deviation $11`
   - `Acceleration X $120`
   - `Acceleration Y $121`
4. Start conservative, test one page, then increase acceleration gradually.
5. Use **Apply tuning to GRBL ($11/$120/$121)** to write values directly.
6. Record runtime + quality, then iterate.

Recommended safe starting point:
- `$11 = 0.030`
- `$120 = 300`
- `$121 = 300`

---

## Quick reference (from repo root)


| Action                  | Command                                                                           |
| ----------------------- | --------------------------------------------------------------------------------- |
| Build fonter (Qt) only  | `cd fonter/cachy && cmake -B build && cmake --build build`                        |
| Run fonter (Qt) only    | `./fonter/cachy/build/HandwriteQt`                                                |
| Build + run fonter (Qt) | `cd fonter/cachy && cmake -B build && cmake --build build && ./build/HandwriteQt` |
| Dev fonter (web)        | `cd fonter/web && npm install && npm run dev`                                     |
| Build writer only       | `cd writer && cmake -B build && cmake --build build`                              |
| Run writer only         | `./writer/build/WriterQt`                                                         |
| Build + run writer      | `cd writer && cmake -B build && cmake --build build && ./build/WriterQt`          |


Release builds (optional):

```bash
cd fonter/cachy && cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
cd writer && cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
```

