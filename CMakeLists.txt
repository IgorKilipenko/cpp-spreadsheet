cmake_minimum_required(VERSION 3.24)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(PROJECT_NAME "cpp-spreadsheet-app")
project(${PROJECT_NAME} VERSION 1.1.0 LANGUAGES CXX)

add_subdirectory("./spreadsheet")
