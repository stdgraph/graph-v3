/**
 * @file test_incidence.cpp
 * @brief Comprehensive tests for incidence view
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/views/incidence.hpp>
#include <graph/views/vertexlist.hpp>
#include <vector>
#include <deque>
#include <map>
#include <string>
#include <algorithm>

using namespace graph;
using namespace graph::views;
using namespace graph::adj_list;

// =============================================================================
// Test 1: Empty Vertex (No Edges)
// =============================================================================

TEST_CASE("incidence - vertex with no edges", "[incidence][empty]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {},    // vertex 0 - no edges
        {0},   // vertex 1 - edge to 0
        {0, 1} // vertex 2 - edges to 0 and 1
  };

  SECTION("no value function - empty iteration") {
    auto v0    = vertex_t<Graph>{0}; // vertex 0 has no edges
    auto ilist = incidence(g, v0);

    REQUIRE(ilist.begin() == ilist.end());
    REQUIRE(ilist.size() == 0);
  }

  SECTION("with value function - empty iteration") {
    auto v0    = vertex_t<Graph>{0};
    auto ilist = incidence(g, v0, [](const auto&, auto /*e*/) { return 42; });

    REQUIRE(ilist.begin() == ilist.end());
  }
}

// =============================================================================
// Test 2: Single Edge
// =============================================================================

TEST_CASE("incidence - vertex with single edge", "[incidence][single]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1}, // vertex 0 -> edge to 1
        {}   // vertex 1 - no edges
  };

  SECTION("no value function") {
    auto v0    = vertex_t<Graph>{0};
    auto ilist = incidence(g, v0);

    REQUIRE(ilist.size() == 1);

    auto it = ilist.begin();
    REQUIRE(it != ilist.end());

    auto ei = *it;
    // edge_data<void, false, edge_t<G>, void> has just an 'edge' member
    // For vov graph, edge_t<G> is an edge_descriptor
    auto target = target_id(g, ei.edge);
    REQUIRE(target == 1);

    ++it;
    REQUIRE(it == ilist.end());
  }

  SECTION("with value function") {
    auto v0    = vertex_t<Graph>{0};
    auto ilist = incidence(g, v0, [](const auto& g, auto e) { return target_id(g, e) * 10; });

    REQUIRE(ilist.size() == 1);

    auto ei = *ilist.begin();
    REQUIRE(ei.value == 10); // target_id(1) * 10
  }
}

// =============================================================================
// Test 3: Multiple Edges
// =============================================================================

TEST_CASE("incidence - vertex with multiple edges", "[incidence][multiple]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2, 3}, // vertex 0 -> edges to 1, 2, 3
        {2, 3},    // vertex 1 -> edges to 2, 3
        {3},       // vertex 2 -> edge to 3
        {}         // vertex 3 - no edges
  };

  SECTION("no value function - iteration") {
    auto v0    = vertex_t<Graph>{0};
    auto ilist = incidence(g, v0);

    REQUIRE(ilist.size() == 3);

    std::vector<int> targets;
    for (auto ei : ilist) {
      targets.push_back(target_id(g, ei.edge));
    }

    REQUIRE(targets == std::vector<int>{1, 2, 3});
  }

  SECTION("with value function") {
    auto v1    = vertex_t<Graph>{1};
    auto ilist = incidence(g, v1, [](const auto& g, auto e) { return static_cast<int>(target_id(g, e) * 100); });

    std::vector<int> values;
    for (auto ei : ilist) {
      values.push_back(ei.value);
    }

    REQUIRE(values == std::vector<int>{200, 300});
  }

  SECTION("structured binding - no value function") {
    auto v0    = vertex_t<Graph>{0};
    auto ilist = incidence(g, v0);

    std::vector<int> targets;
    for (auto [tid, e] : ilist) {
      targets.push_back(target_id(g, e));
    }
    REQUIRE(targets == std::vector<int>{1, 2, 3});
  }

  SECTION("structured binding - with value function") {
    auto v0    = vertex_t<Graph>{0};
    auto ilist = incidence(g, v0, [](const auto& g, auto e) { return target_id(g, e) + 100; });

    std::vector<int> edge_targets;
    std::vector<int> values;
    for (auto [tid, e, val] : ilist) {
      edge_targets.push_back(target_id(g, e));
      values.push_back(val);
    }

    REQUIRE(edge_targets == std::vector<int>{1, 2, 3});
    REQUIRE(values == std::vector<int>{101, 102, 103});
  }
}

// =============================================================================
// Test 4: Value Function Types
// =============================================================================

TEST_CASE("incidence - value function types", "[incidence][evf]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, // vertex 0 -> edges to 1, 2
                 {},
                 {}};
  auto  v0    = vertex_t<Graph>{0};

  SECTION("returning string") {
    auto ilist = incidence(g, v0, [](const auto& g, auto e) { return "edge_to_" + std::to_string(target_id(g, e)); });

    std::vector<std::string> names;
    for (auto [tid, e, name] : ilist) {
      names.push_back(name);
    }

    REQUIRE(names == std::vector<std::string>{"edge_to_1", "edge_to_2"});
  }

  SECTION("returning double") {
    auto ilist = incidence(g, v0, [](const auto& g, auto e) { return static_cast<double>(target_id(g, e)) * 1.5; });

    std::vector<double> values;
    for (auto [tid, e, val] : ilist) {
      values.push_back(val);
    }

    REQUIRE(values[0] == 1.5);
    REQUIRE(values[1] == 3.0);
  }

  SECTION("capturing lambda") {
    int  multiplier = 7;
    auto ilist      = incidence(
          g, v0, [multiplier](const auto& g, auto e) { return static_cast<int>(target_id(g, e) * multiplier); });

    std::vector<int> values;
    for (auto [tid, e, val] : ilist) {
      values.push_back(val);
    }

    REQUIRE(values == std::vector<int>{7, 14});
  }
}

// =============================================================================
// Test 5: Edge Descriptor Access
// =============================================================================

TEST_CASE("incidence - edge descriptor access", "[incidence][descriptor]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2, 3}, // vertex 0 -> edges to 1, 2, 3
                 {},
                 {},
                 {}};
  auto  v0    = vertex_t<Graph>{0};

  SECTION("source_id access") {
    auto ilist = incidence(g, v0);

    for (auto [tid, e] : ilist) {
      // Every edge from v0 should have source_id == 0
      REQUIRE(source_id(g, e) == 0);
    }
  }

  SECTION("target_id access") {
    auto ilist = incidence(g, v0);

    std::vector<int> targets;
    for (auto [tid, e] : ilist) {
      targets.push_back(target_id(g, e));
    }

    REQUIRE(targets == std::vector<int>{1, 2, 3});
  }
}

// =============================================================================
// Test 6: Weighted Graph (Pair Edges)
// =============================================================================

TEST_CASE("incidence - weighted graph", "[incidence][weighted]") {
  // Graph with weighted edges: vector<vector<pair<target, weight>>>
  using Graph = std::vector<std::vector<std::pair<int, double>>>;
  Graph g     = {{{1, 1.5}, {2, 2.5}}, // vertex 0 -> (1, 1.5), (2, 2.5)
                 {{2, 3.5}},           // vertex 1 -> (2, 3.5)
                 {}};

  SECTION("no value function") {
    auto v0    = vertex_t<Graph>{0};
    auto ilist = incidence(g, v0);

    REQUIRE(ilist.size() == 2);

    std::vector<int> targets;
    for (auto [tid, e] : ilist) {
      targets.push_back(target_id(g, e));
    }

    REQUIRE(targets == std::vector<int>{1, 2});
  }

  SECTION("value function accessing edge weight") {
    auto v0    = vertex_t<Graph>{0};
    auto ilist = incidence(g, v0, [](const auto& g, auto e) {
      return edge_value(g, e); // Get the weight
    });

    std::vector<double> weights;
    for (auto [tid, e, w] : ilist) {
      weights.push_back(w);
    }

    REQUIRE(weights[0] == 1.5);
    REQUIRE(weights[1] == 2.5);
  }
}

// =============================================================================
// Test 7: Range Concepts
// =============================================================================

TEST_CASE("incidence - range concepts", "[incidence][concepts]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {}, {}};
  auto  v0    = vertex_t<Graph>{0};

  SECTION("no value function") {
    auto ilist = incidence(g, v0);

    STATIC_REQUIRE(std::ranges::input_range<decltype(ilist)>);
    STATIC_REQUIRE(std::ranges::forward_range<decltype(ilist)>);
    STATIC_REQUIRE(std::ranges::sized_range<decltype(ilist)>);
    STATIC_REQUIRE(std::ranges::view<decltype(ilist)>);
  }

  SECTION("with value function") {
    auto ilist = incidence(g, v0, [](const auto& g, auto e) { return target_id(g, e); });

    STATIC_REQUIRE(std::ranges::input_range<decltype(ilist)>);
    STATIC_REQUIRE(std::ranges::forward_range<decltype(ilist)>);
    STATIC_REQUIRE(std::ranges::sized_range<decltype(ilist)>);
    // Note: view concept may not be satisfied if EVF is not assignable (e.g., lambda)
    // The view_base is inherited, but movable is required
  }
}

// =============================================================================
// Test 8: Iterator Properties
// =============================================================================

TEST_CASE("incidence - iterator properties", "[incidence][iterator]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2, 3}, {}, {}, {}};
  auto  v0    = vertex_t<Graph>{0};

  SECTION("pre-increment returns reference") {
    auto  ilist = incidence(g, v0);
    auto  it    = ilist.begin();
    auto& ref   = ++it;
    REQUIRE(&ref == &it);
  }

  SECTION("post-increment returns copy") {
    auto ilist = incidence(g, v0);
    auto it    = ilist.begin();
    auto copy  = it++;
    REQUIRE(copy != it);
  }

  SECTION("equality comparison") {
    auto ilist = incidence(g, v0);
    auto it1   = ilist.begin();
    auto it2   = ilist.begin();
    REQUIRE(it1 == it2);
    ++it1;
    REQUIRE(it1 != it2);
  }
}

// =============================================================================
// Test 9: edge_data Type Verification
// =============================================================================

TEST_CASE("incidence - edge_data type verification", "[incidence][types]") {
  using Graph    = std::vector<std::vector<int>>;
  using EdgeType = edge_t<Graph>;

  SECTION("no value function - edge_data<vertex_id_t<Graph>, false, edge_t, void>") {
    using ViewType = incidence_view<Graph, void>;
    using InfoType = typename ViewType::info_type;

    STATIC_REQUIRE(std::is_same_v<InfoType, edge_data<vertex_id_t<Graph>, false, EdgeType, void>>);

    // Verify edge_data members
    STATIC_REQUIRE(std::is_same_v<typename InfoType::source_id_type, void>);
    STATIC_REQUIRE(std::is_same_v<typename InfoType::target_id_type, vertex_id_t<Graph>>);
    STATIC_REQUIRE(std::is_same_v<typename InfoType::edge_type, EdgeType>);
    STATIC_REQUIRE(std::is_same_v<typename InfoType::value_type, void>);
  }

  SECTION("with value function - edge_data<vertex_id_t<Graph>, false, edge_t, int>") {
    using VVF      = int (*)(const Graph&, EdgeType);
    using ViewType = incidence_view<Graph, VVF>;
    using InfoType = typename ViewType::info_type;

    STATIC_REQUIRE(std::is_same_v<InfoType, edge_data<vertex_id_t<Graph>, false, EdgeType, int>>);

    // Verify edge_data members
    STATIC_REQUIRE(std::is_same_v<typename InfoType::source_id_type, void>);
    STATIC_REQUIRE(std::is_same_v<typename InfoType::target_id_type, vertex_id_t<Graph>>);
    STATIC_REQUIRE(std::is_same_v<typename InfoType::edge_type, EdgeType>);
    STATIC_REQUIRE(std::is_same_v<typename InfoType::value_type, int>);
  }
}

// =============================================================================
// Test 10: std::ranges Algorithms
// =============================================================================

TEST_CASE("incidence - std::ranges algorithms", "[incidence][algorithms]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2, 3, 4, 5}, // vertex 0 -> edges to 1-5
                 {},
                 {},
                 {},
                 {},
                 {}};
  auto  v0    = vertex_t<Graph>{0};

  SECTION("std::ranges::distance") {
    auto ilist = incidence(g, v0);
    REQUIRE(std::ranges::distance(ilist) == 5);
  }

  SECTION("std::ranges::count_if") {
    auto ilist = incidence(g, v0);
    auto count = std::ranges::count_if(ilist, [](auto&& ei) { return ei.target_id > 2; });
    REQUIRE(count == 3); // targets 3, 4, 5
  }
}

// =============================================================================
// Test 11: Deque-based Graph
// =============================================================================

TEST_CASE("incidence - deque-based graph", "[incidence][deque]") {
  using Graph = std::deque<std::deque<int>>;
  Graph g     = {{1, 2}, {2}, {}};

  auto v0    = vertex_t<Graph>{0};
  auto ilist = incidence(g, v0);

  std::vector<int> targets;
  for (auto [tid, e] : ilist) {
    targets.push_back(target_id(g, e));
  }

  REQUIRE(targets == std::vector<int>{1, 2});
}

// =============================================================================
// Test 12: All Vertices Iteration
// =============================================================================

TEST_CASE("incidence - iterating all vertices", "[incidence][all]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // vertex 0 -> 1, 2
        {2},    // vertex 1 -> 2
        {}      // vertex 2 -> no edges
  };

  // Collect all edges from all vertices
  std::vector<std::pair<int, int>> all_edges;

  for (auto [id, v] : vertexlist(g)) {
    for (auto [tid, e] : incidence(g, v)) {
      all_edges.emplace_back(source_id(g, e), tid);
    }
  }

  REQUIRE(all_edges.size() == 3);
  REQUIRE(all_edges[0] == std::pair<int, int>{0, 1});
  REQUIRE(all_edges[1] == std::pair<int, int>{0, 2});
  REQUIRE(all_edges[2] == std::pair<int, int>{1, 2});
}

// =============================================================================
// Test 13: Map-Based Vertex Container (Sparse Vertex IDs)
// =============================================================================

TEST_CASE("incidence - map vertices vector edges", "[incidence][map]") {
  // Map-based graphs have sparse, non-contiguous vertex IDs
  using Graph = std::map<int, std::vector<int>>;
  Graph g     = {
        {100, {200, 300}}, // vertex 100 -> edges to 200, 300
        {200, {300}},      // vertex 200 -> edge to 300
        {300, {}}          // vertex 300 -> no edges
  };

  SECTION("iteration over edges from sparse vertex") {
    // Get vertex 100
    auto verts = vertices(g);
    auto v100  = *verts.begin();
    REQUIRE(v100.vertex_id() == 100);

    auto ilist = incidence(g, v100);

    REQUIRE(ilist.size() == 2);

    std::vector<int> targets;
    for (auto [tid, e] : ilist) {
      targets.push_back(target_id(g, e));
    }

    REQUIRE(targets == std::vector<int>{200, 300});
  }

  SECTION("source_id is correct for map vertex") {
    auto verts = vertices(g);
    auto v100  = *verts.begin();

    std::vector<int> targets_found;
    for (auto [tid, e] : incidence(g, v100)) {
      REQUIRE(source_id(g, e) == 100);
      targets_found.push_back(tid);
    }
    REQUIRE(targets_found == std::vector<int>{200, 300});
  }

  SECTION("empty edge list") {
    auto verts = vertices(g);
    auto it    = verts.begin();
    std::advance(it, 2); // Get vertex 300
    auto v300 = *it;
    REQUIRE(v300.vertex_id() == 300);

    auto ilist = incidence(g, v300);
    REQUIRE(ilist.size() == 0);
    REQUIRE(ilist.begin() == ilist.end());
  }

  SECTION("with value function") {
    auto verts = vertices(g);
    auto v100  = *verts.begin();

    auto ilist = incidence(g, v100, [](const auto& g, auto e) {
      return target_id(g, e) - 100; // Offset from base
    });

    std::vector<int> offsets;
    for (auto [tid, e, offset] : ilist) {
      offsets.push_back(offset);
    }

    REQUIRE(offsets == std::vector<int>{100, 200}); // 200-100=100, 300-100=200
  }

  SECTION("iterate all vertices and edges") {
    std::vector<std::pair<int, int>> all_edges;

    for (auto [id, v] : vertexlist(g)) {
      for (auto [tid, e] : incidence(g, v)) {
        all_edges.emplace_back(source_id(g, e), tid);
      }
    }

    REQUIRE(all_edges.size() == 3);
    REQUIRE(all_edges[0] == std::pair<int, int>{100, 200});
    REQUIRE(all_edges[1] == std::pair<int, int>{100, 300});
    REQUIRE(all_edges[2] == std::pair<int, int>{200, 300});
  }
}

// =============================================================================
// Test 14: Map-Based Edge Container (Sorted Edges)
// =============================================================================

TEST_CASE("incidence - vector vertices map edges", "[incidence][edge_map]") {
  // Edges stored in map (sorted by target, with edge values)
  using Graph = std::vector<std::map<int, double>>;
  Graph g     = {
        {{1, 1.5}, {2, 2.5}}, // vertex 0 -> (1, 1.5), (2, 2.5)
        {{2, 3.5}},           // vertex 1 -> (2, 3.5)
        {}                    // vertex 2 -> no edges
  };

  SECTION("iteration") {
    auto v0    = vertex_t<Graph>{0};
    auto ilist = incidence(g, v0);

    REQUIRE(ilist.size() == 2);

    std::vector<int> targets;
    for (auto [tid, e] : ilist) {
      targets.push_back(target_id(g, e));
    }

    // Map edges are sorted by target_id (key)
    REQUIRE(targets == std::vector<int>{1, 2});
  }

  SECTION("accessing edge weights via edge_value") {
    auto v0    = vertex_t<Graph>{0};
    auto ilist = incidence(g, v0, [](const auto& g, auto e) { return edge_value(g, e); });

    std::vector<double> weights;
    for (auto [tid, e, w] : ilist) {
      weights.push_back(w);
    }

    REQUIRE(weights == std::vector<double>{1.5, 2.5});
  }

  SECTION("single edge vertex") {
    auto v1    = vertex_t<Graph>{1};
    auto ilist = incidence(g, v1);

    REQUIRE(ilist.size() == 1);

    auto [tid, e] = *ilist.begin();
    REQUIRE(target_id(g, e) == 2);
    REQUIRE(edge_value(g, e) == 3.5);
  }
}

// =============================================================================
// Test 15: Map Vertices + Map Edges (Fully Sparse Graph)
// =============================================================================

TEST_CASE("incidence - map vertices map edges", "[incidence][map][edge_map]") {
  // Both vertices and edges in maps - fully sparse graph
  using Graph = std::map<int, std::map<int, double>>;
  Graph g     = {
        {10, {{20, 1.0}, {30, 2.0}}}, // vertex 10 -> (20, 1.0), (30, 2.0)
        {20, {{30, 3.0}}},            // vertex 20 -> (30, 3.0)
        {30, {}}                      // vertex 30 -> no edges
  };

  SECTION("iteration") {
    auto verts = vertices(g);
    auto v10   = *verts.begin();
    REQUIRE(v10.vertex_id() == 10);

    auto ilist = incidence(g, v10);

    REQUIRE(ilist.size() == 2);

    std::vector<int> targets;
    for (auto [tid, e] : ilist) {
      targets.push_back(target_id(g, e));
    }

    REQUIRE(targets == std::vector<int>{20, 30});
  }

  SECTION("with value function for edge weights") {
    auto verts = vertices(g);
    auto v10   = *verts.begin();

    auto ilist = incidence(g, v10, [](const auto& g, auto e) { return edge_value(g, e); });

    std::vector<double> weights;
    for (auto [tid, e, w] : ilist) {
      weights.push_back(w);
    }

    REQUIRE(weights == std::vector<double>{1.0, 2.0});
  }

  SECTION("source_id correct for sparse vertex") {
    auto verts = vertices(g);
    auto it    = verts.begin();
    ++it; // vertex 20
    auto v20 = *it;

    for (auto [tid, e] : incidence(g, v20)) {
      REQUIRE(source_id(g, e) == 20);
      REQUIRE(tid == 30);
    }
  }

  SECTION("all edges traversal") {
    std::vector<std::tuple<int, int, double>> all_edges;

    for (auto [id, v] : vertexlist(g)) {
      for (auto [tid, e, w] : incidence(g, v, [](const auto& g, auto e) { return edge_value(g, e); })) {
        all_edges.emplace_back(source_id(g, e), tid, w);
      }
    }

    REQUIRE(all_edges.size() == 3);
    REQUIRE(std::get<0>(all_edges[0]) == 10);
    REQUIRE(std::get<1>(all_edges[0]) == 20);
    REQUIRE(std::get<2>(all_edges[0]) == 1.0);
  }
}

// =============================================================================
// Test 16: undirected_adjacency_list - True Undirected Graph
// =============================================================================
// Note: The undirected_adjacency_list is a true undirected graph where edges
// are not duplicated. Each edge is stored once but can be traversed from both
// endpoints. The incidence view tests verify this behavior.

#include <graph/container/undirected_adjacency_list.hpp>
#include <set>

TEST_CASE("incidence - undirected_adjacency_list basic", "[incidence][undirected]") {
  using Graph = graph::container::undirected_adjacency_list<int, int>;
  Graph g;

  // Create vertices: 0, 1, 2, 3, 4
  g.create_vertex(100); // vertex 0, value=100
  g.create_vertex(200); // vertex 1, value=200
  g.create_vertex(300); // vertex 2, value=300
  g.create_vertex(400); // vertex 3, value=400
  g.create_vertex(500); // vertex 4, value=500

  // Create edges from vertex 0 to multiple targets (star topology from 0)
  // These are undirected edges - each creates one edge accessible from both ends
  g.create_edge(0, 1, 10); // 0 -- 1, weight=10
  g.create_edge(0, 2, 20); // 0 -- 2, weight=20
  g.create_edge(0, 3, 30); // 0 -- 3, weight=30
  g.create_edge(0, 4, 40); // 0 -- 4, weight=40

  // Additional edges to make vertex 2 a hub
  g.create_edge(2, 3, 23); // 2 -- 3, weight=23
  g.create_edge(2, 4, 24); // 2 -- 4, weight=24

  SECTION("vertex 0 has 4 incident edges") {
    auto verts = vertices(g);
    auto v0    = *verts.begin();

    auto inc = incidence(g, v0);
    REQUIRE(inc.size() == 4);

    // Collect all target IDs
    std::set<unsigned int> targets;
    for (auto [tid, e] : inc) {
      targets.insert(target_id(g, e));
    }

    // Should have edges to 1, 2, 3, 4
    REQUIRE(targets.size() == 4);
    REQUIRE(targets.count(1) == 1);
    REQUIRE(targets.count(2) == 1);
    REQUIRE(targets.count(3) == 1);
    REQUIRE(targets.count(4) == 1);
  }

  SECTION("vertex 1 has 1 incident edge (back to 0)") {
    auto v1_it = find_vertex(g, 1u);
    auto v1    = *v1_it;

    auto inc = incidence(g, v1);
    REQUIRE(inc.size() == 1);

    auto [tid, e] = *inc.begin();
    REQUIRE(target_id(g, e) == 0); // Points back to vertex 0
  }

  SECTION("vertex 2 has 3 incident edges") {
    auto v2_it = find_vertex(g, 2u);
    auto v2    = *v2_it;

    auto inc = incidence(g, v2);
    REQUIRE(inc.size() == 3);

    std::set<unsigned int> targets;
    for (auto [tid, e] : inc) {
      targets.insert(target_id(g, e));
    }

    // Should have edges to 0, 3, 4
    REQUIRE(targets.count(0) == 1);
    REQUIRE(targets.count(3) == 1);
    REQUIRE(targets.count(4) == 1);
  }

  SECTION("iterate with value function - edge weights") {
    auto v0_it = find_vertex(g, 0u);
    auto v0    = *v0_it;

    auto inc = incidence(g, v0, [](const auto& g, auto e) { return edge_value(g, e); });

    std::vector<int>          weights;
    std::vector<unsigned int> targets;
    for (auto [tid, e, w] : inc) {
      targets.push_back(target_id(g, e));
      weights.push_back(w);
    }

    REQUIRE(weights.size() == 4);

    // Verify each target has the correct weight
    for (size_t i = 0; i < targets.size(); ++i) {
      // Weight should be target * 10
      REQUIRE(weights[i] == static_cast<int>(targets[i] * 10));
    }
  }

  SECTION("source_id is consistent for all edges from a vertex") {
    auto v2_it = find_vertex(g, 2u);
    auto v2    = *v2_it;

    for (auto [tid, e] : incidence(g, v2)) {
      REQUIRE(source_id(g, e) == 2);
    }
  }
}

TEST_CASE("incidence - undirected_adjacency_list iteration order", "[incidence][undirected]") {
  using Graph = graph::container::undirected_adjacency_list<int, int>;
  Graph g;

  // Create a simple triangle: 0 -- 1 -- 2 -- 0
  g.create_vertex(0);
  g.create_vertex(1);
  g.create_vertex(2);

  g.create_edge(0, 1, 1);
  g.create_edge(1, 2, 2);
  g.create_edge(2, 0, 3);

  SECTION("each vertex has exactly 2 incident edges") {
    for (auto [id, v] : vertexlist(g)) {
      auto inc = incidence(g, v);
      REQUIRE(inc.size() == 2);
    }
  }

  SECTION("full graph traversal - each edge visited twice (once per direction)") {
    std::vector<std::pair<unsigned int, unsigned int>> all_edges;

    for (auto [id, v] : vertexlist(g)) {
      for (auto [tid, e] : incidence(g, v)) {
        all_edges.emplace_back(source_id(g, e), tid);
      }
    }

    // Triangle has 3 edges, each visited from both directions = 6 entries
    REQUIRE(all_edges.size() == 6);

    // Verify we see each edge from both directions
    std::set<std::pair<unsigned int, unsigned int>> edge_set(all_edges.begin(), all_edges.end());
    REQUIRE(edge_set.count({0, 1}) == 1);
    REQUIRE(edge_set.count({1, 0}) == 1);
    REQUIRE(edge_set.count({1, 2}) == 1);
    REQUIRE(edge_set.count({2, 1}) == 1);
    REQUIRE(edge_set.count({2, 0}) == 1);
    REQUIRE(edge_set.count({0, 2}) == 1);
  }
}

TEST_CASE("incidence - undirected_adjacency_list range algorithms", "[incidence][undirected][algorithms]") {
  using Graph = graph::container::undirected_adjacency_list<int, int>;
  Graph g;

  // Create vertices
  for (int i = 0; i < 5; ++i) {
    g.create_vertex(i * 100);
  }

  // Create a hub at vertex 0 with many edges
  g.create_edge(0, 1, 10);
  g.create_edge(0, 2, 20);
  g.create_edge(0, 3, 30);
  g.create_edge(0, 4, 40);

  auto v0_it = find_vertex(g, 0u);
  auto v0    = *v0_it;

  SECTION("std::ranges::distance") {
    auto inc = incidence(g, v0);
    REQUIRE(std::ranges::distance(inc) == 4);
  }

  SECTION("std::ranges::count_if - count edges with weight > 20") {
    auto inc = incidence(g, v0, [](const auto& g, auto e) { return edge_value(g, e); });

    auto count = std::ranges::count_if(inc, [](auto ei) { return ei.value > 20; });

    REQUIRE(count == 2); // edges with weights 30 and 40
  }

  SECTION("std::ranges::for_each - sum weights") {
    auto inc = incidence(g, v0, [](const auto& g, auto e) { return edge_value(g, e); });

    int total_weight = 0;
    std::ranges::for_each(inc, [&total_weight](auto ei) { total_weight += ei.value; });

    REQUIRE(total_weight == 100); // 10 + 20 + 30 + 40
  }

  SECTION("range-based for with structured bindings") {
    auto inc = incidence(g, v0, [](const auto& g, auto e) { return std::to_string(edge_value(g, e)); });

    std::vector<std::string> weight_strs;
    for (auto [tid, e, w_str] : inc) {
      weight_strs.push_back(w_str);
    }

    REQUIRE(weight_strs.size() == 4);
  }
}

TEST_CASE("incidence - undirected_adjacency_list empty and single edge", "[incidence][undirected][edge_cases]") {
  using Graph = graph::container::undirected_adjacency_list<int, int>;

  SECTION("vertex with no edges") {
    Graph g;
    g.create_vertex(0);
    g.create_vertex(1);
    // No edges created

    auto v0_it = find_vertex(g, 0u);
    auto v0    = *v0_it;

    auto inc = incidence(g, v0);
    REQUIRE(inc.size() == 0);
    REQUIRE(inc.begin() == inc.end());
  }

  SECTION("single edge - both endpoints see it") {
    Graph g;
    g.create_vertex(0);
    g.create_vertex(1);
    g.create_edge(0, 1, 42);

    auto v0_it = find_vertex(g, 0u);
    auto v0    = *v0_it;
    auto v1_it = find_vertex(g, 1u);
    auto v1    = *v1_it;

    // From vertex 0
    auto inc0 = incidence(g, v0);
    REQUIRE(inc0.size() == 1);
    auto [tid0, e0] = *inc0.begin();
    REQUIRE(source_id(g, e0) == 0);
    REQUIRE(tid0 == 1);
    REQUIRE(edge_value(g, e0) == 42);

    // From vertex 1
    auto inc1 = incidence(g, v1);
    REQUIRE(inc1.size() == 1);
    auto [tid1, e1] = *inc1.begin();
    REQUIRE(source_id(g, e1) == 1);
    REQUIRE(tid1 == 0);
    REQUIRE(edge_value(g, e1) == 42); // Same edge, same weight
  }
}
