add_library(server_lib
    server.cpp
)

target_link_libraries(server_lib
    Boost::boost
    Boost::system
    Boost::thread
)

target_include_directories(server_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PUBLIC ${CMAKE_SOURCE_DIR}/common/include
    PRIVATE ${nlohmann_json_SOURCE_DIR}/include
)
