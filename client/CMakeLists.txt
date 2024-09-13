find_package(Boost 1.74 REQUIRED COMPONENTS system thread)

add_subdirectory(src)

add_executable(client
    main.cpp
)

target_link_libraries(client 
    client_lib 
    common_lib
)