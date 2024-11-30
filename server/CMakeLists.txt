add_library(server_lib
    src/server.cpp
    src/postgres_db_manager.cpp
    src/user_metadata_repository.cpp
    src/message_metadata_repository.cpp
    src/message_text_repository.cpp
    src/request_handler.cpp
    src/repository_manager.cpp
    src/connected_clients_manager.cpp
)

target_link_libraries(server_lib
    file_server_client_lib
    crypto_lib
    Boost::boost
    Boost::system
    Boost::thread
    nlohmann_json::nlohmann_json
    pqxx
    redis++
    hiredis
)

target_include_directories(server_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC ${CMAKE_SOURCE_DIR}/common/include
    PUBLIC ${CMAKE_SOURCE_DIR}/crypto/include
    PUBLIC ${CMAKE_SOURCE_DIR}/file_server/include
    PRIVATE ${nlohmann_json_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/thirdparty/libpqxx
    ${redis_plus_plus_SOURCE_DIR}/src/sw/redis++/
    ${hiredis_SOURCE_DIR}
)

add_executable(server
    main.cpp
)

target_link_libraries(server 
    server_lib 
    common_lib
)