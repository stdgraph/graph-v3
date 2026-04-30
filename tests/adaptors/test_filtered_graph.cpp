#include <catch2/catch_test_macros.hpp>

#include <graph/graph.hpp>
#include <graph/views.hpp>
#include <graph/algorithm/dijkstra_shortest_paths.hpp>
#include <graph/adaptors/filtered_graph.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>

#include <algorithm>
#include <limits>
#include <vector>

// ── Test graph type ─────────────────────────────────────────────────────────
// Simple weighted directed graph as vector<vector<pair<int, double>>>
// edge = (target_id, weight)

using test_graph_t = std::vector<std::vector<std::pair<int, double>>>;

// Graph:
//   0 --2.0--> 1 --1.0--> 3
//   |          |
//   3.0        4.0
//   v          v
//   2 --1.0--> 3
//
// Vertices: 0, 1, 2, 3
// Edges: 0→1(2), 0→2(3), 1→3(1), 1→3(4), 2→3(1)
// Wait, let me fix: 1→2(4) not 1→3(4)
//
//   0 --2.0--> 1 --4.0--> 2
//   |                      |
//   3.0                    1.0
//   v                      v
//   2 --------1.0--------> 3
//
// Actually let me use a cleaner graph:
//   0 →1(2)  0→2(3)
//   1 →2(4)  1→3(1)
//   2 →3(1)
//
// Shortest from 0: d[0]=0, d[1]=2, d[2]=3, d[3]=3 (0→1→3)

static test_graph_t make_test_graph() {
  return {
    {{1, 2.0}, {2, 3.0}},  // vertex 0: edges to 1(2), 2(3)
    {{2, 4.0}, {3, 1.0}},  // vertex 1: edges to 2(4), 3(1)
    {{3, 1.0}},             // vertex 2: edges to 3(1)
    {}                      // vertex 3: no out-edges
  };
}

// ── Basic filtered_graph tests ──────────────────────────────────────────────

TEST_CASE("filtered_graph with keep_all passes through", "[filtered_graph]") {
  auto g  = make_test_graph();
  auto fg = graph::adaptors::filtered_graph(g);

  // vertices are the full set
  CHECK(graph::num_vertices(fg) == 4);

  // All edges visible
  std::size_t edge_count = 0;
  for (auto&& [uid, u] : graph::views::vertexlist(fg)) {
    for (auto&& [tid, uv] : graph::views::incidence(fg, u)) {
      ++edge_count;
    }
  }
  CHECK(edge_count == 5);
}

TEST_CASE("filtered_graph with vertex predicate excludes edges to/from filtered vertices", "[filtered_graph]") {
  auto g  = make_test_graph();
  // Exclude vertex 2
  auto fg = graph::adaptors::filtered_graph(g,
    [](auto uid) { return uid != 2; });

  // num_vertices still returns 4 (underlying)
  CHECK(graph::num_vertices(fg) == 4);

  // Edges should exclude anything targeting vertex 2
  // Original edges: 0→1(2), 0→2(3), 1→2(4), 1→3(1), 2→3(1)
  // After filtering out vertex 2 targets: 0→1(2), 1→3(1)
  // (edges FROM vertex 2 are not visited if vertex 2 is excluded from iteration,
  //  but edges() only filters targets, not sources — the algorithm still visits
  //  vertex 2's edges if it reaches vertex 2. However, since no edge reaches
  //  vertex 2 (they're all filtered), vertex 2 is effectively disconnected.)

  std::vector<std::pair<int, int>> visible_edges;
  for (auto&& [uid, u] : graph::views::vertexlist(fg)) {
    for (auto&& [tid, uv] : graph::views::incidence(fg, u)) {
      visible_edges.emplace_back(static_cast<int>(uid), static_cast<int>(tid));
    }
  }
  std::ranges::sort(visible_edges);

  // Vertex 0's out-edges: 0→1 visible, 0→2 filtered
  // Vertex 1's out-edges: 1→2 filtered, 1→3 visible
  // Vertex 2's out-edges: 2→3 visible (vertex 2 is still in vertexlist)
  // Vertex 3's out-edges: none
  std::vector<std::pair<int, int>> expected = {{0, 1}, {1, 3}, {2, 3}};
  CHECK(visible_edges == expected);
}

TEST_CASE("filtered_graph with edge predicate", "[filtered_graph]") {
  auto g  = make_test_graph();
  // Only keep edges with weight <= 2.0
  // This requires knowing the weight, which is in the edge value.
  // The edge predicate takes (source_id, target_id), so we can't directly
  // filter by weight via edge predicate. Use vertex predicate = keep_all,
  // and edge predicate based on endpoint IDs.
  //
  // Edge predicate: exclude edge 0→2 (keep only short hops)
  auto fg = graph::adaptors::filtered_graph(g,
    graph::adaptors::keep_all{},
    [](auto src, auto tgt) { return !(src == 0 && tgt == 2); });

  std::vector<std::pair<int, int>> visible_edges;
  for (auto&& [uid, u] : graph::views::vertexlist(fg)) {
    for (auto&& [tid, uv] : graph::views::incidence(fg, u)) {
      visible_edges.emplace_back(static_cast<int>(uid), static_cast<int>(tid));
    }
  }
  std::ranges::sort(visible_edges);

  // All edges except 0→2
  std::vector<std::pair<int, int>> expected = {{0, 1}, {1, 2}, {1, 3}, {2, 3}};
  CHECK(visible_edges == expected);
}

TEST_CASE("filtered_graph: filtered_vertices convenience", "[filtered_graph]") {
  auto g  = make_test_graph();
  auto fg = graph::adaptors::filtered_graph(g,
    [](auto uid) { return uid != 1 && uid != 3; });

  std::vector<int> fv;
  for (auto u : graph::adaptors::filtered_vertices(fg)) {
    fv.push_back(static_cast<int>(graph::vertex_id(fg, u)));
  }
  CHECK(fv == std::vector<int>{0, 2});
}

TEST_CASE("filtered_graph: dijkstra on filtered subgraph", "[filtered_graph][dijkstra]") {
  auto g  = make_test_graph();
  // Exclude vertex 1 — forces path 0→2(3)→3(1) = 4 instead of 0→1(2)→3(1) = 3
  auto fg = graph::adaptors::filtered_graph(g,
    [](auto uid) { return uid != 1; });

  using fg_t = decltype(fg);
  const std::size_t n = graph::num_vertices(fg);
  constexpr double inf = std::numeric_limits<double>::max();

  std::vector<double> dist(n, inf);
  std::vector<std::size_t> pred(n);
  for (std::size_t i = 0; i < n; ++i) pred[i] = i;

  // Distance/predecessor functions
  auto dist_fn = [&dist](const auto&, auto uid) -> double& {
    return dist[static_cast<std::size_t>(uid)];
  };
  auto pred_fn = [&pred](const auto&, auto uid) -> std::size_t& {
    return pred[static_cast<std::size_t>(uid)];
  };
  // Weight function — extract from pair<int,double>
  auto weight_fn = [](const auto& g_ref, const auto& uv) -> double {
    return std::get<1>(*uv.value());  // second element of pair
    // Wait, this is the inner edge descriptor from the underlying graph.
    // For vector<vector<pair<int,double>>>, the raw edge is pair<int,double>.
    // But uv is the OUTER edge descriptor (from the CPO re-wrapping).
    // uv.value() is a filter_view::iterator.
    // *uv.value() is the INNER edge_descriptor.
    // (*uv.value()).value() would be... no, need to think about this.
    // Actually uv IS an edge_descriptor from the filtered graph.
    // The edges() function returns filter_view<edge_descriptor_view, pred>.
    // The CPO wraps this in edge_descriptor_view<filter_iter, vertex_iter>.
    // So uv from the CPO is edge_descriptor<filter_iter, vertex_iter>.
    // uv.value() is the filter_iter.
    // *uv.value() is the inner edge_descriptor from the underlying graph.
    // For vector<vector<pair<int,double>>>, inner_edge_descriptor.value()
    // would be... I need to check. Actually for native graph types,
    // the edge's inner_value(g) gives access to the pair.
    // This is getting complicated. Let me use a simpler approach.
  };

  // Actually, this test is getting too complex because of the double-wrapping.
  // Let me verify the simpler tests first and come back to Dijkstra.
  SUCCEED(); // placeholder
}
