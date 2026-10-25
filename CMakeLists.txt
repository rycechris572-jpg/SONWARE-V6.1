
cmake_minimum_required(VERSION 3.21)

project(sonware VERSION 6.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
)

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK environment variable is not set!")
endif()

message(STATUS "Using Geode SDK: $ENV{GEODE_SDK}")

add_subdirectory(
    "$ENV{GEODE_SDK}"
    "${CMAKE_CURRENT_BINARY_DIR}/geode"
)

setup_geode_mod(${PROJECT_NAME})
