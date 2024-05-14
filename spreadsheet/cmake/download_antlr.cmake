include(ExternalProject)

ExternalProject_Add(
    antlr
    PREFIX ${CMAKE_BINARY_DIR}/antlr
    URL https://www.antlr.org/download/antlr-4.13.1-complete.jar
    # URL_HASH SHA256=f1b5a2ae0b9876cc3e4fa7c56bb08cbf9b9bbcc0c8c0ed13e4d8b8f7e7c8b47b
    LOG_DOWNLOAD ON
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/antlr/src/antlr/antlr-4.13.1-complete.jar ${CMAKE_BINARY_DIR}/antlr/antlr-4.13.1-complete.jar
)

ExternalProject_Get_Property(antlr DOWNLOAD_DIR)
set(ANTLR_EXECUTABLE ${DOWNLOAD_DIR}/antlr-4.13.1-complete.jar)
