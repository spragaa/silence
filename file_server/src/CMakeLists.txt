add_library(file_server_lib
    file_server.cpp
)

target_link_libraries(file_server_lib
    pistache
)

target_include_directories(file_server_lib
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${CMAKE_SOURCE_DIR}/common/include
    PRIVATE ${Pistache_INCLUDE_DIRS}
)