# adj_list tests executable
add_executable(graph3_adj_list_tests
    # Concepts
    concepts/test_edge_concepts.cpp
    concepts/test_vertex_concepts.cpp
    concepts/test_adjacency_list_edge_concepts.cpp
    concepts/test_adjacency_list_vertex_concepts.cpp
    concepts/test_bidirectional_concepts.cpp
    
    # Descriptors
    descriptors/test_vertex_descriptor.cpp
    descriptors/test_edge_descriptor.cpp
    descriptors/test_descriptor_traits.cpp
    
    # CPOs
    cpo/test_vertices_cpo.cpp
    cpo/test_vertex_id_cpo.cpp
    cpo/test_find_vertex_cpo.cpp
    cpo/test_edges_cpo.cpp
    cpo/test_edges_uid_cpo.cpp
    cpo/test_target_id_cpo.cpp
    cpo/test_target_cpo.cpp
    cpo/test_source_id_cpo.cpp
    cpo/test_source_cpo.cpp
    cpo/test_num_vertices_cpo.cpp
    cpo/test_num_edges_cpo.cpp
    cpo/test_degree_cpo.cpp
    cpo/test_in_edges_cpo.cpp
    cpo/test_find_in_edge_cpo.cpp
    cpo/test_contains_in_edge_cpo.cpp
    cpo/test_find_vertex_edge_cpo.cpp
    cpo/test_contains_edge_cpo.cpp
    cpo/test_has_edge_cpo.cpp
    cpo/test_vertex_value_cpo.cpp
    cpo/test_edge_value_cpo.cpp
    cpo/test_edge_value_cpo_value_method.cpp
    cpo/test_graph_value_cpo.cpp
    cpo/test_partition_id_cpo.cpp
    cpo/test_num_partitions_cpo.cpp
    cpo/test_vertices_pid_cpo.cpp
    cpo/test_num_vertices_pid_cpo.cpp
    
    # Traits
    traits/test_type_aliases.cpp
    traits/test_incoming_edge_traits.cpp
    traits/test_adjacency_list_traits.cpp
)

target_link_libraries(graph3_adj_list_tests
    PRIVATE
        graph3
        Catch2::Catch2WithMain
)

# MSVC requires /bigobj for files with many template instantiations
if(MSVC)
    target_compile_options(graph3_adj_list_tests PRIVATE /bigobj)
endif()

# Register tests with CTest
catch_discover_tests(graph3_adj_list_tests)
