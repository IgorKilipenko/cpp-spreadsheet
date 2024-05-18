# Registering tests
message(STATUS "BUILD_UNIT_TESTS: ${SPREADSHEET_BUILD_TESTS}")
if(SPREADSHEET_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(${PROJECT_TESTS_DIR})
endif()
