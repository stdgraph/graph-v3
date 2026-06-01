# Standard project settings applied to all targets
# Guards against in-source builds
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are not allowed. Please create a build directory "
        "and run cmake from there. You may need to remove CMakeCache.txt.")
endif()

# Generate compile_commands.json for IDE support and static analysis
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

# Position independent code for shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Enable folder organization in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# MSVC exception model: always enable standard C++ EH unwind semantics.
#
# This avoids warning C4530 ("C++ exception handler used, but unwind semantics
# are not enabled") in Debug builds, especially from third-party targets such as
# Catch2, and keeps behavior consistent across all targets.
if(MSVC)
    add_compile_options(/EHsc)
endif()

# Enable parallel compilation for MSVC cl.exe (clang-cl doesn't support /MP)
if(MSVC AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Use /MP to enable parallel compilation with all available cores
    add_compile_options(/MP)
    message(STATUS "MSVC parallel compilation enabled with /MP")
endif()

# Default build type for single-config generators
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING 
        "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()
