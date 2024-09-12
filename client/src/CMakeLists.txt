add_library(client_lib
    client.cpp
)

target_include_directories(client_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
