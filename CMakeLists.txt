cmake_minimum_required(VERSION 3.2)
project(high-shift)

# Flags
set(CMAKE_CXX_STANDARD 20)
set(${CMAKE_CXX_FLAGS} "${CMAKE_CXX_FLAGS} -fno-exceptions")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(UNIVERSAL_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

add_definitions(-mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mpopcnt -mavx -mfma -mavx2)


# Target
set(TEST_TARGET tests)

file(GLOB_RECURSE TEST_SOURCES
        "${PROJECT_SOURCE_DIR}/test/*.h"
        "${PROJECT_SOURCE_DIR}/test/*.cpp")

file(GLOB_RECURSE ENGINE_SOURCES
        "${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/*.cpp")

include_directories(include)
add_library(${PROJECT_NAME} STATIC ${ENGINE_SOURCES} include/render/lights.h)
add_executable(${TEST_TARGET} ${TEST_SOURCES})
target_link_libraries(${TEST_TARGET} ${PROJECT_NAME})

# GLFW
set(GLFW_DIR "${LIB_DIR}/glfw")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory("${GLFW_DIR}")

target_link_libraries(${PROJECT_NAME} "glfw" "${GLFW_LIBRARIES}")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLFW_DIR}/include")

# GLAD
set(GLAD_DIR "${LIB_DIR}/glad")
add_library("glad" "${GLAD_DIR}/src/glad.c")
target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")

target_include_directories(${PROJECT_NAME} PRIVATE "${GLAD_DIR}/include")
target_link_libraries(${PROJECT_NAME} "glad" "${CMAKE_DL_LIBS}")

# STB_IMAGE
set(STB_DIR "${LIB_DIR}/stb_image")
add_subdirectory(${STB_DIR})

target_include_directories(${PROJECT_NAME} PRIVATE "${STB_DIR}/include")
target_link_libraries(${PROJECT_NAME} "stb_image")

# GLM
set(GLM_DIR "${LIB_DIR}/glm")
include_directories("${GLM_DIR}/include")

# Assimp
set(ASSIMP_DIR "${UNIVERSAL_LIB_DIR}/assimp")

add_subdirectory(${ASSIMP_DIR} assimp_build EXCLUDE_FROM_ALL)

set(ASSIMP_BUILD_STATIC_LIB  ON CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)

target_link_libraries(${PROJECT_NAME} "assimp")

# Other libs
include_directories("${LIB_DIR}/other")

#examples
file(GLOB EXAMPLE_SOURCES examples/*)
foreach (exampledir ${EXAMPLE_SOURCES})
    add_subdirectory(${exampledir} EXCLUDE_FROM_ALL)
endforeach ()
