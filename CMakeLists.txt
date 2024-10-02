cmake_minimum_required(VERSION 3.14)
project(chat_application)

set(CMAKE_CXX_STANDARD 20)

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

find_package(Boost 1.74 REQUIRED COMPONENTS system thread)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(libpqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git
    GIT_TAG 7.9.2
)
FetchContent_MakeAvailable(libpqxx)

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