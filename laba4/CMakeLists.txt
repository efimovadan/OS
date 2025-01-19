cmake_minimum_required(VERSION 3.10)

project(laba4)

set(SOURCES_COM_PORT com_port/com_port.cpp)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(temperature_generator temperature_generator.cpp ${SOURCES_COM_PORT})

add_executable(main main.cpp ${SOURCES_COM_PORT})
