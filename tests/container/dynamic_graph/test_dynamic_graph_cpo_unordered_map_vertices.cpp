/**
 * @file test_dynamic_graph_cpo_unordered_map_vertices.cpp
 * @brief Consolidated CPO tests for unordered_map-based vertex containers (uol, uov, uod, uofl, uos, uous)
 * 
 * Unordered_map-based vertex containers have key differences:
 * - Vertices are created on-demand from edge endpoints (no resize_vertices)
 * - Vertex IDs can be sparse (non-contiguous, e.g., 100, 500, 1000)
 * - Vertex iteration order is UNSPECIFIED (hash-based)
 * - Tests use sorted comparison or contains checks
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "../../common/graph_test_types.hpp"
#include "../../common/map_graph_test_data.hpp"
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <string>
#include <vector>
#include <algorithm>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::container;
using namespace graph::test;
using namespace graph::test::map_data;

//==================================================================================================
// 1. vertices(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered_map CPO vertices(g)", "[dynamic_graph][cpo][vertices][unordered_map]",
                   uol_tag, uov_tag, uod_tag, uofl_tag, uos_tag, uous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("basic edges - contiguous IDs") {
        auto g = make_basic_graph_void<Graph_void>();
        
        size_t count = 0;
        for ([[maybe_unused]] auto v : vertices(g)) {
            ++count;
        }
        REQUIRE(count == basic_expected::vertex_count);
    }

    SECTION("sparse vertex IDs - key feature") {
        auto g = make_sparse_graph_void<Graph_void>();
        
        // Collect vertex IDs (order unspecified)
        std::vector<uint32_t> ids;
        for (auto v : vertices(g)) {
            ids.push_back(vertex_id(g, v));
        }
        
        // Sort for comparison
        std::ranges::sort(ids);
        REQUIRE(ids.size() == sparse_expected::vertex_count);
        for (size_t i = 0; i < ids.size(); ++i) {
            REQUIRE(ids[i] == sparse_expected::vertex_ids_sorted[i]);
        }
    }

    SECTION("very sparse IDs - large gaps") {
        auto g = make_very_sparse_graph<Graph_void>();
        
        std::vector<uint32_t> ids;
        for (auto v : vertices(g)) {
            ids.push_back(vertex_id(g, v));
        }
        
        std::ranges::sort(ids);
        REQUIRE(ids.size() == very_sparse_expected::vertex_count);
        for (size_t i = 0; i < ids.size(); ++i) {
            REQUIRE(ids[i] == very_sparse_expected::vertex_ids_sorted[i]);
        }
    }
}

//==================================================================================================
// 2. num_vertices(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered_map CPO num_vertices(g)", "[dynamic_graph][cpo][num_vertices][unordered_map]",
                   uol_tag, uov_tag, uod_tag, uofl_tag, uos_tag, uous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("basic edges") {
        auto g = make_basic_graph_void<Graph_void>();
        REQUIRE(num_vertices(g) == basic_expected::vertex_count);
    }

    SECTION("sparse IDs") {
        auto g = make_sparse_graph_void<Graph_void>();
        REQUIRE(num_vertices(g) == sparse_expected::vertex_count);
    }

    SECTION("empty graph") {
        Graph_void g;
        REQUIRE(num_vertices(g) == 0);
    }
}

//==================================================================================================
// 3. find_vertex(g, uid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered_map CPO find_vertex(g, uid)", "[dynamic_graph][cpo][find_vertex][unordered_map]",
                   uol_tag, uov_tag, uod_tag, uofl_tag, uos_tag, uous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("find existing vertex - sparse IDs") {
        auto g = make_sparse_graph_void<Graph_void>();
        
        for (auto expected_id : sparse_expected::vertex_ids_sorted) {
            auto it = find_vertex(g, expected_id);
            REQUIRE(it != vertices(g).end());
            REQUIRE(vertex_id(g, *it) == expected_id);
        }
    }

    SECTION("find non-existent vertex") {
        auto g = make_sparse_graph_void<Graph_void>();
        
        auto it = find_vertex(g, uint32_t(999));
        REQUIRE(it == vertices(g).end());
    }
}

//==================================================================================================
// 4. vertex_id(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered_map CPO vertex_id(g, u)", "[dynamic_graph][cpo][vertex_id][unordered_map]",
                   uol_tag, uov_tag, uod_tag, uofl_tag, uos_tag, uous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("sparse IDs - collect and verify") {
        auto g = make_sparse_graph_void<Graph_void>();
        
        std::vector<uint32_t> ids;
        for (auto v : vertices(g)) {
            ids.push_back(vertex_id(g, v));
        }
        
        std::ranges::sort(ids);
        std::vector<uint32_t> expected(sparse_expected::vertex_ids_sorted.begin(),
                                        sparse_expected::vertex_ids_sorted.end());
        REQUIRE(ids == expected);
    }
}

//==================================================================================================
// 5. num_edges(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered_map CPO num_edges(g)", "[dynamic_graph][cpo][num_edges][unordered_map]",
                   uol_tag, uov_tag, uod_tag, uofl_tag, uos_tag, uous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("basic edges") {
        auto g = make_basic_graph_void<Graph_void>();
        REQUIRE(num_edges(g) == basic_expected::edge_count);
    }

    SECTION("sparse IDs") {
        auto g = make_sparse_graph_void<Graph_void>();
        REQUIRE(num_edges(g) == sparse_expected::edge_count);
    }

    SECTION("empty graph") {
        Graph_void g;
        REQUIRE(num_edges(g) == 0);
    }
}

//==================================================================================================
// 6. has_edge(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered_map CPO has_edge(g)", "[dynamic_graph][cpo][has_edge][unordered_map]",
                   uol_tag, uov_tag, uod_tag, uofl_tag, uos_tag, uous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("graph with edges") {
        auto g = make_basic_graph_void<Graph_void>();
        REQUIRE(has_edge(g) == true);
    }

    SECTION("empty graph") {
        Graph_void g;
        REQUIRE(has_edge(g) == false);
    }
}

//==================================================================================================
// 7. edges(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered_map CPO edges(g, u)", "[dynamic_graph][cpo][edges][unordered_map]",
                   uol_tag, uov_tag, uod_tag, uofl_tag, uos_tag, uous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;
    using Graph_int_ev = typename Types::int_ev;

    SECTION("edges from sparse vertex") {
        auto g = make_sparse_graph_void<Graph_void>();
        
        auto v100 = *find_vertex(g, uint32_t(100));
        
        std::vector<uint32_t> targets;
        for (auto uv : edges(g, v100)) {
            targets.push_back(target_id(g, uv));
        }
        
        std::ranges::sort(targets);
        REQUIRE(targets.size() == 2);
        REQUIRE(targets[0] == 500);
        REQUIRE(targets[1] == 1000);
    }

    SECTION("with edge values") {
        auto g = make_sparse_graph_int<Graph_int_ev>();
        
        auto v100 = *find_vertex(g, uint32_t(100));
        
        int sum = 0;
        for (auto uv : edges(g, v100)) {
            sum += edge_value(g, uv);
        }
        REQUIRE(sum == 40);  // 15 + 25
    }
}

//==================================================================================================
// 8. degree(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered_map CPO degree(g, u)", "[dynamic_graph][cpo][degree][unordered_map]",
                   uol_tag, uov_tag, uod_tag, uofl_tag, uos_tag, uous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("sparse vertices") {
        auto g = make_sparse_graph_void<Graph_void>();
        
        auto v100 = *find_vertex(g, uint32_t(100));
        REQUIRE(degree(g, v100) == 2);
        
        auto v5000 = *find_vertex(g, uint32_t(5000));
        REQUIRE(degree(g, v5000) == 0);
    }
}

//==================================================================================================
// 9. target_id(g, uv) and target(g, uv) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered_map CPO target_id(g, uv)", "[dynamic_graph][cpo][target_id][unordered_map]",
                   uol_tag, uov_tag, uod_tag, uofl_tag, uos_tag, uous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("sparse targets") {
        auto g = make_sparse_graph_void<Graph_void>();
        
        auto v100 = *find_vertex(g, uint32_t(100));
        
        std::vector<uint32_t> targets;
        for (auto uv : edges(g, v100)) {
            targets.push_back(target_id(g, uv));
        }
        
        std::ranges::sort(targets);
        REQUIRE(targets[0] == 500);
        REQUIRE(targets[1] == 1000);
    }
}

//==================================================================================================
// 10. find_vertex_edge and contains_edge CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered_map CPO contains_edge(g, uid, vid)", "[dynamic_graph][cpo][contains_edge][unordered_map]",
                   uol_tag, uov_tag, uod_tag, uofl_tag, uos_tag, uous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("edge exists - sparse IDs") {
        auto g = make_sparse_graph_void<Graph_void>();
        
        REQUIRE(contains_edge(g, uint32_t(100), uint32_t(500)) == true);
        REQUIRE(contains_edge(g, uint32_t(100), uint32_t(1000)) == true);
    }

    SECTION("edge does not exist") {
        auto g = make_sparse_graph_void<Graph_void>();
        
        REQUIRE(contains_edge(g, uint32_t(100), uint32_t(5000)) == false);
        REQUIRE(contains_edge(g, uint32_t(500), uint32_t(100)) == false);
    }
}

//==================================================================================================
// 11. vertex_value(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered_map CPO vertex_value(g, u)", "[dynamic_graph][cpo][vertex_value][unordered_map]",
                   uol_tag, uov_tag, uod_tag, uofl_tag, uos_tag, uous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_int_vv = typename Types::int_vv;

    SECTION("access and modify") {
        auto g = make_sparse_graph_void<Graph_int_vv>();
        
        auto v100 = *find_vertex(g, uint32_t(100));
        vertex_value(g, v100) = 42;
        REQUIRE(vertex_value(g, v100) == 42);
    }
}

//==================================================================================================
// 12. edge_value(g, uv) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered_map CPO edge_value(g, uv)", "[dynamic_graph][cpo][edge_value][unordered_map]",
                   uol_tag, uov_tag, uod_tag, uofl_tag) {  // uos, uous use set - const edges
    using Types = graph_test_types<TestType>;
    using Graph_int_ev = typename Types::int_ev;

    SECTION("access edge values") {
        auto g = make_sparse_graph_int<Graph_int_ev>();
        
        auto v100 = *find_vertex(g, uint32_t(100));
        
        int sum = 0;
        for (auto uv : edges(g, v100)) {
            sum += edge_value(g, uv);
        }
        REQUIRE(sum == 40);
    }
}

//==================================================================================================
// 13. graph_value(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered_map CPO graph_value(g)", "[dynamic_graph][cpo][graph_value][unordered_map]",
                   uol_tag, uov_tag, uod_tag, uofl_tag, uos_tag, uous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_all_int = typename Types::all_int;

    SECTION("access and modify") {
        auto g = make_sparse_graph_int<Graph_all_int>();
        
        graph_value(g) = 99;
        REQUIRE(graph_value(g) == 99);
    }
}

//==================================================================================================
// 14. source_id(g, uv) CPO Tests (Sourced=true)
//==================================================================================================

TEMPLATE_TEST_CASE("unordered_map CPO source_id(g, uv)", "[dynamic_graph][cpo][source_id][unordered_map]",
                   uol_tag, uov_tag, uod_tag, uofl_tag, uos_tag, uous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_sourced = typename Types::sourced_void;

    SECTION("sparse source IDs") {
        auto g = make_sparse_graph_void<Graph_sourced>();
        
        auto v100 = *find_vertex(g, uint32_t(100));
        
        for (auto uv : edges(g, v100)) {
            REQUIRE(source_id(g, uv) == 100);
        }
    }
}

//==================================================================================================
// 15. partition_id and num_partitions CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered_map CPO partition_id(g, u)", "[dynamic_graph][cpo][partition_id][unordered_map]",
                   uol_tag, uov_tag, uod_tag, uofl_tag, uos_tag, uous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("default partition") {
        auto g = make_sparse_graph_void<Graph_void>();
        
        for (auto v : vertices(g)) {
            REQUIRE(partition_id(g, v) == 0);
        }
    }
}

TEMPLATE_TEST_CASE("unordered_map CPO num_partitions(g)", "[dynamic_graph][cpo][num_partitions][unordered_map]",
                   uol_tag, uov_tag, uod_tag, uofl_tag, uos_tag, uous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("default single partition") {
        auto g = make_sparse_graph_void<Graph_void>();
        REQUIRE(num_partitions(g) == 1);
    }
}

//==================================================================================================
// 16. Integration Tests - Sparse IDs
//==================================================================================================

TEMPLATE_TEST_CASE("unordered_map CPO integration: sparse traversal", "[dynamic_graph][cpo][integration][unordered_map]",
                   uol_tag, uov_tag, uod_tag, uofl_tag, uos_tag, uous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_int_ev = typename Types::int_ev;

    SECTION("traverse all edges with sparse IDs") {
        auto g = make_sparse_graph_int<Graph_int_ev>();
        
        int total = 0;
        size_t edge_count = 0;
        
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                total += edge_value(g, uv);
                ++edge_count;
            }
        }
        
        REQUIRE(edge_count == sparse_expected::edge_count);
        REQUIRE(total == sparse_expected::edge_value_sum);
    }
}
