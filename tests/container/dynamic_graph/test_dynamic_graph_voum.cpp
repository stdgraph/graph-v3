/**
 * @file test_dynamic_graph_voum.cpp
 * @brief Comprehensive tests for dynamic_graph with vector vertices + unordered_map edges
 *
 * Tests voum_graph_traits (vector vertices + unordered_map edges)
 *
 * Key characteristics of std::unordered_map edges:
 * - Automatic deduplication (only one edge per target vertex)
 * - Edges stored in unordered fashion (hash-bucket order, not sorted)
 * - O(1) average edge insertion, lookup, and deletion
 * - Forward iterators only (no bidirectional or random access)
 * - Requires std::hash<VId> and operator== on VId
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/traits/voum_graph_traits.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <ranges>
#include <set>
#include <unordered_map>

using namespace graph::container;

// Type aliases for common test configurations
using voum_void_void_void =
      dynamic_graph<void, void, void, uint32_t, false, voum_graph_traits<void, void, void, uint32_t, false>>;
using voum_int_void_void =
      dynamic_graph<int, void, void, uint32_t, false, voum_graph_traits<int, void, void, uint32_t, false>>;
using voum_void_int_void =
      dynamic_graph<void, int, void, uint32_t, false, voum_graph_traits<void, int, void, uint32_t, false>>;
using voum_int_int_void =
      dynamic_graph<int, int, void, uint32_t, false, voum_graph_traits<int, int, void, uint32_t, false>>;
using voum_void_void_int =
      dynamic_graph<void, void, int, uint32_t, false, voum_graph_traits<void, void, int, uint32_t, false>>;
using voum_int_int_int =
      dynamic_graph<int, int, int, uint32_t, false, voum_graph_traits<int, int, int, uint32_t, false>>;

using voum_string_string_string =
      dynamic_graph<std::string,
                    std::string,
                    std::string,
                    uint32_t,
                    false,
                    voum_graph_traits<std::string, std::string, std::string, uint32_t, false>>;

using voum_sourced =
      dynamic_graph<void, void, void, uint32_t, true, voum_graph_traits<void, void, void, uint32_t, true>>;
using voum_int_sourced =
      dynamic_graph<int, void, void, uint32_t, true, voum_graph_traits<int, void, void, uint32_t, true>>;

// Edge and vertex data types for loading
using edge_void  = copyable_edge_t<uint32_t, void>;
using edge_int   = copyable_edge_t<uint32_t, int>;
using vertex_int = copyable_vertex_t<uint32_t, int>;

// Helper function to count total edges in graph
template <typename G>
size_t count_all_edges(G& g) {
  size_t count = 0;
  for (auto& v : g) {
    count += static_cast<size_t>(std::ranges::distance(v.edges()));
  }
  return count;
}

//==================================================================================================
// 1. Construction Tests
//==================================================================================================

TEST_CASE("voum default construction", "[voum][construction]") {
  SECTION("creates empty graph") {
    voum_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with void types") {
    voum_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with int edge values") {
    voum_int_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with int vertex values") {
    voum_void_int_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with int graph value") {
    voum_void_void_int g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with all int values") {
    voum_int_int_int g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with string values") {
    voum_string_string_string g;
    REQUIRE(g.size() == 0);
  }
}

TEST_CASE("voum constructor with graph value", "[voum][construction]") {
  SECTION("void GV - no graph value can be passed") {
    voum_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("int GV") {
    voum_void_void_int g(42);
    REQUIRE(g.size() == 0);
    REQUIRE(g.graph_value() == 42);
  }
}

//==================================================================================================
// 2. Load Edges Tests
//==================================================================================================

TEST_CASE("voum load_edges", "[voum][load]") {
  SECTION("simple edges") {
    voum_void_void_void    g;
    std::vector<edge_void> ee = {{0, 1}, {0, 2}, {1, 2}};
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 3);
    REQUIRE(count_all_edges(g) == 3);
  }

  SECTION("edges with vertex count") {
    voum_void_void_void    g;
    std::vector<edge_void> ee = {{0, 1}, {1, 2}};
    g.load_edges(ee, std::identity{}, 6); // Request 6 vertices

    REQUIRE(g.size() == 6); // 0 through 5
    REQUIRE(count_all_edges(g) == 2);
  }

  SECTION("edges with values") {
    voum_int_void_void    g;
    std::vector<edge_int> ee = {{0, 1, 100}, {0, 2, 200}};
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 3);
    REQUIRE(count_all_edges(g) == 2);

    // Collect edge values (order is unspecified for unordered_map)
    auto&                              v0 = g[0];
    std::unordered_map<uint32_t, int>  edge_vals;
    for (const auto& edge : v0.edges()) {
      edge_vals[edge.second.target_id()] = edge.second.value();
    }
    REQUIRE(edge_vals[1] == 100);
    REQUIRE(edge_vals[2] == 200);
  }
}

//==================================================================================================
// 3. Initializer List Construction Tests
//==================================================================================================

TEST_CASE("voum initializer list construction", "[voum][construction][initializer_list]") {
  SECTION("simple initializer list") {
    voum_void_void_void g({{0, 1}, {0, 2}, {1, 2}});

    REQUIRE(g.size() == 3);
    REQUIRE(count_all_edges(g) == 3);
  }
}

//==================================================================================================
// 4. Deduplication Tests (unordered_map guarantees unique keys)
//==================================================================================================

TEST_CASE("voum edge deduplication", "[voum][unordered_map][deduplication]") {
  SECTION("duplicate edges are ignored - unsourced") {
    voum_void_void_void g;
    // Load edges with duplicates
    std::vector<edge_void> ee = {
          {0, 1}, {0, 1}, {0, 1}, // Three identical edges
          {0, 2}, {0, 2},         // Two identical edges
          {1, 2}                  // One unique edge
    };
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 3);
    // Deduplication: only 3 unique edges should exist
    REQUIRE(count_all_edges(g) == 3);

    // Verify each vertex has correct number of edges
    auto& v0 = g[0];
    auto& v1 = g[1];
    REQUIRE(std::distance(v0.edges().begin(), v0.edges().end()) == 2); // 0->1, 0->2
    REQUIRE(std::distance(v1.edges().begin(), v1.edges().end()) == 1); // 1->2
  }

  SECTION("duplicate edges with different values - first value wins") {
    voum_int_void_void    g;
    std::vector<edge_int> ee = {
          {0, 1, 100}, {0, 1, 200}, {0, 1, 300} // Same edge, different values
    };
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 2);
    REQUIRE(count_all_edges(g) == 1); // Only one edge stored

    // First inserted value should be kept
    auto& v0 = g[0];
    REQUIRE(v0.edges().begin()->second.value() == 100);
  }

  SECTION("sourced edges - deduplication by (source_id, target_id)") {
    voum_sourced           g;
    std::vector<edge_void> ee = {
          {0, 1},
          {0, 1}, // Duplicates
          {1, 0},
          {1, 0} // Different direction, also duplicates
    };
    g.load_edges(ee, std::identity{});

    // Should have exactly 2 unique edges (0->1 and 1->0)
    REQUIRE(count_all_edges(g) == 2);
  }
}

//==================================================================================================
// 5. Unordered Behavior Tests
//==================================================================================================

TEST_CASE("voum edges are unordered", "[voum][unordered_map][unordered]") {
  SECTION("all target_ids are present regardless of order") {
    voum_void_void_void    g;
    std::vector<edge_void> ee = {{0, 5}, {0, 2}, {0, 8}, {0, 1}, {0, 3}};
    g.load_edges(ee, std::identity{});

    // Collect all target_ids (order is unspecified)
    auto&                     v0 = g[0];
    std::set<uint32_t>        target_ids;
    for (const auto& edge : v0.edges()) {
      target_ids.insert(edge.second.target_id());
    }

    REQUIRE(target_ids == std::set<uint32_t>{1, 2, 3, 5, 8});
  }

  SECTION("sourced edges - all target_ids present") {
    voum_sourced           g;
    std::vector<edge_void> ee = {{0, 7}, {0, 3}, {0, 9}, {0, 1}};
    g.load_edges(ee, std::identity{});

    auto&              v0 = g[0];
    std::set<uint32_t> target_ids;
    for (const auto& edge : v0.edges()) {
      target_ids.insert(edge.second.target_id());
    }

    REQUIRE(target_ids == std::set<uint32_t>{1, 3, 7, 9});
  }
}

//==================================================================================================
// 6. Vertex Access Tests
//==================================================================================================

TEST_CASE("voum vertex access", "[voum][vertex][access]") {
  SECTION("operator[] access") {
    voum_void_void_void g({{0, 1}, {1, 2}, {2, 3}});

    REQUIRE(g.size() == 4);
    // Access each vertex
    auto& v0 = g[0];
    auto& v1 = g[1];
    auto& v2 = g[2];
    auto& v3 = g[3];

    // Verify edge counts
    REQUIRE(std::distance(v0.edges().begin(), v0.edges().end()) == 1);
    REQUIRE(std::distance(v1.edges().begin(), v1.edges().end()) == 1);
    REQUIRE(std::distance(v2.edges().begin(), v2.edges().end()) == 1);
    REQUIRE(std::distance(v3.edges().begin(), v3.edges().end()) == 0);
  }

  SECTION("const operator[] access") {
    const voum_void_void_void g({{0, 1}, {1, 2}});

    const auto& v0 = g[0];
    const auto& v1 = g[1];

    REQUIRE(std::distance(v0.edges().begin(), v0.edges().end()) == 1);
    REQUIRE(std::distance(v1.edges().begin(), v1.edges().end()) == 1);
  }
}

TEST_CASE("voum vertex iteration", "[voum][vertex][iteration]") {
  SECTION("range-based for") {
    voum_void_void_void g({{0, 1}, {1, 2}, {2, 0}});

    size_t count = 0;
    for (const auto& vertex : g) {
      (void)vertex;
      ++count;
    }
    REQUIRE(count == 3);
  }

  SECTION("begin/end iteration") {
    voum_void_void_void g({{0, 1}, {1, 2}});

    auto it = g.begin();
    REQUIRE(it != g.end());
    ++it;
    REQUIRE(it != g.end());
    ++it;
    REQUIRE(it != g.end());
    ++it;
    REQUIRE(it == g.end());
  }
}

//==================================================================================================
// 7. Edge Access Tests
//==================================================================================================

TEST_CASE("voum edge access", "[voum][edge][access]") {
  SECTION("edges() returns unordered_map") {
    voum_void_void_void g({{0, 1}, {0, 2}, {0, 3}});

    auto& v0       = g[0];
    auto& edge_map = v0.edges();

    REQUIRE(std::distance(edge_map.begin(), edge_map.end()) == 3);
  }

  SECTION("edge target_id access") {
    voum_void_void_void g({{0, 5}});

    auto& v0 = g[0];
    auto  it = v0.edges().begin();
    REQUIRE(it->second.target_id() == 5);
  }

  SECTION("edge value access") {
    voum_int_void_void    g;
    std::vector<edge_int> ee = {{0, 1, 42}};
    g.load_edges(ee, std::identity{});

    auto& v0 = g[0];
    auto  it = v0.edges().begin();
    REQUIRE(it->second.value() == 42);
  }
}

TEST_CASE("voum edge forward iteration", "[voum][edge][iteration]") {
  SECTION("forward iteration covers all edges") {
    voum_void_void_void g({{0, 1}, {0, 2}, {0, 3}});

    auto&              v0 = g[0];
    std::set<uint32_t> targets;
    for (const auto& edge : v0.edges()) {
      targets.insert(edge.second.target_id());
    }

    REQUIRE(targets.size() == 3);
    REQUIRE(targets == std::set<uint32_t>{1, 2, 3});
  }

  // Note: No reverse iteration test — unordered_map only provides forward iterators
}

//==================================================================================================
// 8. Vertex and Edge Value Tests
//==================================================================================================

TEST_CASE("voum vertex values", "[voum][vertex][value]") {
  SECTION("vertex value access") {
    voum_void_int_void      g;
    std::vector<vertex_int> vv = {{0, 100}, {1, 200}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_void> ee = {{0, 1}};
    g.load_edges(ee, std::identity{});

    REQUIRE(g[0].value() == 100);
    REQUIRE(g[1].value() == 200);
  }
}

TEST_CASE("voum edge values", "[voum][edge][value]") {
  SECTION("edge values preserved after deduplication") {
    voum_int_void_void    g;
    std::vector<edge_int> ee = {{0, 1, 100}, {0, 2, 200}};
    g.load_edges(ee, std::identity{});

    auto&                             v0 = g[0];
    std::unordered_map<uint32_t, int> edge_vals;
    for (const auto& edge : v0.edges()) {
      edge_vals[edge.second.target_id()] = edge.second.value();
    }
    REQUIRE(edge_vals[1] == 100);
    REQUIRE(edge_vals[2] == 200);
  }
}

//==================================================================================================
// 9. Sourced Edge Tests
//==================================================================================================

TEST_CASE("voum sourced edges", "[voum][sourced]") {
  SECTION("source_id access") {
    voum_sourced g({{0, 1}, {0, 2}, {1, 0}});

    auto& v0 = g[0];
    for (const auto& edge : v0.edges()) {
      REQUIRE(edge.second.source_id() == 0);
    }

    auto& v1 = g[1];
    for (const auto& edge : v1.edges()) {
      REQUIRE(edge.second.source_id() == 1);
    }
  }

  SECTION("sourced edge with values") {
    voum_int_sourced      g;
    std::vector<edge_int> ee = {{0, 1, 100}, {1, 0, 200}};
    g.load_edges(ee, std::identity{});

    // Verify edges from vertex 0
    auto&                             v0 = g[0];
    auto                              it0 = v0.edges().begin();
    REQUIRE(it0->second.source_id() == 0);
    REQUIRE(it0->second.target_id() == 1);
    REQUIRE(it0->second.value() == 100);

    // Verify edges from vertex 1
    auto& v1  = g[1];
    auto  it1 = v1.edges().begin();
    REQUIRE(it1->second.source_id() == 1);
    REQUIRE(it1->second.target_id() == 0);
    REQUIRE(it1->second.value() == 200);
  }
}

//==================================================================================================
// 10. Self-Loop Tests
//==================================================================================================

TEST_CASE("voum self-loops", "[voum][self-loop]") {
  SECTION("single self-loop") {
    voum_void_void_void g({{0, 0}});

    REQUIRE(g.size() == 1);
    REQUIRE(count_all_edges(g) == 1);

    auto& v0 = g[0];
    REQUIRE(std::distance(v0.edges().begin(), v0.edges().end()) == 1);
    REQUIRE(v0.edges().begin()->second.target_id() == 0);
  }

  SECTION("self-loop deduplication") {
    voum_void_void_void g({{0, 0}, {0, 0}, {0, 0}});

    // Only one self-loop should exist
    REQUIRE(count_all_edges(g) == 1);
  }

  SECTION("self-loop with outgoing edges") {
    voum_void_void_void g({{0, 0}, {0, 1}, {0, 2}});

    REQUIRE(count_all_edges(g) == 3);

    auto&              v0 = g[0];
    std::set<uint32_t> targets;
    for (const auto& edge : v0.edges()) {
      targets.insert(edge.second.target_id());
    }

    REQUIRE(targets == std::set<uint32_t>{0, 1, 2});
  }
}

//==================================================================================================
// 11. Large Graph Tests
//==================================================================================================

TEST_CASE("voum large graph", "[voum][performance]") {
  SECTION("1000 vertices linear chain") {
    std::vector<edge_void> ee;
    for (uint32_t i = 0; i < 999; ++i) {
      ee.push_back({i, i + 1});
    }

    voum_void_void_void g;
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 1000);
    REQUIRE(count_all_edges(g) == 999);
  }

  SECTION("star graph with 100 spokes") {
    std::vector<edge_void> ee;
    for (uint32_t i = 1; i <= 100; ++i) {
      ee.push_back({0, i});
    }

    voum_void_void_void g;
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 101);
    REQUIRE(count_all_edges(g) == 100);

    // Vertex 0 should have all 100 edges
    auto& v0 = g[0];
    REQUIRE(std::distance(v0.edges().begin(), v0.edges().end()) == 100);
  }
}

//==================================================================================================
// 12. Iterator Tests (forward only for unordered_map)
//==================================================================================================

TEST_CASE("voum forward iterator behavior", "[voum][unordered_map][iterator]") {
  SECTION("edge iterators are forward") {
    voum_void_void_void g({{0, 1}, {0, 2}, {0, 3}});

    auto& v0       = g[0];
    auto& edge_map = v0.edges();

    // Forward iteration
    std::set<uint32_t> visited;
    for (auto it = edge_map.begin(); it != edge_map.end(); ++it) {
      visited.insert(it->second.target_id());
    }
    REQUIRE(visited == std::set<uint32_t>{1, 2, 3});
  }

  SECTION("find by key") {
    voum_void_void_void g({{0, 1}, {0, 2}, {0, 3}});

    auto& v0       = g[0];
    auto& edge_map = v0.edges();

    // unordered_map supports O(1) average find
    auto it = edge_map.find(2);
    REQUIRE(it != edge_map.end());
    REQUIRE(it->second.target_id() == 2);

    auto it_miss = edge_map.find(99);
    REQUIRE(it_miss == edge_map.end());
  }
}

//==================================================================================================
// 13. Algorithm Compatibility Tests
//==================================================================================================

TEST_CASE("voum algorithm compatibility", "[voum][algorithm]") {
  SECTION("std::ranges::for_each on vertices") {
    voum_void_void_void g({{0, 1}, {1, 2}, {2, 0}});

    size_t count = 0;
    std::ranges::for_each(g, [&count](const auto& v) {
      (void)v;
      ++count;
    });

    REQUIRE(count == 3);
  }

  SECTION("std::ranges::for_each on edges") {
    voum_void_void_void g({{0, 1}, {0, 2}, {0, 3}});

    auto&  v0    = g[0];
    size_t count = 0;
    std::ranges::for_each(v0.edges(), [&count](const auto& e) {
      (void)e;
      ++count;
    });

    REQUIRE(count == 3);
  }

  SECTION("std::find_if on edges") {
    voum_int_void_void    g;
    std::vector<edge_int> ee = {{0, 1, 100}, {0, 2, 200}, {0, 3, 300}};
    g.load_edges(ee, std::identity{});

    auto& v0 = g[0];
    auto  it = std::ranges::find_if(v0.edges(), [](const auto& e) { return e.second.value() == 200; });

    REQUIRE(it != v0.edges().end());
    REQUIRE(it->second.target_id() == 2);
  }
}

//==================================================================================================
// 14. Edge Case Tests
//==================================================================================================

TEST_CASE("voum edge cases", "[voum][edge-cases]") {
  SECTION("empty graph operations") {
    voum_void_void_void g;

    REQUIRE(g.size() == 0);
    REQUIRE(count_all_edges(g) == 0);
    REQUIRE(g.begin() == g.end());
  }

  SECTION("single vertex no edges") {
    voum_void_void_void    g;
    std::vector<edge_void> empty_edges;
    g.load_edges(empty_edges, std::identity{}, 1);

    REQUIRE(g.size() == 1);
    REQUIRE(count_all_edges(g) == 0);

    auto& v0 = g[0];
    REQUIRE(v0.edges().empty());
  }

  SECTION("vertices with no outgoing edges") {
    voum_void_void_void    g;
    std::vector<edge_void> ee = {{0, 1}};
    g.load_edges(ee, std::identity{}, 6);

    REQUIRE(g.size() == 6); // 0 through 5

    // Only vertex 0 has an edge
    REQUIRE(std::distance(g[0].edges().begin(), g[0].edges().end()) == 1);

    // Vertices 2-5 have no outgoing edges
    for (uint32_t i = 2; i <= 5; ++i) {
      REQUIRE(g[i].edges().empty());
    }
  }
}

//==================================================================================================
// 15. Type Trait Tests
//==================================================================================================

TEST_CASE("voum type traits", "[voum][traits]") {
  SECTION("edge_type is correct") {
    using traits = voum_graph_traits<int, void, void, uint32_t, false>;
    using edge_t = traits::edge_type;

    static_assert(std::is_same_v<edge_t::value_type, int>);
    static_assert(std::is_same_v<edge_t::vertex_id_type, uint32_t>);
  }

  SECTION("edges_type is std::unordered_map") {
    using traits  = voum_graph_traits<void, void, void, uint32_t, false>;
    using edges_t = traits::edges_type;

    // Verify it's an unordered_map by checking it has key_type and hasher
    static_assert(requires { typename edges_t::key_type; });
    static_assert(requires { typename edges_t::hasher; });
  }

  SECTION("sourced trait") {
    using traits_unsourced = voum_graph_traits<void, void, void, uint32_t, false>;
    using traits_sourced   = voum_graph_traits<void, void, void, uint32_t, true>;

    static_assert(traits_unsourced::sourced == false);
    static_assert(traits_sourced::sourced == true);
  }
}

//==================================================================================================
// 16. Complex Graph Structure Tests
//==================================================================================================

TEST_CASE("voum complex structures", "[voum][complex]") {
  SECTION("complete graph K4") {
    std::vector<edge_void> ee;
    for (uint32_t i = 0; i < 4; ++i) {
      for (uint32_t j = 0; j < 4; ++j) {
        if (i != j) {
          ee.push_back({i, j});
        }
      }
    }

    voum_void_void_void g;
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 4);
    REQUIRE(count_all_edges(g) == 12); // 4 * 3 directed edges

    // Each vertex should have 3 outgoing edges
    for (uint32_t i = 0; i < 4; ++i) {
      REQUIRE(std::distance(g[i].edges().begin(), g[i].edges().end()) == 3);
    }
  }

  SECTION("cycle graph C5") {
    voum_void_void_void g({{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0}});

    REQUIRE(g.size() == 5);
    REQUIRE(count_all_edges(g) == 5);
  }

  SECTION("binary tree depth 3") {
    voum_void_void_void g({
          {0, 1},
          {0, 2}, // Level 1
          {1, 3},
          {1, 4}, // Level 2 left
          {2, 5},
          {2, 6} // Level 2 right
    });

    REQUIRE(g.size() == 7);
    REQUIRE(count_all_edges(g) == 6);

    // Root has 2 children
    REQUIRE(std::distance(g[0].edges().begin(), g[0].edges().end()) == 2);

    // Internal nodes have 2 children each
    REQUIRE(std::distance(g[1].edges().begin(), g[1].edges().end()) == 2);
    REQUIRE(std::distance(g[2].edges().begin(), g[2].edges().end()) == 2);

    // Leaves have no children
    for (uint32_t i = 3; i <= 6; ++i) {
      REQUIRE(g[i].edges().empty());
    }
  }
}
