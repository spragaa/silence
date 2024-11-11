add_library(crypto_lib
    src/aes256.cpp
    src/el_gamal.cpp
)

target_link_libraries(crypto_lib
    common_lib
)

target_include_directories(crypto_lib
    PUBLIC ${CMAKE_SOURCE_DIR}/common/include
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

