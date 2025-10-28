cmake_minimum_required(VERSION 3.16)
project(mygame)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Debug configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Find raylib using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)

# Add executable
add_executable(${PROJECT_NAME} main.cpp)

# Link raylib with explicit library paths
target_link_libraries(${PROJECT_NAME} ${RAYLIB_LDFLAGS})
target_include_directories(${PROJECT_NAME} PRIVATE ${RAYLIB_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PRIVATE ${RAYLIB_CFLAGS_OTHER})

# Platform-specific settings
if(WIN32)
    target_link_libraries(${PROJECT_NAME} opengl32 gdi32 winmm)
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    target_link_libraries(${PROJECT_NAME}
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
        ${COREFOUNDATION_LIBRARY}
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} GL m pthread dl rt X11)
endif()
