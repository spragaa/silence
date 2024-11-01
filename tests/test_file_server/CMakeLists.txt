add_executable(test_file_server
    main.cpp
    src/test_file_server.cpp
)

target_link_libraries(test_file_server
    file_server_lib 
    common_lib
    gtest
    gtest_main
    pistache
)

target_include_directories(test_file_server
    PRIVATE ${CMAKE_SOURCE_DIR}/file_server/include
    PRIVATE ${CMAKE_SOURCE_DIR}/common/include
    PRIVATE ${Pistache_INCLUDE_DIRS}
)

include(GoogleTest)
gtest_discover_tests(test_file_server)