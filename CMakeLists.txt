cmake_minimum_required(VERSION 3.30.2)
project(RamenGL VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # compile_commands.json
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")


add_subdirectory(./dependencies/SDL EXCLUDE_FROM_ALL)
option(PHYSFS_BUILD_TEST "Build physfs test program" OFF)
add_subdirectory(./dependencies/physfs/)
set(BUILD_SHARED_LIBS ON)
add_subdirectory(./dependencies/tinyobjloader/)
set(BUILD_SHARED_LIBS OFF)


add_subdirectory(./tasks_src/)
