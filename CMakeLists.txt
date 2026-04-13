cmake_minimum_required(VERSION 3.20)
project(gol_showcase LANGUAGES CXX)

# Zielt aktuell nur auf Win32

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

add_executable(gol_showcase main.cpp)
target_compile_definitions(gol_showcase PRIVATE GLFW_INCLUDE_NONE)
target_link_libraries(gol_showcase PRIVATE glfw opengl32)

if (MSVC)
  target_compile_options(gol_showcase PRIVATE /W4 /permissive-)
else()
  target_compile_options(gol_showcase PRIVATE -Wall -Wextra -Wpedantic)
endif()

# shaders/ kopieren, weil zur Laufzeit gelesen
add_custom_command(TARGET gol_showcase POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:gol_showcase>/shaders
)
