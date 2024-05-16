cmake_minimum_required(VERSION 3.24)
project(spreadsheet
    VERSION 1.1.0
    DESCRIPTION "spreadsheet"
    HOMEPAGE_URL "https://github.com/IgorKilipenko/cpp-spreadsheet"
    LANGUAGES CXX
)

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

# Add threads library for all lib targets
find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

# Find Java
find_package(Java REQUIRED)

# Include the ANTLR setup
include(cmake/install_antlr.cmake)

# Include the source files and create targets
include(cmake/install.cmake)

if(CMAKE_PROJECT_NAME STREQUAL "spreadsheet") # if spreadsheet is top-level project
    # add include path
    include_directories(include)
    include_directories(src/include)
    include_directories(src)

    include(cmake/utils.cmake)
    include(cmake/develop.cmake)
endif()

set_directory_properties(PROPERTIES VS_STARTUP_PROJECT spreadsheet)
