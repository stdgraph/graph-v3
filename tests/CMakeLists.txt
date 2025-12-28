cmake_minimum_required(VERSION 3.20)

include(FetchContent)

# Fetch Catch2 v3
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.0
)
FetchContent_MakeAvailable(Catch2)

# Test executable
add_executable(graph3_tests
    test_main.cpp
    test_vertex_descriptor.cpp
    test_edge_descriptor.cpp
    test_descriptor_traits.cpp
    test_edge_concepts.cpp
    test_vertex_concepts.cpp
    test_vertices_cpo.cpp
    test_vertex_id_cpo.cpp
    test_find_vertex_cpo.cpp
    test_edges_cpo.cpp
    test_edges_uid_cpo.cpp
    test_target_id_cpo.cpp
    test_target_cpo.cpp
    test_num_vertices_cpo.cpp
    test_num_edges_cpo.cpp
    test_degree_cpo.cpp
    test_find_vertex_edge_cpo.cpp
    test_contains_edge_cpo.cpp
    test_has_edge_cpo.cpp
    test_vertex_value_cpo.cpp
    test_edge_value_cpo.cpp
    test_graph_value_cpo.cpp
    test_partition_id_cpo.cpp
    test_num_partitions_cpo.cpp
    test_vertices_pid_cpo.cpp
    test_num_vertices_pid_cpo.cpp
    test_type_aliases.cpp
    test_adjacency_list_edge_concepts.cpp
    test_adjacency_list_vertex_concepts.cpp
    test_adjacency_list_traits.cpp
    test_compressed_graph.cpp
    test_compressed_graph_cpo.cpp
    test_dynamic_graph_vofl.cpp
    test_dynamic_graph_vol.cpp
    test_dynamic_graph_vov.cpp
    test_dynamic_graph_vod.cpp
    test_dynamic_graph_dofl.cpp
    test_dynamic_graph_dol.cpp
    test_dynamic_graph_dov.cpp
    test_dynamic_graph_dod.cpp
    test_dynamic_graph_mofl.cpp
    test_dynamic_graph_mol.cpp
    test_dynamic_graph_mov.cpp
    test_dynamic_graph_mod.cpp
    test_dynamic_graph_uofl.cpp
    test_dynamic_graph_uol.cpp
    test_dynamic_graph_uov.cpp
    test_dynamic_graph_uod.cpp
    test_dynamic_graph_common.cpp
    test_dynamic_graph_cpo_vofl.cpp
    test_dynamic_graph_cpo_vol.cpp
    test_dynamic_graph_cpo_vov.cpp
    test_dynamic_graph_cpo_vod.cpp
    test_dynamic_graph_cpo_dofl.cpp
    test_dynamic_graph_cpo_dol.cpp
    test_dynamic_graph_cpo_dov.cpp
    test_dynamic_graph_cpo_dod.cpp
    test_dynamic_graph_cpo_mofl.cpp
    test_dynamic_graph_cpo_mol.cpp
    test_dynamic_graph_cpo_mov.cpp
    test_dynamic_graph_cpo_mod.cpp
    test_dynamic_graph_cpo_uofl.cpp
    test_dynamic_graph_cpo_uol.cpp
    test_dynamic_graph_cpo_uov.cpp
    test_dynamic_graph_cpo_uod.cpp
    test_dynamic_edge_comparison.cpp
    test_dynamic_graph_vos.cpp
    test_dynamic_graph_cpo_vos.cpp
    test_dynamic_graph_dos.cpp
    test_dynamic_graph_cpo_dos.cpp
    test_dynamic_graph_mos.cpp
    test_dynamic_graph_cpo_mos.cpp
    test_dynamic_graph_vous.cpp
    test_dynamic_graph_cpo_vous.cpp
    test_dynamic_graph_dous.cpp
    test_dynamic_graph_cpo_dous.cpp
)

target_link_libraries(graph3_tests
    PRIVATE
        graph3
        Catch2::Catch2WithMain
)

# Register tests with CTest
include(CTest)
include(Catch)
catch_discover_tests(graph3_tests)
