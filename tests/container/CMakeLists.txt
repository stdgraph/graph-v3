# container tests executable
add_executable(graph3_container_tests
    # compressed_graph
    compressed_graph/test_compressed_graph.cpp
    compressed_graph/test_compressed_graph_cpo.cpp
    
    # dynamic_graph - non-CPO tests
    dynamic_graph/test_dynamic_graph_vofl.cpp
    dynamic_graph/test_dynamic_graph_vol.cpp
    dynamic_graph/test_dynamic_graph_vov.cpp
    dynamic_graph/test_dynamic_graph_vod.cpp
    dynamic_graph/test_dynamic_graph_dofl.cpp
    dynamic_graph/test_dynamic_graph_dol.cpp
    dynamic_graph/test_dynamic_graph_dov.cpp
    dynamic_graph/test_dynamic_graph_dod.cpp
    dynamic_graph/test_dynamic_graph_mofl.cpp
    dynamic_graph/test_dynamic_graph_mol.cpp
    dynamic_graph/test_dynamic_graph_mov.cpp
    dynamic_graph/test_dynamic_graph_mod.cpp
    dynamic_graph/test_dynamic_graph_uofl.cpp
    dynamic_graph/test_dynamic_graph_uol.cpp
    dynamic_graph/test_dynamic_graph_uov.cpp
    dynamic_graph/test_dynamic_graph_uod.cpp
    dynamic_graph/test_dynamic_graph_vos.cpp
    dynamic_graph/test_dynamic_graph_vom.cpp
    dynamic_graph/test_dynamic_graph_voum.cpp
    dynamic_graph/test_dynamic_graph_mom.cpp
    dynamic_graph/test_dynamic_graph_dos.cpp
    dynamic_graph/test_dynamic_graph_mos.cpp
    dynamic_graph/test_dynamic_graph_uos.cpp
    dynamic_graph/test_dynamic_graph_vous.cpp
    dynamic_graph/test_dynamic_graph_dous.cpp
    dynamic_graph/test_dynamic_graph_mous.cpp
    dynamic_graph/test_dynamic_graph_uous.cpp
    dynamic_graph/test_dynamic_graph_common.cpp
    dynamic_graph/test_dynamic_edge_comparison.cpp
    dynamic_graph/test_dynamic_graph_nonintegral_ids.cpp
    dynamic_graph/test_dynamic_graph_integration.cpp
    dynamic_graph/test_dynamic_graph_stl_algorithms.cpp
    dynamic_graph/test_dynamic_graph_generic_queries.cpp
    dynamic_graph/test_dynamic_graph_traversal_helpers.cpp
    dynamic_graph/test_dynamic_graph_transformations.cpp
    dynamic_graph/test_dynamic_graph_validation.cpp
    dynamic_graph/test_dynamic_graph_type_erasure.cpp
    dynamic_graph/test_dynamic_graph_mixed_types.cpp
    dynamic_graph/test_dynamic_graph_conversions.cpp
    dynamic_graph/test_dynamic_graph_heterogeneous.cpp
    
    # dynamic_graph - CPO tests (consolidated)
    dynamic_graph/test_dynamic_graph_cpo_random_access.cpp
    dynamic_graph/test_dynamic_graph_cpo_forward_list.cpp
    dynamic_graph/test_dynamic_graph_cpo_sorted.cpp
    dynamic_graph/test_dynamic_graph_cpo_unordered.cpp
    dynamic_graph/test_dynamic_graph_cpo_map_vertices.cpp
    dynamic_graph/test_dynamic_graph_cpo_unordered_map_vertices.cpp
    
    # dynamic_graph - CPO tests (edge map containers - vom, mom)
    dynamic_graph/test_dynamic_graph_cpo_edge_map.cpp
    
    # undirected_adjacency_list
    undirected_adjacency_list/test_undirected_adjacency_list.cpp
    undirected_adjacency_list/test_undirected_adjacency_list_cpo.cpp
    undirected_adjacency_list/test_undirected_bidirectional.cpp
)

target_link_libraries(graph3_container_tests
    PRIVATE
        graph3
        Catch2::Catch2WithMain
)

# MSVC requires /bigobj for files with many template instantiations
if(MSVC)
    target_compile_options(graph3_container_tests PRIVATE /bigobj)
endif()

# Register tests with CTest
catch_discover_tests(graph3_container_tests)
