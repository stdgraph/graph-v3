/**
 * @file map_graph_fixtures.hpp
 * @brief Reusable graph fixtures for testing algorithms with map-based vertex containers
 *
 * Provides factory functions that build small graphs matching topologies used in
 * existing algorithm tests, but using sparse vertex IDs for map-based graph types.
 * Index-based graph types get standard contiguous IDs for comparison.
 *
 * Uses fixture_selector<Graph>::use_sparse from algorithm_test_types.hpp to
 * automatically choose the ID scheme based on graph type.
 *
 * Usage:
 *   auto g = map_fixtures::clrs_dijkstra_graph<Graph>();
 *   auto expected = map_fixtures::clrs_dijkstra_expected<Graph>();
 */

#ifndef MAP_GRAPH_FIXTURES_HPP
#define MAP_GRAPH_FIXTURES_HPP

#include <graph/graph.hpp>
#include <graph/container/dynamic_graph.hpp>
#include "algorithm_test_types.hpp"
#include <array>
#include <limits>
#include <cstdint>

namespace graph::test::map_fixtures {

using namespace graph::test::algorithm;

template <typename T>
constexpr T infinity = std::numeric_limits<T>::max();

// =============================================================================
// CLRS Dijkstra Example (Figure 24.6) — Sparse Vertex IDs
//
// Topology identical to fixtures::clrs_dijkstra_graph but with sparse IDs:
//   Index: s=0, t=1, x=2, y=3, z=4
//   Sparse: s=10, t=20, x=30, y=40, z=50
// =============================================================================

struct clrs_dijkstra_sparse_expected {
  static constexpr size_t                  num_vertices = 5;
  static constexpr size_t                  num_edges    = 10;
  static constexpr std::array<uint32_t, 5> vertex_ids   = {10, 20, 30, 40, 50};

  // Shortest distances from s (vertex 10)
  static constexpr std::array<int, 5> distances = {0, 8, 9, 5, 7}; // s,t,x,y,z

  static constexpr uint32_t s = 10;
  static constexpr uint32_t t = 20;
  static constexpr uint32_t x = 30;
  static constexpr uint32_t y = 40;
  static constexpr uint32_t z = 50;
};

template <typename Graph>
Graph clrs_dijkstra_graph_sparse() {
  // Same topology as CLRS Figure 24.6 but with sparse IDs: 10,20,30,40,50
  return Graph({
        {10, 20, 10},
        {10, 40, 5},  // s -> t, s -> y
        {20, 30, 1},
        {20, 40, 2},  // t -> x, t -> y
        {30, 50, 4},  // x -> z
        {40, 20, 3},
        {40, 30, 9},
        {40, 50, 2},  // y -> t, y -> x, y -> z
        {50, 10, 7},
        {50, 30, 6}   // z -> s, z -> x
  });
}

/**
 * @brief Unified factory: returns CLRS Dijkstra graph with appropriate IDs.
 * Uses contiguous IDs for index graphs, sparse IDs for map-based graphs.
 */
template <typename Graph>
Graph clrs_dijkstra_graph() {
  if constexpr (is_sparse_vertex_container_v<Graph>) {
    return clrs_dijkstra_graph_sparse<Graph>();
  } else {
    return fixtures::clrs_dijkstra_graph<Graph>();
  }
}

// =============================================================================
// Simple BFS/DFS Graph — Sparse
//
// Topology: 0->1, 0->2, 1->3, 2->3, 3->4
// Sparse:  10->20, 10->30, 20->40, 30->40, 40->50
// =============================================================================

struct bfs_sparse_expected {
  static constexpr size_t                  num_vertices = 5;
  static constexpr size_t                  num_edges    = 5;
  static constexpr std::array<uint32_t, 5> vertex_ids   = {10, 20, 30, 40, 50};
  static constexpr uint32_t               source       = 10;
  // BFS from 10: discovers 20,30 (distance 1), then 40 (distance 2), then 50 (distance 3)
};

template <typename Graph>
Graph bfs_graph_sparse() {
  return Graph({{10, 20, 1}, {10, 30, 1}, {20, 40, 1}, {30, 40, 1}, {40, 50, 1}});
}

template <typename Graph>
Graph bfs_graph_contiguous() {
  return Graph({{0, 1, 1}, {0, 2, 1}, {1, 3, 1}, {2, 3, 1}, {3, 4, 1}});
}

/**
 * @brief Unified factory: returns BFS graph with appropriate IDs.
 */
template <typename Graph>
Graph bfs_graph() {
  if constexpr (is_sparse_vertex_container_v<Graph>) {
    return bfs_graph_sparse<Graph>();
  } else {
    return bfs_graph_contiguous<Graph>();
  }
}

/// Returns the source vertex ID for BFS tests, appropriate for the graph type.
template <typename Graph>
constexpr auto bfs_source() {
  if constexpr (is_sparse_vertex_container_v<Graph>) {
    return vertex_id_t<Graph>(10);
  } else {
    return vertex_id_t<Graph>(0);
  }
}

// =============================================================================
// DAG for Topological Sort — Sparse
//
// Topology (diamond): 0->1, 0->2, 1->3, 2->3
// Sparse:            10->20, 10->30, 20->40, 30->40
// =============================================================================

struct dag_sparse_expected {
  static constexpr size_t                  num_vertices = 4;
  static constexpr size_t                  num_edges    = 4;
  static constexpr std::array<uint32_t, 4> vertex_ids   = {10, 20, 30, 40};
  // Valid topological orders (sparse): [10,20,30,40], [10,30,20,40]
};

template <typename Graph>
Graph dag_graph_sparse() {
  return Graph({{10, 20, 1}, {10, 30, 1}, {20, 40, 1}, {30, 40, 1}});
}

/**
 * @brief Unified factory: returns DAG with appropriate IDs.
 */
template <typename Graph>
Graph dag_graph() {
  if constexpr (is_sparse_vertex_container_v<Graph>) {
    return dag_graph_sparse<Graph>();
  } else {
    return fixtures::diamond_dag<Graph>();
  }
}

// =============================================================================
// Cycle Graph — Sparse (for DFS cycle detection)
//
// Topology: 0->1->2->3->0  (directed cycle)
// Sparse:  10->20->30->40->10
// =============================================================================

template <typename Graph>
Graph cycle_graph_sparse() {
  return Graph({{10, 20, 1}, {20, 30, 1}, {30, 40, 1}, {40, 10, 1}});
}

template <typename Graph>
Graph cycle_graph_contiguous() {
  return Graph({{0, 1, 1}, {1, 2, 1}, {2, 3, 1}, {3, 0, 1}});
}

/**
 * @brief Unified factory: returns cycle graph with appropriate IDs.
 */
template <typename Graph>
Graph cycle_graph() {
  if constexpr (is_sparse_vertex_container_v<Graph>) {
    return cycle_graph_sparse<Graph>();
  } else {
    return cycle_graph_contiguous<Graph>();
  }
}

/// Returns the source vertex ID for cycle graph tests, appropriate for the graph type.
template <typename Graph>
constexpr auto cycle_source() {
  if constexpr (is_sparse_vertex_container_v<Graph>) {
    return vertex_id_t<Graph>(10);
  } else {
    return vertex_id_t<Graph>(0);
  }
}

// =============================================================================
// Disconnected Graph — Sparse (two components)
//
// Component 1: 100 -> 200
// Component 2: 300 -> 400 -> 500
// =============================================================================

struct disconnected_sparse_expected {
  static constexpr size_t num_vertices   = 5;
  static constexpr size_t num_edges      = 3;
  static constexpr size_t num_components = 2;
};

template <typename Graph>
Graph disconnected_graph_sparse() {
  return Graph({{100, 200, 1}, {300, 400, 1}, {400, 500, 1}});
}

/**
 * @brief Unified factory: returns disconnected graph with appropriate IDs.
 */
template <typename Graph>
Graph disconnected_graph() {
  if constexpr (is_sparse_vertex_container_v<Graph>) {
    return disconnected_graph_sparse<Graph>();
  } else {
    return fixtures::disconnected_graph<Graph>();
  }
}

} // namespace graph::test::map_fixtures

#endif // MAP_GRAPH_FIXTURES_HPP
