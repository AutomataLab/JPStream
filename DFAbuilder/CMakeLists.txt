project("DFAbuilder")

#add_definitions(-D_GLIBCXX_DEBUG) 
#add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0) ## It's important for gtest

## CPP Compiler 
SET (CMAKE_CXX_FLAGS "-std=c++11")
SET (CMAKE_CXX_FLAGS_DEBUG "-g")
SET (CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")
SET (CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

## Flex/Bison configuration
# find_package(BISON REQUIRED)
# find_package(FLEX REQUIRED)

# FLEX_TARGET(SCANNER ${CMAKE_CURRENT_SOURCE_DIR}/src/bnf_parser/scanner.l
# ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/scanner.h COMPILE_FLAGS ${ADDITIONAL_FLEX_FLAGS})
# BISON_TARGET(PARSER ${CMAKE_CURRENT_SOURCE_DIR}/src/bnf_parser/parser.y
# ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp COMPILE_FLAGS "-d -v")
# ADD_FLEX_BISON_DEPENDENCY(SCANNER PARSER)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB lib_source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(${PROJECT_NAME} STATIC ${lib_source_files})
