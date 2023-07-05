cmake_minimum_required(VERSION 3.2)
project(lighting)

# Flags
set(CMAKE_CXX_STANDARD 20)
set(${CMAKE_CXX_FLAGS} "${CMAKE_CXX_FLAGS} -fno-exceptions")

# Target
file(GLOB_RECURSE SOURCES
        "${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/*.cpp")
include_directories(include)
add_executable(${PROJECT_NAME} src/main.cpp ${SOURCES})

# Main library
target_link_libraries(${PROJECT_NAME} "high-shift")
