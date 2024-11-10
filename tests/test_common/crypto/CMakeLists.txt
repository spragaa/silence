add_executable(test_crypto
    main.cpp
    src/test_aes256.cpp
)

target_link_libraries(test_crypto
    crypto_lib
    gtest
    gtest_main
)

target_include_directories(test_crypto
    PUBLIC ${CMAKE_SOURCE_DIR}/common/crypto/include
)

target_compile_definitions(test_crypto
    PRIVATE
        TESTING
)

include(GoogleTest)
gtest_discover_tests(test_crypto)  