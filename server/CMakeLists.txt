find_package(Boost 1.74 REQUIRED COMPONENTS system thread)

add_subdirectory(src)

add_executable(server
    main.cpp
)

target_link_libraries(server 
    server_lib 
    common_lib
)