message(STATUS "DEVELOP SETTING...")

# Enable address sanitizer
option(ENABLE_SANITIZER "Enable sanitizer(Debug+Gcc/Clang/AppleClang)" ON)

if(ENABLE_SANITIZER AND NOT MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        check_asan(HAS_ASAN)
        if(HAS_ASAN)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
            message(STATUS "AddressSanitizer enabled")
        else()
            message(WARNING "sanitizer is no supported with current tool-chains")
        endif()
    endif()
endif()

# warning
option(ENABLE_WARNING "Enable warning for all project " OFF)

if(ENABLE_WARNING)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        list(APPEND MSVC_OPTIONS "/W3")
        if(MSVC_VERSION GREATER 1900) # Allow non fatal security warnings for msvc 2015
            list(APPEND MSVC_OPTIONS "/WX")
        endif()
        add_compile_options(MSVC_OPTIONS)
    else()
        add_compile_options(-Wall
                            -Wextra
                            -Wconversion
                            -pedantic
                            -Werror
                            -Wfatal-errors)
    endif()
endif()

