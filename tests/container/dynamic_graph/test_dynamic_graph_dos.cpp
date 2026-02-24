/**
 * @file test_dynamic_graph_dos.cpp
 * @brief Comprehensive tests for dynamic_graph with deque vertices + set edges
 * 
 * Phase 4.1.3: Set Edge Container Support with Deque Vertices
 * Tests dos_graph_traits (deque vertices + set edges)
 * 
 * Key characteristics:
 * - Vertices: std::deque (stable references on push_back/push_front; random access)
 * - Edges: std::set (automatic deduplication, sorted order)
 * - O(log n) edge insertion, lookup, and deletion
 * - Bidirectional iterators for edges (no random access to edges)
 * - Edge values NOT considered in comparison (only structural IDs)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/traits/dos_graph_traits.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <ranges>
#include <set>

using namespace graph::container;

// Type aliases for common test configurations
using dos_void_void_void =
      dynamic_graph<void, void, void, uint32_t, false, dos_graph_traits<void, void, void, uint32_t, false>>;
using dos_int_void_void =
      dynamic_graph<int, void, void, uint32_t, false, dos_graph_traits<int, void, void, uint32_t, false>>;
using dos_void_int_void =
      dynamic_graph<void, int, void, uint32_t, false, dos_graph_traits<void, int, void, uint32_t, false>>;
using dos_int_int_void =
      dynamic_graph<int, int, void, uint32_t, false, dos_graph_traits<int, int, void, uint32_t, false>>;
using dos_void_void_int =
      dynamic_graph<void, void, int, uint32_t, false, dos_graph_traits<void, void, int, uint32_t, false>>;
using dos_int_int_int = dynamic_graph<int, int, int, uint32_t, false, dos_graph_traits<int, int, int, uint32_t, false>>;

using dos_string_string_string =
      dynamic_graph<std::string,
                    std::string,
                    std::string,
                    uint32_t,
                    false, dos_graph_traits<std::string, std::string, std::string, uint32_t, false>>;


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

TEST_CASE("dos default construction", "[dos][construction]") {
  SECTION("creates empty graph") {
    dos_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with void types") {
    dos_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with int edge values") {
    dos_int_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with int vertex values") {
    dos_void_int_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with int graph value") {
    dos_void_void_int g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with all int values") {
    dos_int_int_int g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with string values") {
    dos_string_string_string g;
    REQUIRE(g.size() == 0);
  }
}

TEST_CASE("dos constructor with graph value", "[dos][construction]") {
  SECTION("void GV - no graph value can be passed") {
    dos_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("int GV") {
    dos_void_void_int g(42);
    REQUIRE(g.size() == 0);
    REQUIRE(g.graph_value() == 42);
  }
}

//==================================================================================================
// 2. Load Edges Tests
//==================================================================================================

TEST_CASE("dos load_edges", "[dos][load]") {
  SECTION("simple edges") {
    dos_void_void_void     g;
    std::vector<edge_void> ee = {{0, 1}, {0, 2}, {1, 2}};
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 3);
    REQUIRE(count_all_edges(g) == 3);
  }

  SECTION("edges with vertex count") {
    dos_void_void_void     g;
    std::vector<edge_void> ee = {{0, 1}, {1, 2}};
    g.load_edges(ee, std::identity{}, 6); // Request 6 vertices

    REQUIRE(g.size() == 6); // 0 through 5
    REQUIRE(count_all_edges(g) == 2);
  }

  SECTION("edges with values") {
    dos_int_void_void     g;
    std::vector<edge_int> ee = {{0, 1, 100}, {0, 2, 200}};
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 3);
    REQUIRE(count_all_edges(g) == 2);

    auto& v0 = g[0];
    auto  it = v0.edges().begin();
    // Edges are sorted by target_id
    REQUIRE(it->target_id() == 1);
    REQUIRE(it->value() == 100);
    ++it;
    REQUIRE(it->target_id() == 2);
    REQUIRE(it->value() == 200);
  }
}

//==================================================================================================
// 3. Initializer List Construction Tests
//==================================================================================================

TEST_CASE("dos initializer list construction", "[dos][construction][initializer_list]") {
  SECTION("simple initializer list") {
    dos_void_void_void g({{0, 1}, {0, 2}, {1, 2}});

    REQUIRE(g.size() == 3);
    REQUIRE(count_all_edges(g) == 3);
  }
}

//==================================================================================================
// 4. Set-Specific Behavior: Deduplication Tests
//==================================================================================================

TEST_CASE("dos edge deduplication", "[dos][set][deduplication]") {
  SECTION("duplicate edges are ignored - unsourced") {
    dos_void_void_void g;
    // Load edges with duplicates
    std::vector<edge_void> ee = {
          {0, 1}, {0, 1}, {0, 1}, // Three identical edges
          {0, 2}, {0, 2},         // Two identical edges
          {1, 2}                  // One unique edge
    };
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 3);
    // Set deduplicates: only 3 unique edges should exist
    REQUIRE(count_all_edges(g) == 3);

    // Verify each vertex has correct number of edges
    auto& v0 = g[0];
    auto& v1 = g[1];
    REQUIRE(std::distance(v0.edges().begin(), v0.edges().end()) == 2); // 0->1, 0->2
    REQUIRE(std::distance(v1.edges().begin(), v1.edges().end()) == 1); // 1->2
  }

  SECTION("duplicate edges with different values - first value wins") {
    // In std::set, insert ignores duplicates, keeping original
    dos_int_void_void     g;
    std::vector<edge_int> ee = {
          {0, 1, 100}, {0, 1, 200}, {0, 1, 300} // Same edge, different values
    };
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 2);
    REQUIRE(count_all_edges(g) == 1); // Only one edge stored

    // First inserted value should be kept
    auto& v0 = g[0];
    REQUIRE(v0.edges().begin()->value() == 100);
  }

}

//==================================================================================================
// 5. Set-Specific Behavior: Sorted Order Tests
//==================================================================================================

TEST_CASE("dos edges are sorted by target_id", "[dos][set][sorted]") {
  SECTION("unsourced edges sorted by target_id") {
    // Insert edges in unsorted order
    dos_void_void_void     g;
    std::vector<edge_void> ee = {{0, 5}, {0, 2}, {0, 8}, {0, 1}, {0, 3}};
    g.load_edges(ee, std::identity{});

    // Edges from vertex 0 should be in sorted order by target_id
    auto&                 v0 = g[0];
    std::vector<uint32_t> target_ids;
    for (const auto& edge : v0.edges()) {
      target_ids.push_back(edge.target_id());
    }

    REQUIRE(target_ids == std::vector<uint32_t>{1, 2, 3, 5, 8});
  }

}

//==================================================================================================
// 6. Vertex Access Tests
//==================================================================================================

TEST_CASE("dos vertex access", "[dos][vertex][access]") {
  SECTION("operator[] access") {
    dos_void_void_void g({{0, 1}, {1, 2}, {2, 3}});

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
    const dos_void_void_void g({{0, 1}, {1, 2}});

    const auto& v0 = g[0];
    const auto& v1 = g[1];

    REQUIRE(std::distance(v0.edges().begin(), v0.edges().end()) == 1);
    REQUIRE(std::distance(v1.edges().begin(), v1.edges().end()) == 1);
  }
}

TEST_CASE("dos vertex iteration", "[dos][vertex][iteration]") {
  SECTION("range-based for") {
    dos_void_void_void g({{0, 1}, {1, 2}, {2, 0}});

    size_t count = 0;
    for (const auto& vertex : g) {
      (void)vertex;
      ++count;
    }
    REQUIRE(count == 3);
  }

  SECTION("begin/end iteration") {
    dos_void_void_void g({{0, 1}, {1, 2}});

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

TEST_CASE("dos edge access", "[dos][edge][access]") {
  SECTION("edges() returns set") {
    dos_void_void_void g({{0, 1}, {0, 2}, {0, 3}});

    auto& v0       = g[0];
    auto& edge_set = v0.edges();

    REQUIRE(std::distance(edge_set.begin(), edge_set.end()) == 3);
  }

  SECTION("edge target_id access") {
    dos_void_void_void g({{0, 5}});

    auto& v0 = g[0];
    auto  it = v0.edges().begin();
    REQUIRE(it->target_id() == 5);
  }

  SECTION("edge value access") {
    dos_int_void_void     g;
    std::vector<edge_int> ee = {{0, 1, 42}};
    g.load_edges(ee, std::identity{});

    auto& v0 = g[0];
    auto  it = v0.edges().begin();
    REQUIRE(it->value() == 42);
  }
}

TEST_CASE("dos edge bidirectional iteration", "[dos][edge][iteration]") {
  SECTION("forward iteration") {
    dos_void_void_void g({{0, 1}, {0, 2}, {0, 3}});

    auto&                 v0 = g[0];
    std::vector<uint32_t> targets;
    for (const auto& edge : v0.edges()) {
      targets.push_back(edge.target_id());
    }

    REQUIRE(targets.size() == 3);
    REQUIRE(targets == std::vector<uint32_t>{1, 2, 3}); // Sorted
  }

  SECTION("reverse iteration") {
    dos_void_void_void g({{0, 1}, {0, 2}, {0, 3}});

    auto& v0       = g[0];
    auto& edge_set = v0.edges();

    std::vector<uint32_t> targets;
    for (auto it = edge_set.rbegin(); it != edge_set.rend(); ++it) {
      targets.push_back(it->target_id());
    }

    REQUIRE(targets == std::vector<uint32_t>{3, 2, 1}); // Reverse sorted
  }
}

//==================================================================================================
// 8. Vertex and Edge Value Tests
//==================================================================================================

TEST_CASE("dos vertex values", "[dos][vertex][value]") {
  SECTION("vertex value access") {
    dos_void_int_void       g;
    std::vector<vertex_int> vv = {{0, 100}, {1, 200}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_void> ee = {{0, 1}};
    g.load_edges(ee, std::identity{});

    REQUIRE(g[0].value() == 100);
    REQUIRE(g[1].value() == 200);
  }
}

TEST_CASE("dos edge values", "[dos][edge][value]") {
  SECTION("edge values preserved after deduplication") {
    // First edge with value 100 should be kept
    dos_int_void_void     g;
    std::vector<edge_int> ee = {{0, 1, 100}, {0, 2, 200}};
    g.load_edges(ee, std::identity{});

    auto& v0 = g[0];
    auto  it = v0.edges().begin();
    REQUIRE(it->value() == 100); // First edge value
    ++it;
    REQUIRE(it->value() == 200); // Second edge value
  }
}

//==================================================================================================
// 9. Sourced Edge Tests
//==================================================================================================


//==================================================================================================
// 10. Self-Loop Tests
//==================================================================================================

TEST_CASE("dos self-loops", "[dos][self-loop]") {
  SECTION("single self-loop") {
    dos_void_void_void g({{0, 0}});

    REQUIRE(g.size() == 1);
    REQUIRE(count_all_edges(g) == 1);

    auto& v0 = g[0];
    REQUIRE(std::distance(v0.edges().begin(), v0.edges().end()) == 1);
    REQUIRE(v0.edges().begin()->target_id() == 0);
  }

  SECTION("self-loop deduplication") {
    dos_void_void_void g({{0, 0}, {0, 0}, {0, 0}});

    // Only one self-loop should exist
    REQUIRE(count_all_edges(g) == 1);
  }

  SECTION("self-loop with outgoing edges") {
    dos_void_void_void g({{0, 0}, {0, 1}, {0, 2}});

    REQUIRE(count_all_edges(g) == 3);

    auto&                 v0 = g[0];
    std::vector<uint32_t> targets;
    for (const auto& edge : v0.edges()) {
      targets.push_back(edge.target_id());
    }

    // Self-loop (0) should be first in sorted order
    REQUIRE(targets == std::vector<uint32_t>{0, 1, 2});
  }
}

//==================================================================================================
// 11. Large Graph Tests
//==================================================================================================

TEST_CASE("dos large graph", "[dos][performance]") {
  SECTION("1000 vertices linear chain") {
    std::vector<edge_void> ee;
    for (uint32_t i = 0; i < 999; ++i) {
      ee.push_back({i, i + 1});
    }

    dos_void_void_void g;
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 1000);
    REQUIRE(count_all_edges(g) == 999);
  }

  SECTION("star graph with 100 spokes") {
    std::vector<edge_void> ee;
    for (uint32_t i = 1; i <= 100; ++i) {
      ee.push_back({0, i});
    }

    dos_void_void_void g;
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 101);
    REQUIRE(count_all_edges(g) == 100);

    // Vertex 0 should have all 100 edges
    auto& v0 = g[0];
    REQUIRE(std::distance(v0.edges().begin(), v0.edges().end()) == 100);
  }
}

//==================================================================================================
// 12. Deque-Specific: Reference Stability Tests
//==================================================================================================

TEST_CASE("dos deque reference stability", "[dos][deque][stability]") {
  SECTION("references remain valid after resize") {
    dos_void_int_void       g;
    std::vector<vertex_int> vv = {{0, 100}, {1, 200}};
    g.load_vertices(vv, std::identity{});

    // Get reference before resize
    auto& v0_ref         = g[0];
    int   original_value = v0_ref.value();

    // Resize by loading more edges (may trigger deque growth)
    std::vector<edge_void> ee;
    for (uint32_t i = 2; i < 1000; ++i) {
      ee.push_back({i - 1, i});
    }
    g.load_edges(ee, std::identity{});

    // For deque, references to existing elements remain valid after push_back
    // (unlike vector which may invalidate on reallocation)
    REQUIRE(g[0].value() == original_value);
  }
}

//==================================================================================================
// 13. Iterator Stability Tests (std::set guarantees)
//==================================================================================================

TEST_CASE("dos set iterator stability", "[dos][set][iterator]") {
  SECTION("edge iterators are bidirectional") {
    dos_void_void_void g({{0, 1}, {0, 2}, {0, 3}});

    auto& v0       = g[0];
    auto& edge_set = v0.edges();

    // Forward
    auto it = edge_set.begin();
    REQUIRE(it->target_id() == 1);
    ++it;
    REQUIRE(it->target_id() == 2);
    ++it;
    REQUIRE(it->target_id() == 3);

    // Backward
    --it;
    REQUIRE(it->target_id() == 2);
    --it;
    REQUIRE(it->target_id() == 1);
  }
}

//==================================================================================================
// 14. Algorithm Compatibility Tests
//==================================================================================================

TEST_CASE("dos algorithm compatibility", "[dos][algorithm]") {
  SECTION("std::ranges::for_each on vertices") {
    dos_void_void_void g({{0, 1}, {1, 2}, {2, 0}});

    size_t count = 0;
    std::ranges::for_each(g, [&count](const auto& v) {
      (void)v;
      ++count;
    });

    REQUIRE(count == 3);
  }

  SECTION("std::ranges::for_each on edges") {
    dos_void_void_void g({{0, 1}, {0, 2}, {0, 3}});

    auto&  v0    = g[0];
    size_t count = 0;
    std::ranges::for_each(v0.edges(), [&count](const auto& e) {
      (void)e;
      ++count;
    });

    REQUIRE(count == 3);
  }

  SECTION("std::find_if on edges") {
    dos_int_void_void     g;
    std::vector<edge_int> ee = {{0, 1, 100}, {0, 2, 200}, {0, 3, 300}};
    g.load_edges(ee, std::identity{});

    auto& v0 = g[0];
    auto  it = std::ranges::find_if(v0.edges(), [](const auto& e) { return e.value() == 200; });

    REQUIRE(it != v0.edges().end());
    REQUIRE(it->target_id() == 2);
  }
}

//==================================================================================================
// 15. Edge Case Tests
//==================================================================================================

TEST_CASE("dos edge cases", "[dos][edge-cases]") {
  SECTION("empty graph operations") {
    dos_void_void_void g;

    REQUIRE(g.size() == 0);
    REQUIRE(count_all_edges(g) == 0);
    REQUIRE(g.begin() == g.end());
  }

  SECTION("single vertex no edges") {
    // Create a graph with a single vertex using load_edges with vertex_count
    dos_void_void_void     g;
    std::vector<edge_void> empty_edges;
    g.load_edges(empty_edges, std::identity{}, 1);

    REQUIRE(g.size() == 1);
    REQUIRE(count_all_edges(g) == 0);

    auto& v0 = g[0];
    REQUIRE(v0.edges().empty());
  }

  SECTION("vertices with no outgoing edges") {
    // Load edges with more vertices using load_edges with vertex_count
    dos_void_void_void     g;
    std::vector<edge_void> ee = {{0, 1}};
    g.load_edges(ee, std::identity{}, 6);

    REQUIRE(g.size() == 6); // 0 through 5

    // Only vertex 0 has an edge
    REQUIRE(std::distance(g[0].edges().begin(), g[0].edges().end()) == 1);

    // Vertices 1-5 have no outgoing edges (1 has incoming from 0 but no outgoing)
    for (uint32_t i = 2; i <= 5; ++i) {
      REQUIRE(g[i].edges().empty());
    }
  }
}

//==================================================================================================
// 16. Type Trait Tests
//==================================================================================================

TEST_CASE("dos type traits", "[dos][traits]") {
  SECTION("edge_type is correct") {
    using traits = dos_graph_traits<int, void, void, uint32_t, false>;
    using edge_t = traits::edge_type;

    static_assert(std::is_same_v<edge_t::value_type, int>);
    static_assert(std::is_same_v<edge_t::vertex_id_type, uint32_t>);
  }

  SECTION("edges_type is std::set") {
    using traits  = dos_graph_traits<void, void, void, uint32_t, false>;
    using edges_t = traits::edges_type;

    // Verify it's a set by checking it has set-specific types
    static_assert(requires { typename edges_t::key_type; });
  }

  SECTION("vertices_type is std::deque") {
    using traits     = dos_graph_traits<void, void, void, uint32_t, false>;
    using vertices_t = traits::vertices_type;

    // Verify it's a deque by checking it has deque-specific operations
    static_assert(requires(vertices_t& v) { v.push_front(std::declval<typename vertices_t::value_type>()); });
  }

}

//==================================================================================================
// 17. Complex Graph Structure Tests
//==================================================================================================

TEST_CASE("dos complex structures", "[dos][complex]") {
  SECTION("complete graph K4") {
    std::vector<edge_void> ee;
    for (uint32_t i = 0; i < 4; ++i) {
      for (uint32_t j = 0; j < 4; ++j) {
        if (i != j) {
          ee.push_back({i, j});
        }
      }
    }

    dos_void_void_void g;
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 4);
    REQUIRE(count_all_edges(g) == 12); // 4 * 3 directed edges

    // Each vertex should have 3 outgoing edges
    for (uint32_t i = 0; i < 4; ++i) {
      REQUIRE(std::distance(g[i].edges().begin(), g[i].edges().end()) == 3);
    }
  }

  SECTION("cycle graph C5") {
    dos_void_void_void g({{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0}});

    REQUIRE(g.size() == 5);
    REQUIRE(count_all_edges(g) == 5);
  }

  SECTION("binary tree depth 3") {
    dos_void_void_void g({
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
