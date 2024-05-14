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

set(ANTLR_EXECUTABLE "./antlr-${ANTLR_TAG}-complete.jar") # a local path for antlr.jar
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

# Macro to generate ANTLR files
macro(antlr_target NAME INPUT_FILE)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cpp ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.h
        COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR_EXECUTABLE} -Dlanguage=Cpp -o ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
        COMMENT "Running ANTLR on ${INPUT_FILE}"
    )
    add_custom_target(${NAME}_antlr DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cpp ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.h)
endmacro()

# Generate ANTLR files for FormulaParser
antlr_target(FormulaParser antlr/grammar/Formula.g4)

# Include generated files directory
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Add all source files
file(GLOB_RECURSE sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

# Exclude main.cpp from sources
list(FILTER sources EXCLUDE REGEX ".*main\\.cpp$")

# Create library target
add_library(libspreadsheet ${sources})

# Add dependencies
add_dependencies(libspreadsheet FormulaParser_antlr)

# Create executable target
add_executable(spreadsheet main.cpp)

# Link libraries
target_link_libraries(spreadsheet PRIVATE libspreadsheet)
