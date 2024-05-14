cmake_minimum_required(VERSION 3.22)
project(spreadsheet)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /JMC")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-implicit-fallthrough")
endif()

# Add the download_antlr.cmake script
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/download_antlr.cmake)

find_package(Java REQUIRED)

ExternalProject_Get_Property(antlr INSTALL_DIR)

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

antlr_target(FormulaParser Formula.g4)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB sources
    *.cpp
    *.h
)

set(sources_lib ${sources})

list(FILTER sources_lib EXCLUDE REGEX ".*main\\.cpp$")

add_library(
    libspreadsheet
    ${sources_lib}
)

add_executable(
    spreadsheet
    main.cpp
)

add_dependencies(spreadsheet FormulaParser_antlr)

target_link_libraries(spreadsheet PRIVATE libspreadsheet)
