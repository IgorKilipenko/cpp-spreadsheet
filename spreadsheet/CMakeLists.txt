cmake_minimum_required(VERSION 3.22)
project(spreadsheet)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /JMC")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-implicit-fallthrough")
endif()

# Include the download_antlr.cmake script
include(cmake/download_antlr.cmake)

# Find Java
find_package(Java REQUIRED)

# Get the installation directory of ANTLR
ExternalProject_Get_Property(antlr INSTALL_DIR)

# Set the path to the ANTLR executable
set(ANTLR_EXECUTABLE ${INSTALL_DIR}/antlr-4.13.1-complete.jar)

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

# Add include directory containing 'FormulaBaseListener.h'
target_include_directories(libspreadsheet PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Create executable target
add_executable(spreadsheet main.cpp)

# Add dependencies
add_dependencies(spreadsheet FormulaParser_antlr)

# Link libraries
target_link_libraries(spreadsheet PRIVATE libspreadsheet)
