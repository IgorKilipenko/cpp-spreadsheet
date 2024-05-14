cmake_minimum_required(VERSION 3.24)
project(spreadsheet)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /JMC")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-implicit-fallthrough")
endif()

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
