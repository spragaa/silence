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

option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
option(ENABLE_SERVER_COVERAGE "Enable coverage for server" OFF)
option(ENABLE_CLIENT_COVERAGE "Enable coverage for client" OFF)

if(ENABLE_COVERAGE)
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)
endif()

option(BUILD_TESTS "Build the test suite" OFF)
option(DEBUG "Build with debug logs" OFF)

if(DEBUG)
    add_compile_definitions(DEBUG_LEVEL=1)
else()
    add_compile_definitions(DEBUG_LEVEL=0)
endif()

add_definitions(-DSOURCE_DIR="${CMAKE_SOURCE_DIR}")

# this works fine, but I'd like to directly fetch boost library, I think there is a way to fetch only needed boost libs
find_package(Boost 1.74 REQUIRED COMPONENTS system thread)
include(FetchContent)

message("Fetching https://github.com/nlohmann/json.git, tag v3.11.2")
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

message("Fetching https://github.com/jtv/libpqxx.git, tag 7.9.2")
FetchContent_Declare(
    libpqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git
    GIT_TAG 7.9.2
)
set(PQXX_STATIC OFF CACHE BOOL "" FORCE)
set(SKIP_BUILD_TEST ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(libpqxx)

message("Fetching https://github.com/redis/hiredis.git, tag v1.0.2")
FetchContent_Declare(
    hiredis
    GIT_REPOSITORY https://github.com/redis/hiredis.git
    GIT_TAG v1.0.2
)
FetchContent_MakeAvailable(hiredis)

message("Fetching https://github.com/sewenew/redis-plus-plus.git, tag 1.3.5")
FetchContent_Declare( 
    redis_plus_plus
    GIT_REPOSITORY https://github.com/sewenew/redis-plus-plus.git
    GIT_TAG 1.3.5
)
set(REDIS_PLUS_PLUS_CXX_STANDARD 20)
set(REDIS_PLUS_PLUS_BUILD_TEST OFF)
set(REDIS_PLUS_PLUS_USE_TLS OFF)
FetchContent_MakeAvailable(redis_plus_plus)

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

add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(file_server)
add_subdirectory(client)

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

if(ENABLE_COVERAGE)
  add_custom_target(coverage_clean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
    COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete
    COMMAND ${CMAKE_CTEST_COMMAND}
    COMMAND lcov --capture --directory . --output-file coverage.info
    COMMAND lcov --remove coverage.info '/usr/*' '*/googletest/*' '*/libpqxx/*' --output-file coverage_filtered.info
    COMMAND genhtml coverage_filtered.info --output-directory coverage_report
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating clean coverage report"
  )
endif()