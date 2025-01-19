cmake_minimum_required(VERSION 3.10)

project(lab2)

add_subdirectory(background)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(sleep sleep.cpp)

add_executable(lab2 main.cpp)

target_link_libraries(lab2 PRIVATE background)