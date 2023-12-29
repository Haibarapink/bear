cmake_minimum_required(VERSION 3.22)
project(bear)

set(CMAKE_CXX_STANDARD 23)
include_directories(tests ${PROJECT_SOURCE_DIR})
add_subdirectory(tests)