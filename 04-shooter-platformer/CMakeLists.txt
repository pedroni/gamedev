cmake_minimum_required(VERSION 3.15)
project(mygame)

# Set C++ standard to C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Force use of Clang
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wunused -Wunused-variable -Wunused-parameter -Wunused-function -Wshadow")

# Output binary to build/mygame
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Use pkg-config to find SDL3 from Homebrew
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)

# Add source files
set(SOURCES
    src/main.cpp
)

# Create executable
add_executable(mygame ${SOURCES})

# Link SDL3
target_link_directories(mygame PRIVATE ${SDL3_LIBRARY_DIRS})
target_link_libraries(mygame PRIVATE ${SDL3_LIBRARIES})

# Include directories
target_include_directories(mygame PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SDL3_INCLUDE_DIRS}
)

target_compile_options(mygame PRIVATE ${SDL3_CFLAGS_OTHER})
