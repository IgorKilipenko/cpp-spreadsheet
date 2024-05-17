cmake_minimum_required(VERSION 3.24)
project(spreadsheet
    VERSION 1.1.0
    DESCRIPTION "spreadsheet"
    HOMEPAGE_URL "https://github.com/IgorKilipenko/cpp-spreadsheet"
    LANGUAGES CXX
)

# Set C++ standard to 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable creation of compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source directiory
set(PROJECT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Public include directiory
set(PROJECT_PUBLIC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Installation directiory
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# Find required packages
find_package(Threads REQUIRED)
find_package(Java REQUIRED)

# Add threads library for all lib targets
link_libraries(Threads::Threads)

# Include ANTLR setup
include(cmake/install_antlr.cmake)

# Include source files and create targets
include(cmake/install.cmake)

if(CMAKE_PROJECT_NAME STREQUAL "spreadsheet") # if spreadsheet is top-level project
    # Set warning flags for Debug build
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(ENABLE_WARNING ON)
    endif()

    # Add src include dirs
    include_directories(include)
    include_directories(include/spreadsheet)
    include_directories(src/include)
    include_directories(src)

    include(cmake/utils.cmake)
    include(cmake/develop.cmake)
endif()

set_directory_properties(PROPERTIES VS_STARTUP_PROJECT spreadsheet)
