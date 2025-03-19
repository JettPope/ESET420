cmake_minimum_required(VERSION 3.10)
project(BaseUnit C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Use SDL2_ttf instead of sdl2-ttf
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2_TTF REQUIRED IMPORTED_TARGET SDL2_ttf)

add_executable(BaseUnit BaseUnit.c)

target_link_libraries(BaseUnit ${SDL2_LIBRARIES} PkgConfig::SDL2_TTF)
