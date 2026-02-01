cmake_minimum_required(VERSION 3.20)

# Read version from VERSION file
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" PROJECT_VERSION)
string(STRIP "${PROJECT_VERSION}" PROJECT_VERSION)

project(graph3_library 
    VERSION ${PROJECT_VERSION}
    DESCRIPTION "Modern C++20 graph library"
    HOMEPAGE_URL "https://github.com/pratzl/desc"
    LANGUAGES CXX
)

# Include custom CMake modules
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(StandardProjectSettings)
include(CompilerWarnings)
include(Sanitizers)
include(CodeCoverage)
include(CompilerCache)
include(PrecompiledHeaders)
include(CPM)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
option(BUILD_DOCS "Build documentation" OFF)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_SANITIZERS "Enable sanitizers (address, undefined)" OFF)
option(ENABLE_UNITY_BUILD "Enable unity builds for faster compilation" OFF)
option(ENABLE_CACHE "Enable compiler cache (ccache/sccache)" ON)
option(ENABLE_PCH "Enable precompiled headers" ON)

# Enable compiler cache
enable_compiler_cache()

# Enable unity builds if requested
if(ENABLE_UNITY_BUILD)
    set(CMAKE_UNITY_BUILD ON)
    message(STATUS "Unity builds enabled")
endif()

# Library target (header-only)
add_library(graph3 INTERFACE)
add_library(graph::graph3 ALIAS graph3)

target_include_directories(graph3 INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(graph3 INTERFACE cxx_std_20)

# Link tl::expected for optional cycle detection in topological sort
target_link_libraries(graph3 INTERFACE tl::expected)

# Apply compiler warnings
set_project_warnings(graph3)

# Apply sanitizers if enabled
enable_sanitizers(graph3)

# Apply code coverage if enabled
enable_coverage(graph3)

# Apply precompiled headers if enabled
enable_precompiled_headers(graph3)

# Testing
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Benchmarks
if(BUILD_BENCHMARKS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmark/CMakeLists.txt")
        add_subdirectory(benchmark)
    else()
        message(STATUS "Benchmark directory exists but no CMakeLists.txt found - skipping")
    endif()
endif()

# Documentation
if(BUILD_DOCS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/docs/CMakeLists.txt")
        add_subdirectory(docs)
    else()
        message(STATUS "Docs directory exists but no CMakeLists.txt found - skipping")
    endif()
endif()

# Installation configuration
include(InstallConfig)

# Packaging with CPack
set(CPACK_PACKAGE_NAME "graph3")
set(CPACK_PACKAGE_VENDOR "pratzl")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modern C++20 graph library")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "graph3")

# Optional license and readme
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
endif()

# Package generators based on platform
set(CPACK_GENERATOR "TGZ;ZIP")
if(WIN32)
    list(APPEND CPACK_GENERATOR "NSIS")
elseif(APPLE)
    list(APPEND CPACK_GENERATOR "DragNDrop")
elseif(UNIX)
    list(APPEND CPACK_GENERATOR "DEB;RPM")
endif()

# Debian package metadata
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "pratzl")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "all")

# RPM package metadata
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
set(CPACK_RPM_PACKAGE_ARCHITECTURE "noarch")

include(CPack)
