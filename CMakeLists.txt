cmake_minimum_required(VERSION 3.24)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(cpp-spreadsheet-app VERSION 1.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add subdirectory with the library
add_subdirectory(spreadsheet)
