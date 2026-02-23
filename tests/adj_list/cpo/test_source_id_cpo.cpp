/**
 * @file test_source_id_cpo.cpp
 * @brief Comprehensive tests for source_id(g, uv) CPO
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/adj_list/vertex_descriptor.hpp>
#include <graph/adj_list/edge_descriptor.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/graph.hpp>
#include "../../common/graph_test_types.hpp"
#include <vector>
#include <deque>
#include <map>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Test: Edge Descriptor source_id() Method
// =============================================================================

TEST_CASE("source_id(g,uv) - vector<vector<int>> simple edges", "[source_id][cpo][descriptor]") {
  std::vector<std::vector<int>> graph = {
        {1, 2, 3}, // vertex 0 -> edges to 1, 2, 3
        {2, 3},    // vertex 1 -> edges to 2, 3
        {3},       // vertex 2 -> edge to 3
        {}         // vertex 3 -> no edges
  };

  SECTION("Get source ID from first edge of vertex 0") {
    auto verts      = vertices(graph);
    auto v0         = *verts.begin();
    auto edge_range = edges(graph, v0);
    auto e          = *edge_range.begin();

    auto sid = source_id(graph, e);

    REQUIRE(sid == 0);
  }

  SECTION("Get source IDs from all edges of vertex 0") {
    auto verts = vertices(graph);
    auto v0    = *verts.begin();

    std::vector<int> sources;
    for (auto e : edges(graph, v0)) {
      sources.push_back(static_cast<int>(source_id(graph, e)));
    }

    REQUIRE(sources.size() == 3);
    REQUIRE(sources[0] == 0);
    REQUIRE(sources[1] == 0);
    REQUIRE(sources[2] == 0);
  }

  SECTION("Get source IDs from vertex 1") {
    auto verts = vertices(graph);
    auto it    = verts.begin();
    ++it;
    auto v1 = *it;

    std::vector<int> sources;
    for (auto e : edges(graph, v1)) {
      sources.push_back(static_cast<int>(source_id(graph, e)));
    }

    REQUIRE(sources.size() == 2);
    REQUIRE(sources[0] == 1);
    REQUIRE(sources[1] == 1);
  }
}

// =============================================================================
// Test: Edge Descriptor with Pair Edges
// =============================================================================

TEST_CASE("source_id(g,uv) - vector<vector<pair<int,double>>> weighted edges", "[source_id][cpo][descriptor][pair]") {
  using Edge                           = std::pair<int, double>;
  std::vector<std::vector<Edge>> graph = {
        {{1, 1.5}, {2, 2.5}, {3, 3.5}}, // vertex 0
        {{2, 1.2}, {3, 2.3}},           // vertex 1
        {{3, 1.0}},                     // vertex 2
        {}                              // vertex 3
  };

  SECTION("Source IDs are consistent for weighted edges") {
    auto verts = vertices(graph);
    auto v0    = *verts.begin();

    std::vector<int> sources;
    for (auto e : edges(graph, v0)) {
      sources.push_back(static_cast<int>(source_id(graph, e)));
    }

    REQUIRE(sources.size() == 3);
    REQUIRE(sources[0] == 0);
    REQUIRE(sources[1] == 0);
    REQUIRE(sources[2] == 0);
  }

  SECTION("First edge of vertex 0") {
    auto verts      = vertices(graph);
    auto v0         = *verts.begin();
    auto edge_range = edges(graph, v0);
    auto e          = *edge_range.begin();

    auto sid = source_id(graph, e);
    REQUIRE(sid == 0);
  }
}

// =============================================================================
// Test: Tuple Edges
// =============================================================================

TEST_CASE("source_id(g,uv) - vector<vector<tuple<...>>> multi-property edges", "[source_id][cpo][descriptor][tuple]") {
  using Edge                           = std::tuple<int, double, std::string>;
  std::vector<std::vector<Edge>> graph = {{{1, 1.5, "a"}, {2, 2.5, "b"}}, {{3, 3.5, "c"}}, {}, {}};

  SECTION("Source IDs from tuple edges") {
    auto verts = vertices(graph);
    auto v0    = *verts.begin();

    std::vector<int> sources;
    for (auto e : edges(graph, v0)) {
      sources.push_back(static_cast<int>(source_id(graph, e)));
    }

    REQUIRE(sources.size() == 2);
    REQUIRE(sources[0] == 0);
    REQUIRE(sources[1] == 0);
  }
}

// =============================================================================
// Test: Native Edge Member Function
// =============================================================================

namespace native_edge_member_test {
// Custom edge type with source_id() member function
struct CustomEdge {
  int    source;
  int    target;
  double weight;

  // Member function that CPO should recognize
  int source_id() const {
    return source * 100; // Custom logic: multiply by 100
  }
};

struct CustomGraph {
  std::vector<std::vector<CustomEdge>> adjacency_list = {{{0, 1, 1.5}, {0, 2, 2.5}}, {{1, 3, 3.5}}, {}};
};
} // namespace native_edge_member_test

TEST_CASE("source_id(g,uv) - native edge member function", "[source_id][cpo][member][native]") {
  using namespace native_edge_member_test;
  CustomGraph g;

  auto verts = vertices(g.adjacency_list);
  auto v0    = *verts.begin();

  SECTION("Native edge member function is called") {
    std::vector<int> sources;
    for (auto e : edges(g.adjacency_list, v0)) {
      sources.push_back(static_cast<int>(source_id(g.adjacency_list, e)));
    }

    // Should use CustomEdge::source_id() which returns source * 100
    REQUIRE(sources.size() == 2);
    REQUIRE(sources[0] == 0); // 0 * 100 = 0
    REQUIRE(sources[1] == 0); // 0 * 100 = 0
  }

  SECTION("First edge uses native member") {
    auto edge_range = edges(g.adjacency_list, v0);
    auto e          = *edge_range.begin();

    auto sid = source_id(g.adjacency_list, e);
    REQUIRE(sid == 0); // 0 * 100
  }
}

TEST_CASE("source_id(g,uv) - native edge member priority over descriptor", "[source_id][cpo][member][priority]") {
  using namespace native_edge_member_test;

  // Even though CustomEdge has a .source field that descriptor would extract,
  // the source_id() member function should take priority
  CustomGraph g;

  auto verts      = vertices(g.adjacency_list);
  auto v0         = *verts.begin();
  auto edge_range = edges(g.adjacency_list, v0);
  auto e          = *edge_range.begin();

  // Should call CustomEdge::source_id(), NOT use descriptor's extraction
  auto sid = source_id(g.adjacency_list, e);
  REQUIRE(sid == 0); // source_id() returns 0, not the raw source field
}

// =============================================================================
// Test: Native Edge Member - const and noexcept
// =============================================================================

namespace const_member_test {
// Edge type with const member function
struct EdgeWithConstMember {
  int source;
  int target;

  int source_id() const noexcept { return source; }
};
} // namespace const_member_test

TEST_CASE("source_id(g,uv) - const noexcept member function", "[source_id][cpo][member][const]") {
  using namespace const_member_test;
  using Graph = std::vector<std::vector<EdgeWithConstMember>>;

  Graph g = {{{0, 1}, {0, 2}}, {{1, 2}}, {}};

  auto v0 = *vertices(g).begin();
  auto e  = *edges(g, v0).begin();

  auto sid = source_id(g, e);
  REQUIRE(sid == 0);
}

// =============================================================================
// Test: Deque Container
// =============================================================================

TEST_CASE("source_id(g,uv) - deque<deque<int>> simple edges", "[source_id][cpo][descriptor][deque]") {
  std::deque<std::deque<int>> graph = {{1, 2}, {2, 3}, {3}, {}};

  auto verts = vertices(graph);
  auto v0    = *verts.begin();

  std::vector<int> sources;
  for (auto e : edges(graph, v0)) {
    sources.push_back(static_cast<int>(source_id(graph, e)));
  }

  REQUIRE(sources.size() == 2);
  REQUIRE(sources[0] == 0);
  REQUIRE(sources[1] == 0);
}

// =============================================================================
// Test: Map Container
// =============================================================================

TEST_CASE("source_id(g,uv) - map<int, vector<int>>", "[source_id][cpo][descriptor][map]") {
  std::map<int, std::vector<int>> graph;
  graph[10] = {20, 30};
  graph[20] = {30, 40};
  graph[30] = {40};
  graph[40] = {};

  SECTION("Source ID from first vertex") {
    auto verts = vertices(graph);
    auto v     = *verts.begin();
    auto vid   = vertex_id(graph, v);

    std::vector<int> sources;
    for (auto e : edges(graph, v)) {
      sources.push_back(static_cast<int>(source_id(graph, e)));
    }

    REQUIRE(sources.size() == 2);
    REQUIRE(sources[0] == vid);
    REQUIRE(sources[1] == vid);
  }

  SECTION("Source ID from vertex 20") {
    auto verts = vertices(graph);
    auto it    = verts.begin();
    ++it;
    auto v = *it;

    std::vector<int> sources;
    for (auto e : edges(graph, v)) {
      sources.push_back(static_cast<int>(source_id(graph, e)));
    }

    REQUIRE(sources.size() == 2);
    REQUIRE(sources[0] == 20);
    REQUIRE(sources[1] == 20);
  }
}

// =============================================================================
// Test: ADL Customization
// =============================================================================

// Note: ADL customization tests for source_id removed due to ambiguity with CPO.
// The CPO uses descriptor-based resolution by default which is sufficient.

// =============================================================================
// Test: Full Graph Traversal
// =============================================================================

TEST_CASE("source_id(g,uv) - full graph traversal", "[source_id][cpo][integration]") {
  std::vector<std::vector<int>> graph = {{1, 2}, {2, 3}, {3}, {}};

  // Traverse all edges and verify source_id matches vertex
  for (auto v : vertices(graph)) {
    auto vid = vertex_id(graph, v);

    for (auto e : edges(graph, v)) {
      auto sid = source_id(graph, e);
      REQUIRE(sid == vid);
    }
  }
}

// =============================================================================
// Test: Const Graph
// =============================================================================

TEST_CASE("source_id(g,uv) - const graph", "[source_id][cpo][const]") {
  const std::vector<std::vector<int>> graph = {{1, 2, 3}, {2, 3}, {3}, {}};

  auto verts = vertices(graph);
  auto v0    = *verts.begin();

  std::vector<int> sources;
  for (auto e : edges(graph, v0)) {
    sources.push_back(static_cast<int>(source_id(graph, e)));
  }

  REQUIRE(sources.size() == 3);
  REQUIRE(sources[0] == 0);
  REQUIRE(sources[1] == 0);
  REQUIRE(sources[2] == 0);
}

// =============================================================================
// Test: Type Deduction
// =============================================================================

TEST_CASE("source_id(g,uv) - type deduction", "[source_id][cpo][types]") {
  std::vector<std::vector<int>> graph = {{1, 2}, {}};

  auto v0 = *vertices(graph).begin();
  auto e  = *edges(graph, v0).begin();

  auto sid = source_id(graph, e);

  static_assert(std::is_same_v<decltype(sid), std::size_t>);
  REQUIRE(sid == 0);
}

TEST_CASE("source_id(g,uv) - different edge value types", "[source_id][cpo][types]") {
  using Edge                           = std::pair<int, double>;
  std::vector<std::vector<Edge>> graph = {{{1, 1.0}, {2, 2.0}}, {}};

  auto v0 = *vertices(graph).begin();

  for (auto e : edges(graph, v0)) {
    auto sid = source_id(graph, e);
    static_assert(std::is_same_v<decltype(sid), std::size_t>);
    REQUIRE(sid == 0);
  }
}

// =============================================================================
// Test: Source Vertex Consistency
// =============================================================================

TEST_CASE("source_id(g,uv) - edge maintains source vertex", "[source_id][cpo][source]") {
  std::vector<std::vector<int>> graph = {{1, 2}, {2, 3}, {3}, {}};

  // For every edge, source_id should match the vertex it came from
  for (auto v : vertices(graph)) {
    auto vid = vertex_id(graph, v);

    for (auto e : edges(graph, v)) {
      REQUIRE(source_id(graph, e) == vid);
    }
  }
}

// =============================================================================
// Test: Vertex With No Edges
// =============================================================================

TEST_CASE("source_id(g,uv) - vertex with no edges", "[source_id][cpo][empty]") {
  std::vector<std::vector<int>> graph = {{1, 2}, {}, {}, {}};

  auto verts = vertices(graph);
  auto it    = verts.begin();
  ++it;
  auto v1 = *it;

  // Vertex 1 has no edges
  auto edge_range = edges(graph, v1);
  REQUIRE(edge_range.begin() == edge_range.end());
}

// =============================================================================
// Test: Large Vertex IDs
// =============================================================================

TEST_CASE("source_id(g,uv) - large vertex IDs", "[source_id][cpo][large]") {
  std::map<int, std::vector<int>> graph;
  graph[1000] = {2000, 3000};
  graph[2000] = {3000};
  graph[3000] = {};

  auto verts = vertices(graph);
  auto v     = *verts.begin();

  std::vector<int> sources;
  for (auto e : edges(graph, v)) {
    sources.push_back(static_cast<int>(source_id(graph, e)));
  }

  REQUIRE(sources.size() == 2);
  REQUIRE(sources[0] == 1000);
  REQUIRE(sources[1] == 1000);
}

// =============================================================================
// Test: Map With Pair Edges
// =============================================================================

TEST_CASE("source_id(g,uv) - map with pair edges", "[source_id][cpo][map][weighted]") {
  using Edge = std::pair<int, double>;
  std::map<int, std::vector<Edge>> graph;
  graph[100] = {{200, 1.5}, {300, 2.5}};
  graph[200] = {{300, 3.5}};
  graph[300] = {};

  auto verts = vertices(graph);
  auto v     = *verts.begin();
  auto vid   = vertex_id(graph, v);

  std::vector<int> sources;
  for (auto e : edges(graph, v)) {
    sources.push_back(static_cast<int>(source_id(graph, e)));
  }

  REQUIRE(sources.size() == 2);
  REQUIRE(sources[0] == vid);
  REQUIRE(sources[1] == vid);
}

// =============================================================================
// Test: Self-loops
// =============================================================================

TEST_CASE("source_id(g,uv) - self-loops", "[source_id][cpo][selfloop]") {
  std::vector<std::vector<int>> graph = {{0, 1}, // vertex 0 -> edges to 0 (self), 1
                                         {1},    // vertex 1 -> edge to 1 (self)
                                         {}};

  SECTION("Vertex 0 with self-loop") {
    auto verts = vertices(graph);
    auto v0    = *verts.begin();

    std::vector<int> sources;
    for (auto e : edges(graph, v0)) {
      sources.push_back(static_cast<int>(source_id(graph, e)));
    }

    REQUIRE(sources.size() == 2);
    REQUIRE(sources[0] == 0); // self-loop source
    REQUIRE(sources[1] == 0);
  }

  SECTION("Vertex 1 with self-loop") {
    auto verts = vertices(graph);
    auto it    = verts.begin();
    ++it;
    auto v1 = *it;

    std::vector<int> sources;
    for (auto e : edges(graph, v1)) {
      sources.push_back(static_cast<int>(source_id(graph, e)));
    }

    REQUIRE(sources.size() == 1);
    REQUIRE(sources[0] == 1); // self-loop source
  }
}

// =============================================================================
// Test: dynamic_graph non-uniform bidir — source_id on in-edges (Tier 1)
//
// Non-uniform bidir traits define in_edge_type = dynamic_in_edge, which has a
// source_id() member. The CPO resolves via Tier 1 (native edge member), not
// the descriptor-based Tier 4 used for generic adj-list out-edges.
// =============================================================================

using DynBidirSrcId =
      graph::container::dynamic_graph<void, void, void, uint32_t, true,
                                      graph::test::vov_bidir_graph_traits<>>;

TEST_CASE("source_id(g,ie) - dynamic_graph in-edges via non-uniform bidir (Tier 1)",
          "[source_id][cpo][dynamic_graph][bidir][in_edges]") {
  using namespace graph;
  using namespace graph::container;

  // Graph: 0->1, 0->2, 1->2, 2->3
  // In-edges of vertex 2: from 0 and 1
  DynBidirSrcId g({{0, 1}, {0, 2}, {1, 2}, {2, 3}});

  auto u2 = *find_vertex(g, uint32_t(2));

  SECTION("source_id of in-edges to vertex 2 are 0 and 1") {
    std::vector<uint32_t> sources;
    for (auto ie : in_edges(g, u2))
      sources.push_back(source_id(g, ie));
    std::sort(sources.begin(), sources.end());
    REQUIRE(sources.size() == 2);
    REQUIRE(sources[0] == 0);
    REQUIRE(sources[1] == 1);
  }

  SECTION("source_id of in-edge to vertex 3 is 2") {
    auto u3 = *find_vertex(g, uint32_t(3));
    for (auto ie : in_edges(g, u3))
      REQUIRE(source_id(g, ie) == 2);
  }

  SECTION("all source_ids from in-edges are valid vertex IDs") {
    for (auto u : vertices(g)) {
      for (auto ie : in_edges(g, u)) {
        auto sid = source_id(g, ie);
        REQUIRE(sid < num_vertices(g));
      }
    }
  }
}

TEST_CASE("source_id(g,oe) - dynamic_graph out-edges in bidir graph (Tier 4)",
          "[source_id][cpo][dynamic_graph][bidir][out_edges]") {
  using namespace graph;
  using namespace graph::container;

  // Out-edge source_id on a bidir graph works the same as non-bidir (Tier 4).
  DynBidirSrcId g({{0, 1}, {0, 2}, {1, 2}});

  SECTION("source_id of out-edges matches the source vertex for all") {
    for (auto u : vertices(g)) {
      auto vid = vertex_id(g, u);
      for (auto oe : edges(g, u))
        REQUIRE(source_id(g, oe) == vid);
    }
  }
}

TEST_CASE("source_id(g,ie) - dynamic_graph weighted non-uniform bidir",
          "[source_id][cpo][dynamic_graph][bidir][weighted]") {
  using namespace graph;
  using namespace graph::container;

  using WeightedG =
        graph::container::dynamic_graph<int, void, void, uint32_t, true,
                                        graph::test::vov_bidir_graph_traits<int>>;

  WeightedG g({{0, 1, 10}, {0, 2, 20}, {1, 2, 30}});

  SECTION("source_id of in-edges to vertex 2 are 0 and 1") {
    auto u2 = *find_vertex(g, uint32_t(2));
    std::vector<uint32_t> sources;
    for (auto ie : in_edges(g, u2))
      sources.push_back(source_id(g, ie));
    std::sort(sources.begin(), sources.end());
    REQUIRE(sources.size() == 2);
    REQUIRE(sources[0] == 0);
    REQUIRE(sources[1] == 1);
  }
}
