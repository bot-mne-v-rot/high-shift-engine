cmake_minimum_required(VERSION 3.2)
project(high-shift)

# Flags
set(CMAKE_CXX_STANDARD 20)
set(${CMAKE_CXX_FLAGS} "${CMAKE_CXX_FLAGS} -fno-exceptions")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

# Target
set(SOURCES src/shader.cpp)
set(TEST_SOURCES test/test.cpp test/ecs/test_world.cpp test/ecs/test_system.cpp test/ecs/test_dispatcher.cpp test/ecs/storages/test_vec_storage.cpp test/ecs/test_id_set.cpp test/ecs/test_join.cpp)
set(TEST_TARGET tests)

include_directories(include)
add_executable(${PROJECT_NAME} src/main.cpp ${SOURCES})
add_executable(${TEST_TARGET} ${SOURCES} ${TEST_SOURCES})

# GLFW
set(GLFW_DIR "${LIB_DIR}/glfw")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory("${GLFW_DIR}")

target_link_libraries(${PROJECT_NAME} "glfw" "${GLFW_LIBRARIES}")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLFW_DIR}/include")

target_link_libraries(${TEST_TARGET} "glfw" "${GLFW_LIBRARIES}")
target_include_directories(${TEST_TARGET} PRIVATE "${GLFW_DIR}/include")

# GLAD
set(GLAD_DIR "${LIB_DIR}/glad")
add_library("glad" "${GLAD_DIR}/src/glad.c")
target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")

target_include_directories(${PROJECT_NAME} PRIVATE "${GLAD_DIR}/include")
target_link_libraries(${PROJECT_NAME} "glad" "${CMAKE_DL_LIBS}")

target_include_directories(${TEST_TARGET} PRIVATE "${GLAD_DIR}/include")
target_link_libraries(${TEST_TARGET} "glad" "${CMAKE_DL_LIBS}")

# STB_IMAGE
set(STB_DIR "${LIB_DIR}/stb_image")
add_subdirectory(${STB_DIR})

target_include_directories(${PROJECT_NAME} PRIVATE "${STB_DIR}/include")
target_link_libraries(${PROJECT_NAME} "stb_image")

target_include_directories(${TEST_TARGET} PRIVATE "${STB_DIR}/include")
target_link_libraries(${TEST_TARGET} "stb_image")

# GLM
set(GLM_DIR "${LIB_DIR}/glm")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLM_DIR}/include")
target_include_directories(${TEST_TARGET} PRIVATE "${GLM_DIR}/include")

# Other libs
include_directories("${LIB_DIR}/other")
