# Handwrite software

This repository contains two Qt 6 desktop applications:

| Directory | Target (executable) | Purpose |
|-----------|---------------------|---------|
| [`fonter/`](fonter/) | `HandwriteQt` | Draw glyphs in selections and export font `.txt` files |
| [`writer/`](writer/) | `WriterQt` | Preview typed text as handwriting from a font folder, layout, and run simulation |

**Requirements:** CMake 3.21+, a C++17 toolchain, and Qt 6.5+ (Quick, QuickControls2, Widgets).

During configuration you may see `Could NOT find WrapVulkanHeaders` — that is normal for these apps and can be ignored unless you are building Qt Vulkan-specific features yourself.

---

## Fonter (`HandwriteQt`)

### Build

```bash
cd fonter
cmake -B build
cmake --build build
```

### Run

From the `fonter` directory (after a successful build):

```bash
./build/HandwriteQt
```

From the **repository root**:

```bash
./fonter/build/HandwriteQt
```

### Build and run (one copy-paste block)

```bash
cd fonter && cmake -B build && cmake --build build && ./build/HandwriteQt
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
| Build fonter only | `cd fonter && cmake -B build && cmake --build build` |
| Run fonter only | `./fonter/build/HandwriteQt` |
| Build + run fonter | `cd fonter && cmake -B build && cmake --build build && ./build/HandwriteQt` |
| Build writer only | `cd writer && cmake -B build && cmake --build build` |
| Run writer only | `./writer/build/WriterQt` |
| Build + run writer | `cd writer && cmake -B build && cmake --build build && ./build/WriterQt` |

Release builds (optional):

```bash
cd fonter && cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
cd writer && cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
```
