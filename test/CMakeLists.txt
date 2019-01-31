enable_testing()

include_directories(
    ${CMAKE_SOURCE_DIR}/DFABuilder/include
    ${CMAKE_SOURCE_DIR}/DFABuilder/interface
    ${CMAKE_SOURCE_DIR}/JSONPathParser
    )

add_executable(dfa_test ${CMAKE_CURRENT_SOURCE_DIR}/dfa_test.cpp)
target_link_libraries(dfa_test DFABuilder)
add_test(NAME DFATest COMMAND dfa_test)

add_executable(jsonpath_test ${CMAKE_CURRENT_SOURCE_DIR}/jsonpath_test.c)
target_link_libraries(jsonpath_test DFABuilder JSONPathParser)
add_test(NAME JSONPathTest COMMAND jsonpath_test)


add_executable(query_test ${CMAKE_CURRENT_SOURCE_DIR}/query_test.c)
target_link_libraries(query_test JSONQueryEngine DFABuilder JSONPathParser pthread)
# add_test(NAME JSONQueryTest COMMAND query_test)


add_custom_target(run_test ALL
                    DEPENDS dfa_test jsonpath_test)
add_custom_command(TARGET run_test
                    POST_BUILD
                    COMMAND ctest . --output-on-failure)
