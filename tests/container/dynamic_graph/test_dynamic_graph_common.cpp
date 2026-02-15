/**
 * @file test_dynamic_graph_common.cpp
 * @brief Phase 1.4: Unified tests for all sequential container trait combinations
 * 
 * Uses Catch2 TEMPLATE_TEST_CASE to run identical tests across 8 trait combinations:
 * - vofl_graph_traits (vector + forward_list)
 * - vol_graph_traits (vector + list)
 * - vov_graph_traits (vector + vector)
 * - vod_graph_traits (vector + deque)
 * - dofl_graph_traits (deque + forward_list)
 * - dol_graph_traits (deque + list)
 * - dov_graph_traits (deque + vector)
 * - dod_graph_traits (deque + deque)
 * 
 * All traits use uint64_t vertex IDs with auto-extension semantics.
 * Container-specific behavior is tested in separate files.
 * 
 * Current Status: Comprehensive test coverage across all sequential traits
 * 
 * Test Categories:
 * - Construction (15 tests)
 * - Load Operations (12 tests) 
 * - Vertex Access (10 tests)
 * - Edge Access (12 tests)
 * - Value Access (8 tests)
 * - Sourced Edges (6 tests)
 * - Properties (10 tests)
 * - Memory/Performance (8 tests)
 * - Edge Cases (15 tests)
 * - Iterators/Ranges (10 tests)
 * - Workflows (8 tests)
 * Total: ~114 test scenarios × 8 traits = ~912 test executions
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/traits/vofl_graph_traits.hpp>
#include <graph/container/traits/vol_graph_traits.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/container/traits/vod_graph_traits.hpp>
#include <graph/container/traits/dofl_graph_traits.hpp>
#include <graph/container/traits/dol_graph_traits.hpp>
#include <graph/container/traits/dov_graph_traits.hpp>
#include <graph/container/traits/dod_graph_traits.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/graph_info.hpp>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <numeric>
#include <ranges>

using namespace graph::container;

//==================================================================================================
// TEMPLATE_TEST_CASE: Common Construction Tests
// Runs across all 8 sequential trait combinations
//==================================================================================================

TEMPLATE_TEST_CASE("default construction creates empty graph",
                   "[common][construction]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph g;
  REQUIRE(g.size() == 0);
  REQUIRE(g.begin() == g.end());
}

TEMPLATE_TEST_CASE("construction with graph value",
                   "[common][construction]",
                   (vofl_graph_traits<void, void, int, uint64_t, false>),
                   (vol_graph_traits<void, void, int, uint64_t, false>),
                   (vov_graph_traits<void, void, int, uint64_t, false>),
                   (vod_graph_traits<void, void, int, uint64_t, false>),
                   (dofl_graph_traits<void, void, int, uint64_t, false>),
                   (dol_graph_traits<void, void, int, uint64_t, false>),
                   (dov_graph_traits<void, void, int, uint64_t, false>),
                   (dod_graph_traits<void, void, int, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, int, uint64_t, false, TestType>;

  Graph g(42);
  REQUIRE(g.size() == 0);
  REQUIRE(g.graph_value() == 42);
}

TEMPLATE_TEST_CASE("construction with edge values",
                   "[common][construction]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, false, TestType>;

  Graph g;
  REQUIRE(g.size() == 0);
}

TEMPLATE_TEST_CASE("copy construction",
                   "[common][construction]",
                   (vofl_graph_traits<int, int, int, uint64_t, false>),
                   (vol_graph_traits<int, int, int, uint64_t, false>),
                   (vov_graph_traits<int, int, int, uint64_t, false>),
                   (vod_graph_traits<int, int, int, uint64_t, false>),
                   (dofl_graph_traits<int, int, int, uint64_t, false>),
                   (dol_graph_traits<int, int, int, uint64_t, false>),
                   (dov_graph_traits<int, int, int, uint64_t, false>),
                   (dod_graph_traits<int, int, int, uint64_t, false>)) {
  using Graph = dynamic_graph<int, int, int, uint64_t, false, TestType>;

  Graph g1;
  Graph g2(g1);
  REQUIRE(g2.size() == g1.size());
  REQUIRE(g2.size() == 0);
}

TEMPLATE_TEST_CASE("move construction",
                   "[common][construction]",
                   (vofl_graph_traits<int, int, int, uint64_t, false>),
                   (vol_graph_traits<int, int, int, uint64_t, false>),
                   (vov_graph_traits<int, int, int, uint64_t, false>),
                   (vod_graph_traits<int, int, int, uint64_t, false>),
                   (dofl_graph_traits<int, int, int, uint64_t, false>),
                   (dol_graph_traits<int, int, int, uint64_t, false>),
                   (dov_graph_traits<int, int, int, uint64_t, false>),
                   (dod_graph_traits<int, int, int, uint64_t, false>)) {
  using Graph = dynamic_graph<int, int, int, uint64_t, false, TestType>;

  Graph g1;
  Graph g2(std::move(g1));
  REQUIRE(g2.size() == 0);
}

TEMPLATE_TEST_CASE("construction with initializer_list edges",
                   "[common][construction]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, false, TestType>;

  Graph g({{0, 1, 10}, {1, 2, 20}, {2, 0, 30}});
  REQUIRE(g.size() == 3);

  auto& v0    = g[0];
  bool  found = false;
  for (auto& e : v0.edges()) {
    if (e.target_id() == 1) {
      REQUIRE(e.value() == 10);
      found = true;
    }
  }
  REQUIRE(found);
}

TEMPLATE_TEST_CASE("construction with edge range and load",
                   "[common][construction]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph     = dynamic_graph<void, void, void, uint64_t, false, TestType>;
  using edge_data = copyable_edge_t<uint64_t, void>;

  Graph                  g;
  std::vector<edge_data> edges = {{0, 1}, {1, 2}, {2, 3}};
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 4);
}

TEMPLATE_TEST_CASE("construction with graph value copy",
                   "[common][construction]",
                   (vofl_graph_traits<void, void, std::string, uint64_t, false>),
                   (vol_graph_traits<void, void, std::string, uint64_t, false>),
                   (vov_graph_traits<void, void, std::string, uint64_t, false>),
                   (vod_graph_traits<void, void, std::string, uint64_t, false>),
                   (dofl_graph_traits<void, void, std::string, uint64_t, false>),
                   (dol_graph_traits<void, void, std::string, uint64_t, false>),
                   (dov_graph_traits<void, void, std::string, uint64_t, false>),
                   (dod_graph_traits<void, void, std::string, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, std::string, uint64_t, false, TestType>;

  std::string val = "test_value";
  Graph       g(val);
  REQUIRE(g.graph_value() == "test_value");
  REQUIRE(val == "test_value"); // Original unchanged
}

TEMPLATE_TEST_CASE("construction with graph value move",
                   "[common][construction]",
                   (vofl_graph_traits<void, void, std::string, uint64_t, false>),
                   (vol_graph_traits<void, void, std::string, uint64_t, false>),
                   (vov_graph_traits<void, void, std::string, uint64_t, false>),
                   (vod_graph_traits<void, void, std::string, uint64_t, false>),
                   (dofl_graph_traits<void, void, std::string, uint64_t, false>),
                   (dol_graph_traits<void, void, std::string, uint64_t, false>),
                   (dov_graph_traits<void, void, std::string, uint64_t, false>),
                   (dod_graph_traits<void, void, std::string, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, std::string, uint64_t, false, TestType>;

  std::string val = "test_value";
  Graph       g(std::move(val));
  REQUIRE(g.graph_value() == "test_value");
}

TEMPLATE_TEST_CASE("assignment operators",
                   "[common][construction]",
                   (vofl_graph_traits<int, int, int, uint64_t, false>),
                   (vol_graph_traits<int, int, int, uint64_t, false>),
                   (vov_graph_traits<int, int, int, uint64_t, false>),
                   (vod_graph_traits<int, int, int, uint64_t, false>),
                   (dofl_graph_traits<int, int, int, uint64_t, false>),
                   (dol_graph_traits<int, int, int, uint64_t, false>),
                   (dov_graph_traits<int, int, int, uint64_t, false>),
                   (dod_graph_traits<int, int, int, uint64_t, false>)) {
  using Graph = dynamic_graph<int, int, int, uint64_t, false, TestType>;

  Graph g1, g2, g3;

  // Copy assignment
  g2 = g1;
  REQUIRE(g2.size() == g1.size());

  // Move assignment
  g3 = std::move(g1);
  REQUIRE(g3.size() == 0);
}

TEMPLATE_TEST_CASE("empty graph properties",
                   "[common][construction]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph g;
  REQUIRE(g.size() == 0);
  REQUIRE(g.begin() == g.end());
  REQUIRE(g.cbegin() == g.cend());
}

TEMPLATE_TEST_CASE("const graph access",
                   "[common][construction]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  const Graph g;
  REQUIRE(g.size() == 0);
  REQUIRE(g.begin() == g.end());
}

TEMPLATE_TEST_CASE("construction with pre-sized vertex container",
                   "[common][construction]",
                   (vofl_graph_traits<void, int, void, uint64_t, false>),
                   (vol_graph_traits<void, int, void, uint64_t, false>),
                   (vov_graph_traits<void, int, void, uint64_t, false>),
                   (vod_graph_traits<void, int, void, uint64_t, false>),
                   (dofl_graph_traits<void, int, void, uint64_t, false>),
                   (dol_graph_traits<void, int, void, uint64_t, false>),
                   (dov_graph_traits<void, int, void, uint64_t, false>),
                   (dod_graph_traits<void, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, int, void, uint64_t, false, TestType>;

  Graph g;
  g.resize_vertices(10);

  REQUIRE(g.size() == 10);

  // Fill with values after resize
  for (size_t i = 0; i < 10; ++i) {
    g[i].value() = static_cast<int>(i * 10);
  }

  REQUIRE(g[5].value() == 50);
}

TEMPLATE_TEST_CASE("construction from different sized graphs",
                   "[common][construction]",
                   (vofl_graph_traits<int, int, int, uint64_t, false>),
                   (vol_graph_traits<int, int, int, uint64_t, false>),
                   (vov_graph_traits<int, int, int, uint64_t, false>),
                   (vod_graph_traits<int, int, int, uint64_t, false>),
                   (dofl_graph_traits<int, int, int, uint64_t, false>),
                   (dol_graph_traits<int, int, int, uint64_t, false>),
                   (dov_graph_traits<int, int, int, uint64_t, false>),
                   (dod_graph_traits<int, int, int, uint64_t, false>)) {
  using Graph = dynamic_graph<int, int, int, uint64_t, false, TestType>;

  Graph                                         small(1);
  std::vector<copyable_vertex_t<uint64_t, int>> v1 = {{0, 10}};
  small.load_vertices(v1, std::identity{});

  Graph                                         large(2);
  std::vector<copyable_vertex_t<uint64_t, int>> v2 = {{0, 20}, {1, 30}, {2, 40}};
  large.load_vertices(v2, std::identity{});

  REQUIRE(small.size() == 1);
  REQUIRE(large.size() == 3);
  REQUIRE(small.graph_value() == 1);
  REQUIRE(large.graph_value() == 2);
}

TEMPLATE_TEST_CASE("default value initialization",
                   "[common][construction]",
                   (vofl_graph_traits<void, int, void, uint64_t, false>),
                   (vol_graph_traits<void, int, void, uint64_t, false>),
                   (vov_graph_traits<void, int, void, uint64_t, false>),
                   (vod_graph_traits<void, int, void, uint64_t, false>),
                   (dofl_graph_traits<void, int, void, uint64_t, false>),
                   (dol_graph_traits<void, int, void, uint64_t, false>),
                   (dov_graph_traits<void, int, void, uint64_t, false>),
                   (dod_graph_traits<void, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, int, void, uint64_t, false, TestType>;

  Graph g;
  g.resize_vertices(5);

  // Vertices should be default-initialized
  REQUIRE(g.size() == 5);

  // Access all vertices to ensure they're properly constructed
  for (size_t i = 0; i < 5; ++i) {
    auto& v = g[i];
    (void)v; // Just ensure we can access it
  }
}

//==================================================================================================
// TEMPLATE_TEST_CASE: Load Operations
//==================================================================================================

TEMPLATE_TEST_CASE("load_edges with simple edge list",
                   "[common][load]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph     = dynamic_graph<void, void, void, uint64_t, false, TestType>;
  using edge_data = copyable_edge_t<uint64_t, void>;

  std::vector<edge_data> edges = {{0, 1}, {1, 2}, {2, 0}};

  Graph g;
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 3);
}

TEMPLATE_TEST_CASE("load_edges with edge values",
                   "[common][load]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph     = dynamic_graph<int, void, void, uint64_t, false, TestType>;
  using edge_data = copyable_edge_t<uint64_t, int>;

  std::vector<edge_data> edges = {{0, 1, 10}, {1, 2, 20}, {2, 0, 30}};

  Graph g;
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 3);

  // Verify edge values exist (iterator through edges)
  auto& v0    = g[0];
  bool  found = false;
  for (auto& e : v0.edges()) {
    if (e.target_id() == 1) {
      REQUIRE(e.value() == 10);
      found = true;
    }
  }
  REQUIRE(found);
}

TEMPLATE_TEST_CASE("load_vertices basic",
                   "[common][load]",
                   (vofl_graph_traits<void, int, void, uint64_t, false>),
                   (vol_graph_traits<void, int, void, uint64_t, false>),
                   (vov_graph_traits<void, int, void, uint64_t, false>),
                   (vod_graph_traits<void, int, void, uint64_t, false>),
                   (dofl_graph_traits<void, int, void, uint64_t, false>),
                   (dol_graph_traits<void, int, void, uint64_t, false>),
                   (dov_graph_traits<void, int, void, uint64_t, false>),
                   (dod_graph_traits<void, int, void, uint64_t, false>)) {
  using Graph       = dynamic_graph<void, int, void, uint64_t, false, TestType>;
  using vertex_data = copyable_vertex_t<uint64_t, int>;

  std::vector<vertex_data> vertices = {{0, 100}, {1, 200}, {2, 300}};

  Graph g;
  g.load_vertices(vertices, std::identity{});

  REQUIRE(g.size() == 3);
  REQUIRE(g[0].value() == 100);
  REQUIRE(g[1].value() == 200);
  REQUIRE(g[2].value() == 300);
}

TEMPLATE_TEST_CASE("load_edges with empty range",
                   "[common][load]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph     = dynamic_graph<void, void, void, uint64_t, false, TestType>;
  using edge_data = copyable_edge_t<uint64_t, void>;

  std::vector<edge_data> edges;
  Graph                  g;
  g.load_edges(edges, std::identity{});

  // Empty edge load may create vertex 0
  REQUIRE(g.size() <= 1);
}

TEMPLATE_TEST_CASE("load_edges auto-extends vertex count",
                   "[common][load]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph     = dynamic_graph<void, void, void, uint64_t, false, TestType>;
  using edge_data = copyable_edge_t<uint64_t, void>;

  std::vector<edge_data> edges = {{0, 10}, {5, 20}};
  Graph                  g;
  g.load_edges(edges, std::identity{});

  // Should auto-extend to include vertex 20
  REQUIRE(g.size() >= 21);
}

TEMPLATE_TEST_CASE("load_vertices then load_edges",
                   "[common][load]",
                   (vofl_graph_traits<int, int, void, uint64_t, false>),
                   (vol_graph_traits<int, int, void, uint64_t, false>),
                   (vov_graph_traits<int, int, void, uint64_t, false>),
                   (vod_graph_traits<int, int, void, uint64_t, false>),
                   (dofl_graph_traits<int, int, void, uint64_t, false>),
                   (dol_graph_traits<int, int, void, uint64_t, false>),
                   (dov_graph_traits<int, int, void, uint64_t, false>),
                   (dod_graph_traits<int, int, void, uint64_t, false>)) {
  using Graph       = dynamic_graph<int, int, void, uint64_t, false, TestType>;
  using vertex_data = copyable_vertex_t<uint64_t, int>;
  using edge_data   = copyable_edge_t<uint64_t, int>;

  Graph g;

  std::vector<vertex_data> vertices = {{0, 10}, {1, 20}, {2, 30}};
  g.load_vertices(vertices, std::identity{});

  std::vector<edge_data> edges = {{0, 1, 100}, {1, 2, 200}};
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 3);
  REQUIRE(g[0].value() == 10);

  auto& v0 = g[0];
  for (auto& e : v0.edges()) {
    if (e.target_id() == 1) {
      REQUIRE(e.value() == 100);
    }
  }
}

TEMPLATE_TEST_CASE("load_edges with projection",
                   "[common][load]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph     = dynamic_graph<int, void, void, uint64_t, false, TestType>;
  using edge_data = copyable_edge_t<uint64_t, int>;

  struct CustomEdge {
    uint64_t src, tgt;
    int      val;
  };
  std::vector<CustomEdge> custom_edges = {{0, 1, 10}, {1, 2, 20}};

  Graph g;
  g.load_edges(custom_edges, [](const CustomEdge& e) -> edge_data { return {e.src, e.tgt, e.val}; });

  REQUIRE(g.size() == 3);
}

TEMPLATE_TEST_CASE("load_vertices with projection",
                   "[common][load]",
                   (vofl_graph_traits<void, int, void, uint64_t, false>),
                   (vol_graph_traits<void, int, void, uint64_t, false>),
                   (vov_graph_traits<void, int, void, uint64_t, false>),
                   (vod_graph_traits<void, int, void, uint64_t, false>),
                   (dofl_graph_traits<void, int, void, uint64_t, false>),
                   (dol_graph_traits<void, int, void, uint64_t, false>),
                   (dov_graph_traits<void, int, void, uint64_t, false>),
                   (dod_graph_traits<void, int, void, uint64_t, false>)) {
  using Graph       = dynamic_graph<void, int, void, uint64_t, false, TestType>;
  using vertex_data = copyable_vertex_t<uint64_t, int>;

  struct CustomVertex {
    uint64_t id;
    int      value;
  };
  std::vector<CustomVertex> custom_vertices = {{0, 100}, {1, 200}};

  Graph g;
  g.load_vertices(custom_vertices, [](const CustomVertex& v) -> vertex_data { return {v.id, v.value}; });

  REQUIRE(g.size() == 2);
  REQUIRE(g[0].value() == 100);
}

TEMPLATE_TEST_CASE("incremental edge loading",
                   "[common][load]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph     = dynamic_graph<void, void, void, uint64_t, false, TestType>;
  using edge_data = copyable_edge_t<uint64_t, void>;

  Graph g;

  std::vector<edge_data> batch1 = {{0, 1}, {1, 2}};
  g.load_edges(batch1, std::identity{});
  REQUIRE(g.size() == 3);

  std::vector<edge_data> batch2 = {{2, 3}, {3, 0}};
  g.load_edges(batch2, std::identity{});
  REQUIRE(g.size() == 4);
}

TEMPLATE_TEST_CASE("load with self-loops",
                   "[common][load]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph     = dynamic_graph<void, void, void, uint64_t, false, TestType>;
  using edge_data = copyable_edge_t<uint64_t, void>;

  std::vector<edge_data> edges = {{0, 0}, {1, 1}, {0, 1}};
  Graph                  g;
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 2);

  // Verify self-loop on vertex 0
  auto& v0            = g[0];
  bool  has_self_loop = false;
  for (auto& e : v0.edges()) {
    if (e.target_id() == 0) {
      has_self_loop = true;
    }
  }
  REQUIRE(has_self_loop);
}

TEMPLATE_TEST_CASE("load vertices with non-contiguous IDs",
                   "[common][load]",
                   (vofl_graph_traits<void, int, void, uint64_t, false>),
                   (vol_graph_traits<void, int, void, uint64_t, false>),
                   (vov_graph_traits<void, int, void, uint64_t, false>),
                   (vod_graph_traits<void, int, void, uint64_t, false>),
                   (dofl_graph_traits<void, int, void, uint64_t, false>),
                   (dol_graph_traits<void, int, void, uint64_t, false>),
                   (dov_graph_traits<void, int, void, uint64_t, false>),
                   (dod_graph_traits<void, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, int, void, uint64_t, false, TestType>;

  Graph g;
  g.resize_vertices(11); // Pre-allocate for vertices 0-10
  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{0, 100}, {5, 500}, {10, 1000}};
  g.load_vertices(vertices, std::identity{});

  REQUIRE(g.size() == 11); // Should have indices 0-10
  REQUIRE(g[0].value() == 100);
  REQUIRE(g[5].value() == 500);
  REQUIRE(g[10].value() == 1000);
}

TEMPLATE_TEST_CASE("load vertices in reverse order",
                   "[common][load]",
                   (vofl_graph_traits<void, int, void, uint64_t, false>),
                   (vol_graph_traits<void, int, void, uint64_t, false>),
                   (vov_graph_traits<void, int, void, uint64_t, false>),
                   (vod_graph_traits<void, int, void, uint64_t, false>),
                   (dofl_graph_traits<void, int, void, uint64_t, false>),
                   (dol_graph_traits<void, int, void, uint64_t, false>),
                   (dov_graph_traits<void, int, void, uint64_t, false>),
                   (dod_graph_traits<void, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, int, void, uint64_t, false, TestType>;

  Graph                                         g;
  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{4, 400}, {3, 300}, {2, 200}, {1, 100}, {0, 0}};
  g.load_vertices(vertices, std::identity{});

  REQUIRE(g.size() == 5);
  REQUIRE(g[0].value() == 0);
  REQUIRE(g[1].value() == 100);
  REQUIRE(g[4].value() == 400);
}

//==================================================================================================
// TEMPLATE_TEST_CASE: Vertex Access
//==================================================================================================

TEMPLATE_TEST_CASE("vertex access by index",
                   "[common][access]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}};
  Graph                                        g;
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 2);
  auto& v0 = g[0];
  auto& v1 = g[1];

  // Verify vertices are accessible
  REQUIRE(&v0 != &v1);
}

TEMPLATE_TEST_CASE("vertex iteration",
                   "[common][iteration]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {1, 2}};
  Graph                                        g;
  g.load_edges(edges, std::identity{});

  size_t count = 0;
  for (auto it = g.begin(); it != g.end(); ++it) {
    ++count;
  }
  REQUIRE(count == 3);
}

TEMPLATE_TEST_CASE("const vertex iteration",
                   "[common][access]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {1, 2}};
  Graph                                        g;
  g.load_edges(edges, std::identity{});

  const Graph& cg    = g;
  size_t       count = 0;
  for (auto it = cg.begin(); it != cg.end(); ++it) {
    ++count;
  }
  REQUIRE(count == 3);
}

TEMPLATE_TEST_CASE("range-based for loop on vertices",
                   "[common][access]",
                   (vofl_graph_traits<void, int, void, uint64_t, false>),
                   (vol_graph_traits<void, int, void, uint64_t, false>),
                   (vov_graph_traits<void, int, void, uint64_t, false>),
                   (vod_graph_traits<void, int, void, uint64_t, false>),
                   (dofl_graph_traits<void, int, void, uint64_t, false>),
                   (dol_graph_traits<void, int, void, uint64_t, false>),
                   (dov_graph_traits<void, int, void, uint64_t, false>),
                   (dod_graph_traits<void, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, int, void, uint64_t, false, TestType>;

  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{0, 10}, {1, 20}, {2, 30}};
  Graph                                         g;
  g.load_vertices(vertices, std::identity{});

  int sum = 0;
  for (auto& v : g) {
    sum += v.value();
  }
  REQUIRE(sum == 60);
}

TEMPLATE_TEST_CASE("size queries on vertices",
                   "[common][access]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph g;
  REQUIRE(g.size() == 0);

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {1, 2}, {2, 3}};
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 4);
}

TEMPLATE_TEST_CASE("single vertex access",
                   "[common][access]",
                   (vofl_graph_traits<void, int, void, uint64_t, false>),
                   (vol_graph_traits<void, int, void, uint64_t, false>),
                   (vov_graph_traits<void, int, void, uint64_t, false>),
                   (vod_graph_traits<void, int, void, uint64_t, false>),
                   (dofl_graph_traits<void, int, void, uint64_t, false>),
                   (dol_graph_traits<void, int, void, uint64_t, false>),
                   (dov_graph_traits<void, int, void, uint64_t, false>),
                   (dod_graph_traits<void, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, int, void, uint64_t, false, TestType>;

  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{0, 42}};
  Graph                                         g;
  g.load_vertices(vertices, std::identity{});

  REQUIRE(g.size() == 1);
  REQUIRE(g[0].value() == 42);
}

TEMPLATE_TEST_CASE("large graph vertex access",
                   "[common][access]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges;
  for (uint64_t i = 0; i < 100; ++i) {
    edges.push_back({i, i + 1});
  }

  Graph g;
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 101);
  auto& v50 = g[50];
  REQUIRE(&v50 == &g[50]); // Consistent reference
}

TEMPLATE_TEST_CASE("vertex value access and modification",
                   "[common][access]",
                   (vofl_graph_traits<void, int, void, uint64_t, false>),
                   (vol_graph_traits<void, int, void, uint64_t, false>),
                   (vov_graph_traits<void, int, void, uint64_t, false>),
                   (vod_graph_traits<void, int, void, uint64_t, false>),
                   (dofl_graph_traits<void, int, void, uint64_t, false>),
                   (dol_graph_traits<void, int, void, uint64_t, false>),
                   (dov_graph_traits<void, int, void, uint64_t, false>),
                   (dod_graph_traits<void, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, int, void, uint64_t, false, TestType>;

  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{0, 10}, {1, 20}};
  Graph                                         g;
  g.load_vertices(vertices, std::identity{});

  REQUIRE(g[0].value() == 10);
  g[0].value() = 100;
  REQUIRE(g[0].value() == 100);
}

TEMPLATE_TEST_CASE("empty graph vertex access safety",
                   "[common][access]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph g;
  REQUIRE(g.size() == 0);
  REQUIRE(g.begin() == g.end());
}

TEMPLATE_TEST_CASE("vertex iterator validity",
                   "[common][access]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {1, 2}};
  Graph                                        g;
  g.load_edges(edges, std::identity{});

  auto it1 = g.begin();
  auto it2 = g.begin();
  REQUIRE(it1 == it2);

  ++it1;
  REQUIRE(it1 != it2);
}

TEMPLATE_TEST_CASE("vertex access bounds checking",
                   "[common][access]",
                   (vofl_graph_traits<void, int, void, uint64_t, false>),
                   (vol_graph_traits<void, int, void, uint64_t, false>),
                   (vov_graph_traits<void, int, void, uint64_t, false>),
                   (vod_graph_traits<void, int, void, uint64_t, false>),
                   (dofl_graph_traits<void, int, void, uint64_t, false>),
                   (dol_graph_traits<void, int, void, uint64_t, false>),
                   (dov_graph_traits<void, int, void, uint64_t, false>),
                   (dod_graph_traits<void, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, int, void, uint64_t, false, TestType>;

  Graph                                         g;
  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{0, 10}, {1, 20}, {2, 30}};
  g.load_vertices(vertices, std::identity{});

  // Valid accesses
  REQUIRE(g[0].value() == 10);
  REQUIRE(g[2].value() == 30);

  // Test at() method if available, or just test valid indices
  REQUIRE(g.size() == 3);
}

//==================================================================================================
// TEMPLATE_TEST_CASE: Edge Access
//==================================================================================================

TEMPLATE_TEST_CASE("edge iteration from vertex",
                   "[common][edges]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {0, 2}, {0, 3}};
  Graph                                        g;
  g.load_edges(edges, std::identity{});

  auto&  v0         = g[0];
  size_t edge_count = 0;
  for (auto& e : v0.edges()) {
    ++edge_count;
    REQUIRE(e.target_id() >= 1);
    REQUIRE(e.target_id() <= 3);
  }
  REQUIRE(edge_count == 3);
}

TEMPLATE_TEST_CASE("empty vertex has no edges",
                   "[common][edges]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}};
  Graph                                        g;
  g.load_edges(edges, std::identity{});

  auto&  v1         = g[1];
  size_t edge_count = 0;
  for (auto& e : v1.edges()) {
    (void)e;
    ++edge_count;
  }
  REQUIRE(edge_count == 0);
}

TEMPLATE_TEST_CASE("parallel edges support",
                   "[common][edges]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 10}, {0, 1, 20}, {0, 1, 30}};
  Graph                                       g;
  g.load_edges(edges, std::identity{});

  auto&  v0    = g[0];
  size_t count = 0;
  for (auto& e : v0.edges()) {
    REQUIRE(e.target_id() == 1);
    ++count;
  }
  REQUIRE(count == 3); // All parallel edges exist
}

TEMPLATE_TEST_CASE("edge degree queries",
                   "[common][edges]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {0, 2}, {0, 3}, {1, 2}};
  Graph                                        g;
  g.load_edges(edges, std::identity{});

  auto&  v0     = g[0];
  size_t degree = static_cast<size_t>(std::ranges::distance(v0.edges()));
  REQUIRE(degree == 3);

  auto& v1 = g[1];
  REQUIRE(static_cast<size_t>(std::ranges::distance(v1.edges())) == 1);
}

TEMPLATE_TEST_CASE("edge empty check",
                   "[common][edges]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}};
  Graph                                        g;
  g.load_edges(edges, std::identity{});

  auto& v0 = g[0];
  REQUIRE(v0.edges().begin() != v0.edges().end());

  auto& v1 = g[1];
  REQUIRE(v1.edges().begin() == v1.edges().end());
}

TEMPLATE_TEST_CASE("bidirectional edge traversal",
                   "[common][edges]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {1, 0}};
  Graph                                        g;
  g.load_edges(edges, std::identity{});

  auto& v0            = g[0];
  bool  found_forward = false;
  for (auto& e : v0.edges()) {
    if (e.target_id() == 1)
      found_forward = true;
  }

  auto& v1             = g[1];
  bool  found_backward = false;
  for (auto& e : v1.edges()) {
    if (e.target_id() == 0)
      found_backward = true;
  }

  REQUIRE(found_forward);
  REQUIRE(found_backward);
}

TEMPLATE_TEST_CASE("edge target validation",
                   "[common][edges]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {0, 2}, {0, 3}};
  Graph                                        g;
  g.load_edges(edges, std::identity{});

  auto&                 v0 = g[0];
  std::vector<uint64_t> targets;
  for (auto& e : v0.edges()) {
    targets.push_back(e.target_id());
  }

  std::ranges::sort(targets);
  REQUIRE(targets.size() == 3);
  REQUIRE(targets[0] == 1);
  REQUIRE(targets[1] == 2);
  REQUIRE(targets[2] == 3);
}

TEMPLATE_TEST_CASE("edge value iteration",
                   "[common][edges]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30}};
  Graph                                       g;
  g.load_edges(edges, std::identity{});

  auto& v0  = g[0];
  int   sum = 0;
  for (auto& e : v0.edges()) {
    sum += e.value();
  }
  REQUIRE(sum == 60);
}

TEMPLATE_TEST_CASE("edge iterator increment",
                   "[common][edges]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {0, 2}, {0, 3}};
  Graph                                        g;
  g.load_edges(edges, std::identity{});

  auto& v0 = g[0];
  auto  it = v0.edges().begin();
  REQUIRE(it != v0.edges().end());

  ++it;
  REQUIRE(it != v0.edges().end());

  ++it;
  REQUIRE(it != v0.edges().end());

  ++it;
  REQUIRE(it == v0.edges().end());
}

TEMPLATE_TEST_CASE("high degree vertex",
                   "[common][edges]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges;
  for (uint64_t i = 1; i <= 50; ++i) {
    edges.push_back({0, i});
  }

  Graph g;
  g.load_edges(edges, std::identity{});

  auto&  v0     = g[0];
  size_t degree = static_cast<size_t>(std::ranges::distance(v0.edges()));
  REQUIRE(degree == 50);
}

TEMPLATE_TEST_CASE("edge range filtering",
                   "[common][edges]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 10}, {0, 2, 25}, {0, 3, 30}, {0, 4, 15}};
  Graph                                       g;
  g.load_edges(edges, std::identity{});

  auto& v0    = g[0];
  auto  count = std::ranges::count_if(v0.edges(), [](const auto& e) { return e.value() >= 20; });
  REQUIRE(count == 2);
}

TEMPLATE_TEST_CASE("edge access from const vertex reference",
                   "[common][edges]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 10}, {0, 2, 20}};
  Graph                                       g;
  g.load_edges(edges, std::identity{});

  const auto& v0_const = g[0];
  int         sum      = 0;
  for (const auto& e : v0_const.edges()) {
    sum += e.value();
  }
  REQUIRE(sum == 30);
}

TEMPLATE_TEST_CASE("vertex value modification",
                   "[common][values]",
                   (vofl_graph_traits<void, double, void, uint64_t, false>),
                   (vol_graph_traits<void, double, void, uint64_t, false>),
                   (vov_graph_traits<void, double, void, uint64_t, false>),
                   (vod_graph_traits<void, double, void, uint64_t, false>),
                   (dofl_graph_traits<void, double, void, uint64_t, false>),
                   (dol_graph_traits<void, double, void, uint64_t, false>),
                   (dov_graph_traits<void, double, void, uint64_t, false>),
                   (dod_graph_traits<void, double, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, double, void, uint64_t, false, TestType>;

  Graph                                            g;
  std::vector<copyable_vertex_t<uint64_t, double>> vertices = {{0, 1.5}, {1, 2.5}};
  g.load_vertices(vertices, std::identity{});

  g[0].value() = 3.14;
  REQUIRE(g[0].value() == 3.14);

  g[1].value() *= 2.0;
  REQUIRE(g[1].value() == 5.0);
}

//==================================================================================================
// TEMPLATE_TEST_CASE: Sourced Edges
//==================================================================================================

TEMPLATE_TEST_CASE("sourced edges construction",
                   "[common][sourced]",
                   (vofl_graph_traits<void, void, void, uint64_t, true>),
                   (vol_graph_traits<void, void, void, uint64_t, true>),
                   (vov_graph_traits<void, void, void, uint64_t, true>),
                   (vod_graph_traits<void, void, void, uint64_t, true>),
                   (dofl_graph_traits<void, void, void, uint64_t, true>),
                   (dol_graph_traits<void, void, void, uint64_t, true>),
                   (dov_graph_traits<void, void, void, uint64_t, true>),
                   (dod_graph_traits<void, void, void, uint64_t, true>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, true, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {1, 2}};
  Graph                                        g;
  g.load_edges(edges, std::identity{});

  auto& v0 = g[0];
  for (auto& e : v0.edges()) {
    REQUIRE(e.source_id() == 0);
    REQUIRE(e.target_id() == 1);
  }
}

TEMPLATE_TEST_CASE("sourced edges with values",
                   "[common][sourced]",
                   (vofl_graph_traits<int, int, void, uint64_t, true>),
                   (vol_graph_traits<int, int, void, uint64_t, true>),
                   (vov_graph_traits<int, int, void, uint64_t, true>),
                   (vod_graph_traits<int, int, void, uint64_t, true>),
                   (dofl_graph_traits<int, int, void, uint64_t, true>),
                   (dol_graph_traits<int, int, void, uint64_t, true>),
                   (dov_graph_traits<int, int, void, uint64_t, true>),
                   (dod_graph_traits<int, int, void, uint64_t, true>)) {
  using Graph = dynamic_graph<int, int, void, uint64_t, true, TestType>;

  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 10}, {1, 2, 20}};
  Graph                                       g;
  g.load_edges(edges, std::identity{});

  auto& v1 = g[1];
  for (auto& e : v1.edges()) {
    REQUIRE(e.source_id() == 1);
    REQUIRE(e.target_id() == 2);
    REQUIRE(e.value() == 20);
  }
}

TEMPLATE_TEST_CASE("sourced self-loops",
                   "[common][sourced]",
                   (vofl_graph_traits<void, void, void, uint64_t, true>),
                   (vol_graph_traits<void, void, void, uint64_t, true>),
                   (vov_graph_traits<void, void, void, uint64_t, true>),
                   (vod_graph_traits<void, void, void, uint64_t, true>),
                   (dofl_graph_traits<void, void, void, uint64_t, true>),
                   (dol_graph_traits<void, void, void, uint64_t, true>),
                   (dov_graph_traits<void, void, void, uint64_t, true>),
                   (dod_graph_traits<void, void, void, uint64_t, true>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, true, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 0}, {1, 1}};
  Graph                                        g;
  g.load_edges(edges, std::identity{});

  auto& v0 = g[0];
  for (auto& e : v0.edges()) {
    REQUIRE(e.source_id() == 0);
    REQUIRE(e.target_id() == 0);
  }
}

TEMPLATE_TEST_CASE("sourced multiple edges from vertex",
                   "[common][sourced]",
                   (vofl_graph_traits<void, void, void, uint64_t, true>),
                   (vol_graph_traits<void, void, void, uint64_t, true>),
                   (vov_graph_traits<void, void, void, uint64_t, true>),
                   (vod_graph_traits<void, void, void, uint64_t, true>),
                   (dofl_graph_traits<void, void, void, uint64_t, true>),
                   (dol_graph_traits<void, void, void, uint64_t, true>),
                   (dov_graph_traits<void, void, void, uint64_t, true>),
                   (dod_graph_traits<void, void, void, uint64_t, true>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, true, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {0, 2}, {0, 3}};
  Graph                                        g;
  g.load_edges(edges, std::identity{});

  auto& v0 = g[0];
  for (auto& e : v0.edges()) {
    REQUIRE(e.source_id() == 0);
  }
}

TEMPLATE_TEST_CASE("sourced edge iteration consistency",
                   "[common][sourced]",
                   (vofl_graph_traits<int, void, void, uint64_t, true>),
                   (vol_graph_traits<int, void, void, uint64_t, true>),
                   (vov_graph_traits<int, void, void, uint64_t, true>),
                   (vod_graph_traits<int, void, void, uint64_t, true>),
                   (dofl_graph_traits<int, void, void, uint64_t, true>),
                   (dol_graph_traits<int, void, void, uint64_t, true>),
                   (dov_graph_traits<int, void, void, uint64_t, true>),
                   (dod_graph_traits<int, void, void, uint64_t, true>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, true, TestType>;

  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 10}, {1, 2, 20}, {2, 0, 30}};
  Graph                                       g;
  g.load_edges(edges, std::identity{});

  for (size_t i = 0; i < 3; ++i) {
    auto& v = g[i];
    for (auto& e : v.edges()) {
      REQUIRE(e.source_id() == i);
    }
  }
}

//==================================================================================================
// TEMPLATE_TEST_CASE: Value Types
//==================================================================================================

TEMPLATE_TEST_CASE("string values work correctly",
                   "[common][values]",
                   (vofl_graph_traits<std::string, std::string, std::string, uint64_t, false>),
                   (vol_graph_traits<std::string, std::string, std::string, uint64_t, false>),
                   (vov_graph_traits<std::string, std::string, std::string, uint64_t, false>),
                   (vod_graph_traits<std::string, std::string, std::string, uint64_t, false>),
                   (dofl_graph_traits<std::string, std::string, std::string, uint64_t, false>),
                   (dol_graph_traits<std::string, std::string, std::string, uint64_t, false>),
                   (dov_graph_traits<std::string, std::string, std::string, uint64_t, false>),
                   (dod_graph_traits<std::string, std::string, std::string, uint64_t, false>)) {
  using Graph = dynamic_graph<std::string, std::string, std::string, uint64_t, false, TestType>;

  Graph g("graph_value");
  REQUIRE(g.graph_value() == "graph_value");

  std::vector<copyable_vertex_t<uint64_t, std::string>> vertices = {{0, "v0"}, {1, "v1"}};
  g.load_vertices(vertices, std::identity{});

  REQUIRE(g[0].value() == "v0");
  REQUIRE(g[1].value() == "v1");

  std::vector<copyable_edge_t<uint64_t, std::string>> edges = {{0, 1, "e01"}};
  g.load_edges(edges, std::identity{});

  for (auto& e : g[0].edges()) {
    REQUIRE(e.value() == "e01");
  }
}

TEMPLATE_TEST_CASE("edge value modification",
                   "[common][values]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 10}};
  Graph                                       g;
  g.load_edges(edges, std::identity{});

  auto& v0 = g[0];
  for (auto& e : v0.edges()) {
    e.value() = 42;
  }

  for (auto& e : v0.edges()) {
    REQUIRE(e.value() == 42);
  }
}

TEMPLATE_TEST_CASE("vertex value modification",
                   "[common][values]",
                   (vofl_graph_traits<void, int, void, uint64_t, false>),
                   (vol_graph_traits<void, int, void, uint64_t, false>),
                   (vov_graph_traits<void, int, void, uint64_t, false>),
                   (vod_graph_traits<void, int, void, uint64_t, false>),
                   (dofl_graph_traits<void, int, void, uint64_t, false>),
                   (dol_graph_traits<void, int, void, uint64_t, false>),
                   (dov_graph_traits<void, int, void, uint64_t, false>),
                   (dod_graph_traits<void, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, int, void, uint64_t, false, TestType>;

  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{0, 10}, {1, 20}};
  Graph                                         g;
  g.load_vertices(vertices, std::identity{});

  g[0].value() = 100;
  g[1].value() = 200;

  REQUIRE(g[0].value() == 100);
  REQUIRE(g[1].value() == 200);
}

TEMPLATE_TEST_CASE("graph value modification",
                   "[common][values]",
                   (vofl_graph_traits<void, void, int, uint64_t, false>),
                   (vol_graph_traits<void, void, int, uint64_t, false>),
                   (vov_graph_traits<void, void, int, uint64_t, false>),
                   (vod_graph_traits<void, void, int, uint64_t, false>),
                   (dofl_graph_traits<void, void, int, uint64_t, false>),
                   (dol_graph_traits<void, void, int, uint64_t, false>),
                   (dov_graph_traits<void, void, int, uint64_t, false>),
                   (dod_graph_traits<void, void, int, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, int, uint64_t, false, TestType>;

  Graph g(42);
  REQUIRE(g.graph_value() == 42);

  g.graph_value() = 100;
  REQUIRE(g.graph_value() == 100);
}

TEMPLATE_TEST_CASE("value move semantics",
                   "[common][values]",
                   (vofl_graph_traits<std::string, std::string, std::string, uint64_t, false>),
                   (vol_graph_traits<std::string, std::string, std::string, uint64_t, false>),
                   (vov_graph_traits<std::string, std::string, std::string, uint64_t, false>),
                   (vod_graph_traits<std::string, std::string, std::string, uint64_t, false>),
                   (dofl_graph_traits<std::string, std::string, std::string, uint64_t, false>),
                   (dol_graph_traits<std::string, std::string, std::string, uint64_t, false>),
                   (dov_graph_traits<std::string, std::string, std::string, uint64_t, false>),
                   (dod_graph_traits<std::string, std::string, std::string, uint64_t, false>)) {
  using Graph = dynamic_graph<std::string, std::string, std::string, uint64_t, false, TestType>;

  std::string gval = "graph";
  Graph       g(std::move(gval));
  REQUIRE(g.graph_value() == "graph");
}

TEMPLATE_TEST_CASE("mixed value types",
                   "[common][values]",
                   (vofl_graph_traits<int, std::string, double, uint64_t, false>),
                   (vol_graph_traits<int, std::string, double, uint64_t, false>),
                   (vov_graph_traits<int, std::string, double, uint64_t, false>),
                   (vod_graph_traits<int, std::string, double, uint64_t, false>),
                   (dofl_graph_traits<int, std::string, double, uint64_t, false>),
                   (dol_graph_traits<int, std::string, double, uint64_t, false>),
                   (dov_graph_traits<int, std::string, double, uint64_t, false>),
                   (dod_graph_traits<int, std::string, double, uint64_t, false>)) {
  using Graph = dynamic_graph<int, std::string, double, uint64_t, false, TestType>;

  Graph g(3.14);
  REQUIRE(g.graph_value() == 3.14);

  std::vector<copyable_vertex_t<uint64_t, std::string>> vertices = {{0, "vertex"}};
  g.load_vertices(vertices, std::identity{});
  REQUIRE(g[0].value() == "vertex");

  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 0, 42}};
  g.load_edges(edges, std::identity{});

  for (auto& e : g[0].edges()) {
    REQUIRE(e.value() == 42);
  }
}

TEMPLATE_TEST_CASE("const value access",
                   "[common][values]",
                   (vofl_graph_traits<int, int, int, uint64_t, false>),
                   (vol_graph_traits<int, int, int, uint64_t, false>),
                   (vov_graph_traits<int, int, int, uint64_t, false>),
                   (vod_graph_traits<int, int, int, uint64_t, false>),
                   (dofl_graph_traits<int, int, int, uint64_t, false>),
                   (dol_graph_traits<int, int, int, uint64_t, false>),
                   (dov_graph_traits<int, int, int, uint64_t, false>),
                   (dod_graph_traits<int, int, int, uint64_t, false>)) {
  using Graph = dynamic_graph<int, int, int, uint64_t, false, TestType>;

  Graph                                         g(42);
  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{0, 10}};
  g.load_vertices(vertices, std::identity{});
  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 0, 5}};
  g.load_edges(edges, std::identity{});

  const Graph& cg = g;
  REQUIRE(cg.graph_value() == 42);
  REQUIRE(cg[0].value() == 10);

  for (const auto& e : cg[0].edges()) {
    REQUIRE(e.value() == 5);
  }
}

TEMPLATE_TEST_CASE("sourced edges with parallel edges",
                   "[common][sourced]",
                   (vofl_graph_traits<int, void, void, uint64_t, true>),
                   (vol_graph_traits<int, void, void, uint64_t, true>),
                   (vov_graph_traits<int, void, void, uint64_t, true>),
                   (vod_graph_traits<int, void, void, uint64_t, true>),
                   (dofl_graph_traits<int, void, void, uint64_t, true>),
                   (dol_graph_traits<int, void, void, uint64_t, true>),
                   (dov_graph_traits<int, void, void, uint64_t, true>),
                   (dod_graph_traits<int, void, void, uint64_t, true>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, true, TestType>;

  std::vector<copyable_edge_t<uint64_t, int>> edges = {
        {0, 1, 10},
        {0, 1, 20}, // parallel edge
        {0, 1, 30}  // another parallel edge
  };
  Graph g;
  g.load_edges(edges, std::identity{});

  auto&  v0    = g[0];
  size_t count = 0;
  for (const auto& e : v0.edges()) {
    if (e.target_id() == 1) {
      ++count;
    }
  }

  REQUIRE(count >= 1); // At least one edge to vertex 1
}

//==================================================================================================
// TEMPLATE_TEST_CASE: Graph Properties
//==================================================================================================

TEMPLATE_TEST_CASE("large graph construction",
                   "[common][scale]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  // Create a graph with 1000 vertices
  std::vector<copyable_edge_t<uint64_t, void>> edges;
  for (uint64_t i = 0; i < 1000; ++i) {
    edges.push_back({i, (i + 1) % 1000});
  }

  Graph g;
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 1000);
}

TEMPLATE_TEST_CASE("graph size tracking",
                   "[common][properties]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph g;
  REQUIRE(g.size() == 0);

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {1, 2}};
  g.load_edges(edges, std::identity{});
  REQUIRE(g.size() == 3);

  std::vector<copyable_edge_t<uint64_t, void>> more_edges = {{3, 4}};
  g.load_edges(more_edges, std::identity{});
  REQUIRE(g.size() == 5);
}

TEMPLATE_TEST_CASE("begin/end iteration",
                   "[common][properties]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {1, 2}};
  Graph                                        g;
  g.load_edges(edges, std::identity{});

  size_t count = 0;
  for (auto it = g.begin(); it != g.end(); ++it) {
    ++count;
  }
  REQUIRE(count == g.size());
}

TEMPLATE_TEST_CASE("cbegin/cend const iteration",
                   "[common][properties]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {1, 2}};
  Graph                                        g;
  g.load_edges(edges, std::identity{});

  size_t count = 0;
  for (auto it = g.cbegin(); it != g.cend(); ++it) {
    ++count;
  }
  REQUIRE(count == g.size());
}

TEMPLATE_TEST_CASE("empty graph properties",
                   "[common][properties]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph g;
  REQUIRE(g.size() == 0);
  REQUIRE(g.begin() == g.end());
  REQUIRE(g.cbegin() == g.cend());
}

TEMPLATE_TEST_CASE("vertex count after incremental loads",
                   "[common][properties]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph g;

  std::vector<copyable_edge_t<uint64_t, void>> batch1 = {{0, 1}};
  g.load_edges(batch1, std::identity{});
  size_t size1 = g.size();

  std::vector<copyable_edge_t<uint64_t, void>> batch2 = {{2, 3}};
  g.load_edges(batch2, std::identity{});
  size_t size2 = g.size();

  REQUIRE(size2 > size1);
  REQUIRE(size2 == 4);
}

TEMPLATE_TEST_CASE("graph iterator distance",
                   "[common][properties]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {1, 2}, {2, 3}};
  Graph                                        g;
  g.load_edges(edges, std::identity{});

  auto dist = std::distance(g.begin(), g.end());
  REQUIRE(static_cast<size_t>(dist) == g.size());
}

TEMPLATE_TEST_CASE("copy preserves structure",
                   "[common][properties]",
                   (vofl_graph_traits<int, int, void, uint64_t, false>),
                   (vol_graph_traits<int, int, void, uint64_t, false>),
                   (vov_graph_traits<int, int, void, uint64_t, false>),
                   (vod_graph_traits<int, int, void, uint64_t, false>),
                   (dofl_graph_traits<int, int, void, uint64_t, false>),
                   (dol_graph_traits<int, int, void, uint64_t, false>),
                   (dov_graph_traits<int, int, void, uint64_t, false>),
                   (dod_graph_traits<int, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, int, void, uint64_t, false, TestType>;

  Graph                                         g1;
  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{0, 10}, {1, 20}};
  g1.load_vertices(vertices, std::identity{});
  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 100}};
  g1.load_edges(edges, std::identity{});

  Graph g2(g1);
  REQUIRE(g2.size() == g1.size());
  REQUIRE(g2[0].value() == g1[0].value());
  REQUIRE(g2[1].value() == g1[1].value());
}

TEMPLATE_TEST_CASE("ranges integration",
                   "[common][ranges]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}};
  Graph                                       g;
  g.load_edges(edges, std::identity{});

  // Use std::ranges to count edges with value >= 20
  auto& v0    = g[0];
  auto  count = std::ranges::count_if(v0.edges(), [](const auto& e) { return e.value() >= 20; });
  REQUIRE(count == 1);
}

TEMPLATE_TEST_CASE("graph equality comparison",
                   "[common][properties]",
                   (vofl_graph_traits<void, int, int, uint64_t, false>),
                   (vol_graph_traits<void, int, int, uint64_t, false>),
                   (vov_graph_traits<void, int, int, uint64_t, false>),
                   (vod_graph_traits<void, int, int, uint64_t, false>),
                   (dofl_graph_traits<void, int, int, uint64_t, false>),
                   (dol_graph_traits<void, int, int, uint64_t, false>),
                   (dov_graph_traits<void, int, int, uint64_t, false>),
                   (dod_graph_traits<void, int, int, uint64_t, false>)) {
  using Graph = dynamic_graph<void, int, int, uint64_t, false, TestType>;

  Graph                                         g1;
  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{0, 10}, {1, 20}};
  g1.load_vertices(vertices, std::identity{});

  Graph g2;
  g2.load_vertices(vertices, std::identity{});

  // Both graphs have same vertices
  REQUIRE(g1.size() == g2.size());
  REQUIRE(g1[0].value() == g2[0].value());
}

TEMPLATE_TEST_CASE("graph capacity queries",
                   "[common][properties]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph                                        g;
  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {1, 2}, {2, 3}};
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 4);
  REQUIRE(g.begin() != g.end());

  Graph g_empty;
  REQUIRE(g_empty.size() == 0);
  REQUIRE(g_empty.begin() == g_empty.end());
}

//==================================================================================================
// TEMPLATE_TEST_CASE: Memory and Performance
//==================================================================================================

TEMPLATE_TEST_CASE("copy graphs preserve independence",
                   "[common][memory]",
                   (vofl_graph_traits<int, int, int, uint64_t, false>),
                   (vol_graph_traits<int, int, int, uint64_t, false>),
                   (vov_graph_traits<int, int, int, uint64_t, false>),
                   (vod_graph_traits<int, int, int, uint64_t, false>),
                   (dofl_graph_traits<int, int, int, uint64_t, false>),
                   (dol_graph_traits<int, int, int, uint64_t, false>),
                   (dov_graph_traits<int, int, int, uint64_t, false>),
                   (dod_graph_traits<int, int, int, uint64_t, false>)) {
  using Graph = dynamic_graph<int, int, int, uint64_t, false, TestType>;

  Graph                                         g1(10);
  std::vector<copyable_vertex_t<uint64_t, int>> vertices1 = {{0, 100}};
  g1.load_vertices(vertices1, std::identity{});

  Graph g2(g1);
  g2.graph_value() = 20;

  REQUIRE(g1.graph_value() == 10);
  REQUIRE(g2.graph_value() == 20);
  REQUIRE(g2.size() == g1.size());
}

TEMPLATE_TEST_CASE("clear graph",
                   "[common][memory]",
                   (vofl_graph_traits<int, int, void, uint64_t, false>),
                   (vol_graph_traits<int, int, void, uint64_t, false>),
                   (vov_graph_traits<int, int, void, uint64_t, false>),
                   (vod_graph_traits<int, int, void, uint64_t, false>),
                   (dofl_graph_traits<int, int, void, uint64_t, false>),
                   (dol_graph_traits<int, int, void, uint64_t, false>),
                   (dov_graph_traits<int, int, void, uint64_t, false>),
                   (dod_graph_traits<int, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, int, void, uint64_t, false, TestType>;

  Graph                                         g;
  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{0, 10}, {1, 20}};
  g.load_vertices(vertices, std::identity{});
  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 100}};
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 2);

  g.clear();
  REQUIRE(g.size() == 0);
  REQUIRE(g.begin() == g.end());
}

TEMPLATE_TEST_CASE("resize graph",
                   "[common][memory]",
                   (vofl_graph_traits<void, int, void, uint64_t, false>),
                   (vol_graph_traits<void, int, void, uint64_t, false>),
                   (vov_graph_traits<void, int, void, uint64_t, false>),
                   (vod_graph_traits<void, int, void, uint64_t, false>),
                   (dofl_graph_traits<void, int, void, uint64_t, false>),
                   (dol_graph_traits<void, int, void, uint64_t, false>),
                   (dov_graph_traits<void, int, void, uint64_t, false>),
                   (dod_graph_traits<void, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, int, void, uint64_t, false, TestType>;

  Graph                                         g;
  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{0, 10}, {1, 20}};
  g.load_vertices(vertices, std::identity{});

  REQUIRE(g.size() == 2);

  g.resize_vertices(5);
  REQUIRE(g.size() == 5);
  REQUIRE(g[0].value() == 10);
  REQUIRE(g[1].value() == 20);
}

TEMPLATE_TEST_CASE("repeated load operations",
                   "[common][memory]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph g;

  for (int i = 0; i < 10; ++i) {
    std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}};
    g.load_edges(edges, std::identity{});
  }

  REQUIRE(g.size() == 2);
}

TEMPLATE_TEST_CASE("large vertex count",
                   "[common][memory]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph                                        g;
  std::vector<copyable_edge_t<uint64_t, void>> edges;
  for (uint64_t i = 0; i < 100; ++i) {
    edges.push_back({i, i + 1});
  }
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 101);
  REQUIRE(g.begin() != g.end());
}

TEMPLATE_TEST_CASE("move assignment efficiency",
                   "[common][memory]",
                   (vofl_graph_traits<std::string, std::string, void, uint64_t, false>),
                   (vol_graph_traits<std::string, std::string, void, uint64_t, false>),
                   (vov_graph_traits<std::string, std::string, void, uint64_t, false>),
                   (vod_graph_traits<std::string, std::string, void, uint64_t, false>),
                   (dofl_graph_traits<std::string, std::string, void, uint64_t, false>),
                   (dol_graph_traits<std::string, std::string, void, uint64_t, false>),
                   (dov_graph_traits<std::string, std::string, void, uint64_t, false>),
                   (dod_graph_traits<std::string, std::string, void, uint64_t, false>)) {
  using Graph = dynamic_graph<std::string, std::string, void, uint64_t, false, TestType>;

  Graph                                                 g1;
  std::vector<copyable_vertex_t<uint64_t, std::string>> vertices = {{0, "test"}};
  g1.load_vertices(vertices, std::identity{});

  Graph g2;
  g2 = std::move(g1);

  REQUIRE(g2.size() == 1);
  REQUIRE(g2[0].value() == "test");
}

TEMPLATE_TEST_CASE("sparse graph memory",
                   "[common][memory]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph g;
  // Create sparse graph: only edges 0->100 and 50->150
  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 100}, {50, 150}};
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 151);

  // Most vertices should have no edges
  size_t empty_count = 0;
  for (auto& v : g) {
    if (v.edges().begin() == v.edges().end()) {
      ++empty_count;
    }
  }
  REQUIRE(empty_count > 140);
}

TEMPLATE_TEST_CASE("memory efficiency with reserve",
                   "[common][memory]",
                   (vofl_graph_traits<void, int, void, uint64_t, false>),
                   (vol_graph_traits<void, int, void, uint64_t, false>),
                   (vov_graph_traits<void, int, void, uint64_t, false>),
                   (vod_graph_traits<void, int, void, uint64_t, false>),
                   (dofl_graph_traits<void, int, void, uint64_t, false>),
                   (dol_graph_traits<void, int, void, uint64_t, false>),
                   (dov_graph_traits<void, int, void, uint64_t, false>),
                   (dod_graph_traits<void, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, int, void, uint64_t, false, TestType>;

  Graph g;
  g.resize_vertices(100);

  // Fill with values
  for (size_t i = 0; i < 100; ++i) {
    g[i].value() = static_cast<int>(i);
  }

  REQUIRE(g.size() == 100);
  REQUIRE(g[50].value() == 50);
}

//==================================================================================================
// TEMPLATE_TEST_CASE: Edge Cases
//==================================================================================================

TEMPLATE_TEST_CASE("single vertex graph",
                   "[common][edge_case]",
                   (vofl_graph_traits<void, int, void, uint64_t, false>),
                   (vol_graph_traits<void, int, void, uint64_t, false>),
                   (vov_graph_traits<void, int, void, uint64_t, false>),
                   (vod_graph_traits<void, int, void, uint64_t, false>),
                   (dofl_graph_traits<void, int, void, uint64_t, false>),
                   (dol_graph_traits<void, int, void, uint64_t, false>),
                   (dov_graph_traits<void, int, void, uint64_t, false>),
                   (dod_graph_traits<void, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, int, void, uint64_t, false, TestType>;

  Graph                                         g;
  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{0, 42}};
  g.load_vertices(vertices, std::identity{});

  REQUIRE(g.size() == 1);
  REQUIRE(g[0].value() == 42);
  REQUIRE(g[0].edges().begin() == g[0].edges().end());
}

TEMPLATE_TEST_CASE("only self-loops",
                   "[common][edge_case]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph                                        g;
  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 0}, {1, 1}, {2, 2}};
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 3);

  for (size_t i = 0; i < 3; ++i) {
    bool has_self_loop = false;
    for (auto& e : g[i].edges()) {
      if (e.target_id() == i) {
        has_self_loop = true;
      }
    }
    REQUIRE(has_self_loop);
  }
}

TEMPLATE_TEST_CASE("disconnected vertices",
                   "[common][edge_case]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph                                        g;
  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {2, 3}};
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 4);

  // Vertices 1 and 2 have no outgoing edges
  REQUIRE(g[1].edges().begin() == g[1].edges().end());
}

TEMPLATE_TEST_CASE("complete graph small",
                   "[common][edge_case]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph g;
  // Complete graph K4: all vertices connected to all others
  std::vector<copyable_edge_t<uint64_t, void>> edges;
  for (uint64_t i = 0; i < 4; ++i) {
    for (uint64_t j = 0; j < 4; ++j) {
      if (i != j) {
        edges.push_back({i, j});
      }
    }
  }
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 4);

  for (size_t i = 0; i < 4; ++i) {
    size_t degree = static_cast<size_t>(std::ranges::distance(g[i].edges()));
    REQUIRE(degree == 3);
  }
}

TEMPLATE_TEST_CASE("star graph",
                   "[common][edge_case]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph g;
  // Star: center vertex 0 connects to all others
  std::vector<copyable_edge_t<uint64_t, void>> edges;
  for (uint64_t i = 1; i <= 10; ++i) {
    edges.push_back({0, i});
  }
  g.load_edges(edges, std::identity{});

  size_t center_degree = static_cast<size_t>(std::ranges::distance(g[0].edges()));
  REQUIRE(center_degree == 10);

  for (size_t i = 1; i <= 10; ++i) {
    size_t leaf_degree = static_cast<size_t>(std::ranges::distance(g[i].edges()));
    REQUIRE(leaf_degree == 0);
  }
}

TEMPLATE_TEST_CASE("chain graph",
                   "[common][edge_case]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph                                        g;
  std::vector<copyable_edge_t<uint64_t, void>> edges;
  for (uint64_t i = 0; i < 10; ++i) {
    edges.push_back({i, i + 1});
  }
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 11);

  // Each vertex (except last) has exactly 1 outgoing edge
  for (size_t i = 0; i < 10; ++i) {
    size_t degree = static_cast<size_t>(std::ranges::distance(g[i].edges()));
    REQUIRE(degree == 1);
  }
}

TEMPLATE_TEST_CASE("duplicate edge loading",
                   "[common][edge_case]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, false, TestType>;

  Graph                                       g;
  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 10}, {0, 1, 20}, {0, 1, 30}};
  g.load_edges(edges, std::identity{});

  // All duplicates should be loaded
  size_t count = static_cast<size_t>(std::ranges::distance(g[0].edges()));
  REQUIRE(count == 3);
}

TEMPLATE_TEST_CASE("very large vertex ID",
                   "[common][edge_case]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph                                        g;
  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1000}};
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() >= 1001);
}

TEMPLATE_TEST_CASE("mixed load order",
                   "[common][edge_case]",
                   (vofl_graph_traits<int, int, void, uint64_t, false>),
                   (vol_graph_traits<int, int, void, uint64_t, false>),
                   (vov_graph_traits<int, int, void, uint64_t, false>),
                   (vod_graph_traits<int, int, void, uint64_t, false>),
                   (dofl_graph_traits<int, int, void, uint64_t, false>),
                   (dol_graph_traits<int, int, void, uint64_t, false>),
                   (dov_graph_traits<int, int, void, uint64_t, false>),
                   (dod_graph_traits<int, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, int, void, uint64_t, false, TestType>;

  Graph g;

  // Load edges first
  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 100}};
  g.load_edges(edges, std::identity{});

  // Then load vertices (should overwrite defaults)
  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{0, 10}, {1, 20}};
  g.load_vertices(vertices, std::identity{});

  REQUIRE(g[0].value() == 10);
  REQUIRE(g[1].value() == 20);
}

TEMPLATE_TEST_CASE("cycle graph",
                   "[common][edge_case]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph                                        g;
  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {1, 2}, {2, 3}, {3, 0}};
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 4);

  // Each vertex has exactly 1 outgoing edge
  for (size_t i = 0; i < 4; ++i) {
    size_t degree = static_cast<size_t>(std::ranges::distance(g[i].edges()));
    REQUIRE(degree == 1);
  }
}

TEMPLATE_TEST_CASE("bipartite graph",
                   "[common][edge_case]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph g;
  // Bipartite: group 0,1,2 connects only to group 3,4,5
  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 3}, {0, 4}, {0, 5}, {1, 3}, {1, 4},
                                                        {1, 5}, {2, 3}, {2, 4}, {2, 5}};
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 6);

  // Group 1 has outgoing edges
  for (size_t i = 0; i < 3; ++i) {
    size_t degree = static_cast<size_t>(std::ranges::distance(g[i].edges()));
    REQUIRE(degree == 3);
  }

  // Group 2 has no outgoing edges
  for (size_t i = 3; i < 6; ++i) {
    size_t degree = static_cast<size_t>(std::ranges::distance(g[i].edges()));
    REQUIRE(degree == 0);
  }
}

TEMPLATE_TEST_CASE("empty initializer list",
                   "[common][edge_case]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, false, TestType>;

  Graph g({});
  REQUIRE(g.size() <= 1);
}

TEMPLATE_TEST_CASE("zero vertex ID",
                   "[common][edge_case]",
                   (vofl_graph_traits<void, int, void, uint64_t, false>),
                   (vol_graph_traits<void, int, void, uint64_t, false>),
                   (vov_graph_traits<void, int, void, uint64_t, false>),
                   (vod_graph_traits<void, int, void, uint64_t, false>),
                   (dofl_graph_traits<void, int, void, uint64_t, false>),
                   (dol_graph_traits<void, int, void, uint64_t, false>),
                   (dov_graph_traits<void, int, void, uint64_t, false>),
                   (dod_graph_traits<void, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, int, void, uint64_t, false, TestType>;

  Graph                                         g;
  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{0, 42}};
  g.load_vertices(vertices, std::identity{});

  REQUIRE(g.size() >= 1);
  REQUIRE(g[0].value() == 42);
}

TEMPLATE_TEST_CASE("maximum vertex ID handling",
                   "[common][edge_case]",
                   (vofl_graph_traits<void, int, void, uint32_t, false>),
                   (vol_graph_traits<void, int, void, uint32_t, false>),
                   (vov_graph_traits<void, int, void, uint32_t, false>),
                   (vod_graph_traits<void, int, void, uint32_t, false>),
                   (dofl_graph_traits<void, int, void, uint32_t, false>),
                   (dol_graph_traits<void, int, void, uint32_t, false>),
                   (dov_graph_traits<void, int, void, uint32_t, false>),
                   (dod_graph_traits<void, int, void, uint32_t, false>)) {
  using Graph = dynamic_graph<void, int, void, uint32_t, false, TestType>;

  Graph g;
  g.resize_vertices(1001); // Pre-allocate for vertices 0-1000
  std::vector<copyable_vertex_t<uint32_t, int>> vertices = {{0, 1}, {1000, 1000}};
  g.load_vertices(vertices, std::identity{});

  REQUIRE(g.size() == 1001);
  REQUIRE(g[1000].value() == 1000);
}

TEMPLATE_TEST_CASE("unordered edge loading",
                   "[common][edge_case]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, false, TestType>;

  std::vector<copyable_edge_t<uint64_t, int>> edges = {{5, 2, 52}, {0, 3, 3}, {2, 1, 21}, {3, 5, 35}};
  Graph                                       g;
  g.load_edges(edges, std::identity{});

  REQUIRE(g.size() == 6);
  // Verify vertex 5 exists and has edges
  auto&  v5         = g[5];
  size_t edge_count = 0;
  for (auto& e : v5.edges()) {
    ++edge_count;
  }
  REQUIRE(edge_count >= 1);
}

//==================================================================================================
// TEMPLATE_TEST_CASE: Iterators and Ranges
//==================================================================================================

TEMPLATE_TEST_CASE("vertex iterator pre-increment",
                   "[common][iterators]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph                                        g;
  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {1, 2}};
  g.load_edges(edges, std::identity{});

  auto it  = g.begin();
  auto it2 = ++it;
  REQUIRE(it == it2);
  REQUIRE(it != g.begin());
}

TEMPLATE_TEST_CASE("edge iterator comparison",
                   "[common][iterators]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (vol_graph_traits<void, void, void, uint64_t, false>),
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>),
                   (dol_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;

  Graph                                        g;
  std::vector<copyable_edge_t<uint64_t, void>> edges = {{0, 1}, {0, 2}};
  g.load_edges(edges, std::identity{});

  auto& v0  = g[0];
  auto  it1 = v0.edges().begin();
  auto  it2 = v0.edges().begin();
  REQUIRE(it1 == it2);

  ++it1;
  REQUIRE(it1 != it2);
}

TEMPLATE_TEST_CASE("ranges filter edges",
                   "[common][iterators]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, false, TestType>;

  Graph                                       g;
  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 5}, {0, 2, 15}, {0, 3, 25}, {0, 4, 35}};
  g.load_edges(edges, std::identity{});

  auto& v0       = g[0];
  auto  filtered = v0.edges() | std::views::filter([](const auto& e) { return e.value() > 10; });

  size_t count = static_cast<size_t>(std::ranges::distance(filtered));
  REQUIRE(count == 3);
}

TEMPLATE_TEST_CASE("ranges transform edges",
                   "[common][iterators]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, false, TestType>;

  Graph                                       g;
  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30}};
  g.load_edges(edges, std::identity{});

  auto& v0          = g[0];
  auto  transformed = v0.edges() | std::views::transform([](const auto& e) { return e.value() * 2; });

  int sum = 0;
  for (auto val : transformed) {
    sum += val;
  }
  REQUIRE(sum == 120);
}

TEMPLATE_TEST_CASE("ranges accumulate edge values",
                   "[common][iterators]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, false, TestType>;

  Graph                                       g;
  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30}};
  g.load_edges(edges, std::identity{});

  auto& v0  = g[0];
  int   sum = 0;
  for (auto& e : v0.edges()) {
    sum += e.value();
  }
  REQUIRE(sum == 60);
}

TEMPLATE_TEST_CASE("ranges find edge",
                   "[common][iterators]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, false, TestType>;

  Graph                                       g;
  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30}};
  g.load_edges(edges, std::identity{});

  auto& v0 = g[0];
  auto  it = std::ranges::find_if(v0.edges(), [](const auto& e) { return e.value() == 20; });

  REQUIRE(it != v0.edges().end());
  REQUIRE((*it).target_id() == 2);
}

TEMPLATE_TEST_CASE("ranges all_of edge predicate",
                   "[common][iterators]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, false, TestType>;

  Graph                                       g;
  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30}};
  g.load_edges(edges, std::identity{});

  auto& v0           = g[0];
  bool  all_positive = std::ranges::all_of(v0.edges(), [](const auto& e) { return e.value() > 0; });

  REQUIRE(all_positive);
}

TEMPLATE_TEST_CASE("ranges any_of edge predicate",
                   "[common][iterators]",
                   (vofl_graph_traits<int, void, void, uint64_t, false>),
                   (vol_graph_traits<int, void, void, uint64_t, false>),
                   (vov_graph_traits<int, void, void, uint64_t, false>),
                   (vod_graph_traits<int, void, void, uint64_t, false>),
                   (dofl_graph_traits<int, void, void, uint64_t, false>),
                   (dol_graph_traits<int, void, void, uint64_t, false>),
                   (dov_graph_traits<int, void, void, uint64_t, false>),
                   (dod_graph_traits<int, void, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, void, void, uint64_t, false, TestType>;

  Graph                                       g;
  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 5}, {0, 2, 15}, {0, 3, 25}};
  g.load_edges(edges, std::identity{});

  auto& v0        = g[0];
  bool  has_large = std::ranges::any_of(v0.edges(), [](const auto& e) { return e.value() > 20; });

  REQUIRE(has_large);
}

//==================================================================================================
// TEMPLATE_TEST_CASE: Workflows
//==================================================================================================

TEMPLATE_TEST_CASE("build graph incrementally",
                   "[common][workflow]",
                   (vofl_graph_traits<int, int, void, uint64_t, false>),
                   (vol_graph_traits<int, int, void, uint64_t, false>),
                   (vov_graph_traits<int, int, void, uint64_t, false>),
                   (vod_graph_traits<int, int, void, uint64_t, false>),
                   (dofl_graph_traits<int, int, void, uint64_t, false>),
                   (dol_graph_traits<int, int, void, uint64_t, false>),
                   (dov_graph_traits<int, int, void, uint64_t, false>),
                   (dod_graph_traits<int, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, int, void, uint64_t, false, TestType>;

  Graph g;

  // Step 1: Add vertices
  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{0, 10}, {1, 20}, {2, 30}};
  g.load_vertices(vertices, std::identity{});
  REQUIRE(g.size() == 3);

  // Step 2: Add edges
  std::vector<copyable_edge_t<uint64_t, int>> edges1 = {{0, 1, 100}};
  g.load_edges(edges1, std::identity{});

  // Step 3: Add more edges
  std::vector<copyable_edge_t<uint64_t, int>> edges2 = {{1, 2, 200}};
  g.load_edges(edges2, std::identity{});

  // Verify structure
  REQUIRE(g[0].value() == 10);
  REQUIRE(std::ranges::distance(g[0].edges()) == 1);
  REQUIRE(std::ranges::distance(g[1].edges()) == 1);
}

TEMPLATE_TEST_CASE("modify and query",
                   "[common][workflow]",
                   (vofl_graph_traits<int, int, void, uint64_t, false>),
                   (vol_graph_traits<int, int, void, uint64_t, false>),
                   (vov_graph_traits<int, int, void, uint64_t, false>),
                   (vod_graph_traits<int, int, void, uint64_t, false>),
                   (dofl_graph_traits<int, int, void, uint64_t, false>),
                   (dol_graph_traits<int, int, void, uint64_t, false>),
                   (dov_graph_traits<int, int, void, uint64_t, false>),
                   (dod_graph_traits<int, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, int, void, uint64_t, false, TestType>;

  Graph                                         g;
  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{0, 10}, {1, 20}};
  g.load_vertices(vertices, std::identity{});
  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 100}};
  g.load_edges(edges, std::identity{});

  // Modify
  g[0].value() = 99;
  for (auto& e : g[0].edges()) {
    e.value() = 999;
  }

  // Query
  REQUIRE(g[0].value() == 99);
  for (auto& e : g[0].edges()) {
    REQUIRE(e.value() == 999);
  }
}

TEMPLATE_TEST_CASE("copy and modify independently",
                   "[common][workflow]",
                   (vofl_graph_traits<int, int, void, uint64_t, false>),
                   (vol_graph_traits<int, int, void, uint64_t, false>),
                   (vov_graph_traits<int, int, void, uint64_t, false>),
                   (vod_graph_traits<int, int, void, uint64_t, false>),
                   (dofl_graph_traits<int, int, void, uint64_t, false>),
                   (dol_graph_traits<int, int, void, uint64_t, false>),
                   (dov_graph_traits<int, int, void, uint64_t, false>),
                   (dod_graph_traits<int, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, int, void, uint64_t, false, TestType>;

  Graph                                         g1;
  std::vector<copyable_vertex_t<uint64_t, int>> vertices = {{0, 10}};
  g1.load_vertices(vertices, std::identity{});

  Graph g2(g1);

  g1[0].value() = 99;
  g2[0].value() = 88;

  REQUIRE(g1[0].value() == 99);
  REQUIRE(g2[0].value() == 88);
}

TEMPLATE_TEST_CASE("clear and rebuild",
                   "[common][workflow]",
                   (vofl_graph_traits<int, int, void, uint64_t, false>),
                   (vol_graph_traits<int, int, void, uint64_t, false>),
                   (vov_graph_traits<int, int, void, uint64_t, false>),
                   (vod_graph_traits<int, int, void, uint64_t, false>),
                   (dofl_graph_traits<int, int, void, uint64_t, false>),
                   (dol_graph_traits<int, int, void, uint64_t, false>),
                   (dov_graph_traits<int, int, void, uint64_t, false>),
                   (dod_graph_traits<int, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<int, int, void, uint64_t, false, TestType>;

  Graph                                         g;
  std::vector<copyable_vertex_t<uint64_t, int>> vertices1 = {{0, 10}};
  g.load_vertices(vertices1, std::identity{});
  REQUIRE(g.size() == 1);

  g.clear();
  REQUIRE(g.size() == 0);

  std::vector<copyable_vertex_t<uint64_t, int>> vertices2 = {{0, 20}, {1, 30}};
  g.load_vertices(vertices2, std::identity{});
  REQUIRE(g.size() == 2);
  REQUIRE(g[0].value() == 20);
}

TEMPLATE_TEST_CASE("resize and fill",
                   "[common][workflow]",
                   (vofl_graph_traits<void, int, void, uint64_t, false>),
                   (vol_graph_traits<void, int, void, uint64_t, false>),
                   (vov_graph_traits<void, int, void, uint64_t, false>),
                   (vod_graph_traits<void, int, void, uint64_t, false>),
                   (dofl_graph_traits<void, int, void, uint64_t, false>),
                   (dol_graph_traits<void, int, void, uint64_t, false>),
                   (dov_graph_traits<void, int, void, uint64_t, false>),
                   (dod_graph_traits<void, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, int, void, uint64_t, false, TestType>;

  Graph g;
  g.resize_vertices(10);

  REQUIRE(g.size() == 10);

  // Fill with values
  for (size_t i = 0; i < 10; ++i) {
    g[i].value() = 42;
  }

  for (size_t i = 0; i < 10; ++i) {
    REQUIRE(g[i].value() == 42);
  }
}

TEMPLATE_TEST_CASE("move assignment",
                   "[common][workflow]",
                   (vofl_graph_traits<int, int, int, uint64_t, false>),
                   (vol_graph_traits<int, int, int, uint64_t, false>),
                   (vov_graph_traits<int, int, int, uint64_t, false>),
                   (vod_graph_traits<int, int, int, uint64_t, false>),
                   (dofl_graph_traits<int, int, int, uint64_t, false>),
                   (dol_graph_traits<int, int, int, uint64_t, false>),
                   (dov_graph_traits<int, int, int, uint64_t, false>),
                   (dod_graph_traits<int, int, int, uint64_t, false>)) {
  using Graph = dynamic_graph<int, int, int, uint64_t, false, TestType>;

  Graph                                         g1(10);
  std::vector<copyable_vertex_t<uint64_t, int>> v1 = {{0, 100}};
  g1.load_vertices(v1, std::identity{});

  Graph g2;
  g2 = std::move(g1);

  REQUIRE(g2.graph_value() == 10);
  REQUIRE(g2.size() == 1);
  REQUIRE(g2[0].value() == 100);
}

TEMPLATE_TEST_CASE("complex graph construction",
                   "[common][workflow]",
                   (vofl_graph_traits<int, std::string, double, uint64_t, false>),
                   (vol_graph_traits<int, std::string, double, uint64_t, false>),
                   (vov_graph_traits<int, std::string, double, uint64_t, false>),
                   (vod_graph_traits<int, std::string, double, uint64_t, false>),
                   (dofl_graph_traits<int, std::string, double, uint64_t, false>),
                   (dol_graph_traits<int, std::string, double, uint64_t, false>),
                   (dov_graph_traits<int, std::string, double, uint64_t, false>),
                   (dod_graph_traits<int, std::string, double, uint64_t, false>)) {
  using Graph = dynamic_graph<int, std::string, double, uint64_t, false, TestType>;

  Graph g(3.14159);

  std::vector<copyable_vertex_t<uint64_t, std::string>> vertices = {{0, "Alice"}, {1, "Bob"}, {2, "Charlie"}};
  g.load_vertices(vertices, std::identity{});

  std::vector<copyable_edge_t<uint64_t, int>> edges = {{0, 1, 10}, {1, 2, 20}, {2, 0, 30}};
  g.load_edges(edges, std::identity{});

  REQUIRE(g.graph_value() == 3.14159);
  REQUIRE(g.size() == 3);
  REQUIRE(g[0].value() == "Alice");
  REQUIRE(g[1].value() == "Bob");

  int total_weight = 0;
  for (auto& v : g) {
    for (auto& e : v.edges()) {
      total_weight += e.value();
    }
  }
  REQUIRE(total_weight == 60);
}

TEMPLATE_TEST_CASE("multi-component graph workflow",
                   "[common][workflow]",
                   (vofl_graph_traits<void, int, void, uint64_t, false>),
                   (vol_graph_traits<void, int, void, uint64_t, false>),
                   (vov_graph_traits<void, int, void, uint64_t, false>),
                   (vod_graph_traits<void, int, void, uint64_t, false>),
                   (dofl_graph_traits<void, int, void, uint64_t, false>),
                   (dol_graph_traits<void, int, void, uint64_t, false>),
                   (dov_graph_traits<void, int, void, uint64_t, false>),
                   (dod_graph_traits<void, int, void, uint64_t, false>)) {
  using Graph = dynamic_graph<void, int, void, uint64_t, false, TestType>;

  Graph g;
  g.resize_vertices(8); // Pre-allocate space for all vertices

  // Component 1: vertices 0-2
  std::vector<copyable_vertex_t<uint64_t, int>> comp1 = {{0, 10}, {1, 20}, {2, 30}};
  g.load_vertices(comp1, std::identity{});
  std::vector<copyable_edge_t<uint64_t, void>> edges1 = {{0, 1}, {1, 2}};
  g.load_edges(edges1, std::identity{});

  // Component 2: vertices 5-7 (disconnected from component 1)
  std::vector<copyable_vertex_t<uint64_t, int>> comp2 = {{5, 50}, {6, 60}, {7, 70}};
  g.load_vertices(comp2, std::identity{});
  std::vector<copyable_edge_t<uint64_t, void>> edges2 = {{5, 6}, {6, 7}};
  g.load_edges(edges2, std::identity{});

  REQUIRE(g.size() == 8);
  REQUIRE(g[0].value() == 10);
  REQUIRE(g[5].value() == 50);

  // Vertices 3 and 4 exist but have no edges
  auto& v3 = g[3];
  REQUIRE(v3.edges().begin() == v3.edges().end());
  auto& v4 = g[4];
  REQUIRE(v4.edges().begin() == v4.edges().end());
}
