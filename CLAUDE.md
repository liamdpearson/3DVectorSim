# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

A 3D vector visualization tool built with C++ and SFML 2.5.1. Single-file project (`main.cpp`, ~200 LOC) implementing a custom perspective projection engine with interactive camera control.

## Build & Run

SFML must be installed at `C:/SFML-2.5.1/`. Compiler is TDM-GCC 64-bit.

```bash
# Compile
g++ -c main.cpp -I"C:/SFML-2.5.1/include" -DSFML_STATIC

# Link
g++ main.o -o main -L"C:/SFML-2.5.1/lib" -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lfreetype -lwinmm -lgdi32 -mwindows

# Run
./main.exe
```

No test infrastructure or linter is configured.

## Architecture

Everything lives in `main.cpp`. Key types:

- **`Vector3`** — core 3D vector struct with `+`, `-`, `==`, and `length()`
- **`ColoredVector`** — `Vector3` + SFML color for rendering
- **`TextVector`** — `Vector3` + label string (defined but unused)
- **`dot_prod()` / `proj()`** — dot product and vector projection utilities

**Rendering pipeline** (runs every frame in the main loop):

1. Mouse input updates spherical camera state (`cam_yaw`, `cam_pitch`, `cam_dist`)
2. Camera basis vectors are rebuilt from spherical coords each frame (forward, right, up via cross products)
3. Each 3D vector is projected onto the camera plane using vector rejection, then perspective-divided by z-depth
4. SFML draws lines from the origin to each projected screen point

**Non-obvious implementation details:**

- Perspective projection is done manually with vector math rather than a matrix — `pixel_distance = screen_distance / z_distance`
- Vectors nearly behind the camera (`along < 1.0`) are clamped to `z = 0.001f` to avoid division-by-zero
- Camera up vector is the rejection of the world Y-axis from the camera forward vector (not a fixed up vector)
