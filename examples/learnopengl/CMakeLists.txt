cmake_minimum_required(VERSION 3.2)
project(learnopengl)

# Flags
set(CMAKE_CXX_STANDARD 20)
set(${CMAKE_CXX_FLAGS} "${CMAKE_CXX_FLAGS} -fno-exceptions")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

# Target
set(SOURCES src/shader.cpp)
include_directories(include)
add_executable(${PROJECT_NAME} src/main.cpp ${SOURCES})

# GLFW
set(GLFW_DIR "${UNIVERSAL_LIB_DIR}/glfw")

target_link_libraries(${PROJECT_NAME} "glfw" "${GLFW_LIBRARIES}")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLFW_DIR}/include")

# GLAD
set(GLAD_DIR "${UNIVERSAL_LIB_DIR}/glad")
target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")

target_include_directories(${PROJECT_NAME} PRIVATE "${GLAD_DIR}/include")
target_link_libraries(${PROJECT_NAME} "glad" "${CMAKE_DL_LIBS}")

# STB_IMAGE
set(STB_DIR "${UNIVERSAL_LIB_DIR}/stb_image")

target_include_directories(${PROJECT_NAME} PRIVATE "${STB_DIR}/include")
target_link_libraries(${PROJECT_NAME} "stb_image")

# GLM
set(GLM_DIR "${UNIVERSAL_LIB_DIR}/glm")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLM_DIR}/include")

# Other libs
include_directories("${LIB_DIR}/other")
