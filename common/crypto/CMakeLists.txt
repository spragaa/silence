add_library(crypto_lib
    src/crypto_utils.cpp
    src/aes256.cpp
    src/el_gamal_encryption.cpp
    src/dsa.cpp
    src/hybrid_crypto_system.cpp
    src/user_crypto_keys.cpp
    src/sha256.cpp
)

target_link_libraries(crypto_lib
    common_lib
    Boost::boost
    nlohmann_json::nlohmann_json
)

target_include_directories(crypto_lib
    PUBLIC ${CMAKE_SOURCE_DIR}/common/include
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${nlohmann_json_SOURCE_DIR}/include
)