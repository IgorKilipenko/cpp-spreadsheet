set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/tests)

add_executable(spreadsheet_tests
    main.cpp
    test_spreadsheet.cpp
)
add_dependencies(spreadsheet_tests doctest::doctest libspreadsheet)
target_link_libraries(spreadsheet_tests PRIVATE doctest::doctest libspreadsheet)
target_include_directories(spreadsheet_tests
    PRIVATE
    ${PROJECT_PUBLIC_INCLUDE_DIR}
    $<TARGET_PROPERTY:libspreadsheet,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:doctest,INTERFACE_INCLUDE_DIRECTORIES>
)
add_test(NAME spreadsheet_tests COMMAND spreadsheet_tests)
