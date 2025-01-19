add_library(background STATIC background.cpp)

target_include_directories(background PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS background
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

install(FILES process.h DESTINATION include)