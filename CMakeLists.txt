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
set(CMAKE_POLICY_DEFAULT_CMP0167 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0169 OLD)
set(CMAKE_POLICY_DEFAULT_CMP0175 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0002 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0148 NEW)

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
    GIT_SHALLOW TRUE
)
set(PQXX_STATIC OFF CACHE BOOL "" FORCE)
set(SKIP_BUILD_TEST ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(libpqxx)

message("Fetching https://github.com/redis/hiredis.git, tag v1.0.2")
FetchContent_Declare(
    hiredis
    GIT_REPOSITORY https://github.com/redis/hiredis.git
    GIT_TAG v1.0.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(hiredis)

message("Fetching https://github.com/sewenew/redis-plus-plus.git, tag 1.3.5")
FetchContent_Declare( 
    redis_plus_plus
    GIT_REPOSITORY https://github.com/sewenew/redis-plus-plus.git
    GIT_TAG 1.3.5
    GIT_SHALLOW TRUE
)
set(REDIS_PLUS_PLUS_CXX_STANDARD 20)
set(REDIS_PLUS_PLUS_BUILD_TEST OFF)
set(REDIS_PLUS_PLUS_USE_TLS OFF)
set(HIREDIS_HEADER ${hiredis_SOURCE_DIR})
set(HIREDIS_LIB $<TARGET_FILE:hiredis>)
FetchContent_MakeAvailable(redis_plus_plus)

message("Fetching https://github.com/pistacheio/pistache.git, tag 0.0.5")
FetchContent_Declare(
    pistache
    GIT_REPOSITORY https://github.com/pistacheio/pistache.git
    GIT_TAG 0.0.5
    GIT_SHALLOW TRUE
)
set(PISTACHE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(PISTACHE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(PISTACHE_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(PISTACHE_USE_RAPIDJSON OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(pistache)

# there is an error: 
# Failure!  build file wasn't produced.
# Please read INSTALL.md and associated NOTES-* files.  You may also have to
# look over your available compiler tool chain or change your configuration.

# ***** Unsupported options: no-samples
# CMake Error at CMakeLists.txt:137 (message):
#   Failed to configure OpenSSL

# message("Fetching https://github.com/openssl/openssl.git, tag openssl-3.4.0")
# FetchContent_Declare(
#     openssl
#     GIT_REPOSITORY https://github.com/openssl/openssl.git
#     GIT_TAG openssl-3.4.0
#     GIT_SHALLOW TRUE
# )

# FetchContent_GetProperties(openssl)
# if(NOT openssl_POPULATED)
#     FetchContent_Populate(openssl)

#     set(OPENSSL_BUILD_DIR "${openssl_BINARY_DIR}")
#     set(OPENSSL_INSTALL_DIR "${openssl_BINARY_DIR}/install")

#     execute_process(
#         COMMAND "${openssl_SOURCE_DIR}/config"
#             --prefix=${OPENSSL_INSTALL_DIR}
#             --openssldir=${OPENSSL_INSTALL_DIR}
#             no-docs
#             no-tests
#             no-samples
#             shared
#         WORKING_DIRECTORY ${OPENSSL_BUILD_DIR}
#         RESULT_VARIABLE OPENSSL_CONFIG_RESULT
#     )

#     if(NOT OPENSSL_CONFIG_RESULT EQUAL 0)
#         message(FATAL_ERROR "Failed to configure OpenSSL")
#     endif()

#     # build openssl
#     execute_process(
#         COMMAND make -j${CMAKE_BUILD_PARALLEL_LEVEL}
#         WORKING_DIRECTORY ${OPENSSL_BUILD_DIR}
#         RESULT_VARIABLE OPENSSL_BUILD_RESULT
#     )

#     if(NOT OPENSSL_BUILD_RESULT EQUAL 0)
#         message(FATAL_ERROR "Failed to build OpenSSL")
#     endif()

#     # install openssls
#     execute_process(
#         COMMAND make install_sw
#         WORKING_DIRECTORY ${OPENSSL_BUILD_DIR}
#         RESULT_VARIABLE OPENSSL_INSTALL_RESULT
#     )

#     if(NOT OPENSSL_INSTALL_RESULT EQUAL 0)
#         message(FATAL_ERROR "Failed to install OpenSSL")
#     endif()

#     set(OPENSSL_ROOT_DIR ${OPENSSL_INSTALL_DIR})
#     set(OPENSSL_INCLUDE_DIR "${OPENSSL_INSTALL_DIR}/include")
#     set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_INSTALL_DIR}/lib/libcrypto${CMAKE_SHARED_LIBRARY_SUFFIX}")
#     set(OPENSSL_SSL_LIBRARY "${OPENSSL_INSTALL_DIR}/lib/libssl${CMAKE_SHARED_LIBRARY_SUFFIX}")

#     add_library(OpenSSL::Crypto SHARED IMPORTED)
#     set_target_properties(OpenSSL::Crypto PROPERTIES
#         IMPORTED_LOCATION "${OPENSSL_CRYPTO_LIBRARY}"
#         INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
#     )

#     add_library(OpenSSL::SSL SHARED IMPORTED)
#     set_target_properties(OpenSSL::SSL PROPERTIES
#         IMPORTED_LOCATION "${OPENSSL_SSL_LIBRARY}"
#         INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
#         INTERFACE_LINK_LIBRARIES OpenSSL::Crypto
#     )
# endif()

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
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    set(BUILD_GTEST ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(googletest)
    add_subdirectory(tests)
endif()