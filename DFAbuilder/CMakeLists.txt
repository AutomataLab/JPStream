project("DFAbuilder")

#add_definitions(-D_GLIBCXX_DEBUG) 
#add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0) ## It's important for gtest

## Flex/Bison configuration
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

FLEX_TARGET(RSCANNER ${CMAKE_CURRENT_SOURCE_DIR}/src/Lexer/r_scanner.l
${CMAKE_CURRENT_BINARY_DIR}/r_scanner.cpp COMPILE_FLAGS "--header-file=${CMAKE_CURRENT_BINARY_DIR}/r_scanner.h")
BISON_TARGET(RPARSER ${CMAKE_CURRENT_SOURCE_DIR}/src/Lexer/r_parser.y
${CMAKE_CURRENT_BINARY_DIR}/r_parser.cpp COMPILE_FLAGS "-d -v")
ADD_FLEX_BISON_DEPENDENCY(RSCANNER RPARSER)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(src include runtime)

file(GLOB lib_source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Lexer/*.cpp
)

add_library(${PROJECT_NAME} STATIC ${lib_source_files} ${BISON_RPARSER_OUTPUTS} ${FLEX_RSCANNER_OUTPUTS})
