cmake_minimum_required(VERSION 3.24)
project(spreadsheet)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /JMC")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-implicit-fallthrough")
endif()

# Find Java
find_package(Java REQUIRED)

set(ANTLR4_WITH_STATIC_CRT OFF) # using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR_BUILD_SHARED ON) # in my case I built the shared version
set(ANTLR_TAG 4.13.1) # to specify the GitHub tag for the jar too
set(ANTLR_BUILD_CPP_TESTS OFF) # to avoid creating dist folder

set(ANTLR_EXECUTABLE "${CMAKE_BINARY_DIR}/antlr/antlr-${ANTLR_TAG}-complete.jar") # a local path for antlr.jar
if(NOT EXISTS "${ANTLR_EXECUTABLE}")
    file(
        DOWNLOAD
        "https://www.antlr.org/download/antlr-${ANTLR_TAG}-complete.jar"
        "${ANTLR_EXECUTABLE}"
    )
endif()

include(FetchContent)

FetchContent_Declare(
    antlr
    GIT_REPOSITORY https://github.com/antlr/antlr4
    GIT_TAG ${ANTLR_TAG}
    SOURCE_SUBDIR "runtime/Cpp"
)
FetchContent_MakeAvailable(antlr) # populates antlr_SOURCE_DIR
include(${antlr_SOURCE_DIR}/runtime/Cpp/cmake/FindANTLR.cmake) # populates ANTLR_TARGET

antlr_target(FormulaParser Formula.g4 LEXER PARSER LISTENER)

# Include antlr4-runtime directory
include_directories(${antlr_SOURCE_DIR}/runtime/Cpp/src)

# Add all source files
file(GLOB_RECURSE sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

# Exclude main.cpp from sources
list(FILTER sources EXCLUDE REGEX ".*main\\.cpp$")

add_library(libspreadsheet ${ANTLR_FormulaParser_CXX_OUTPUTS} ${sources})
target_link_libraries(libspreadsheet antlr4_static)

# Add include directory containing 'FormulaBaseListener.h'
#target_include_directories(libspreadsheet PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
#target_include_directories(libspreadsheet PRIVATE "${antlr_SOURCE_DIR}/runtime/Cpp/runtime/src" ${ANTLR_FormulaParser_OUTPUT_DIR})
target_include_directories(libspreadsheet PRIVATE "${antlr_SOURCE_DIR}/runtime/Cpp/runtime/src" ${ANTLR_FormulaParser_OUTPUT_DIR})

# Create executable target
add_executable(spreadsheet main.cpp)

# Add dependencies
add_dependencies(spreadsheet libspreadsheet)

# Link libraries
target_link_libraries(spreadsheet PRIVATE libspreadsheet)

install(
    TARGETS spreadsheet
    DESTINATION bin
    EXPORT libspreadsheet
)

set_directory_properties(PROPERTIES VS_STARTUP_PROJECT spreadsheet)