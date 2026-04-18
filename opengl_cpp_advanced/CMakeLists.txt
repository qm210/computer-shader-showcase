cmake_minimum_required(VERSION 3.20)
project(gol_plus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# für Linux geeignet: "Vendor-Neutral Dispatch" von OpenGL.
# Apple hat eh verloren, weil wir OpenGL 4.3 brauchen :(
if(NOT APPLE)
  set(OpenGL_GL_PREFERENCE GLVND)
endif()

include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

add_executable(gol_plus main.cpp)
target_compile_definitions(gol_plus PRIVATE GLFW_INCLUDE_NONE)
target_link_libraries(gol_plus PRIVATE glfw OpenGL::GL)

if (MSVC)
  target_compile_options(gol_plus PRIVATE /W4 /permissive-)
else()
  target_compile_options(gol_plus PRIVATE -Wall -Wextra -Wpedantic)
endif()

# shaders/ kopieren, weil zur Laufzeit gelesen
add_custom_command(TARGET gol_plus POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:gol_plus>/shaders
)
