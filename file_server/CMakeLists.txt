add_subdirectory(src)

add_executable(file_server
    main.cpp
)

target_link_libraries(file_server 
    file_server_lib 
    common_lib
)

target_include_directories(file_server
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)