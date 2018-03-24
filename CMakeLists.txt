cmake_minimum_required(VERSION 2.8)
project("JSONPath")

include_directories(src include DFAbuilder/include DFAbuilder/runtime)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

## Compiler Standard
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

add_subdirectory(DFAbuilder)
add_subdirectory(XPathParser)

file(GLOB source_files
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_executable(${PROJECT_NAME} ${source_files})
target_link_libraries(${PROJECT_NAME} DFAbuilder)