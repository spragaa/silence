add_executable(test_server
    main.cpp
    src/test_user_metadata_repository.cpp
    src/test_message_metadata_repository.cpp
    # src/test_message_text_repository.cpp
)

target_link_libraries(test_server
    PRIVATE
    server_lib 
    common_lib
    gtest
    gtest_main
    nlohmann_json::nlohmann_json
    pqxx
    redis++
    hiredis
)

if(ENABLE_COVERAGE)
    target_link_libraries(test_server PRIVATE gcov)
endif()

target_include_directories(test_server
    PUBLIC ${CMAKE_SOURCE_DIR}/tests/test_server/include
    PUBLIC ${CMAKE_SOURCE_DIR}/common/include
    PUBLIC ${CMAKE_SOURCE_DIR}/server/include
    PRIVATE ${nlohmann_json_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/thirdparty/libpqxx
    ${redis_plus_plus_SOURCE_DIR}/src/sw/redis++/
    ${hiredis_SOURCE_DIR}
)

include(GoogleTest)
gtest_discover_tests(test_server)