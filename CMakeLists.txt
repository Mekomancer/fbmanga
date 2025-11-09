cmake_minimum_required(VERSION 3.31)#abitrary

set(CMAKE_CXX_STANDARD 26)# we going crazy here, its not even 2026 yet
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
#set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE iwyu)
#set(CMAKE_CXX_CLANG_TIDY clang-tidy)
#set(CMAKE_CXX_CPPCHECK cppcheck)
set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) #for clangd
set(CMAKE_PCH_WARN_INVALID ON)
set(CMAKE_BINARY_DIR ./build/)
set(CMAKE_SOURCE_DIR ./src/)
set(CMAKE_CXX_FLAGS "--stdlib=libc++ -mcpu='cortex-a53'")
#needed for __crc32b() builtin-------^^^^^^^^^^^^^^^^^^

find_package(RapidJSON)

project(FBManga
  VERSION 0.0.0.0
  DESCRIPTION "Frame-buffer based manga viewer"
  LANGUAGES CXX
)

add_executable(fbmanga
  src/fb.cpp
  src/main.cpp
  src/manga.cpp
  src/png.cpp
  src/ui.cpp
  src/util.cpp
)

target_precompile_headers(fbmanga
  PRIVATE
#C++ stdlib
    <array>
    <bit>
    <chrono>
    <cstdint>
    <cstring>
    <format>
    <functional>
    <map>
    <memory>
    <numbers>
    <print>
    <regex>
    <span>
    <string>
    <vector>
#System
    <arm_acle.h>
    <fcntl.h>
    <linux/fb.h>
    <sys/mman.h>
    <sys/stat.h>
    <sys/ioctl.h>
    <unistd.h>
#Libs
    <curl/curl.h>
    <curses.h>
    <rapidjson/document.h>
    <zlib.h>
)

target_link_libraries(fbmanga curl ncurses z)

