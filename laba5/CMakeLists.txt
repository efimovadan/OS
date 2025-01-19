cmake_minimum_required(VERSION 3.10)

project(laba4)

set(SOURCES_COM_PORT com_port/com_port.cpp)

include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

include_directories(${PQXX_INCLUDE_DIRS})
link_directories(${PQXX_LIBRARY_DIRS})

add_executable(temperature_generator temperature_generator.cpp ${SOURCES_COM_PORT})
target_link_libraries(temperature_generator ${PQXX_LIBRARIES})

add_executable(main main.cpp ${SOURCES_COM_PORT})
target_link_libraries(main ${PQXX_LIBRARIES})

if (WIN32)
    target_link_libraries(main ws2_32)
endif()