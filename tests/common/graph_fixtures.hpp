/**
 * @file graph_fixtures.hpp
 * @brief Reusable graph fixtures for algorithm testing
 * 
 * Provides a comprehensive suite of graph test data suitable for algorithm development,
 * testing, and benchmarking. Each fixture includes:
 * - Factory functions for creating graphs with various container types
 * - Expected results structures for validation
 * - Support for both directed and undirected graphs where applicable
 * - Real-world themed examples for better understanding
 * 
 * Usage patterns:
 *   // 1. Create graph with specific container type
 *   auto g = test::fixtures::path_graph_4<vov_graph>();
 * 
 *   // 2. Use in parameterized tests
 *   TEMPLATE_TEST_CASE_METHOD(TestFixture, "algo", "[tag]", vov_tag, dov_tag) {
 *       using Types = graph_test_types<TestType>;
 *       using Graph = typename Types::int_ev;
 *       auto g = test::fixtures::cycle_graph_5<Graph>();
 *       // ... test algorithm
 *   }
 * 
 *   // 3. Access expected results
 *   auto expected = test::fixtures::path_graph_4_results;
 *   REQUIRE(num_vertices(g) == expected.num_vertices);
 */

#ifndef GRAPH_FIXTURES_HPP
#define GRAPH_FIXTURES_HPP

#include <graph/graph.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <vector>
#include <array>
#include <string>
#include <limits>
#include <cstdint>

namespace graph::test::fixtures {

// =============================================================================
// Helper Types
// =============================================================================

template <typename T>
constexpr T infinity = std::numeric_limits<T>::max();

// =============================================================================
// Empty Graph Fixture
// =============================================================================

struct empty_graph_results {
  static constexpr size_t num_vertices = 0;
  static constexpr size_t num_edges    = 0;
};

template <typename Graph>
Graph empty_graph() {
  return Graph();
}

// =============================================================================
// Single Vertex Fixture
// =============================================================================

struct single_vertex_results {
  static constexpr size_t num_vertices = 1;
  static constexpr size_t num_edges    = 0;
};

template <typename Graph>
Graph single_vertex() {
  Graph g;
  g.resize_vertices(1);
  return g;
}

// =============================================================================
// Single Edge Fixture (0 -> 1)
// =============================================================================

struct single_edge_results {
  static constexpr size_t num_vertices = 2;
  static constexpr size_t num_edges    = 1;
};

template <typename Graph>
Graph single_edge() {
  return Graph({{0, 1}});
}

template <typename Graph>
Graph single_edge_weighted() {
  return Graph({{0, 1, 10}});
}

// =============================================================================
// Self-Loop Fixture (vertex with edge to itself)
// =============================================================================

struct self_loop_results {
  static constexpr size_t num_vertices = 1;
  static constexpr size_t num_edges    = 1;
};

template <typename Graph>
Graph self_loop() {
  return Graph({{0, 0}});
}

// =============================================================================
// Path Graph Fixture: 0 -> 1 -> 2 -> 3
// Linear chain of vertices
// =============================================================================

struct path_graph_4_results {
  static constexpr size_t                  num_vertices = 4;
  static constexpr size_t                  num_edges    = 3;
  static constexpr std::array<uint32_t, 4> vertices     = {0, 1, 2, 3};
  static constexpr std::array<size_t, 4>   out_degrees  = {1, 1, 1, 0};

  // For shortest path algorithms from vertex 0
  static constexpr std::array<int, 4> distances    = {0, 1, 2, 3};
  static constexpr std::array<int, 4> predecessors = {0, 0, 1, 2}; // -1 would be better but constexpr issues
};

template <typename Graph>
Graph path_graph_4() {
  return Graph({{0, 1}, {1, 2}, {2, 3}});
}

template <typename Graph>
Graph path_graph_4_weighted() {
  return Graph({{0, 1, 1}, {1, 2, 1}, {2, 3, 1}});
}

// =============================================================================
// Cycle Graph Fixture: 0 -> 1 -> 2 -> 3 -> 4 -> 0
// Circular path
// =============================================================================

struct cycle_graph_5_results {
  static constexpr size_t                  num_vertices = 5;
  static constexpr size_t                  num_edges    = 5;
  static constexpr std::array<uint32_t, 5> vertices     = {0, 1, 2, 3, 4};
  static constexpr std::array<size_t, 5>   out_degrees  = {1, 1, 1, 1, 1};
};

template <typename Graph>
Graph cycle_graph_5() {
  return Graph({{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0}});
}

template <typename Graph>
Graph cycle_graph_5_weighted() {
  return Graph({{0, 1, 1}, {1, 2, 1}, {2, 3, 1}, {3, 4, 1}, {4, 0, 1}});
}

// =============================================================================
// Complete Graph K4: Every vertex connected to every other vertex
// =============================================================================

struct complete_graph_4_results {
  static constexpr size_t                  num_vertices = 4;
  static constexpr size_t                  num_edges    = 12; // directed: n*(n-1) = 4*3
  static constexpr std::array<uint32_t, 4> vertices     = {0, 1, 2, 3};
  static constexpr std::array<size_t, 4>   out_degrees  = {3, 3, 3, 3};
};

template <typename Graph>
Graph complete_graph_4() {
  return Graph({{0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 2}, {1, 3}, {2, 0}, {2, 1}, {2, 3}, {3, 0}, {3, 1}, {3, 2}});
}

// =============================================================================
// Star Graph: Central vertex 0 connected to all others
// Pattern: hub-and-spoke topology (useful for network algorithms)
// =============================================================================

struct star_graph_5_results {
  static constexpr size_t                  num_vertices  = 5;
  static constexpr size_t                  num_edges     = 4; // undirected edges from center
  static constexpr std::array<uint32_t, 5> vertices      = {0, 1, 2, 3, 4};
  static constexpr size_t                  center_vertex = 0;
  static constexpr std::array<size_t, 5>   out_degrees   = {4, 0, 0, 0, 0}; // directed version
};

template <typename Graph>
Graph star_graph_5() {
  return Graph({{0, 1}, {0, 2}, {0, 3}, {0, 4}});
}

// =============================================================================
// Binary Tree: Complete binary tree with 7 vertices
//       0
//      / \
//     1   2
//    / \ / \
//   3  4 5  6
// =============================================================================

struct binary_tree_7_results {
  static constexpr size_t                  num_vertices = 7;
  static constexpr size_t                  num_edges    = 6;
  static constexpr std::array<uint32_t, 7> vertices     = {0, 1, 2, 3, 4, 5, 6};
  static constexpr size_t                  root         = 0;
  static constexpr size_t                  height       = 2; // levels: 0, 1, 2
};

template <typename Graph>
Graph binary_tree_7() {
  return Graph({
        {0, 1},
        {0, 2}, // root to level 1
        {1, 3},
        {1, 4}, // left subtree
        {2, 5},
        {2, 6} // right subtree
  });
}

// =============================================================================
// DAG (Directed Acyclic Graph) - Simple Diamond Shape
//     0
//    / \
//   1   2
//    \ /
//     3
// Useful for topological sort, critical path, etc.
// =============================================================================

struct diamond_dag_results {
  static constexpr size_t                  num_vertices = 4;
  static constexpr size_t                  num_edges    = 4;
  static constexpr std::array<uint32_t, 4> vertices     = {0, 1, 2, 3};
  // Valid topological orders: [0,1,2,3], [0,2,1,3]
};

template <typename Graph>
Graph diamond_dag() {
  return Graph({{0, 1}, {0, 2}, {1, 3}, {2, 3}});
}

template <typename Graph>
Graph diamond_dag_weighted() {
  return Graph({{0, 1, 5}, {0, 2, 3}, {1, 3, 2}, {2, 3, 7}});
}

// =============================================================================
// Disconnected Graph: Two separate components
// Component 1: 0 -> 1
// Component 2: 2 -> 3 -> 4
// =============================================================================

struct disconnected_graph_results {
  static constexpr size_t num_vertices   = 5;
  static constexpr size_t num_edges      = 3;
  static constexpr size_t num_components = 2;
};

template <typename Graph>
Graph disconnected_graph() {
  return Graph({{0, 1}, {2, 3}, {3, 4}});
}

// =============================================================================
// Multi-Edge Graph: Multiple edges between same vertex pairs
// 0 -> 1 (weight 10)
// 0 -> 1 (weight 5)
// 1 -> 2 (weight 3)
// =============================================================================

struct multi_edge_graph_results {
  static constexpr size_t num_vertices = 3;
  static constexpr size_t num_edges    = 3; // including parallel edges
};

template <typename Graph>
Graph multi_edge_graph() {
  return Graph({{0, 1, 10}, {0, 1, 5}, {1, 2, 3}});
}

// =============================================================================
// Weighted Graph - CLRS Dijkstra Example (Figure 24.6)
// Classic example from "Introduction to Algorithms" textbook
// Vertices: s=0, t=1, x=2, y=3, z=4
// =============================================================================

struct clrs_dijkstra_results {
  static constexpr size_t                  num_vertices = 5;
  static constexpr size_t                  num_edges    = 10;
  static constexpr std::array<uint32_t, 5> vertices     = {0, 1, 2, 3, 4}; // s,t,x,y,z

  // Shortest distances from s (vertex 0)
  static constexpr std::array<int, 5> distances_from_0 = {0, 8, 9, 5, 7};

  // Named vertex mapping (for documentation)
  static constexpr uint32_t s = 0;
  static constexpr uint32_t t = 1;
  static constexpr uint32_t x = 2;
  static constexpr uint32_t y = 3;
  static constexpr uint32_t z = 4;
};

template <typename Graph>
Graph clrs_dijkstra_graph() {
  return Graph({
        {0, 1, 10},
        {0, 3, 5}, // s -> t, s -> y
        {1, 2, 1},
        {1, 3, 2}, // t -> x, t -> y
        {2, 4, 4}, // x -> z
        {3, 1, 3},
        {3, 2, 9},
        {3, 4, 2}, // y -> t, y -> x, y -> z
        {4, 0, 7},
        {4, 2, 6} // z -> s, z -> x
  });
}

// =============================================================================
// Bipartite Graph: Two sets with edges only between sets
// Set A: {0, 1, 2}
// Set B: {3, 4, 5}
// Edges: 0->3, 0->4, 1->4, 1->5, 2->3, 2->5
// =============================================================================

struct bipartite_graph_results {
  static constexpr size_t                  num_vertices = 6;
  static constexpr size_t                  num_edges    = 6;
  static constexpr std::array<uint32_t, 3> set_a        = {0, 1, 2};
  static constexpr std::array<uint32_t, 3> set_b        = {3, 4, 5};
};

template <typename Graph>
Graph bipartite_graph() {
  return Graph({{0, 3}, {0, 4}, {1, 4}, {1, 5}, {2, 3}, {2, 5}});
}

// =============================================================================
// Real-World Example: City Road Network
// Small road network for shortest path algorithms
// Cities: 0=Seattle, 1=Portland, 2=SanFrancisco, 3=LosAngeles, 4=SanDiego
// =============================================================================

struct road_network_results {
  static constexpr size_t num_vertices = 5;
  static constexpr size_t num_edges    = 7;

  // Named cities (for documentation)
  static constexpr uint32_t seattle       = 0;
  static constexpr uint32_t portland      = 1;
  static constexpr uint32_t san_francisco = 2;
  static constexpr uint32_t los_angeles   = 3;
  static constexpr uint32_t san_diego     = 4;

  // Approximate distances in miles (for weighted version)
  // Seattle-Portland: 173, Portland-SF: 635, Seattle-SF: 808
  // SF-LA: 383, LA-SD: 120, Portland-LA: 965
};

template <typename Graph>
Graph road_network() {
  return Graph({
        {0, 1, 173},
        {1, 0, 173}, // Seattle <-> Portland (bidirectional)
        {1, 2, 635},
        {2, 1, 635}, // Portland <-> SF
        {0, 2, 808},
        {2, 0, 808}, // Seattle <-> SF
        {2, 3, 383},
        {3, 2, 383}, // SF <-> LA
        {3, 4, 120},
        {4, 3, 120}, // LA <-> SD
        {1, 3, 965},
        {3, 1, 965} // Portland <-> LA
  });
}

// =============================================================================
// Real-World Example: Social Network (Actors & Movies)
// "Six Degrees of Kevin Bacon" style graph
// Vertices: actors, Edges: appeared in same movie
// =============================================================================

struct actor_network_results {
  static constexpr size_t num_vertices = 6;
  static constexpr size_t num_edges    = 7;

  // Named actors (for documentation, using numbers in code)
  // 0=Kevin Bacon, 1=Tom Hanks, 2=Gary Sinise, 3=Bill Paxton, 4=Ed Harris, 5=Meg Ryan
};

template <typename Graph>
Graph actor_network() {
  return Graph({
        {0, 1},
        {1, 0}, // Bacon <-> Hanks (Apollo 13)
        {0, 2},
        {2, 0}, // Bacon <-> Sinise (Apollo 13)
        {1, 2},
        {2, 1}, // Hanks <-> Sinise (Forrest Gump, Apollo 13)
        {1, 5},
        {5, 1}, // Hanks <-> Ryan (multiple movies)
        {3, 4},
        {4, 3}, // Paxton <-> Harris (Apollo 13)
        {0, 3},
        {3, 0}, // Bacon <-> Paxton (Apollo 13)
        {0, 4}  // Bacon <-> Harris (Apollo 13)
  });
}

// =============================================================================
// Medium-Scale Graph: Random-ish structure for performance testing
// =============================================================================

struct medium_graph_results {
  static constexpr size_t num_vertices = 50;
  // num_edges will vary based on construction
};

template <typename Graph>
Graph medium_graph_sparse() {
  // Create a graph with ~100 edges (average degree ~2)
  std::vector<std::pair<uint32_t, uint32_t>> edges;

  // Create a connected backbone (path)
  for (uint32_t i = 0; i < 49; ++i) {
    edges.push_back({i, i + 1});
  }

  // Add some random additional edges for complexity
  edges.push_back({0, 10});
  edges.push_back({5, 15});
  edges.push_back({10, 25});
  edges.push_back({15, 35});
  edges.push_back({20, 40});
  edges.push_back({25, 45});
  edges.push_back({30, 48});

  return Graph(edges);
}

// =============================================================================
// Helper: Create graph from edge list (for dynamic construction)
// =============================================================================

template <typename Graph, typename EdgeList>
Graph create_graph_from_edges(const EdgeList& edges, size_t num_vertices) {
  Graph g;
  if constexpr (requires { resize_vertices(g, num_vertices); }) {
    resize_vertices(g, num_vertices);
  }
  for (const auto& e : edges) {
    if constexpr (std::tuple_size_v<std::decay_t<decltype(e)>> == 2) {
      create_edge(g, std::get<0>(e), std::get<1>(e));
    } else {
      create_edge(g, std::get<0>(e), std::get<1>(e), std::get<2>(e));
    }
  }
  return g;
}

} // namespace graph::test::fixtures

#endif // GRAPH_FIXTURES_HPP
