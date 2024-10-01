add_subdirectory(src)

add_executable(client
    main.cpp
)

target_link_libraries(client 
    PRIVATE 
    client_lib 
    common_lib
    nlohmann_json::nlohmann_json
)

if(ENABLE_COVERAGE AND ENABLE_CLIENT_COVERAGE)
    target_link_libraries(client PRIVATE gcov)
endif()