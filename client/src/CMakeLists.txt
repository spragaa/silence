add_library(client_lib
    client.cpp
    aes.cpp
)

target_link_libraries(client_lib
    Boost::boost
    Boost::system
    Boost::thread
)

target_include_directories(client_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PUBLIC ${CMAKE_SOURCE_DIR}/common/include
    PRIVATE ${nlohmann_json_SOURCE_DIR}/include
)
