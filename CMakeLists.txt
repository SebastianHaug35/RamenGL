cmake_minimum_required(VERSION 3.30.2)
project(RamenGL VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # compile_commands.json
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(./dependencies/SDL EXCLUDE_FROM_ALL)

add_executable(task02 
    ./dependencies/glad/src/glad.c

    ./dependencies/tiny_obj_loader.h
    
    ./dependencies/imgui/imgui.cpp
    ./dependencies/imgui/imgui_impl_sdl3.cpp
    ./dependencies/imgui/imgui_impl_opengl3.cpp
    ./dependencies/imgui/imgui_draw.cpp
    ./dependencies/imgui/imgui_tables.cpp
    ./dependencies/imgui/imgui_demo.cpp
    ./dependencies/imgui/imgui_widgets.cpp

    ./ramen/rgl_math.cpp
    ./ramen/rgl_platform.cpp

    task02.cpp)
target_include_directories(task02 PUBLIC
    ./dependencies/glad/include
    ./dependencies/)
target_link_libraries(task02 PRIVATE SDL3::SDL3)
