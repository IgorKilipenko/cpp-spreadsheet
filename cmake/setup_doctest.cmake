message(STATUS "FETCH DOCTEST GIT REPOSITORY...")

# Add FetchContent for doctest
include(FetchContent)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG master
)
FetchContent_MakeAvailable(doctest)

message(STATUS "DOCTEST SOURCE DIR: " ${doctest_SOURCE_DIR})
