/**
 * @file test_dynamic_graph_cpo_random_access.cpp
 * @brief Consolidated CPO tests for all random-access container types (vov, vod, dov, dod)
 * 
 * Uses template infrastructure from graph_test_types.hpp to test all 4 container
 * types with a single set of test cases.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "../../common/graph_test_types.hpp"
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <string>
#include <vector>
#include <algorithm>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::container;
using namespace graph::test;

//==================================================================================================
// 1. vertices(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO vertices(g)", "[dynamic_graph][cpo][vertices]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;
    using Graph_int_vv = typename Types::int_vv;

    SECTION("returns vertex_descriptor_view") {
        Graph_void g;
        g.resize_vertices(5);
        
        auto v_range = vertices(g);
        
        REQUIRE(std::ranges::size(v_range) == 5);
        
        size_t count = 0;
        for ([[maybe_unused]] auto v : v_range) {
            ++count;
        }
        REQUIRE(count == 5);
    }

    SECTION("const correctness") {
        const Graph_void g;
        
        auto v_range = vertices(g);
        REQUIRE(std::ranges::size(v_range) == 0);
    }

    SECTION("with values") {
        Graph_int_vv g;
        g.resize_vertices(3);
        
        auto v_range = vertices(g);
        REQUIRE(std::ranges::size(v_range) == 3);
    }
}

//==================================================================================================
// 2. num_vertices(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO num_vertices(g)", "[dynamic_graph][cpo][num_vertices]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;
    using Graph_int_vv = typename Types::int_vv;

    SECTION("empty graph") {
        Graph_void g;
        REQUIRE(num_vertices(g) == 0);
    }

    SECTION("non-empty") {
        Graph_void g;
        g.resize_vertices(10);
        REQUIRE(num_vertices(g) == 10);
    }

    SECTION("matches vertices size") {
        Graph_int_vv g;
        g.resize_vertices(7);
        REQUIRE(num_vertices(g) == std::ranges::size(vertices(g)));
    }
}

//==================================================================================================
// 3. find_vertex(g, uid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO find_vertex(g, uid)", "[dynamic_graph][cpo][find_vertex]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("with uint32_t") {
        Graph_void g;
        g.resize_vertices(5);
        
        auto v = find_vertex(g, uint32_t{2});
        REQUIRE(v != vertices(g).end());
    }

    SECTION("with int") {
        Graph_void g;
        g.resize_vertices(5);
        
        auto v = find_vertex(g, 3);
        REQUIRE(v != vertices(g).end());
    }

    SECTION("bounds check") {
        Graph_void g;
        g.resize_vertices(3);
        
        auto v0 = find_vertex(g, 0);
        auto v2 = find_vertex(g, 2);
        
        REQUIRE(v0 != vertices(g).end());
        REQUIRE(v2 != vertices(g).end());
    }
}

//==================================================================================================
// 4. vertex_id(g, u) CPO Tests  
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO vertex_id(g, u)", "[dynamic_graph][cpo][vertex_id]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("basic access") {
        Graph_void g;
        g.resize_vertices(5);
        
        auto v_range = vertices(g);
        auto v_it = v_range.begin();
        auto v_desc = *v_it;
        
        auto id = vertex_id(g, v_desc);
        REQUIRE(id == 0);
    }

    SECTION("all vertices") {
        Graph_void g;
        g.resize_vertices(10);
        
        size_t expected_id = 0;
        for (auto v : vertices(g)) {
            REQUIRE(vertex_id(g, v) == expected_id);
            ++expected_id;
        }
    }

    SECTION("with find_vertex") {
        Graph_void g;
        g.resize_vertices(8);
        
        for (uint32_t expected_id = 0; expected_id < 8; ++expected_id) {
            auto v_it = find_vertex(g, expected_id);
            REQUIRE(v_it != vertices(g).end());
            
            auto v_desc = *v_it;
            auto actual_id = vertex_id(g, v_desc);
            REQUIRE(actual_id == expected_id);
        }
    }
}

//==================================================================================================
// 5. num_edges(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO num_edges(g)", "[dynamic_graph][cpo][num_edges]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("empty graph") {
        Graph_void g;
        REQUIRE(num_edges(g) == 0);
    }

    SECTION("with edges") {
        Graph_void g({{0, 1}, {1, 2}, {2, 0}});
        REQUIRE(num_edges(g) == 3);
    }

    SECTION("after multiple edge additions") {
        Graph_void g;
        g.resize_vertices(4);
        
        std::vector<copyable_edge_t<uint32_t, void>> ee = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0}, {0, 2}
        };
        g.load_edges(ee, std::identity{}, 4, 0);
        
        REQUIRE(num_edges(g) == 5);
    }
}

//==================================================================================================
// 6. has_edge(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO has_edge(g)", "[dynamic_graph][cpo][has_edge]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("empty graph") {
        Graph_void g;
        REQUIRE(!has_edge(g));
    }

    SECTION("with edges") {
        Graph_void g({{0, 1}});
        REQUIRE(has_edge(g));
    }

    SECTION("matches num_edges") {
        Graph_void g1;
        Graph_void g2({{0, 1}});
        
        REQUIRE(has_edge(g1) == (num_edges(g1) > 0));
        REQUIRE(has_edge(g2) == (num_edges(g2) > 0));
    }
}

//==================================================================================================
// 7. num_edges(g, u) CPO Tests - random_access containers support this
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO num_edges(g, u)", "[dynamic_graph][cpo][num_edges]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("vertex with no edges") {
        Graph_void g;
        g.resize_vertices(3);
        
        auto u = *find_vertex(g, 0);
        REQUIRE(num_edges(g, u) == 0);
    }

    SECTION("vertex with edges") {
        Graph_void g({{0, 1}, {0, 2}, {0, 3}});
        
        auto u = *find_vertex(g, 0);
        REQUIRE(num_edges(g, u) == 3);
    }

    SECTION("matches degree") {
        Graph_void g({{0, 1}, {0, 2}, {1, 2}});
        
        for (auto u : vertices(g)) {
            REQUIRE(num_edges(g, u) == degree(g, u));
        }
    }
}

//==================================================================================================
// 8. edges(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO edges(g, u)", "[dynamic_graph][cpo][edges]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;
    using Graph_int_ev = typename Types::int_ev;

    SECTION("returns edge range") {
        Graph_void g({{0, 1}, {0, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        static_assert(std::ranges::forward_range<decltype(edge_range)>);
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        REQUIRE(count == 2);
    }

    SECTION("empty edge list") {
        Graph_void g;
        g.resize_vertices(3);
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        REQUIRE(edge_range.begin() == edge_range.end());
    }

    SECTION("with edge values") {
        Graph_int_ev g({{0, 1, 100}, {0, 2, 200}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        std::vector<int> values;
        for (auto uv : edge_range) {
            values.push_back(edge_value(g, uv));
        }
        
        REQUIRE(values.size() == 2);
        REQUIRE(values[0] == 100);
        REQUIRE(values[1] == 200);
    }
}

//==================================================================================================
// 9. edges(g, uid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO edges(g, uid)", "[dynamic_graph][cpo][edges]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("with vertex ID") {
        Graph_void g({{0, 1}, {0, 2}});
        
        auto edge_range = edges(g, uint32_t(0));
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        REQUIRE(count == 2);
    }

    SECTION("consistency with edges(g, u)") {
        Graph_void g({{0, 1}, {0, 2}, {1, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto range_by_id = edges(g, uint32_t(0));
        auto range_by_desc = edges(g, u0);
        
        REQUIRE(std::ranges::distance(range_by_id) == std::ranges::distance(range_by_desc));
    }
}

//==================================================================================================
// 10. degree(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO degree(g, u)", "[dynamic_graph][cpo][degree]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("isolated vertex") {
        Graph_void g;
        g.resize_vertices(3);
        
        for (auto u : vertices(g)) {
            REQUIRE(degree(g, u) == 0);
        }
    }

    SECTION("with edges") {
        Graph_void g({{0, 1}, {0, 2}, {0, 3}, {1, 2}});
        
        auto v0 = *vertices(g).begin();
        REQUIRE(degree(g, v0) == 3);
        
        auto v1 = *std::next(vertices(g).begin(), 1);
        REQUIRE(degree(g, v1) == 1);
    }

    SECTION("by vertex ID") {
        Graph_void g({{0, 1}, {0, 2}, {0, 3}});
        
        REQUIRE(degree(g, uint32_t{0}) == 3);
        REQUIRE(degree(g, uint32_t{1}) == 0);
    }
}

//==================================================================================================
// 11. target_id(g, uv) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO target_id(g, uv)", "[dynamic_graph][cpo][target_id]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("basic access") {
        Graph_void g({{0, 1}, {0, 2}, {1, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_view = edges(g, u0);
        auto it = edge_view.begin();
        
        REQUIRE(it != edge_view.end());
        auto uv0 = *it;
        REQUIRE(target_id(g, uv0) == 1);
        
        ++it;
        auto uv1 = *it;
        REQUIRE(target_id(g, uv1) == 2);
    }

    SECTION("all edges valid") {
        Graph_void g({{0, 1}, {0, 2}, {1, 2}, {1, 3}, {2, 3}});
        
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto tid = target_id(g, uv);
                REQUIRE(tid < num_vertices(g));
            }
        }
    }
}

//==================================================================================================
// 12. target(g, uv) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO target(g, uv)", "[dynamic_graph][cpo][target]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("basic access") {
        Graph_void g({{0, 1}, {0, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_view = edges(g, u0);
        auto uv = *edge_view.begin();
        
        auto target_vertex = target(g, uv);
        REQUIRE(vertex_id(g, target_vertex) == 1);
    }

    SECTION("consistency with target_id") {
        Graph_void g({{0, 1}, {0, 2}, {1, 2}, {1, 3}});
        
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto target_desc = target(g, uv);
                auto tid = target_id(g, uv);
                REQUIRE(vertex_id(g, target_desc) == tid);
            }
        }
    }
}

//==================================================================================================
// 13. find_vertex_edge(g, uid, vid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO find_vertex_edge(g, uid, vid)", "[dynamic_graph][cpo][find_vertex_edge]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;
    using Graph_int_ev = typename Types::int_ev;

    SECTION("basic usage") {
        Graph_void g({{0, 1}, {0, 2}, {1, 2}, {2, 3}});
        
        auto e01 = find_vertex_edge(g, 0, 1);
        auto e02 = find_vertex_edge(g, 0, 2);
        auto e12 = find_vertex_edge(g, 1, 2);
        
        REQUIRE(target_id(g, e01) == 1);
        REQUIRE(target_id(g, e02) == 2);
        REQUIRE(target_id(g, e12) == 2);
    }

    SECTION("with edge values") {
        Graph_int_ev g;
        g.resize_vertices(4);
        
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 10}, {0, 2, 20}, {1, 2, 30}
        };
        g.load_edges(edge_data);
        
        auto e01 = find_vertex_edge(g, 0, 1);
        auto e02 = find_vertex_edge(g, 0, 2);
        
        REQUIRE(edge_value(g, e01) == 10);
        REQUIRE(edge_value(g, e02) == 20);
    }
}

//==================================================================================================
// 14. contains_edge(g, uid, vid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO contains_edge(g, uid, vid)", "[dynamic_graph][cpo][contains_edge]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("edge exists") {
        Graph_void g({{0, 1}, {0, 2}, {1, 2}});
        
        REQUIRE(contains_edge(g, 0, 1));
        REQUIRE(contains_edge(g, 0, 2));
        REQUIRE(contains_edge(g, 1, 2));
    }

    SECTION("edge does not exist") {
        Graph_void g({{0, 1}, {1, 2}});
        
        REQUIRE_FALSE(contains_edge(g, 0, 2));
        REQUIRE_FALSE(contains_edge(g, 1, 0));
        REQUIRE_FALSE(contains_edge(g, 2, 1));
    }

    SECTION("self-loop") {
        Graph_void g({{0, 0}, {0, 1}});
        
        REQUIRE(contains_edge(g, 0, 0));
        REQUIRE(contains_edge(g, 0, 1));
    }
}

//==================================================================================================
// 15. vertex_value(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO vertex_value(g, u)", "[dynamic_graph][cpo][vertex_value]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_int_vv = typename Types::int_vv;
    using Graph_string = typename Types::string_type;

    SECTION("basic access") {
        Graph_int_vv g;
        g.resize_vertices(3);
        
        auto u = *vertices(g).begin();
        vertex_value(g, u) = 42;
        REQUIRE(vertex_value(g, u) == 42);
    }

    SECTION("multiple vertices") {
        Graph_int_vv g;
        g.resize_vertices(5);
        
        int val = 0;
        for (auto u : vertices(g)) {
            vertex_value(g, u) = val;
            val += 100;
        }
        
        val = 0;
        for (auto u : vertices(g)) {
            REQUIRE(vertex_value(g, u) == val);
            val += 100;
        }
    }

    SECTION("with string values") {
        Graph_string g;
        g.resize_vertices(2);
        
        auto it = vertices(g).begin();
        vertex_value(g, *it) = "first";
        ++it;
        vertex_value(g, *it) = "second";
        
        it = vertices(g).begin();
        REQUIRE(vertex_value(g, *it) == "first");
    }
}

//==================================================================================================
// 16. graph_value(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO graph_value(g)", "[dynamic_graph][cpo][graph_value]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_all_int = typename Types::all_int;

    SECTION("basic access") {
        Graph_all_int g({{0, 1, 1}});
        
        graph_value(g) = 42;
        REQUIRE(graph_value(g) == 42);
    }

    SECTION("default initialization") {
        Graph_all_int g;
        REQUIRE(graph_value(g) == 0);
    }

    SECTION("modification") {
        Graph_all_int g({{0, 1, 1}});
        
        graph_value(g) = 10;
        graph_value(g) += 5;
        REQUIRE(graph_value(g) == 15);
    }
}

//==================================================================================================
// 17. partition_id(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO partition_id(g, u)", "[dynamic_graph][cpo][partition_id]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("default single partition") {
        Graph_void g;
        g.resize_vertices(5);
        
        for (auto u : vertices(g)) {
            REQUIRE(partition_id(g, u) == 0);
        }
    }

    SECTION("with edges") {
        Graph_void g({{0, 1}, {1, 2}, {2, 3}});
        
        for (auto u : vertices(g)) {
            REQUIRE(partition_id(g, u) == 0);
        }
    }
}

//==================================================================================================
// 18. num_partitions(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO num_partitions(g)", "[dynamic_graph][cpo][num_partitions]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("default value") {
        Graph_void g;
        REQUIRE(num_partitions(g) == 1);
    }

    SECTION("with vertices") {
        Graph_void g({{0, 1}, {1, 2}});
        REQUIRE(num_partitions(g) == 1);
    }
}

//==================================================================================================
// 19. vertices(g, pid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO vertices(g, pid)", "[dynamic_graph][cpo][vertices][partition]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("partition 0 returns all vertices") {
        Graph_void g({{0, 1}, {1, 2}, {2, 3}});
        
        auto verts_all = vertices(g);
        auto verts_p0 = vertices(g, 0);
        
        REQUIRE(std::ranges::distance(verts_all) == std::ranges::distance(verts_p0));
    }

    SECTION("non-zero partition returns empty") {
        Graph_void g({{0, 1}, {1, 2}});
        
        auto verts_p1 = vertices(g, 1);
        REQUIRE(std::ranges::distance(verts_p1) == 0);
    }
}

//==================================================================================================
// 20. num_vertices(g, pid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO num_vertices(g, pid)", "[dynamic_graph][cpo][num_vertices][partition]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("partition 0 returns total count") {
        Graph_void g({{0, 1}, {1, 2}, {2, 3}});
        
        REQUIRE(num_vertices(g, 0) == num_vertices(g));
    }

    SECTION("non-zero partition returns zero") {
        Graph_void g({{0, 1}, {1, 2}});
        
        REQUIRE(num_vertices(g, 1) == 0);
    }
}

//==================================================================================================
// 21. source_id(g, uv) CPO Tests - Sourced graphs only
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO source_id(g, uv)", "[dynamic_graph][cpo][source_id]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_sourced_void = typename Types::sourced_void;

    SECTION("basic usage") {
        Graph_sourced_void g({{0, 1}, {1, 2}, {0, 2}});
        
        auto u0 = *find_vertex(g, 0);
        for (auto uv : edges(g, u0)) {
            REQUIRE(source_id(g, uv) == 0);
        }
    }

    SECTION("different sources") {
        Graph_sourced_void g({{0, 1}, {1, 2}, {2, 3}});
        
        for (size_t i = 0; i < 3; ++i) {
            auto u = *find_vertex(g, i);
            for (auto uv : edges(g, u)) {
                REQUIRE(source_id(g, uv) == i);
            }
        }
    }
}

//==================================================================================================
// 22. source(g, uv) CPO Tests - Sourced graphs only
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO source(g, uv)", "[dynamic_graph][cpo][source]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_sourced_void = typename Types::sourced_void;

    SECTION("basic usage") {
        Graph_sourced_void g({{0, 1}, {1, 2}});
        
        auto u0 = *find_vertex(g, 0);
        for (auto uv : edges(g, u0)) {
            auto src = source(g, uv);
            REQUIRE(vertex_id(g, src) == 0);
        }
    }

    SECTION("consistency with source_id") {
        Graph_sourced_void g({{0, 1}, {1, 2}, {2, 3}});
        
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto src = source(g, uv);
                REQUIRE(vertex_id(g, src) == source_id(g, uv));
            }
        }
    }
}

//==================================================================================================
// 23. Integration Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO integration", "[dynamic_graph][cpo][integration]",
                   vov_tag, vod_tag, dov_tag, dod_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("graph construction and traversal") {
        Graph_void g({{0, 1}, {1, 2}});
        
        REQUIRE(num_vertices(g) == 3);
        REQUIRE(num_edges(g) == 2);
        REQUIRE(has_edge(g));
    }

    SECTION("empty graph properties") {
        Graph_void g;
        
        REQUIRE(num_vertices(g) == 0);
        REQUIRE(num_edges(g) == 0);
        REQUIRE(!has_edge(g));
    }
}
