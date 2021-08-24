cmake_minimum_required(VERSION 3.10)

project(Neo VERSION 1.0)

configure_file(config.h. in config.h)

include_directories(include)
file(GLOB SOURCES "src/*.cpp")
add_executable(Neo ${SOURCES})

target_include_directories(Neo PUBLIC "${PROJECT_BINARY_DIR}")

 
 