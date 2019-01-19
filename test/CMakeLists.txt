enable_testing()

include_directories(
    ${CMAKE_SOURCE_DIR}/DFAbuilder/include
    ${CMAKE_SOURCE_DIR}/DFAbuilder/interface
    ${CMAKE_SOURCE_DIR}/XPathParser
    )

add_executable(dfa_test ${CMAKE_CURRENT_SOURCE_DIR}/dfa_test.cpp)
target_link_libraries(dfa_test DFAbuilder)
add_test(NAME DFATest COMMAND dfa_test)

add_executable(jsonpath_test ${CMAKE_CURRENT_SOURCE_DIR}/jsonpath_test.c)
target_link_libraries(jsonpath_test XPathParser DFAbuilder)
add_test(NAME JSONPathTest COMMAND jsonpath_test)

add_custom_target(run_test ALL
                    DEPENDS dfa_test jsonpath_test)
add_custom_command(TARGET run_test
                    POST_BUILD
                    COMMAND ctest . --output-on-failure)