# SetupAntlr.cmake - This file handles the setup for ANTLR

set(ANTLR_VERSION 4.13.1)
set(ANTLR4_WITH_STATIC_CRT OFF)
set(ANTLR_BUILD_SHARED OFF)
set(ANTLR_BUILD_CPP_TESTS OFF)

message(STATUS "DOWNLOAD ANTLR4 GENERATOR v" ${ANTLR_VERSION})

# Define the path to the ANTLR GENERATOR JAR file and download it if does not exist
set(ANTLR_EXECUTABLE "${CMAKE_BINARY_DIR}/antlr/antlr-${ANTLR_VERSION}-complete.jar")
if (NOT EXISTS "${ANTLR_EXECUTABLE}")
    file(
        DOWNLOAD
        "https://www.antlr.org/download/antlr-${ANTLR_VERSION}-complete.jar"
        "${ANTLR_EXECUTABLE}"
    )
endif()

message(STATUS "FETCH ANTLR4 RUNTIME CPP...")

# Use FetchContent to download and make available the ANTLR C++ runtime
include(FetchContent)
FetchContent_Declare(
    antlr
    GIT_REPOSITORY https://github.com/antlr/antlr4
    GIT_TAG ${ANTLR_VERSION}
    SOURCE_SUBDIR "runtime/Cpp"
)
FetchContent_MakeAvailable(antlr)

message(STATUS "ANTLR SOURCE DIR: " ${antlr_SOURCE_DIR})

include(${antlr_SOURCE_DIR}/runtime/Cpp/cmake/FindANTLR.cmake)

# Generate ANTLR files for FormulaParser
antlr_target(FormulaParser "${PROJECT_SRC_DIR}/antlr/grammar/Formula.g4" LEXER PARSER LISTENER)

# Include directories for ANTLR
include_directories("${antlr_SOURCE_DIR}/runtime/Cpp/src")

message(STATUS "ANTLR RUNTIME DIR: " ${antlr_SOURCE_DIR}/runtime/Cpp/src)
