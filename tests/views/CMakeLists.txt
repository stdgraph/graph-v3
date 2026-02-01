add_executable(graph3_views_tests
    test_main.cpp
    test_vertex_info.cpp
    test_edge_info.cpp
    test_neighbor_info.cpp
    test_search_base.cpp
    test_view_concepts.cpp
    test_vertexlist.cpp
    test_incidence.cpp
    test_neighbors.cpp
    test_edgelist.cpp
    test_basic_views.cpp
)

target_link_libraries(graph3_views_tests
    PRIVATE
        graph3
        Catch2::Catch2
)

add_test(NAME views_tests COMMAND graph3_views_tests)
