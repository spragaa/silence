add_library(common_lib
    src/debug.cpp
    src/common.cpp
    src/message.cpp
    src/message_metadata.cpp
    src/message_text.cpp
    src/user.cpp
)

target_link_libraries(common_lib
    Boost::boost
    Boost::system
    Boost::thread
    nlohmann_json::nlohmann_json
)

target_include_directories(common_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${nlohmann_json_SOURCE_DIR}/include
)
