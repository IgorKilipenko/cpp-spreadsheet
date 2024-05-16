cmake_minimum_required(VERSION 3.24)
project(spreadsheet)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-implicit-fallthrough")

# Source directiory
set(PROJECT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Public include directiory
set(PROJECT_PUBLIC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Installation directiory
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# Find Java
find_package(Java REQUIRED)

# Include the ANTLR setup
include(cmake/SetupAntlr.cmake)

# Include the source files and create targets
include(cmake/SetupTargets.cmake)

# Install target for spreadsheet
install(
    TARGETS spreadsheet
    DESTINATION bin
    EXPORT libspreadsheet
)

set_directory_properties(PROPERTIES VS_STARTUP_PROJECT spreadsheet)
