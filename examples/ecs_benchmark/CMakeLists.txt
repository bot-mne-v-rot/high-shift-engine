cmake_minimum_required(VERSION 3.2)
project(ecs_benchmark)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Target
set(SOURCES src/idset_iterator_vs_lambda.cpp)
include_directories(include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(${PROJECT_NAME} ${SOURCES})
