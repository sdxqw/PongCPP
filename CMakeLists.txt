cmake_minimum_required(VERSION 3.27)
project(pong)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML 2.6.1 COMPONENTS graphics audio system REQUIRED)
include_directories(${SFML_INCLUDE_DIRS} assets)

add_executable(pong main.cpp)

target_link_libraries(pong sfml-graphics sfml-audio sfml-system)

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})