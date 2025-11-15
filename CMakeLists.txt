cmake_minimum_required(VERSION 4.2)#nice
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
set(CMAKE_CXX_COMPILER_IMPORT_STD 23 26)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_COMPILE_WARNING_AS_ERROR OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) #for clangd
set(CMAKE_PCH_WARN_INVALID ON)
set(CMAKE_BINARY_DIR ./build)
set(CMAKE_SOURCE_DIR ./src)
set(CMAKE_CXX_COMPILER /lib/llvm-20/bin/clang)
set(CMAKE_GENERATOR Ninja)
set(CMAKE_CXX_FLAGS "-mcpu='cortex-a53' -Wall \
-fapprox-func -fmessage-length=80")
#needed for __crc32b() builtin-------^^^^^^^^^^^^^^^^^^

find_package(RapidJSON)


project(FBManga
  VERSION 0.0.0.0
  DESCRIPTION "Frame-buffer based manga viewer"
  LANGUAGES CXX
)

include(FetchContent)
FetchContent_Declare(
  std
  URL "file:///home/mark/llvm-project/build/modules/c++/v1/"
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  SYSTEM
)
FetchContent_MakeAvailable(std)

add_executable(fbmanga
  src/main.cpp
)
add_dependencies(fbmanga std.compat)
target_sources(fbmanga
  PRIVATE
  FILE_SET CXX_MODULES
  BASE_DIRS src
  FILES
  src/png.cppm
  src/pngutil.cppm
  src/fb.cppm
  src/types.cppm
  src/ui.cppm
  src/config.cppm
  src/manga.cppm
)


target_link_libraries(fbmanga curl ncurses z std.compat)

