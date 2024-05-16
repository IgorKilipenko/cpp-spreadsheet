# SetupTargets.cmake - This file handles the creation of targets and linking

# Add all source files
file(GLOB_RECURSE sources CONFIGURE_DEPENDS
    "${PROJECT_SRC_DIR}/*.cpp"
)

# Collect headre files
file(GLOB_RECURSE headers CONFIGURE_DEPENDS
    "${PROJECT_SRC_DIR}/*.h"
)

# Exclude main.cpp from sources
list(FILTER sources EXCLUDE REGEX "${PROJECT_SRC_DIR}/main\\.cpp$")

# Create the library
add_library(libspreadsheet ${ANTLR_FormulaParser_CXX_OUTPUTS} ${sources})
target_link_libraries(libspreadsheet antlr4_static)

# Specify the include directories
target_include_directories(libspreadsheet PUBLIC
    $<BUILD_INTERFACE:${PROJECT_PUBLIC_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    "${antlr_SOURCE_DIR}/runtime/Cpp/runtime/src"
    "${ANTLR_FormulaParser_OUTPUT_DIR}"
)

# Specify the public headers
set_target_properties(libspreadsheet PROPERTIES
    PUBLIC_HEADER "${PROJECT_PUBLIC_INCLUDE_DIR}/spreadsheet.h"
)

install(
    FILES
    "${PROJECT_SRC_DIR}/common.h"
    "${PROJECT_SRC_DIR}/graph.h"
    "${PROJECT_SRC_DIR}/sheet.h"
    DESTINATION include/src
)

# Installation rules
install(TARGETS libspreadsheet antlr4_static
    EXPORT libspreadsheet-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)

install(
    EXPORT libspreadsheet-targets
    FILE libspreadsheet-config.cmake
    DESTINATION lib/cmake/libspreadsheet
)

# Create executable target
add_executable(spreadsheet ${PROJECT_SRC_DIR}/main.cpp)
add_dependencies(spreadsheet libspreadsheet)
target_link_libraries(spreadsheet PRIVATE libspreadsheet)

#install(TARGETS spreadsheet)
