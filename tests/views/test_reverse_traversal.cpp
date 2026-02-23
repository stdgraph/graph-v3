/**
 * @file test_reverse_traversal.cpp
 * @brief Tests for Accessor-parameterized BFS/DFS/topological-sort views.
 *
 * Phase 7: Verifies that:
 * - Forward traversal with default (out_edge_accessor) still works
 * - Reverse traversal with in_edge_accessor follows incoming edges
 * - Existing call sites remain source-compatible
 * - topological_sort with Accessor produces correct orderings
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/views/bfs.hpp>
#include <graph/views/dfs.hpp>
#include <graph/views/topological_sort.hpp>
#include <graph/views/edge_accessor.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <vector>
#include <set>
#include <algorithm>

using namespace graph;
using namespace graph::views;
using namespace graph::adj_list;

// Non-uniform bidirectional traits: in_edge_type = dynamic_in_edge (has source_id())
// so that bidirectional_adjacency_list concept is satisfied.
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t>
struct vov_bidir_graph_traits {
  using edge_value_type   = EV;
  using vertex_value_type = VV;
  using graph_value_type  = GV;
  using vertex_id_type    = VId;
  static constexpr bool bidirectional = true;

  using edge_type    = container::dynamic_out_edge<EV, VV, GV, VId, true, vov_bidir_graph_traits>;
  using in_edge_type = container::dynamic_in_edge<EV, VV, GV, VId, true, vov_bidir_graph_traits>;
  using vertex_type  = container::dynamic_vertex<EV, VV, GV, VId, true, vov_bidir_graph_traits>;
  using graph_type   = container::dynamic_graph<EV, VV, GV, VId, true, vov_bidir_graph_traits>;

  using edges_type    = std::vector<edge_type>;
  using in_edges_type = std::vector<in_edge_type>;
  using vertices_type = std::vector<vertex_type>;
};

// Bidirectional graph type with void edge values using non-uniform bidir traits
using bidir_graph = container::dynamic_graph<void, void, void, uint32_t, true,
                                             vov_bidir_graph_traits<void, void, void, uint32_t>>;

// =============================================================================
// Helper: build a small directed bidirectional graph
//
//   0 -> 1 -> 3
//   |         ^
//   v         |
//   2 --------+
//
// Edges: 0->1, 0->2, 1->3, 2->3
// =============================================================================
static bidir_graph make_diamond() {
  return bidir_graph({{0, 1}, {0, 2}, {1, 3}, {2, 3}});
}

// =============================================================================
// Helper: build a chain  0 -> 1 -> 2 -> 3
// =============================================================================
static bidir_graph make_chain() {
  return bidir_graph({{0, 1}, {1, 2}, {2, 3}});
}

// =============================================================================
// BFS — Forward (default accessor)
// =============================================================================

TEST_CASE("vertices_bfs - default accessor on bidir graph", "[bfs][accessor][forward]") {
  auto g = make_diamond();

  std::vector<uint32_t> visited;
  for (auto [v] : vertices_bfs(g, uint32_t{0})) {
    visited.push_back(vertex_id(g, v));
  }

  REQUIRE(visited.size() == 4);
  REQUIRE(visited[0] == 0);
  // Level 1: {1, 2} in some order
  std::set<uint32_t> level1(visited.begin() + 1, visited.begin() + 3);
  REQUIRE(level1 == std::set<uint32_t>{1, 2});
  // Level 2: {3}
  REQUIRE(visited[3] == 3);
}

// =============================================================================
// BFS — Reverse (in_edge_accessor)
// =============================================================================

TEST_CASE("vertices_bfs - reverse accessor from sink", "[bfs][accessor][reverse]") {
  auto g = make_diamond();

  // BFS backwards from vertex 3 (the sink) using in_edge_accessor
  std::vector<uint32_t> visited;
  for (auto [v] : vertices_bfs<in_edge_accessor>(g, uint32_t{3})) {
    visited.push_back(vertex_id(g, v));
  }

  // From 3 following incoming edges: 3 -> {1, 2} -> {0}
  REQUIRE(visited.size() == 4);
  REQUIRE(visited[0] == 3);
  std::set<uint32_t> level1(visited.begin() + 1, visited.begin() + 3);
  REQUIRE(level1 == std::set<uint32_t>{1, 2});
  REQUIRE(visited[3] == 0);
}

TEST_CASE("edges_bfs - reverse accessor from sink", "[bfs][accessor][reverse]") {
  auto g = make_diamond();

  std::vector<uint32_t> source_ids;
  for (auto [uv] : edges_bfs<in_edge_accessor>(g, uint32_t{3})) {
    source_ids.push_back(source_id(g, uv));
  }

  // From 3 following incoming edges: edges arriving at 3 are from 1 and 2
  // Then from 1 and 2, incoming edges arrive from 0
  REQUIRE(source_ids.size() >= 2); // At least the edges into 3
}

TEST_CASE("vertices_bfs - reverse on chain", "[bfs][accessor][reverse]") {
  auto g = make_chain(); // 0->1->2->3

  // Reverse BFS from 3 should visit 3, 2, 1, 0
  std::vector<uint32_t> visited;
  for (auto [v] : vertices_bfs<in_edge_accessor>(g, uint32_t{3})) {
    visited.push_back(vertex_id(g, v));
  }

  REQUIRE(visited.size() == 4);
  REQUIRE(visited == std::vector<uint32_t>{3, 2, 1, 0});
}

// =============================================================================
// BFS — Forward with value function + Accessor
// =============================================================================

TEST_CASE("vertices_bfs - reverse with value function", "[bfs][accessor][reverse]") {
  auto g   = make_chain();
  auto vvf = [](const auto& gg, auto v) { return vertex_id(gg, v) * 10; };

  std::vector<uint32_t> ids;
  std::vector<uint32_t> vals;
  for (auto [v, val] : vertices_bfs<in_edge_accessor>(g, uint32_t{3}, vvf)) {
    ids.push_back(vertex_id(g, v));
    vals.push_back(val);
  }

  REQUIRE(ids == std::vector<uint32_t>{3, 2, 1, 0});
  REQUIRE(vals == std::vector<uint32_t>{30, 20, 10, 0});
}

// =============================================================================
// DFS — Forward (default accessor)
// =============================================================================

TEST_CASE("vertices_dfs - default accessor on bidir graph", "[dfs][accessor][forward]") {
  auto g = make_chain(); // 0->1->2->3

  std::vector<uint32_t> visited;
  for (auto [v] : vertices_dfs(g, uint32_t{0})) {
    visited.push_back(vertex_id(g, v));
  }

  // DFS from 0 along the chain should visit all 4
  REQUIRE(visited.size() == 4);
  REQUIRE(visited[0] == 0);
  // DFS follows the single path: 0, 1, 2, 3
  REQUIRE(visited == std::vector<uint32_t>{0, 1, 2, 3});
}

// =============================================================================
// DFS — Reverse (in_edge_accessor)
// =============================================================================

TEST_CASE("vertices_dfs - reverse accessor from sink", "[dfs][accessor][reverse]") {
  auto g = make_chain(); // 0->1->2->3

  // Reverse DFS from 3 should visit 3, 2, 1, 0
  std::vector<uint32_t> visited;
  for (auto [v] : vertices_dfs<in_edge_accessor>(g, uint32_t{3})) {
    visited.push_back(vertex_id(g, v));
  }

  REQUIRE(visited.size() == 4);
  REQUIRE(visited == std::vector<uint32_t>{3, 2, 1, 0});
}

TEST_CASE("edges_dfs - reverse accessor from sink", "[dfs][accessor][reverse]") {
  auto g = make_chain();

  std::vector<uint32_t> neighbor_ids;
  for (auto [uv] : edges_dfs<in_edge_accessor>(g, uint32_t{3})) {
    neighbor_ids.push_back(source_id(g, uv));
  }

  // From 3 backwards: edges are 3<-2, 2<-1, 1<-0
  REQUIRE(neighbor_ids.size() == 3);
}

TEST_CASE("vertices_dfs - reverse with value function", "[dfs][accessor][reverse]") {
  auto g   = make_chain();
  auto vvf = [](const auto& gg, auto v) { return vertex_id(gg, v) + 100; };

  std::vector<uint32_t> ids;
  std::vector<uint32_t> vals;
  for (auto [v, val] : vertices_dfs<in_edge_accessor>(g, uint32_t{3}, vvf)) {
    ids.push_back(vertex_id(g, v));
    vals.push_back(val);
  }

  REQUIRE(ids == std::vector<uint32_t>{3, 2, 1, 0});
  REQUIRE(vals == std::vector<uint32_t>{103, 102, 101, 100});
}

// =============================================================================
// DFS — Reverse on diamond
// =============================================================================

TEST_CASE("vertices_dfs - reverse on diamond from sink", "[dfs][accessor][reverse]") {
  auto g = make_diamond(); // 0->1, 0->2, 1->3, 2->3

  std::vector<uint32_t> visited;
  for (auto [v] : vertices_dfs<in_edge_accessor>(g, uint32_t{3})) {
    visited.push_back(vertex_id(g, v));
  }

  // From 3, incoming edges lead to 1 and 2; from those, incoming edge leads to 0
  REQUIRE(visited.size() == 4);
  REQUIRE(visited[0] == 3);
  // All vertices reachable
  std::set<uint32_t> all(visited.begin(), visited.end());
  REQUIRE(all == std::set<uint32_t>{0, 1, 2, 3});
}

// =============================================================================
// Topological Sort — Forward (default accessor)
// =============================================================================

TEST_CASE("vertices_topological_sort - default accessor", "[topo][accessor][forward]") {
  auto g = make_diamond(); // 0->1, 0->2, 1->3, 2->3

  std::vector<uint32_t> order;
  for (auto [v] : vertices_topological_sort(g)) {
    order.push_back(vertex_id(g, v));
  }

  REQUIRE(order.size() == 4);
  // 0 must precede 1, 2; both 1 and 2 must precede 3
  auto pos = [&](uint32_t id) { return std::find(order.begin(), order.end(), id) - order.begin(); };
  REQUIRE(pos(0) < pos(1));
  REQUIRE(pos(0) < pos(2));
  REQUIRE(pos(1) < pos(3));
  REQUIRE(pos(2) < pos(3));
}

// =============================================================================
// Topological Sort — Reverse (in_edge_accessor)
// =============================================================================

TEST_CASE("vertices_topological_sort - reverse accessor", "[topo][accessor][reverse]") {
  auto g = make_diamond(); // 0->1, 0->2, 1->3, 2->3

  // Reverse topo sort: follow incoming edges in DFS.
  // The reversed dependency graph has edges 1->0, 2->0, 3->1, 3->2.
  // Topological order of the reversed graph: 3 before {1,2}, {1,2} before 0
  std::vector<uint32_t> order;
  for (auto [v] : vertices_topological_sort<in_edge_accessor>(g)) {
    order.push_back(vertex_id(g, v));
  }

  REQUIRE(order.size() == 4);
  auto pos = [&](uint32_t id) { return std::find(order.begin(), order.end(), id) - order.begin(); };
  // In reverse topo: 3 before 1 and 2; 1 and 2 before 0
  REQUIRE(pos(3) < pos(1));
  REQUIRE(pos(3) < pos(2));
  REQUIRE(pos(1) < pos(0));
  REQUIRE(pos(2) < pos(0));
}

TEST_CASE("edges_topological_sort - reverse accessor", "[topo][accessor][reverse]") {
  auto g = make_diamond();

  std::set<uint32_t> source_vertices;
  for (auto [uv] : edges_topological_sort<in_edge_accessor>(g)) {
    source_vertices.insert(source_id(g, uv));
  }

  // All vertices with incoming edges (in the reversed direction = original out-edges)
  // should appear as sources
  REQUIRE(!source_vertices.empty());
  REQUIRE(source_vertices.size() <= 4);
}

// =============================================================================
// Topological Sort — Safe variants with Accessor
// =============================================================================

TEST_CASE("vertices_topological_sort_safe - reverse accessor", "[topo][accessor][reverse][safe]") {
  auto g = make_diamond();

  auto result = vertices_topological_sort_safe<in_edge_accessor>(g);
  REQUIRE(result.has_value());

  std::vector<uint32_t> order;
  for (auto [v] : *result) {
    order.push_back(vertex_id(g, v));
  }

  REQUIRE(order.size() == 4);
}

TEST_CASE("vertices_topological_sort_safe - reverse with VVF", "[topo][accessor][reverse][safe]") {
  auto g   = make_diamond();
  auto vvf = [](const auto& gg, auto v) { return vertex_id(gg, v); };

  auto result = vertices_topological_sort_safe<in_edge_accessor>(g, vvf);
  REQUIRE(result.has_value());

  std::vector<uint32_t> order;
  for (auto [v, val] : *result) {
    order.push_back(val);
  }

  REQUIRE(order.size() == 4);
}

TEST_CASE("edges_topological_sort_safe - reverse accessor", "[topo][accessor][reverse][safe]") {
  auto g = make_diamond();

  auto result = edges_topological_sort_safe<in_edge_accessor>(g);
  REQUIRE(result.has_value());

  size_t count = 0;
  for (auto [uv] : *result) {
    ++count;
  }
  REQUIRE(count > 0);
}

// =============================================================================
// Source compatibility — existing call sites still compile
// =============================================================================

TEST_CASE("source compatibility - existing BFS calls unchanged", "[accessor][compat]") {
  // Simple vector<vector<int>> graph — not bidirectional, uses default out_edge_accessor
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {3}, {3}, {}};

  // These must still compile and work without specifying Accessor:
  std::vector<int> visited;
  for (auto [v] : vertices_bfs(g, 0)) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }
  REQUIRE(visited.size() == 4);

  size_t edge_count = 0;
  for (auto [uv] : edges_bfs(g, 0)) {
    ++edge_count;
  }
  REQUIRE(edge_count == 3);
}

TEST_CASE("source compatibility - existing DFS calls unchanged", "[accessor][compat]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {3}, {3}, {}};

  std::vector<int> visited;
  for (auto [v] : vertices_dfs(g, 0)) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }
  REQUIRE(visited.size() == 4);

  size_t edge_count = 0;
  for (auto [uv] : edges_dfs(g, 0)) {
    ++edge_count;
  }
  REQUIRE(edge_count == 3);
}

TEST_CASE("source compatibility - existing topo sort calls unchanged", "[accessor][compat]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {3}, {3}, {}};

  std::vector<int> visited;
  for (auto [v] : vertices_topological_sort(g)) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }
  REQUIRE(visited.size() == 4);

  auto result = vertices_topological_sort_safe(g);
  REQUIRE(result.has_value());
}

// =============================================================================
// Explicit out_edge_accessor matches default behavior
// =============================================================================

TEST_CASE("explicit out_edge_accessor matches default BFS", "[accessor][explicit]") {
  auto g = make_chain();

  std::vector<uint32_t> default_order, explicit_order;

  for (auto [v] : vertices_bfs(g, uint32_t{0})) {
    default_order.push_back(vertex_id(g, v));
  }

  for (auto [v] : vertices_bfs<out_edge_accessor>(g, uint32_t{0})) {
    explicit_order.push_back(vertex_id(g, v));
  }

  REQUIRE(default_order == explicit_order);
}

TEST_CASE("explicit out_edge_accessor matches default DFS", "[accessor][explicit]") {
  auto g = make_chain();

  std::vector<uint32_t> default_order, explicit_order;

  for (auto [v] : vertices_dfs(g, uint32_t{0})) {
    default_order.push_back(vertex_id(g, v));
  }

  for (auto [v] : vertices_dfs<out_edge_accessor>(g, uint32_t{0})) {
    explicit_order.push_back(vertex_id(g, v));
  }

  REQUIRE(default_order == explicit_order);
}

TEST_CASE("explicit out_edge_accessor matches default topo sort", "[accessor][explicit]") {
  auto g = make_diamond();

  std::vector<uint32_t> default_order, explicit_order;

  for (auto [v] : vertices_topological_sort(g)) {
    default_order.push_back(vertex_id(g, v));
  }

  for (auto [v] : vertices_topological_sort<out_edge_accessor>(g)) {
    explicit_order.push_back(vertex_id(g, v));
  }

  REQUIRE(default_order == explicit_order);
}
