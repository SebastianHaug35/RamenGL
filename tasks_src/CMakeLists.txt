cmake_minimum_required(VERSION 3.30.2)
project(Examples VERSION 0.0.1)

if (NOT RAMEN_BUILD_PROGRAM_NAME)
    add_subdirectory(./task02/)
    add_subdirectory(./task03/)
    add_subdirectory(./task04/)
else()
    add_subdirectory(./${RAMEN_BUILD_PROGRAM_NAME}/)
endif()

