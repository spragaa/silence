add_library(common_lib
    debug.cpp
    common.cpp
    message.cpp
    message_metadata.cpp
    message_text.cpp
    user.cpp
)

target_link_libraries(common_lib
    Boost::boost
    Boost::system
    Boost::thread
    nlohmann_json::nlohmann_json
)

target_include_directories(common_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${nlohmann_json_SOURCE_DIR}/include
)
