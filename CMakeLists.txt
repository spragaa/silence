cmake_minimum_required(VERSION 3.14)
project(chat_application)

set(CMAKE_CXX_STANDARD 20)

add_definitions(-DSOURCE_DIR="${CMAKE_SOURCE_DIR}")

find_package(Boost 1.74 REQUIRED COMPONENTS system thread)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

add_subdirectory(common)

add_subdirectory(server)

add_subdirectory(client)
