# SetupAntlr.cmake - This file handles the setup for ANTLR

set(ANTLR_TAG 4.13.1)
set(ANTLR4_WITH_STATIC_CRT OFF)
set(ANTLR_BUILD_SHARED ON)
set(ANTLR_BUILD_CPP_TESTS OFF)

# Define the path to the ANTLR JAR file and download it if it does not exist
set(ANTLR_EXECUTABLE "${CMAKE_BINARY_DIR}/antlr/antlr-${ANTLR_TAG}-complete.jar")
if (NOT EXISTS "${ANTLR_EXECUTABLE}")
    file(
        DOWNLOAD
        "https://www.antlr.org/download/antlr-${ANTLR_TAG}-complete.jar"
        "${ANTLR_EXECUTABLE}"
    )
endif()

# Use FetchContent to download and make available the ANTLR C++ runtime
include(FetchContent)
FetchContent_Declare(
    antlr
    GIT_REPOSITORY https://github.com/antlr/antlr4
    GIT_TAG ${ANTLR_TAG}
    SOURCE_SUBDIR "runtime/Cpp"
)
FetchContent_MakeAvailable(antlr)

include(${antlr_SOURCE_DIR}/runtime/Cpp/cmake/FindANTLR.cmake)

# Generate ANTLR files for FormulaParser
antlr_target(FormulaParser Formula.g4 LEXER PARSER LISTENER)

# Include directories for ANTLR
include_directories(${antlr_SOURCE_DIR}/runtime/Cpp/src)
