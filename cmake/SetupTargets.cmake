# SetupTargets.cmake - This file handles the creation of targets and linking

# Add all source files except main.cpp
file(GLOB_RECURSE sources CONFIGURE_DEPENDS
    "${PROJECT_SRC_DIR}/*.cpp"
)
list(FILTER sources EXCLUDE REGEX "${PROJECT_SRC_DIR}/main\\.cpp$")

# Find all public and private headers
file(GLOB_RECURSE public_headers CONFIGURE_DEPENDS
    "${PROJECT_PUBLIC_INCLUDE_DIR}/spreadsheet/*.h"
)
file(GLOB_RECURSE private_headers CONFIGURE_DEPENDS
    "${PROJECT_PUBLIC_INCLUDE_DIR}/*.h"
)
list(REMOVE_ITEM private_headers ${public_headers})

# Create the library
add_library(libspreadsheet ${sources} ${private_headers} ${ANTLR_FormulaParser_CXX_OUTPUTS})
target_link_libraries(libspreadsheet PRIVATE antlr4_static)
set_target_properties(libspreadsheet PROPERTIES
    PUBLIC_HEADER "${public_headers}"
)

# Specify the include directories
target_include_directories(libspreadsheet PUBLIC
    $<BUILD_INTERFACE:${PROJECT_PUBLIC_INCLUDE_DIR}/spreadsheet>
    $<INSTALL_INTERFACE:include/spreadsheet>
    PRIVATE
    "${antlr_SOURCE_DIR}/runtime/Cpp/runtime/src"
    "${ANTLR_FormulaParser_OUTPUT_DIR}"
    "${PROJECT_PUBLIC_INCLUDE_DIR}"
)

# Installation rules for the library
install(TARGETS libspreadsheet
    EXPORT libspreadsheet_targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/spreadsheet
)
install(
    FILES
    ${private_headers}
    DESTINATION include
)

# Create executable target
add_executable(spreadsheet ${PROJECT_SRC_DIR}/main.cpp)
add_dependencies(spreadsheet libspreadsheet)
target_link_libraries(spreadsheet PRIVATE libspreadsheet)

# Installation rules for the executable
install(TARGETS spreadsheet
    DESTINATION app
)
