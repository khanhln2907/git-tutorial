project(git_tutorial)
cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY /DIST/home/lab_espl_stud10/git-tutorial1/CMakeFiles/../bin/)

include_directories(include)

file(GLOB SOURCES "src/*.c")

add_executable(foo ${SOURCES})

