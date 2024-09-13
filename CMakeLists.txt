cmake_minimum_required(VERSION 3.12)
project(chat_application)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(common)

add_subdirectory(server)

add_subdirectory(client)
