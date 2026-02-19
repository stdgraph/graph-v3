# Algorithm Tests CMakeLists.txt
# Tests for graph algorithms

# Algorithm test executable
add_executable(test_algorithms
    test_dijkstra_shortest_paths.cpp
    test_bellman_ford_shortest_paths.cpp
    test_connected_components.cpp
    test_breadth_first_search.cpp
    test_depth_first_search.cpp
    test_topological_sort.cpp
    test_triangle_count.cpp
    test_mis.cpp
    test_mst.cpp
    test_label_propagation.cpp
    test_articulation_points.cpp
    test_biconnected_components.cpp
    test_jaccard.cpp
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
