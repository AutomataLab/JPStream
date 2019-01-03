enable_testing()
add_executable(dfatest ${CMAKE_CURRENT_SOURCE_DIR}/dfatest.cpp)
target_link_libraries(dfatest ${PROJECT_NAME})
add_test(NAME DFATest COMMAND dfatest)

add_custom_target(run_test ALL
                    DEPENDS dfatest)
add_custom_command(TARGET run_test
                    POST_BUILD
                    COMMAND ctest . --output-on-failure)