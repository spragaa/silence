add_library(file_server_lib
    src/file_server.cpp
)
add_library(file_server_client_lib
    src/file_server_client.cpp
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
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_SOURCE_DIR}/common/include
    PRIVATE ${Pistache_INCLUDE_DIRS}
)
target_include_directories(file_server_client_lib
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_SOURCE_DIR}/common/include
    PRIVATE ${nlohmann_json_SOURCE_DIR}/include
)

add_executable(file_server
    file_server_main.cpp
)
add_executable(file_server_client
    file_server_client_main.cpp
)

target_link_libraries(file_server 
    PRIVATE
    file_server_lib 
    common_lib
)
target_link_libraries(file_server_client 
    PRIVATE
    file_server_client_lib 
    common_lib
)

target_include_directories(file_server
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(file_server_client
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)