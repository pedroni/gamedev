# My Game

A C++11 game project using CMake, Clang, and SDL2.

## Prerequisites

- CMake 3.15 or higher
- Clang compiler
- SDL2 (install via: `brew install sdl2`)

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

## Project Structure

```
.
├── CMakeLists.txt      # CMake configuration
├── .clangd             # Clang language server config
├── .clang-format       # Code formatting rules
├── .gitignore          # Git ignore rules
├── src/                # Source files
│   └── main.cpp
└── include/            # Header files
```
