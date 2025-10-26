cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_PCH_WARN_INVALID ON)
set(CMAKE_BINARY_DIR ./build/)
set(CMAKE_SOURCE_DIR ./src/)

project(FBManga
  VERSION 0.0.0.0
  DESCRIPTION "Frame-buffer based manga viewer"
  LANGUAGES CXX
)

add_executable(fbmanga
  src/main.cpp
  src/view.cpp
  src/manga.cpp
  src/config.cpp
)

target_compile_options(fbmanga
  PUBLIC
    "--stdlib=libc++"
)
target_link_options(fbmanga
  PUBLIC
    "--stdlib=libc++"
)
target_precompile_headers(fbmanga
  PRIVATE
    <fcntl.h>
    <numbers>
    <sys/mman.h>
    <string>
    <chrono>
    <array>
    <vector>
    <bit>
    <unistd.h>
    <curl/curl.h>
    <cstring>
    <print>
    <format>
    <zlib.h>
    <map>
    <functional>
    "src/debug.h"
    <pnged.h>
    <sys/stat.h>
)

target_link_libraries(fbmanga curl z pnged)

