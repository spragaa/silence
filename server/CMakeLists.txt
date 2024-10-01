add_subdirectory(src)

add_executable(server
    main.cpp
)

target_link_libraries(server 
    PRIVATE
    server_lib 
    common_lib
)

if(ENABLE_COVERAGE AND ENABLE_SERVER_COVERAGE)
    target_link_libraries(server PRIVATE gcov)
endif()