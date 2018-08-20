cmake_minimum_required (VERSION 2.6)
project (OpenGLWindow)
# The version number.
set (OpenGLWindow_VERSION_MAJOR 1)
set (OpenGLWindow_VERSION_MINOR 1)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# GLFW
include_directories ("${PROJECT_SOURCE_DIR}/glfw")
# LZMA
include_directories ("${PROJECT_SOURCE_DIR}/LZMA")
# GLAD GLI GLM
include_directories ("${PROJECT_SOURCE_DIR}/include")

# build GLFW
set (GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set (GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory (glfw)
add_subdirectory (LZMA)
set (EXTRA_LIBS ${EXTRA_LIBS} glfw LZMA)

# source & header
file (GLOB SOURCES "src/*.*" "src/TextRender/*.*")

SOURCE_GROUP("TextRender" "${PROJECT_SOURCE_DIR}/src/TextRender")

add_executable (${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")

# link glfw
target_link_libraries (${PROJECT_NAME} ${EXTRA_LIBS})

add_custom_command(TARGET ${PROJECT_NAME} 
    POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/resource "$(OutDir)/resource"
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Shader "$(OutDir)/Shader"
)

