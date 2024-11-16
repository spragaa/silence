add_library(crypto_lib
    src/crypto_utils.cpp
    src/aes256.cpp
    src/el_gamal_encryption.cpp
    src/el_gamal_digital_signature.cpp
)

target_link_libraries(crypto_lib
    common_lib
    Boost::boost
)

target_include_directories(crypto_lib
    PUBLIC ${CMAKE_SOURCE_DIR}/common/include
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)