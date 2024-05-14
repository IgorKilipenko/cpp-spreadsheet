cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(spreadsheet)

set(CMAKE_CXX_STANDARD 17)
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(
        CMAKE_CXX_FLAGS_DEBUG
        "${CMAKE_CXX_FLAGS_DEBUG} /JMC"
    )
else()
    set(
        CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-implicit-fallthrough"
    )
endif()

set(ANTLR_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/antlr-4.12.0-complete.jar)
include(${CMAKE_CURRENT_SOURCE_DIR}/FindANTLR.cmake)

add_definitions(
    -DANTLR4CPP_STATIC
    -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
)

set(WITH_STATIC_CRT OFF CACHE BOOL "Visual C++ static CRT for ANTLR" FORCE)
add_subdirectory(antlr4_runtime)

antlr_target(FormulaParser Formula.g4 LEXER PARSER LISTENER)

include_directories(
    ${ANTLR4_INCLUDE_DIRS}
    ${ANTLR_FormulaParser_OUTPUT_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/antlr4_runtime/runtime/src
)

file(GLOB sources
    *.cpp
    *.h
)

set(sources_lib ${sources})

list(FILTER sources_lib EXCLUDE REGEX ".*main\\.cpp$")

add_library(
    libspreadsheet
    ${ANTLR_FormulaParser_CXX_OUTPUTS}
    ${sources_lib}
)

add_executable(
    spreadsheet
    main.cpp
)

target_link_libraries(libspreadsheet antlr4_static)
if(MSVC)
    target_compile_options(antlr4_static PRIVATE /W0)
endif()

target_link_libraries(spreadsheet PRIVATE libspreadsheet)

install(
    TARGETS spreadsheet
    DESTINATION bin
    EXPORT libspreadsheet
)

set_directory_properties(PROPERTIES VS_STARTUP_PROJECT spreadsheet)