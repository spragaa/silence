add_subdirectory(src)

add_executable(client
    main.cpp
)

target_link_libraries(client 
PRIVATE client_lib 
        common_lib
        nlohmann_json::nlohmann_json
)