add_library(common_lib
    debug.cpp
    common.cpp
    message.cpp
)

target_link_libraries(common_lib
    Boost::boost
    Boost::system
    Boost::thread
)

target_include_directories(common_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
