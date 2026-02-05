# Algorithm Tests CMakeLists.txt
# Tests for graph algorithms

# Algorithm test executable
add_executable(test_algorithms
    test_dijkstra_shortest_paths.cpp
    test_bellman_ford_shortest_paths.cpp
    test_connected_components.cpp
)

target_link_libraries(test_algorithms
    PRIVATE
        graph::graph3
        Catch2::Catch2WithMain
)

# Discover and register tests
catch_discover_tests(test_algorithms
    TEST_PREFIX "algorithms."
    PROPERTIES
        LABELS "algorithms"
)

# Note: Add new algorithm test files in the add_executable() section above
