cmake_minimum_required(VERSION 3.10)

set(SOURCES
    async_process.cpp
    shared_memory.cpp
    logger.cpp
    mutex.cpp
)

project(laba3)

include_directories(include)

set(CMAKE_CXX_STANDARD 17)

add_executable(laba3 main.cpp ${SOURCES})
