cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_PCH_WARN_INVALID ON)
set(CMAKE_BINARY_DIR ./build/)

project(FBManga
  VERSION 0.0.0.0
  DESCRIPTION "Frame-buffer based manga viewer"
  LANGUAGES CXX
)

add_executable(fbmanga
  main.cpp
  png.cpp
  view.cpp
  mangadex.cpp
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
    <sys/mman.h>
    <string>
    <chrono>
    <array>
    <vector>
    <sys/stat.h>
    <libdeflate.h>
    <bit>
    <unistd.h>
    <curl/curl.h>
    <cstring>
    <print>
    <mdspan>
    <format>
    <zlib.h>
    <map>
    <functional>
    "debug.h"
)

target_link_libraries(fbmanga curl z)

