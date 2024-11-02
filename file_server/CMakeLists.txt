add_subdirectory(src)

add_executable(file_server
    file_server_main.cpp
)

add_executable(file_server_client
    file_server_client_main.cpp
)

target_link_libraries(file_server 
    PRIVATE
    file_server_lib 
    common_lib
)

target_link_libraries(file_server_client 
    PRIVATE
    file_server_client_lib 
    common_lib
)

target_include_directories(file_server
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(file_server_client
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)