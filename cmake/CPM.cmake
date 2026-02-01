# CPM.cmake - CMake Package Manager
# Download CPM.cmake if not already present
# https://github.com/cpm-cmake/CPM.cmake

set(CPM_DOWNLOAD_VERSION 0.40.2)

if(CPM_SOURCE_CACHE)
  set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
  set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
  set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

# Ensure the download directory exists
get_filename_component(CPM_DOWNLOAD_DIR "${CPM_DOWNLOAD_LOCATION}" DIRECTORY)
file(MAKE_DIRECTORY "${CPM_DOWNLOAD_DIR}")

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
  message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
  file(DOWNLOAD
       https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
       ${CPM_DOWNLOAD_LOCATION}
       EXPECTED_HASH SHA256=c8cdc32c03816538ce22781ed72964dc864b2a34a310d3b7104812a5ca2d835d
  )
endif()

include(${CPM_DOWNLOAD_LOCATION})

# Fetch tl::expected for C++20 std::expected polyfill
CPMAddPackage(
    NAME expected
    GITHUB_REPOSITORY TartanLlama/expected
    GIT_TAG v1.1.0
    OPTIONS
        "EXPECTED_BUILD_TESTS OFF"
)

# Usage example:
# include(CPM)
# CPMAddPackage("gh:catchorg/Catch2@3.5.0")
# CPMAddPackage(
#     NAME benchmark
#     GITHUB_REPOSITORY google/benchmark
#     VERSION 1.8.3
#     OPTIONS "BENCHMARK_ENABLE_TESTING OFF"
# )
