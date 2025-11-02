# My Game

A C++11 game project using CMake, Clang, and SDL3.

## Prerequisites

- CMake 3.15 or higher
- Clang compiler
- SDL3 (install via: `brew install sdl3`)

## Building

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

The executable will be generated at `build/mygame`.

## Running

```bash
./build/mygame
```

Press ESC or close the window to exit.
