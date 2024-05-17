# Add test executable target: Tests
add_executable(spreadsheet_tests
    tests/test_main.cpp
    tests/test_spreadsheet.cpp
)
add_dependencies(spreadsheet_tests doctest::doctest libspreadsheet)
target_link_libraries(spreadsheet_tests PRIVATE doctest::doctest libspreadsheet)
target_include_directories(spreadsheet_tests
    PRIVATE
    $<BUILD_INTERFACE:${PROJECT_PUBLIC_INCLUDE_DIR}/spreadsheet;${doctest_SOURCE_DIR}/doctest>
)
add_test(NAME spreadsheet_lib_test COMMAND spreadsheet_tests)

file(GLOB test_directories LIST_DIRECTORIES true CONFIGURE_DEPENDS
    "${PROJECT_SRC_DIR}/*/tests/"
    "${PROJECT_SRC_DIR}*/tests/"
)

foreach(test_dir ${test_directories})
    if(IS_DIRECTORY ${test_dir})
        add_subdirectory(${test_dir})
        message(STATUS "Added test directory: ${test_dir}")
    endif()
endforeach()
