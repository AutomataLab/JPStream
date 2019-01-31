project("JSONQuery")

file(GLOB source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    )

list(REMOVE_ITEM source_files ${CMAKE_CURRENT_SOURCE_DIR}/query_test.c)
add_library(JSONQueryEngine ${source_files})

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/query_test.c)
target_link_libraries(${PROJECT_NAME} JSONQueryEngine DFABuilder JSONPathParser pthread )

