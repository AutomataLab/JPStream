project("JSONQuery")

file(GLOB source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    )

add_executable(${PROJECT_NAME} ${source_files})
target_link_libraries(${PROJECT_NAME} XPathParser DFAbuilder pthread )

