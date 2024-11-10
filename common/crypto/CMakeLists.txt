add_library(crypto_lib
    src/aes256.cpp
)

# target_link_libraries(crypto_lib
#     Boost::boost
#     Boost::system
#     Boost::thread
#     nlohmann_json::nlohmann_json
# )

target_include_directories(crypto_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
