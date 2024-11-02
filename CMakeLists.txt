cmake_minimum_required(VERSION 3.14)
project(silence)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

cmake_policy(SET CMP0048 NEW) 
cmake_policy(SET CMP0069 NEW) 
cmake_policy(SET CMP0077 NEW) 

set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)
if(supported)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(WARNING "IPO is not supported: ${error}")
endif()

option(BUILD_TESTS "Build the test suite" OFF)
option(DEBUG "Build with debug logs" OFF)

if(DEBUG)
    add_compile_definitions(DEBUG_LEVEL=1)
else()
    add_compile_definitions(DEBUG_LEVEL=0)
endif()

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

add_subdirectory(file_server)

if(BUILD_TESTS)
    enable_testing()
    
    message("Fetching https://github.com/google/googletest.git, tag release-1.12.1")
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    
    add_subdirectory(tests)
endif()