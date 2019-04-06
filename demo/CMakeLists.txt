include_directories(
    ${CMAKE_SOURCE_DIR}/DFABuilder/include
    ${CMAKE_SOURCE_DIR}/DFABuilder/interface
    ${CMAKE_SOURCE_DIR}/JSONPathParser
    )

add_executable(query_test ${CMAKE_CURRENT_SOURCE_DIR}/query_test.c)
target_link_libraries(query_test JSONQueryEngine DFABuilder JSONPathParser pthread)
add_test(NAME JSONQueryTest COMMAND query_test)

