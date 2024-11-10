add_executable(test_common
    main.cpp
    src/test_user.cpp
    src/test_message.cpp
)

target_link_libraries(test_common
    common_lib
    gtest
    gtest_main
    nlohmann_json::nlohmann_json
)

target_include_directories(test_common
    PUBLIC ${CMAKE_SOURCE_DIR}/common/include
    PRIVATE ${nlohmann_json_SOURCE_DIR}/include
)

include(GoogleTest)
gtest_discover_tests(test_common)  

add_subdirectory(crypto)