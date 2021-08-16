#cmake_minimum_required(VERSION 3.2)
#project(model_loading)
#
## Flags
#set(CMAKE_CXX_STANDARD 20)
#set(${CMAKE_CXX_FLAGS} "${CMAKE_CXX_FLAGS} -fno-exceptions")
#set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
#
## Target
#file(GLOB_RECURSE SOURCES
#        "${PROJECT_SOURCE_DIR}/src/*.h"
#        "${PROJECT_SOURCE_DIR}/src/*.cpp")
#include_directories(include)
#add_executable(${PROJECT_NAME} src/main.cpp ${SOURCES})
#
## Main library
#target_link_libraries(${PROJECT_NAME} "high-shift")
