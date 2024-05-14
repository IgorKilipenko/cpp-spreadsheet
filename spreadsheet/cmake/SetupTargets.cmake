# SetupTargets.cmake - This file handles the creation of targets and linking

# Add all source files
file(GLOB_RECURSE sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

# Exclude main.cpp from sources
list(FILTER sources EXCLUDE REGEX ".*main\\.cpp$")

# Create the library target
add_library(libspreadsheet ${ANTLR_FormulaParser_CXX_OUTPUTS} ${sources})
target_link_libraries(libspreadsheet antlr4_static)

# Include directories for generated ANTLR files
target_include_directories(libspreadsheet PRIVATE "${antlr_SOURCE_DIR}/runtime/Cpp/runtime/src" ${ANTLR_FormulaParser_OUTPUT_DIR})

# Create the executable target
add_executable(spreadsheet main.cpp)

# Add dependencies
add_dependencies(spreadsheet libspreadsheet)

# Link libraries to the executable
target_link_libraries(spreadsheet PRIVATE libspreadsheet)
