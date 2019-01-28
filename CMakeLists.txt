cmake_minimum_required(VERSION 2.8)
project("JSONPath")

include_directories(src include DFABuilder/include JSONPathParser/)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

## Compiler Standard
# SET (CMAKE_C_COMPILER "/usr/bin/clang-6.0")
SET (CMAKE_CXX_FLAGS_DEBUG "-g -D_GLIBCXX_DEBUG")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -mcmodel=medium")
SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -mcmodel=medium")

add_subdirectory(DFABuilder)
add_subdirectory(JSONPathParser)
add_subdirectory(JSONQuery)

add_subdirectory(test)

