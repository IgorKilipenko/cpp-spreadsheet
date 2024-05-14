cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(spreadsheet)

set(CMAKE_CXX_STANDARD 17)
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /JMC")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-implicit-fallthrough")
endif()

# Add CMake module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Java REQUIRED)
find_package(ANTLR REQUIRED)

add_definitions(
    -DANTLR4CPP_STATIC
    -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
)

# Set the path to ANTLR4 runtime
set(ANTLR4_RUNTIME_DIR /usr/local/src/antlr4/runtime/Cpp)

# Set the directory properties
set(WITH_STATIC_CRT OFF CACHE BOOL "Visual C++ static CRT for ANTLR" FORCE)

# Add the ANTLR4 runtime directory
add_subdirectory(${ANTLR4_RUNTIME_DIR} ${CMAKE_CURRENT_BINARY_DIR}/antlr4_runtime)

antlr_target(FormulaParser ${CMAKE_CURRENT_SOURCE_DIR}/Formula.g4 LEXER PARSER LISTENER)

include_directories(
    ${ANTLR4_INCLUDE_DIR}
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
