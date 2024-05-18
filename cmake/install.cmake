message(STATUS "START INSTALL SETTING...")

# Find all public and private headers
file(GLOB_RECURSE public_headers CONFIGURE_DEPENDS
    "${PROJECT_PUBLIC_INCLUDE_DIR}/spreadsheet/*.h"
)
file(GLOB_RECURSE private_headers CONFIGURE_DEPENDS
    "${PROJECT_PUBLIC_INCLUDE_DIR}/*.h"
    "${PROJECT_SRC_DIR}/*.h"
)
list(REMOVE_ITEM private_headers ${public_headers})

# Installation rules for the library
install(TARGETS libspreadsheet antlr4_static
    EXPORT libspreadsheet_Targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/spreadsheet
)
install(EXPORT libspreadsheet_Targets
    FILE libspreadsheet-config.cmake
    DESTINATION lib/cmake/libspreadsheet
    NAMESPACE libspreadsheet::
)
install(
    FILES
    ${private_headers}
    DESTINATION include
)

# Install target for spreadsheet
install(
    TARGETS spreadsheet
    DESTINATION app
)
