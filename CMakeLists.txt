cmake_minimum_required(VERSION 3.14)
project(silence)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

include(CheckIPOSupported)
add_definitions(-DSOURCE_DIR="${CMAKE_SOURCE_DIR}")

include(FetchContent)
message("Fetching https://github.com/pistacheio/pistache.git, tag 0.0.5")
FetchContent_Declare(
    pistache
    GIT_REPOSITORY https://github.com/pistacheio/pistache.git
    GIT_TAG 0.0.5
)
set(PISTACHE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(PISTACHE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(PISTACHE_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(pistache)

add_library(file_server_lib
    file_server.cpp
)

target_link_libraries(file_server_lib
    pistache
)

target_include_directories(file_server_lib
    PRIVATE ${Pistache_INCLUDE_DIRS}
)
add_executable(file_server
    file_server_main.cpp
)

target_include_directories(file_server
    PRIVATE ${Pistache_INCLUDE_DIRS}
)

target_link_libraries(file_server 
    PRIVATE
    file_server_lib
    pistache 
)

enable_testing()

message("Fetching https://github.com/google/googletest.git, tag release-1.12.1")
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

find_package(GTest REQUIRED)

# add_executable(test_file_server
#     test_file_server.cpp
# )

# target_link_libraries(test_file_server
#     PRIVATE
#     file_server_lib
#     gtest
#     pistache
# )

# target_include_directories(test_file_server
#     PRIVATE ${Pistache_INCLUDE_DIRS}
# )

# include(GoogleTest)
# gtest_discover_tests(test_file_server)