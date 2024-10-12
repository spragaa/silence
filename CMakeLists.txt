cmake_minimum_required(VERSION 3.14)
project(chat_application)

set(CMAKE_CXX_STANDARD 20)

option(BUILD_TESTS "Build the test suite" OFF)
option(DEBUG "Build with debug logs" OFF)

if(DEBUG)
    add_compile_definitions(DEBUG_LEVEL=1)
else()
    add_compile_definitions(DEBUG_LEVEL=0)
endif()

add_definitions(-DSOURCE_DIR="${CMAKE_SOURCE_DIR}")

find_package(Boost 1.74 REQUIRED COMPONENTS system thread)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# disable build tests
FetchContent_Declare(
    libpqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git
    GIT_TAG 7.9.2
)
FetchContent_MakeAvailable(libpqxx)
set(SKIP_BUILD_TEST ON)

FetchContent_Declare(
    hiredis
    GIT_REPOSITORY https://github.com/redis/hiredis.git
    GIT_TAG v1.0.2
)
FetchContent_MakeAvailable(hiredis)

FetchContent_Declare( 
    redis_plus_plus
    GIT_REPOSITORY https://github.com/sewenew/redis-plus-plus.git
    GIT_TAG 1.3.5
)
set(REDIS_PLUS_PLUS_CXX_STANDARD 20)
set(REDIS_PLUS_PLUS_BUILD_TEST OFF)
set(REDIS_PLUS_PLUS_USE_TLS OFF)
FetchContent_MakeAvailable(redis_plus_plus)

add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)

if(BUILD_TESTS)
    enable_testing()
    
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    
    add_subdirectory(tests)
endif()