add_library(file_server_lib
    file_server.cpp
)

add_library(file_server_client_lib
    file_server_client.cpp
)

target_link_libraries(file_server_lib
    pistache
)

target_link_libraries(file_server_client_lib
    Boost::boost
    Boost::system
    Boost::thread
    nlohmann_json::nlohmann_json
)

target_include_directories(file_server_lib
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${CMAKE_SOURCE_DIR}/common/include
    PRIVATE ${Pistache_INCLUDE_DIRS}
)

target_include_directories(file_server_client_lib
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${CMAKE_SOURCE_DIR}/common/include
    PRIVATE ${nlohmann_json_SOURCE_DIR}/include
)