add_library(client_lib
    src/client.cpp
)

target_link_libraries(client_lib
    Boost::boost
    Boost::system
    Boost::thread
)

target_include_directories(client_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC ${CMAKE_SOURCE_DIR}/common/include
    PUBLIC ${CMAKE_SOURCE_DIR}/common/crypto/include
    PRIVATE ${nlohmann_json_SOURCE_DIR}/include
)

add_executable(client
    main.cpp
)

target_link_libraries(client 
PRIVATE client_lib 
        common_lib
        crypto_lib
        nlohmann_json::nlohmann_json
)