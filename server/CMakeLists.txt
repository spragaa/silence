add_subdirectory(src)

add_executable(server
    main.cpp
)

target_link_libraries(server 
    server_lib 
    common_lib
    nlohmann_json::nlohmann_json
)