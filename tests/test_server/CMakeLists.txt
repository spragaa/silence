add_executable(test_server
    main.cpp
    src/test_user_metadata_repository.cpp
    src/test_message_metadata_repository.cpp
)

target_link_libraries(test_server
    server_lib 
    common_lib
    gtest
    gtest_main
)

target_include_directories(test_server
    PUBLIC ${CMAKE_SOURCE_DIR}/tests/test_server/include
    PUBLIC ${CMAKE_SOURCE_DIR}/common/include
    PUBLIC ${CMAKE_SOURCE_DIR}/server/include
    PRIVATE ${nlohmann_json_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/thirdparty/libpqxx
)

include(GoogleTest)
gtest_discover_tests(test_server)  