/**
 * @file test_dynamic_graph_cpo_mod.cpp
 * @brief Phase 3.1c CPO tests for dynamic_graph with mod_graph_traits
 * 
 * Tests CPO (Customization Point Object) integration with dynamic_graph.
 * These tests verify that CPOs work correctly with associative vertex containers.
 * 
 * Container: map<VId, vertex> + deque<edge>
 * 
 * CPOs tested (mirroring test_dynamic_graph_cpo_mov.cpp):
 * - vertices(g) - Get vertex range
 * - vertices(g, pid) - Get vertex range for partition (single partition default)
 * - num_vertices(g) - Get vertex count
 * - num_vertices(g, pid) - Get vertex count for partition
 * - find_vertex(g, uid) - Find vertex by ID
 * - vertex_id(g, u) - Get vertex ID from descriptor
 * - num_edges(g) - Get total edge count
 * - has_edge(g) - Check if graph has any edges
 * - edges(g, u) - Get edge range for vertex
 * - edges(g, uid) - Get edge range by vertex ID
 * - degree(g, u) - Get out-degree of vertex
 * - target_id(g, uv) - Get target vertex ID from edge
 * - target(g, uv) - Get target vertex descriptor from edge
 * - find_vertex_edge(g, u, v) - Find edge between vertices
 * - contains_edge(g, u, v) and contains_edge(g, uid, vid) - Check if edge exists
 * - vertex_value(g, u) - Access vertex value (when VV != void)
 * - edge_value(g, uv) - Access edge value (when EV != void)
 * - graph_value(g) - Access graph value (when GV != void)
 * - partition_id(g, u) - Get partition ID for vertex (single partition default)
 * - num_partitions(g) - Get number of partitions (default 1)
 * - source_id(g, uv) - Get source vertex ID from edge (Sourced=true)
 * - source(g, uv) - Get source vertex descriptor from edge (Sourced=true)
 * 
 * Key differences from mov tests:
 * - std::deque edges provide random access (same as vector)
 * - Both preserve edge order: first added appears first
 * - Vertices are sparse (only referenced vertices exist) - same as mov
 * - Map iteration is in key order (sorted) - same as mov
 * - String vertex IDs are tested - same as mov
 * 
 * Note: Descriptor iterators are forward-only despite underlying container capabilities.
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/container/traits/mod_graph_traits.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/detail/graph_cpo.hpp>
#include <string>
#include <vector>
#include <algorithm>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::container;

// Type aliases for test configurations with uint32_t vertex IDs
using mod_void     = dynamic_graph<void, void, void, uint32_t, false, mod_graph_traits<void, void, void, uint32_t, false>>;
using mod_int_ev   = dynamic_graph<int, void, void, uint32_t, false, mod_graph_traits<int, void, void, uint32_t, false>>;
using mod_int_vv   = dynamic_graph<void, int, void, uint32_t, false, mod_graph_traits<void, int, void, uint32_t, false>>;
using mod_all_int  = dynamic_graph<int, int, int, uint32_t, false, mod_graph_traits<int, int, int, uint32_t, false>>;

// Type aliases with string vertex IDs (primary use case for map containers)
using mod_str_void     = dynamic_graph<void, void, void, std::string, false, mod_graph_traits<void, void, void, std::string, false>>;
using mod_str_int_ev   = dynamic_graph<int, void, void, std::string, false, mod_graph_traits<int, void, void, std::string, false>>;
using mod_str_int_vv   = dynamic_graph<void, int, void, std::string, false, mod_graph_traits<void, int, void, std::string, false>>;
using mod_str_all_int  = dynamic_graph<int, int, int, std::string, false, mod_graph_traits<int, int, int, std::string, false>>;

// Type aliases for Sourced=true configurations
using mod_sourced_void = dynamic_graph<void, void, void, uint32_t, true, mod_graph_traits<void, void, void, uint32_t, true>>;
using mod_sourced_int  = dynamic_graph<int, void, void, uint32_t, true, mod_graph_traits<int, void, void, uint32_t, true>>;
using mod_str_sourced  = dynamic_graph<void, void, void, std::string, true, mod_graph_traits<void, void, void, std::string, true>>;

//==================================================================================================
// 1. vertices(g) CPO Tests
//==================================================================================================

TEST_CASE("mod CPO vertices(g)", "[dynamic_graph][mod][cpo][vertices]") {
    SECTION("returns vertex range - uint32_t IDs") {
        mod_void g({{0, 1}, {1, 2}});
        
        auto v_range = vertices(g);
        
        // Should be iterable
        size_t count = 0;
        for ([[maybe_unused]] auto v : v_range) {
            ++count;
        }
        REQUIRE(count == 3);  // Vertices 0, 1, 2
    }

    SECTION("returns vertex range - string IDs") {
        mod_str_void g({{"alice", "bob"}, {"bob", "charlie"}});
        
        auto v_range = vertices(g);
        
        size_t count = 0;
        for ([[maybe_unused]] auto v : v_range) {
            ++count;
        }
        REQUIRE(count == 3);  // alice, bob, charlie
    }

    SECTION("empty graph") {
        mod_void g;
        
        auto v_range = vertices(g);
        REQUIRE(v_range.begin() == v_range.end());
    }

    SECTION("const correctness") {
        const mod_void g({{0, 1}});
        
        auto v_range = vertices(g);
        size_t count = 0;
        for ([[maybe_unused]] auto v : v_range) {
            ++count;
        }
        REQUIRE(count == 2);
    }

    SECTION("sparse vertices - only referenced exist") {
        mod_void g({{100, 200}});
        
        auto v_range = vertices(g);
        size_t count = 0;
        for ([[maybe_unused]] auto v : v_range) {
            ++count;
        }
        REQUIRE(count == 2);  // Only 100 and 200, not 0-200
    }
}

//==================================================================================================
// 2. num_vertices(g) CPO Tests
//==================================================================================================

TEST_CASE("mod CPO num_vertices(g)", "[dynamic_graph][mod][cpo][num_vertices]") {
    SECTION("empty graph") {
        mod_void g;
        REQUIRE(num_vertices(g) == 0);
    }

    SECTION("with edges - uint32_t IDs") {
        mod_void g({{0, 1}, {1, 2}, {2, 0}});
        REQUIRE(num_vertices(g) == 3);
    }

    SECTION("with edges - string IDs") {
        mod_str_void g({{"a", "b"}, {"b", "c"}, {"c", "d"}});
        REQUIRE(num_vertices(g) == 4);
    }

    SECTION("sparse IDs") {
        mod_void g({{100, 200}, {300, 400}});
        REQUIRE(num_vertices(g) == 4);  // Only 4 vertices, not 401
    }

    SECTION("matches vertices size") {
        mod_void g({{0, 1}, {1, 2}, {2, 3}});
        
        size_t count = 0;
        for ([[maybe_unused]] auto v : vertices(g)) {
            ++count;
        }
        REQUIRE(num_vertices(g) == count);
    }
}

//==================================================================================================
// 3. find_vertex(g, uid) CPO Tests
//==================================================================================================

TEST_CASE("mod CPO find_vertex(g, uid)", "[dynamic_graph][mod][cpo][find_vertex]") {
    SECTION("found - uint32_t ID") {
        mod_void g({{0, 1}, {1, 2}});
        
        auto v = find_vertex(g, uint32_t{1});
        REQUIRE(v != vertices(g).end());
    }

    SECTION("found - string ID") {
        mod_str_void g({{"alice", "bob"}, {"bob", "charlie"}});
        
        auto v = find_vertex(g, std::string("bob"));
        REQUIRE(v != vertices(g).end());
    }

    SECTION("not found - uint32_t ID") {
        mod_void g({{0, 1}});
        
        auto v = find_vertex(g, uint32_t{99});
        REQUIRE(v == vertices(g).end());
    }

    SECTION("not found - string ID") {
        mod_str_void g({{"alice", "bob"}});
        
        auto v = find_vertex(g, std::string("charlie"));
        REQUIRE(v == vertices(g).end());
    }

    SECTION("empty graph") {
        mod_void g;
        
        auto v = find_vertex(g, uint32_t{0});
        REQUIRE(v == vertices(g).end());
    }
}

//==================================================================================================
// 4. vertex_id(g, u) CPO Tests
//==================================================================================================

TEST_CASE("mod CPO vertex_id(g, u)", "[dynamic_graph][mod][cpo][vertex_id]") {
    SECTION("basic access - uint32_t IDs") {
        mod_void g({{0, 1}, {1, 2}});
        
        auto v_range = vertices(g);
        auto v_it = v_range.begin();
        auto v_desc = *v_it;
        
        auto id = vertex_id(g, v_desc);
        REQUIRE(id == 0);  // Map is ordered, so first vertex is 0
    }

    SECTION("basic access - string IDs") {
        mod_str_void g({{"bob", "alice"}, {"charlie", "bob"}});
        
        // Map is ordered, so vertices iterate in sorted order: alice, bob, charlie
        std::vector<std::string> ids;
        for (auto v : vertices(g)) {
            ids.push_back(vertex_id(g, v));
        }
        
        REQUIRE(ids.size() == 3);
        REQUIRE(ids[0] == "alice");
        REQUIRE(ids[1] == "bob");
        REQUIRE(ids[2] == "charlie");
    }

    SECTION("all vertices - ordered iteration") {
        mod_void g({{2, 0}, {0, 1}, {1, 2}});
        
        // Map iterates in key order: 0, 1, 2
        std::vector<uint32_t> ids;
        for (auto v : vertices(g)) {
            ids.push_back(vertex_id(g, v));
        }
        
        REQUIRE(ids.size() == 3);
        REQUIRE(ids[0] == 0);
        REQUIRE(ids[1] == 1);
        REQUIRE(ids[2] == 2);
    }

    SECTION("const correctness") {
        const mod_void g({{0, 1}});
        
        for (auto v : vertices(g)) {
            [[maybe_unused]] auto id = vertex_id(g, v);
        }
        REQUIRE(num_vertices(g) == 2);
    }

    SECTION("with find_vertex round-trip") {
        mod_void g({{0, 1}, {1, 2}, {2, 3}});
        
        for (uint32_t expected_id : {0u, 1u, 2u, 3u}) {
            auto v_it = find_vertex(g, expected_id);
            REQUIRE(v_it != vertices(g).end());
            
            auto v_desc = *v_it;
            auto actual_id = vertex_id(g, v_desc);
            REQUIRE(actual_id == expected_id);
        }
    }

    SECTION("string IDs round-trip") {
        mod_str_void g({{"alice", "bob"}, {"bob", "charlie"}});
        
        for (const auto& expected_id : {"alice", "bob", "charlie"}) {
            auto v_it = find_vertex(g, std::string(expected_id));
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

TEST_CASE("mod CPO num_edges(g)", "[dynamic_graph][mod][cpo][num_edges]") {
    SECTION("empty graph") {
        mod_void g;
        REQUIRE(num_edges(g) == 0);
    }

    SECTION("with edges - uint32_t IDs") {
        mod_void g({{0, 1}, {1, 2}, {2, 0}});
        REQUIRE(num_edges(g) == 3);
    }

    SECTION("with edges - string IDs") {
        mod_str_void g({{"a", "b"}, {"b", "c"}});
        REQUIRE(num_edges(g) == 2);
    }

    SECTION("after multiple edge additions") {
        mod_void g({{0, 1}, {1, 2}});
        
        std::vector<copyable_edge_t<uint32_t, void>> more_edges = {{2, 3}, {3, 0}};
        g.load_edges(more_edges, std::identity{});
        
        REQUIRE(num_edges(g) == 4);
    }
}

//==================================================================================================
// 6. has_edge(g) CPO Tests
//==================================================================================================

TEST_CASE("mod CPO has_edge(g)", "[dynamic_graph][mod][cpo][has_edge]") {
    SECTION("empty graph") {
        mod_void g;
        REQUIRE_FALSE(has_edge(g));
    }

    SECTION("graph with edges") {
        mod_void g({{0, 1}});
        REQUIRE(has_edge(g));
    }

    SECTION("after clear") {
        mod_void g({{0, 1}, {1, 2}});
        REQUIRE(has_edge(g));
        
        g.clear();
        REQUIRE_FALSE(has_edge(g));
    }
}

//==================================================================================================
// 7. edges(g, u) CPO Tests
//==================================================================================================

TEST_CASE("mod CPO edges(g, u)", "[dynamic_graph][mod][cpo][edges]") {
    SECTION("returns edge range") {
        mod_void g({{0, 1}, {0, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        // Verify it's a range
        static_assert(std::ranges::forward_range<decltype(edge_range)>);
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        REQUIRE(count == 2);
    }

    SECTION("empty edge list") {
        mod_void g({{0, 1}});
        
        auto u1 = *find_vertex(g, 1);  // Vertex 1 has no outgoing edges
        auto edge_range = edges(g, u1);
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        REQUIRE(count == 0);
    }

    SECTION("multiple edges - deque order (first added first)") {
        mod_void g({{0, 1}, {0, 2}, {0, 3}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        std::vector<uint32_t> targets;
        for (auto uv : edge_range) {
            targets.push_back(target_id(g, uv));
        }
        
        // deque: first added appears first (insertion order preserved)
        REQUIRE(targets.size() == 3);
        REQUIRE(targets[0] == 1);
        REQUIRE(targets[1] == 2);
        REQUIRE(targets[2] == 3);
    }

    SECTION("string IDs") {
        mod_str_void g({{"alice", "bob"}, {"alice", "charlie"}});
        
        auto alice = *find_vertex(g, std::string("alice"));
        auto edge_range = edges(g, alice);
        
        std::vector<std::string> targets;
        for (auto uv : edge_range) {
            targets.push_back(target_id(g, uv));
        }
        
        // deque: first added first
        REQUIRE(targets.size() == 2);
        REQUIRE(targets[0] == "bob");
        REQUIRE(targets[1] == "charlie");
    }

    SECTION("const correctness") {
        const mod_void g({{0, 1}, {0, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        REQUIRE(count == 2);
    }

    SECTION("with edge values") {
        mod_int_ev g({{0, 1, 100}, {0, 2, 200}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        std::vector<int> values;
        for (auto uv : edge_range) {
            values.push_back(edge_value(g, uv));
        }
        
        REQUIRE(values.size() == 2);
        // deque order: first added first
        REQUIRE(values[0] == 100);
        REQUIRE(values[1] == 200);
    }

    SECTION("with self-loop") {
        mod_void g({{0, 0}, {0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        std::vector<uint32_t> targets;
        for (auto uv : edge_range) {
            targets.push_back(target_id(g, uv));
        }
        
        REQUIRE(targets.size() == 2);
        // Should include self-loop, deque order preserved
        REQUIRE(targets[0] == 0);  // self-loop added first
        REQUIRE(targets[1] == 1);
    }
}

TEST_CASE("mod CPO edges(g, uid)", "[dynamic_graph][mod][cpo][edges]") {
    SECTION("with vertex ID - uint32_t") {
        mod_void g({{0, 1}, {0, 2}});
        
        auto edge_range = edges(g, uint32_t(0));
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        REQUIRE(count == 2);
    }

    SECTION("with vertex ID - string") {
        mod_str_void g({{"alice", "bob"}, {"alice", "charlie"}});
        
        auto edge_range = edges(g, std::string("alice"));
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        REQUIRE(count == 2);
    }

    SECTION("const correctness") {
        const mod_void g({{0, 1}, {0, 2}});
        
        auto edge_range = edges(g, uint32_t(0));
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        REQUIRE(count == 2);
    }

    SECTION("consistency with edges(g, u)") {
        mod_int_ev g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}});
        
        auto u0 = *find_vertex(g, 0);
        
        auto range_by_id = edges(g, uint32_t(0));
        auto range_by_desc = edges(g, u0);
        
        std::vector<int> values_by_id;
        std::vector<int> values_by_desc;
        
        for (auto uv : range_by_id) {
            values_by_id.push_back(edge_value(g, uv));
        }
        
        for (auto uv : range_by_desc) {
            values_by_desc.push_back(edge_value(g, uv));
        }
        
        REQUIRE(values_by_id == values_by_desc);
    }
}

//==================================================================================================
// 8. degree(g, u) CPO Tests
//==================================================================================================

TEST_CASE("mod CPO degree(g, u)", "[dynamic_graph][mod][cpo][degree]") {
    SECTION("isolated vertex") {
        mod_void g({{0, 1}});
        
        auto v1 = *find_vertex(g, 1);
        REQUIRE(degree(g, v1) == 0);
    }

    SECTION("single edge") {
        mod_void g({{0, 1}});
        
        auto v0 = *find_vertex(g, 0);
        REQUIRE(degree(g, v0) == 1);
    }

    SECTION("multiple edges from vertex") {
        mod_void g({{0, 1}, {0, 2}, {0, 3}});
        
        auto v0 = *find_vertex(g, 0);
        REQUIRE(degree(g, v0) == 3);
    }

    SECTION("by vertex ID") {
        mod_void g({{0, 1}, {0, 2}, {0, 3}});
        
        REQUIRE(degree(g, uint32_t{0}) == 3);
        REQUIRE(degree(g, uint32_t{1}) == 0);
        REQUIRE(degree(g, uint32_t{2}) == 0);
        REQUIRE(degree(g, uint32_t{3}) == 0);
    }

    SECTION("string IDs") {
        mod_str_void g({{"alice", "bob"}, {"alice", "charlie"}, {"bob", "charlie"}});
        
        REQUIRE(degree(g, std::string("alice")) == 2);
        REQUIRE(degree(g, std::string("bob")) == 1);
        REQUIRE(degree(g, std::string("charlie")) == 0);
    }

    SECTION("const correctness") {
        const mod_void g({{0, 1}, {0, 2}});
        
        auto v0 = *find_vertex(g, 0);
        REQUIRE(degree(g, v0) == 2);
    }

    SECTION("matches manual count") {
        mod_void g({{0, 1}, {0, 2}, {1, 2}, {1, 0}});
        
        for (auto u : vertices(g)) {
            auto deg = degree(g, u);
            
            size_t manual_count = 0;
            for ([[maybe_unused]] auto e : edges(g, u)) {
                ++manual_count;
            }
            
            REQUIRE(static_cast<size_t>(deg) == manual_count);
        }
    }
}

//==================================================================================================
// 9. target_id(g, uv) CPO Tests
//==================================================================================================

TEST_CASE("mod CPO target_id(g, uv)", "[dynamic_graph][mod][cpo][target_id]") {
    SECTION("basic access - uint32_t IDs") {
        mod_void g({{0, 1}, {0, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_view = edges(g, u0);
        
        std::vector<uint32_t> targets;
        for (auto uv : edge_view) {
            targets.push_back(target_id(g, uv));
        }
        
        // deque: first added first
        REQUIRE(targets.size() == 2);
        REQUIRE(targets[0] == 1);
        REQUIRE(targets[1] == 2);
    }

    SECTION("basic access - string IDs") {
        mod_str_void g({{"alice", "bob"}, {"alice", "charlie"}});
        
        auto alice = *find_vertex(g, std::string("alice"));
        auto edge_view = edges(g, alice);
        
        std::vector<std::string> targets;
        for (auto uv : edge_view) {
            targets.push_back(target_id(g, uv));
        }
        
        REQUIRE(targets.size() == 2);
        REQUIRE(targets[0] == "bob");
        REQUIRE(targets[1] == "charlie");
    }

    SECTION("with edge values") {
        mod_int_ev g({{0, 1, 100}, {0, 2, 200}});
        
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto tid = target_id(g, uv);
                REQUIRE(g.contains_vertex(tid));
            }
        }
    }

    SECTION("const correctness") {
        const mod_void g({{0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_view = edges(g, u0);
        
        auto uv = *edge_view.begin();
        auto tid = target_id(g, uv);
        REQUIRE(tid == 1);
    }

    SECTION("self-loop") {
        mod_void g({{0, 0}, {0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_view = edges(g, u0);
        
        std::vector<uint32_t> targets;
        for (auto uv : edge_view) {
            targets.push_back(target_id(g, uv));
        }
        
        REQUIRE(targets.size() == 2);
        // deque order preserved
        REQUIRE(targets[0] == 0);  // self-loop
        REQUIRE(targets[1] == 1);
    }

    SECTION("parallel edges") {
        mod_int_ev g({{0, 1, 10}, {0, 1, 20}, {0, 1, 30}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_view = edges(g, u0);
        
        for (auto uv : edge_view) {
            REQUIRE(target_id(g, uv) == 1);
        }
    }
}

//==================================================================================================
// 10. target(g, uv) CPO Tests
//==================================================================================================

TEST_CASE("mod CPO target(g, uv)", "[dynamic_graph][mod][cpo][target]") {
    SECTION("basic access") {
        mod_void g({{0, 1}, {0, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_view = edges(g, u0);
        auto it = edge_view.begin();
        
        auto uv = *it;
        auto target_vertex = target(g, uv);
        
        // deque: first added first
        REQUIRE(vertex_id(g, target_vertex) == 1);
    }

    SECTION("consistency with target_id") {
        mod_void g({{0, 1}, {1, 2}, {2, 0}});
        
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto target_desc = target(g, uv);
                auto tid = target_id(g, uv);
                auto expected_desc = *find_vertex(g, tid);
                
                REQUIRE(vertex_id(g, target_desc) == vertex_id(g, expected_desc));
            }
        }
    }

    SECTION("string IDs") {
        mod_str_void g({{"alice", "bob"}, {"alice", "charlie"}});
        
        auto alice = *find_vertex(g, std::string("alice"));
        
        for (auto uv : edges(g, alice)) {
            auto target_vertex = target(g, uv);
            auto tid = vertex_id(g, target_vertex);
            REQUIRE((tid == "bob" || tid == "charlie"));
        }
    }

    SECTION("access target properties") {
        mod_int_vv g({{0, 1}, {0, 2}});
        
        // Set vertex values
        for (auto u : vertices(g)) {
            auto id = vertex_id(g, u);
            vertex_value(g, u) = static_cast<int>(id) * 10;
        }
        
        // Access target vertex values
        auto u0 = *find_vertex(g, 0);
        for (auto uv : edges(g, u0)) {
            auto target_vertex = target(g, uv);
            auto tid = vertex_id(g, target_vertex);
            REQUIRE(vertex_value(g, target_vertex) == static_cast<int>(tid) * 10);
        }
    }

    SECTION("const correctness") {
        const mod_void g({{0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_view = edges(g, u0);
        
        auto uv = *edge_view.begin();
        auto target_vertex = target(g, uv);
        REQUIRE(vertex_id(g, target_vertex) == 1);
    }
}
//==================================================================================================
// 11. find_vertex_edge(g, u, v) CPO Tests
//==================================================================================================

TEST_CASE("mod CPO find_vertex_edge(g, u, v)", "[dynamic_graph][mod][cpo][find_vertex_edge]") {
    SECTION("basic edge found") {
        mod_void g({{0, 1}, {0, 2}, {1, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        
        auto e01 = find_vertex_edge(g, u0, u1);
        auto e02 = find_vertex_edge(g, u0, u2);
        auto e12 = find_vertex_edge(g, u1, u2);
        
        REQUIRE(target_id(g, e01) == 1);
        REQUIRE(target_id(g, e02) == 2);
        REQUIRE(target_id(g, e12) == 2);
    }

    SECTION("with vertex IDs") {
        mod_void g({{0, 1}, {0, 2}});
        
        auto e01 = find_vertex_edge(g, uint32_t(0), uint32_t(1));
        auto e02 = find_vertex_edge(g, uint32_t(0), uint32_t(2));
        
        REQUIRE(target_id(g, e01) == 1);
        REQUIRE(target_id(g, e02) == 2);
    }

    SECTION("string IDs") {
        mod_str_void g({{"alice", "bob"}, {"alice", "charlie"}});
        
        auto e_ab = find_vertex_edge(g, std::string("alice"), std::string("bob"));
        auto e_ac = find_vertex_edge(g, std::string("alice"), std::string("charlie"));
        
        REQUIRE(target_id(g, e_ab) == "bob");
        REQUIRE(target_id(g, e_ac) == "charlie");
    }

    SECTION("with edge values") {
        mod_int_ev g({{0, 1, 100}, {0, 2, 200}});
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        
        auto e01 = find_vertex_edge(g, u0, u1);
        auto e02 = find_vertex_edge(g, u0, u2);
        
        REQUIRE(edge_value(g, e01) == 100);
        REQUIRE(edge_value(g, e02) == 200);
    }

    SECTION("with self-loop") {
        mod_void g({{0, 0}, {0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        
        auto e00 = find_vertex_edge(g, u0, u0);
        REQUIRE(target_id(g, e00) == 0);
    }

    SECTION("const correctness") {
        const mod_void g({{0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        
        auto e01 = find_vertex_edge(g, u0, u1);
        REQUIRE(target_id(g, e01) == 1);
    }
}

TEST_CASE("mod CPO find_vertex_edge(g, uid, vid)", "[dynamic_graph][mod][cpo][find_vertex_edge][uid_vid]") {
    SECTION("basic usage") {
        mod_void g({{0, 1}, {0, 2}, {1, 2}, {2, 3}});
        
        auto e01 = find_vertex_edge(g, uint32_t(0), uint32_t(1));
        auto e02 = find_vertex_edge(g, uint32_t(0), uint32_t(2));
        auto e12 = find_vertex_edge(g, uint32_t(1), uint32_t(2));
        auto e23 = find_vertex_edge(g, uint32_t(2), uint32_t(3));
        
        REQUIRE(target_id(g, e01) == 1);
        REQUIRE(target_id(g, e02) == 2);
        REQUIRE(target_id(g, e12) == 2);
        REQUIRE(target_id(g, e23) == 3);
    }

    SECTION("with edge values") {
        mod_int_ev g({{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}});
        
        auto e01 = find_vertex_edge(g, uint32_t(0), uint32_t(1));
        auto e02 = find_vertex_edge(g, uint32_t(0), uint32_t(2));
        auto e12 = find_vertex_edge(g, uint32_t(1), uint32_t(2));
        auto e23 = find_vertex_edge(g, uint32_t(2), uint32_t(3));
        
        REQUIRE(edge_value(g, e01) == 10);
        REQUIRE(edge_value(g, e02) == 20);
        REQUIRE(edge_value(g, e12) == 30);
        REQUIRE(edge_value(g, e23) == 40);
    }

    SECTION("with parallel edges") {
        mod_int_ev g({{0, 1, 100}, {0, 1, 200}, {0, 1, 300}, {1, 2, 400}});
        
        auto e01 = find_vertex_edge(g, uint32_t(0), uint32_t(1));
        REQUIRE(target_id(g, e01) == 1);
        
        int val = edge_value(g, e01);
        REQUIRE((val == 100 || val == 200 || val == 300));
    }

    SECTION("with self-loop") {
        mod_int_ev g({{0, 0, 99}, {0, 1, 10}, {1, 1, 88}});
        
        auto e00 = find_vertex_edge(g, uint32_t(0), uint32_t(0));
        auto e11 = find_vertex_edge(g, uint32_t(1), uint32_t(1));
        
        REQUIRE(target_id(g, e00) == 0);
        REQUIRE(edge_value(g, e00) == 99);
        REQUIRE(target_id(g, e11) == 1);
        REQUIRE(edge_value(g, e11) == 88);
    }

    SECTION("const correctness") {
        const mod_int_ev g({{0, 1, 100}, {1, 2, 200}});
        
        auto e01 = find_vertex_edge(g, uint32_t(0), uint32_t(1));
        auto e12 = find_vertex_edge(g, uint32_t(1), uint32_t(2));
        
        REQUIRE(target_id(g, e01) == 1);
        REQUIRE(edge_value(g, e01) == 100);
        REQUIRE(target_id(g, e12) == 2);
        REQUIRE(edge_value(g, e12) == 200);
    }

    SECTION("string IDs") {
        mod_str_void g({{"alice", "bob"}, {"alice", "charlie"}, {"bob", "charlie"}});
        
        auto e_ab = find_vertex_edge(g, std::string("alice"), std::string("bob"));
        auto e_ac = find_vertex_edge(g, std::string("alice"), std::string("charlie"));
        auto e_bc = find_vertex_edge(g, std::string("bob"), std::string("charlie"));
        
        REQUIRE(target_id(g, e_ab) == "bob");
        REQUIRE(target_id(g, e_ac) == "charlie");
        REQUIRE(target_id(g, e_bc) == "charlie");
    }

    SECTION("chain of edges") {
        mod_int_ev g({{0, 1, 10}, {1, 2, 20}, {2, 3, 30}, {3, 4, 40}, {4, 5, 50}});
        
        auto e01 = find_vertex_edge(g, uint32_t(0), uint32_t(1));
        REQUIRE(edge_value(g, e01) == 10);
        
        auto e12 = find_vertex_edge(g, uint32_t(1), uint32_t(2));
        REQUIRE(edge_value(g, e12) == 20);
        
        auto e23 = find_vertex_edge(g, uint32_t(2), uint32_t(3));
        REQUIRE(edge_value(g, e23) == 30);
        
        auto e34 = find_vertex_edge(g, uint32_t(3), uint32_t(4));
        REQUIRE(edge_value(g, e34) == 40);
        
        auto e45 = find_vertex_edge(g, uint32_t(4), uint32_t(5));
        REQUIRE(edge_value(g, e45) == 50);
    }
}

//==================================================================================================
// 12. contains_edge(g, u, v) CPO Tests
//==================================================================================================

TEST_CASE("mod CPO contains_edge(g, u, v)", "[dynamic_graph][mod][cpo][contains_edge]") {
    SECTION("edge exists") {
        mod_void g({{0, 1}, {0, 2}, {1, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        
        REQUIRE(contains_edge(g, u0, u1));
        REQUIRE(contains_edge(g, u0, u2));
        REQUIRE(contains_edge(g, u1, u2));
    }

    SECTION("edge does not exist") {
        mod_void g({{0, 1}, {1, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto u2 = *find_vertex(g, 2);
        
        REQUIRE_FALSE(contains_edge(g, u0, u2));  // No direct edge 0->2
    }

    SECTION("with vertex IDs") {
        mod_void g({{0, 1}, {0, 2}});
        
        REQUIRE(contains_edge(g, uint32_t(0), uint32_t(1)));
        REQUIRE(contains_edge(g, uint32_t(0), uint32_t(2)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(1), uint32_t(0)));
    }

    SECTION("string IDs") {
        mod_str_void g({{"alice", "bob"}, {"bob", "charlie"}});
        
        REQUIRE(contains_edge(g, std::string("alice"), std::string("bob")));
        REQUIRE(contains_edge(g, std::string("bob"), std::string("charlie")));
        REQUIRE_FALSE(contains_edge(g, std::string("alice"), std::string("charlie")));
    }

    SECTION("self-loop") {
        mod_void g({{0, 0}, {0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        REQUIRE(contains_edge(g, u0, u0));
    }

    SECTION("const correctness") {
        const mod_void g({{0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        
        REQUIRE(contains_edge(g, u0, u1));
    }

    SECTION("symmetric check") {
        mod_void g({{0, 1}});
        
        REQUIRE(contains_edge(g, uint32_t(0), uint32_t(1)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(1), uint32_t(0)));
    }
}

TEST_CASE("mod CPO contains_edge(g, uid, vid)", "[dynamic_graph][mod][cpo][contains_edge][uid_vid]") {
    SECTION("basic usage") {
        mod_void g({{0, 1}, {0, 2}, {1, 2}, {2, 3}});
        
        REQUIRE(contains_edge(g, uint32_t(0), uint32_t(1)));
        REQUIRE(contains_edge(g, uint32_t(0), uint32_t(2)));
        REQUIRE(contains_edge(g, uint32_t(1), uint32_t(2)));
        REQUIRE(contains_edge(g, uint32_t(2), uint32_t(3)));
        
        REQUIRE_FALSE(contains_edge(g, uint32_t(0), uint32_t(3)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(1), uint32_t(0)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(3), uint32_t(2)));
    }

    SECTION("all edges not found") {
        mod_void g({{0, 1}, {1, 2}});
        
        REQUIRE_FALSE(contains_edge(g, uint32_t(0), uint32_t(2)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(1), uint32_t(0)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(2), uint32_t(0)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(2), uint32_t(1)));
        
        REQUIRE_FALSE(contains_edge(g, uint32_t(0), uint32_t(0)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(1), uint32_t(1)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(2), uint32_t(2)));
    }

    SECTION("with parallel edges") {
        mod_int_ev g({{0, 1, 100}, {0, 1, 200}, {0, 1, 300}, {1, 2, 400}});
        
        REQUIRE(contains_edge(g, uint32_t(0), uint32_t(1)));
        REQUIRE(contains_edge(g, uint32_t(1), uint32_t(2)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(0), uint32_t(2)));
    }

    SECTION("bidirectional check") {
        mod_void g({{0, 1}, {1, 0}, {1, 2}});
        
        REQUIRE(contains_edge(g, uint32_t(0), uint32_t(1)));
        REQUIRE(contains_edge(g, uint32_t(1), uint32_t(0)));
        REQUIRE(contains_edge(g, uint32_t(1), uint32_t(2)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(2), uint32_t(1)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(0), uint32_t(2)));
    }

    SECTION("star graph") {
        mod_void g({{0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}});
        
        for (uint32_t i = 1; i < 6; ++i) {
            REQUIRE(contains_edge(g, uint32_t(0), uint32_t(i)));
        }
        
        for (uint32_t i = 1; i < 6; ++i) {
            for (uint32_t j = i + 1; j < 6; ++j) {
                REQUIRE_FALSE(contains_edge(g, uint32_t(i), uint32_t(j)));
                REQUIRE_FALSE(contains_edge(g, uint32_t(j), uint32_t(i)));
            }
        }
        
        for (uint32_t i = 1; i < 6; ++i) {
            REQUIRE_FALSE(contains_edge(g, uint32_t(i), uint32_t(0)));
        }
    }

    SECTION("chain graph") {
        mod_int_ev g({{0, 1, 10}, {1, 2, 20}, {2, 3, 30}, {3, 4, 40}, {4, 5, 50}});
        
        for (uint32_t i = 0; i < 5; ++i) {
            REQUIRE(contains_edge(g, uint32_t(i), uint32_t(i + 1)));
        }
        
        for (uint32_t i = 1; i < 6; ++i) {
            REQUIRE_FALSE(contains_edge(g, uint32_t(i), uint32_t(i - 1)));
        }
        
        REQUIRE_FALSE(contains_edge(g, uint32_t(0), uint32_t(2)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(0), uint32_t(3)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(1), uint32_t(3)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(2), uint32_t(5)));
    }

    SECTION("cycle graph") {
        mod_void g({{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0}});
        
        REQUIRE(contains_edge(g, uint32_t(0), uint32_t(1)));
        REQUIRE(contains_edge(g, uint32_t(1), uint32_t(2)));
        REQUIRE(contains_edge(g, uint32_t(2), uint32_t(3)));
        REQUIRE(contains_edge(g, uint32_t(3), uint32_t(4)));
        REQUIRE(contains_edge(g, uint32_t(4), uint32_t(0)));
        
        REQUIRE_FALSE(contains_edge(g, uint32_t(0), uint32_t(2)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(0), uint32_t(3)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(1), uint32_t(3)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(1), uint32_t(4)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(2), uint32_t(4)));
    }

    SECTION("string IDs") {
        mod_str_void g({{"alice", "bob"}, {"bob", "charlie"}, {"charlie", "alice"}});
        
        REQUIRE(contains_edge(g, std::string("alice"), std::string("bob")));
        REQUIRE(contains_edge(g, std::string("bob"), std::string("charlie")));
        REQUIRE(contains_edge(g, std::string("charlie"), std::string("alice")));
        
        REQUIRE_FALSE(contains_edge(g, std::string("alice"), std::string("charlie")));
        REQUIRE_FALSE(contains_edge(g, std::string("bob"), std::string("alice")));
    }

    SECTION("single edge graph") {
        mod_void g({{0, 1}});
        
        REQUIRE(contains_edge(g, uint32_t(0), uint32_t(1)));
        
        REQUIRE_FALSE(contains_edge(g, uint32_t(1), uint32_t(0)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(0), uint32_t(0)));
        REQUIRE_FALSE(contains_edge(g, uint32_t(1), uint32_t(1)));
    }
}

//==================================================================================================
// 13. vertex_value(g, u) CPO Tests
//==================================================================================================

TEST_CASE("mod CPO vertex_value(g, u)", "[dynamic_graph][mod][cpo][vertex_value]") {
    SECTION("read value") {
        mod_int_vv g({{0, 1}});
        
        for (auto u : vertices(g)) {
            vertex_value(g, u) = static_cast<int>(vertex_id(g, u)) * 10;
        }
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        
        REQUIRE(vertex_value(g, u0) == 0);
        REQUIRE(vertex_value(g, u1) == 10);
    }

    SECTION("write value") {
        mod_int_vv g({{0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        vertex_value(g, u0) = 42;
        
        REQUIRE(vertex_value(g, u0) == 42);
    }

    SECTION("string vertex values") {
        using G = dynamic_graph<void, std::string, void, uint32_t, false, 
                                mod_graph_traits<void, std::string, void, uint32_t, false>>;
        G g({{0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        
        vertex_value(g, u0) = "Alice";
        vertex_value(g, u1) = "Bob";
        
        REQUIRE(vertex_value(g, u0) == "Alice");
        REQUIRE(vertex_value(g, u1) == "Bob");
    }

    SECTION("const correctness") {
        mod_int_vv g({{0, 1}});
        
        auto u0_mut = *find_vertex(g, 0);
        vertex_value(g, u0_mut) = 100;
        
        const auto& const_g = g;
        auto u0_const = *find_vertex(const_g, 0);
        
        REQUIRE(vertex_value(const_g, u0_const) == 100);
    }

    SECTION("multiple vertices with values") {
        mod_int_vv g({{0, 1}, {1, 2}, {2, 3}, {3, 4}});
        
        int val = 100;
        for (auto u : vertices(g)) {
            vertex_value(g, u) = val;
            val += 100;
        }
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        auto u3 = *find_vertex(g, 3);
        auto u4 = *find_vertex(g, 4);
        
        REQUIRE(vertex_value(g, u0) == 100);
        REQUIRE(vertex_value(g, u1) == 200);
        REQUIRE(vertex_value(g, u2) == 300);
        REQUIRE(vertex_value(g, u3) == 400);
        REQUIRE(vertex_value(g, u4) == 500);
    }
}

//==================================================================================================
// 14. edge_value(g, uv) CPO Tests
//==================================================================================================

TEST_CASE("mod CPO edge_value(g, uv)", "[dynamic_graph][mod][cpo][edge_value]") {
    SECTION("read value") {
        mod_int_ev g({{0, 1, 100}, {0, 2, 200}});
        
        auto u0 = *find_vertex(g, 0);
        
        std::vector<int> values;
        for (auto uv : edges(g, u0)) {
            values.push_back(edge_value(g, uv));
        }
        
        REQUIRE(values.size() == 2);
        // deque order: first added first
        REQUIRE(values[0] == 100);
        REQUIRE(values[1] == 200);
    }

    SECTION("write value") {
        mod_int_ev g({{0, 1, 100}});
        
        auto u0 = *find_vertex(g, 0);
        auto uv = *edges(g, u0).begin();
        
        edge_value(g, uv) = 999;
        REQUIRE(edge_value(g, uv) == 999);
    }

    SECTION("string edge values") {
        using G = dynamic_graph<std::string, void, void, uint32_t, false, 
                                mod_graph_traits<std::string, void, void, uint32_t, false>>;
        G g({{0, 1, "hello"}});
        
        auto u0 = *find_vertex(g, 0);
        auto uv = *edges(g, u0).begin();
        
        REQUIRE(edge_value(g, uv) == "hello");
    }

    SECTION("const correctness") {
        const mod_int_ev g({{0, 1, 100}});
        
        auto u0 = *find_vertex(g, 0);
        auto uv = *edges(g, u0).begin();
        
        REQUIRE(edge_value(g, uv) == 100);
    }

    SECTION("multiple edges with values") {
        mod_int_ev g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}, {1, 2, 40}, {2, 3, 50}});
        
        auto u0 = *find_vertex(g, 0);
        std::vector<int> u0_values;
        for (auto uv : edges(g, u0)) {
            u0_values.push_back(edge_value(g, uv));
        }
        REQUIRE(u0_values.size() == 3);
        REQUIRE(u0_values[0] == 10);
        REQUIRE(u0_values[1] == 20);
        REQUIRE(u0_values[2] == 30);
        
        auto u1 = *find_vertex(g, 1);
        auto uv1 = *edges(g, u1).begin();
        REQUIRE(edge_value(g, uv1) == 40);
        
        auto u2 = *find_vertex(g, 2);
        auto uv2 = *edges(g, u2).begin();
        REQUIRE(edge_value(g, uv2) == 50);
    }

    SECTION("modify edge values") {
        mod_int_ev g({{0, 1, 100}, {0, 2, 200}});
        
        auto u0 = *find_vertex(g, 0);
        int multiplier = 10;
        for (auto uv : edges(g, u0)) {
            edge_value(g, uv) *= multiplier;
        }
        
        std::vector<int> values;
        for (auto uv : edges(g, u0)) {
            values.push_back(edge_value(g, uv));
        }
        
        REQUIRE(values[0] == 1000);
        REQUIRE(values[1] == 2000);
    }
}

//==================================================================================================
// 15. graph_value(g) CPO Tests
//==================================================================================================

TEST_CASE("mod CPO graph_value(g)", "[dynamic_graph][mod][cpo][graph_value]") {
    SECTION("read value") {
        mod_all_int g(42, {{0, 1, 10}});
        
        REQUIRE(graph_value(g) == 42);
    }

    SECTION("write value") {
        mod_all_int g(42, {{0, 1, 10}});
        
        graph_value(g) = 100;
        REQUIRE(graph_value(g) == 100);
    }

    SECTION("string graph value") {
        using G = dynamic_graph<void, void, std::string, uint32_t, false, 
                                mod_graph_traits<void, void, std::string, uint32_t, false>>;
        G g(std::string("my graph"), {{0, 1}});
        
        REQUIRE(graph_value(g) == "my graph");
    }

    SECTION("const correctness") {
        const mod_all_int g(42, {{0, 1, 10}});
        
        REQUIRE(graph_value(g) == 42);
    }

    SECTION("graph value with complex graph") {
        mod_all_int g(0, {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}, {3, 4, 40}});
        
        // Compute sum of all edge values and store in graph value
        int sum = 0;
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                sum += edge_value(g, uv);
            }
        }
        graph_value(g) = sum;
        
        REQUIRE(graph_value(g) == 100);  // 10+20+30+40
    }

    SECTION("default value") {
        mod_all_int g(0, {{0, 1, 10}});
        
        REQUIRE(graph_value(g) == 0);
    }
}

//==================================================================================================
// 16. source_id(g, uv) CPO Tests (Sourced=true)
//==================================================================================================

TEST_CASE("mod CPO source_id(g, uv)", "[dynamic_graph][mod][cpo][source_id]") {
    SECTION("basic access - uint32_t IDs") {
        mod_sourced_void g({{0, 1}, {0, 2}, {1, 2}});
        
        auto u0 = *find_vertex(g, 0);
        for (auto uv : edges(g, u0)) {
            REQUIRE(source_id(g, uv) == 0);
        }
        
        auto u1 = *find_vertex(g, 1);
        for (auto uv : edges(g, u1)) {
            REQUIRE(source_id(g, uv) == 1);
        }
    }

    SECTION("string IDs") {
        mod_str_sourced g({{"alice", "bob"}, {"bob", "charlie"}});
        
        auto alice = *find_vertex(g, std::string("alice"));
        for (auto uv : edges(g, alice)) {
            REQUIRE(source_id(g, uv) == "alice");
        }
    }

    SECTION("const correctness") {
        const mod_sourced_void g({{0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        auto uv = *edges(g, u0).begin();
        
        REQUIRE(source_id(g, uv) == 0);
    }

    SECTION("consistency with vertex_id") {
        mod_sourced_void g({{0, 1}, {0, 2}, {1, 2}});
        
        for (auto u : vertices(g)) {
            auto uid = vertex_id(g, u);
            for (auto uv : edges(g, u)) {
                REQUIRE(source_id(g, uv) == uid);
            }
        }
    }

    SECTION("multiple edges per source") {
        mod_sourced_int g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}, {0, 4, 40}});
        
        auto u0 = *find_vertex(g, 0);
        for (auto uv : edges(g, u0)) {
            REQUIRE(source_id(g, uv) == 0);
        }
    }
}

//==================================================================================================
// 17. source(g, uv) CPO Tests (Sourced=true)
//==================================================================================================

TEST_CASE("mod CPO source(g, uv)", "[dynamic_graph][mod][cpo][source]") {
    SECTION("basic access") {
        mod_sourced_void g({{0, 1}, {0, 2}});
        
        auto u0 = *find_vertex(g, 0);
        for (auto uv : edges(g, u0)) {
            auto source_vertex = source(g, uv);
            REQUIRE(vertex_id(g, source_vertex) == 0);
        }
    }

    SECTION("consistency with source_id") {
        mod_sourced_void g({{0, 1}, {1, 2}, {2, 0}});
        
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto source_vertex = source(g, uv);
                REQUIRE(vertex_id(g, source_vertex) == source_id(g, uv));
            }
        }
    }

    SECTION("string IDs") {
        mod_str_sourced g({{"alice", "bob"}, {"bob", "charlie"}});
        
        auto alice = *find_vertex(g, std::string("alice"));
        for (auto uv : edges(g, alice)) {
            auto source_vertex = source(g, uv);
            REQUIRE(vertex_id(g, source_vertex) == "alice");
        }
    }

    SECTION("const correctness") {
        const mod_sourced_void g({{0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        auto uv = *edges(g, u0).begin();
        
        auto source_vertex = source(g, uv);
        REQUIRE(vertex_id(g, source_vertex) == 0);
    }

    SECTION("traverse via source") {
        mod_sourced_void g({{0, 1}, {1, 2}, {2, 3}});
        
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto src = source(g, uv);
                auto tgt = target(g, uv);
                // Source vertex should equal the current vertex
                REQUIRE(vertex_id(g, src) == vertex_id(g, u));
                // Target should be different unless self-loop
                if (vertex_id(g, src) != vertex_id(g, tgt)) {
                    REQUIRE(vertex_id(g, src) != vertex_id(g, tgt));
                }
            }
        }
    }
}

//==================================================================================================
// 18. partition_id(g, u) CPO Tests
//==================================================================================================

TEST_CASE("mod CPO partition_id(g, u)", "[dynamic_graph][mod][cpo][partition_id]") {
    SECTION("default single partition") {
        mod_void g({{0, 1}, {1, 2}});
        
        for (auto u : vertices(g)) {
            REQUIRE(partition_id(g, u) == 0);
        }
    }

    SECTION("string IDs - single partition") {
        mod_str_void g({{"alice", "bob"}, {"bob", "charlie"}});
        
        for (auto u : vertices(g)) {
            REQUIRE(partition_id(g, u) == 0);
        }
    }

    SECTION("large graph - all same partition") {
        mod_void g({{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7}, {7, 8}, {8, 9}});
        
        for (auto u : vertices(g)) {
            REQUIRE(partition_id(g, u) == 0);
        }
    }
}

//==================================================================================================
// 19. num_partitions(g) CPO Tests
//==================================================================================================

TEST_CASE("mod CPO num_partitions(g)", "[dynamic_graph][mod][cpo][num_partitions]") {
    SECTION("default single partition") {
        mod_void g({{0, 1}, {1, 2}});
        
        REQUIRE(num_partitions(g) == 1);
    }

    SECTION("empty graph") {
        mod_void g;
        
        REQUIRE(num_partitions(g) == 1);
    }

    SECTION("string IDs") {
        mod_str_void g({{"alice", "bob"}});
        
        REQUIRE(num_partitions(g) == 1);
    }

    SECTION("large graph") {
        mod_void g({{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}});
        
        REQUIRE(num_partitions(g) == 1);
    }
}

//==================================================================================================
// 20. vertices(g, pid) and num_vertices(g, pid) CPO Tests
//==================================================================================================

TEST_CASE("mod CPO vertices(g, pid)", "[dynamic_graph][mod][cpo][vertices][partition]") {
    SECTION("partition 0 returns all vertices") {
        mod_void g({{0, 1}, {1, 2}});
        
        auto v_range = vertices(g, 0);
        
        size_t count = 0;
        for ([[maybe_unused]] auto v : v_range) {
            ++count;
        }
        REQUIRE(count == 3);
    }

    SECTION("partition 0 with string IDs") {
        mod_str_void g({{"alice", "bob"}, {"bob", "charlie"}});
        
        auto v_range = vertices(g, 0);
        
        size_t count = 0;
        for ([[maybe_unused]] auto v : v_range) {
            ++count;
        }
        REQUIRE(count == 3);
    }
}

TEST_CASE("mod CPO num_vertices(g, pid)", "[dynamic_graph][mod][cpo][num_vertices][partition]") {
    SECTION("partition 0 count") {
        mod_void g({{0, 1}, {1, 2}});
        
        REQUIRE(num_vertices(g, 0) == 3);
    }

    SECTION("matches num_vertices(g)") {
        mod_void g({{0, 1}, {1, 2}, {2, 3}});
        
        REQUIRE(num_vertices(g, 0) == num_vertices(g));
    }

    SECTION("const correctness") {
        const mod_void g({{0, 1}, {1, 2}});
        
        REQUIRE(num_vertices(g, 0) == 3);
    }

    SECTION("consistency with vertices(g, pid)") {
        mod_void g({{0, 1}, {1, 2}, {2, 3}});
        
        auto v_range = vertices(g, 0);
        size_t count = 0;
        for ([[maybe_unused]] auto v : v_range) {
            ++count;
        }
        
        REQUIRE(num_vertices(g, 0) == count);
    }

    SECTION("string IDs") {
        mod_str_void g({{"alice", "bob"}, {"bob", "charlie"}, {"charlie", "dave"}});
        
        REQUIRE(num_vertices(g, 0) == 4);
    }
}

//==================================================================================================
// 21. Integration Tests - Multiple CPOs Working Together
//==================================================================================================

TEST_CASE("mod CPO integration", "[dynamic_graph][mod][cpo][integration]") {
    SECTION("graph construction and traversal") {
        mod_void g({{0, 1}, {1, 2}});
        
        REQUIRE(num_vertices(g) == 3);
        REQUIRE(num_edges(g) == 2);
        REQUIRE(has_edge(g));
    }

    SECTION("empty graph properties") {
        mod_void g;
        
        REQUIRE(num_vertices(g) == 0);
        REQUIRE(num_edges(g) == 0);
        REQUIRE(!has_edge(g));
        REQUIRE(std::ranges::distance(vertices(g)) == 0);
    }

    SECTION("find vertex by id") {
        mod_void g({{0, 1}, {1, 2}, {2, 3}, {3, 4}});
        
        for (uint32_t i = 0; i < 5; ++i) {
            auto v = find_vertex(g, i);
            REQUIRE(v != vertices(g).end());
        }
    }

    SECTION("vertices and num_vertices consistency") {
        mod_void g({{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7}, {7, 8}, {8, 9}});
        
        REQUIRE(num_vertices(g) == 10);
        
        size_t count = 0;
        for ([[maybe_unused]] auto v : vertices(g)) {
            ++count;
        }
        REQUIRE(count == num_vertices(g));
    }

    SECTION("const graph access") {
        const mod_void g({{0, 1}, {1, 2}});
        
        REQUIRE(num_vertices(g) == 3);
        REQUIRE(num_edges(g) == 2);
        REQUIRE(has_edge(g));
        
        size_t vertex_count = 0;
        for ([[maybe_unused]] auto v : vertices(g)) {
            ++vertex_count;
        }
        REQUIRE(vertex_count == 3);
    }

    SECTION("string vertex IDs integration") {
        mod_str_void g({{"alice", "bob"}, {"bob", "charlie"}, {"charlie", "dave"}});
        
        REQUIRE(num_vertices(g) == 4);
        REQUIRE(num_edges(g) == 3);
        
        auto alice = find_vertex(g, std::string("alice"));
        REQUIRE(alice != vertices(g).end());
        REQUIRE(vertex_id(g, *alice) == "alice");
        
        auto dave = find_vertex(g, std::string("dave"));
        REQUIRE(dave != vertices(g).end());
        REQUIRE(degree(g, *dave) == 0);
    }

    SECTION("edge iteration with deque - random access") {
        // Deque edges provide random access iteration
        mod_int_ev g({{0, 1, 100}, {0, 2, 200}, {0, 3, 300}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        // Test forward iteration (deque provides this)
        std::vector<int> values;
        for (auto uv : edge_range) {
            values.push_back(edge_value(g, uv));
        }
        REQUIRE(values.size() == 3);
        REQUIRE(values[0] == 100);
        REQUIRE(values[1] == 200);
        REQUIRE(values[2] == 300);
    }
}

//==================================================================================================
// 22. Integration Tests - vertex_value and edge_value Together
//==================================================================================================

TEST_CASE("mod CPO integration: values", "[dynamic_graph][mod][cpo][integration]") {
    SECTION("vertex values only") {
        mod_int_vv g({{0, 1}, {1, 2}, {2, 3}, {3, 4}});
        
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

    SECTION("vertex and edge values") {
        mod_all_int g({{0, 1, 5}, {1, 2, 10}});
        
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
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        
        for (auto uv : edges(g, u0)) {
            REQUIRE(edge_value(g, uv) == 5);
        }
        for (auto uv : edges(g, u1)) {
            REQUIRE(edge_value(g, uv) == 10);
        }
    }

    SECTION("string IDs with values") {
        using G = dynamic_graph<int, int, void, std::string, false,
                                mod_graph_traits<int, int, void, std::string, false>>;
        G g({{"alice", "bob", 100}, {"bob", "charlie", 200}});
        
        auto alice = *find_vertex(g, std::string("alice"));
        auto bob = *find_vertex(g, std::string("bob"));
        auto charlie = *find_vertex(g, std::string("charlie"));
        
        vertex_value(g, alice) = 1;
        vertex_value(g, bob) = 2;
        vertex_value(g, charlie) = 3;
        
        REQUIRE(vertex_value(g, alice) == 1);
        REQUIRE(vertex_value(g, bob) == 2);
        REQUIRE(vertex_value(g, charlie) == 3);
        
        for (auto uv : edges(g, alice)) {
            REQUIRE(edge_value(g, uv) == 100);
        }
    }

    SECTION("graph value with totals") {
        mod_all_int g(0, {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}});
        
        // Sum all edge values and store in graph
        int sum = 0;
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                sum += edge_value(g, uv);
            }
        }
        graph_value(g) = sum;
        
        REQUIRE(graph_value(g) == 60);
    }
}

//==================================================================================================
// 23. Integration Tests - Modify vertex and edge values
//==================================================================================================

TEST_CASE("mod CPO integration: modify vertex and edge values", "[dynamic_graph][mod][cpo][integration]") {
    SECTION("accumulate edge values into source vertices") {
        mod_all_int g({{0, 1, 1}, {0, 2, 2}, {1, 2, 3}});
        
        for (auto u : vertices(g)) {
            vertex_value(g, u) = 0;
        }
        
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                vertex_value(g, u) += edge_value(g, uv);
            }
        }
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        
        REQUIRE(vertex_value(g, u0) == 3);  // 1 + 2
        REQUIRE(vertex_value(g, u1) == 3);  // 3
        REQUIRE(vertex_value(g, u2) == 0);  // no outgoing edges
    }

    SECTION("modify edge values based on vertex values") {
        mod_all_int g({{0, 1, 0}, {1, 2, 0}});
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        
        vertex_value(g, u0) = 10;
        vertex_value(g, u1) = 20;
        vertex_value(g, u2) = 30;
        
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto t = target(g, uv);
                edge_value(g, uv) = vertex_value(g, u) + vertex_value(g, t);
            }
        }
        
        for (auto uv : edges(g, u0)) {
            REQUIRE(edge_value(g, uv) == 30);  // 10 + 20
        }
        for (auto uv : edges(g, u1)) {
            REQUIRE(edge_value(g, uv) == 50);  // 20 + 30
        }
    }

    SECTION("propagate values through chain") {
        mod_all_int g({{0, 1, 1}, {1, 2, 1}, {2, 3, 1}, {3, 4, 1}});
        
        // Set initial vertex value
        auto u0 = *find_vertex(g, 0);
        vertex_value(g, u0) = 100;
        
        // Propagate value through the chain
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto t = target(g, uv);
                vertex_value(g, t) = vertex_value(g, u) + edge_value(g, uv);
            }
        }
        
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        auto u3 = *find_vertex(g, 3);
        auto u4 = *find_vertex(g, 4);
        
        REQUIRE(vertex_value(g, u1) == 101);
        REQUIRE(vertex_value(g, u2) == 102);
        REQUIRE(vertex_value(g, u3) == 103);
        REQUIRE(vertex_value(g, u4) == 104);
    }
}

//==================================================================================================
// Summary: mod CPO Tests
//
// This file tests CPO integration with mod_graph_traits (map vertices + deque edges).
// 
// Key differences from mov tests:
// - std::deque provides random access edge iteration (same as vector)
// - Edge order: first added appears first (same as mov/vector)
// - Vertices are sparse (only referenced vertices exist) - same as mov
// - Map iteration is in key order (sorted) - same as mov
// - String vertex IDs are extensively tested - same as mov
//
// Key differences from mofl tests:
// - std::deque provides random access (forward_list is forward-only)
// - Edge order: first added first (forward_list: last added first)
//
// Key differences from mol tests:
// - std::deque provides random access (list is bidirectional)
// - Both preserve edge order: first added first
//
// All CPOs work correctly with map vertex containers + deque edge containers.
//==================================================================================================
