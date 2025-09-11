cmake_minimum_required(VERSION 3.10)
project(LocalUnitTest)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/../include
    ${CMAKE_SOURCE_DIR}/../templates
)

# Add GoogleTest
add_subdirectory(googletest)

# Source files
set(SOURCES
    ${CMAKE_SOURCE_DIR}/../../src/main.cpp
    ${CMAKE_SOURCE_DIR}/../../src/test.c
    ${CMAKE_SOURCE_DIR}/templates/optional_test.cpp
    ${CMAKE_SOURCE_DIR}/templates/ring_queue_test.cpp
)

# Create the test executable
add_executable(LocalUnitTest ${SOURCES})

# Link GoogleTest libraries
target_link_libraries(LocalUnitTest gtest gtest_main)