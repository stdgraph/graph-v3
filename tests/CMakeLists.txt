cmake_minimum_required(VERSION 3.20)

include(FetchContent)

# Fetch Catch2 v3
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.0
)
FetchContent_MakeAvailable(Catch2)

# Include CTest and Catch2 integration
include(CTest)
include(Catch)

# Add subdirectories with their own test executables
add_subdirectory(adj_list)
add_subdirectory(algorithms)
add_subdirectory(container)
add_subdirectory(edge_list)
add_subdirectory(views)

# Keep test_main.cpp in root for now (can be moved to common/ later)
# No main executable needed - each subdirectory creates its own
