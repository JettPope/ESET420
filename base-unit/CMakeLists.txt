cmake_minimum_required(VERSION 3.10)
project(BaseUnit C)

# Set the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Find SDL2 and SDL2_ttf packages
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

# Include SDL2 directories
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})

# Add the executable
add_executable(BaseUnit BaseUnit.c)

# Link SDL2 and SDL2_ttf
target_link_libraries(BaseUnit ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})
