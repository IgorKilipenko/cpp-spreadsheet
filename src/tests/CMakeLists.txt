set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/tests)
add_executable(spreadsheet_unit_tests
    test_function.cpp
    test_main.cpp
)
target_include_directories(spreadsheet_unit_tests
    PRIVATE
    $<BUILD_INTERFACE:${doctest_SOURCE_DIR}/doctest>
)
add_test(NAME spreadsheet_unit_tests COMMAND spreadsheet_unit_tests)
