project("XPathBuilder")

## Flex/Bison configuration
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

if (WIN32)
    set (ADDITIONAL_FLEX_FLAGS "--wincompat")
else()
    set (ADDITIONAL_FLEX_FLAGS "")
endif()

FLEX_TARGET(SCANNER ${CMAKE_CURRENT_SOURCE_DIR}/scanner.l
${CMAKE_CURRENT_BINARY_DIR}/scanner.c COMPILE_FLAGS "--header-file=${CMAKE_CURRENT_BINARY_DIR}/scanner.h ${ADDITIONAL_FLEX_FLAGS}")
BISON_TARGET(PARSER ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
${CMAKE_CURRENT_BINARY_DIR}/parser.c COMPILE_FLAGS "-d -v")
ADD_FLEX_BISON_DEPENDENCY(SCANNER PARSER)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB lib_source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

add_library(${PROJECT_NAME} STATIC ${lib_source_files} ${BISON_PARSER_OUTPUTS} ${FLEX_SCANNER_OUTPUTS})
add_executable(xpath_test ${CMAKE_CURRENT_SOURCE_DIR}/test.c)

target_link_libraries(xpath_test ${PROJECT_NAME})