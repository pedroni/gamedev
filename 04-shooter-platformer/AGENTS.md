# Adding Libraries

## pkg-config libraries (SDL3_image)
1. Install via Homebrew
2. Add `pkg_check_modules()` to CMakeLists.txt
3. Link libraries with `target_link_libraries()`
4. Add includes with `target_include_directories()`
5. Check pkg-config name: `pkg-config --list-all | grep <library>`

## CMake package libraries (GLM)
1. Install via Homebrew
2. Add `find_package()` to CMakeLists.txt
3. Link with target name (e.g., `glm::glm`)
4. Explicitly add include path to `target_include_directories()` for clangd auto-imports
5. Use `/opt/homebrew/opt/<package>/include` path

## Clangd auto-import configuration
1. Add explicit include paths to `.clangd` under `CompileFlags.Add`
2. Use versioned Cellar paths for SDL libraries
3. Use `/opt/homebrew/opt/` symlink paths for others
4. Restart editor after changes

## Build workflow
1. `cmake ..` in build directory
2. `make` to compile
3. `compile_commands.json` auto-updates for clangd
