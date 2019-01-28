project("DFABuilder")

file(GLOB lib_source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Lexer/*.cpp
)

add_library(${PROJECT_NAME} STATIC ${lib_source_files})
