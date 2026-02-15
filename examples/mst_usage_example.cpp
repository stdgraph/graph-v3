/**
 * @file mst_usage_example.cpp
 * @brief Example demonstrating the enhanced MST API with return values and validation
 */

#include "graph/algorithm/mst.hpp"
#include "graph/edge_list/edge_list_descriptor.hpp"
#include "graph/container/dynamic_graph.hpp"
#include <iostream>
#include <vector>
#include <format>

using namespace graph;

int main() {
  // Example 1: Kruskal's algorithm with return values
  std::cout << "=== Kruskal's Algorithm Example ===\n\n";

  using Edge              = edge_descriptor<uint32_t, int>;
  std::vector<Edge> edges = {{0, 1, 4}, {1, 2, 8}, {2, 3, 7}, {3, 0, 9}, {0, 2, 2}, {1, 3, 5}};

  std::cout << "Input edges (source, target, weight):\n";
  for (const auto& e : edges) {
    std::cout << std::format("  ({}, {}, {})\n", e.source_id, e.target_id, e.value);
  }

  std::vector<Edge> mst;
  auto [total_weight, num_components] = kruskal(edges, mst);

  std::cout << "\nMinimum Spanning Tree:\n";
  for (const auto& e : mst) {
    std::cout << std::format("  ({}, {}, {})\n", e.source_id, e.target_id, e.value);
  }
  std::cout << std::format("\nTotal MST weight: {}\n", total_weight);
  std::cout << std::format("Number of components: {}\n", num_components);

  // Example 2: Disconnected graph
  std::cout << "\n=== Disconnected Graph Example ===\n\n";

  std::vector<Edge> disconnected_edges = {
        {0, 1, 1},
        {1, 2, 2}, // First component
        {3, 4, 3},
        {4, 5, 4} // Second component
  };

  std::vector<Edge> forest;
  auto [forest_weight, components] = kruskal(disconnected_edges, forest);

  std::cout << std::format("Spanning Forest:\n");
  std::cout << std::format("  Total weight: {}\n", forest_weight);
  std::cout << std::format("  Components: {}\n", components);
  std::cout << std::format("  Edges in forest: {}\n", forest.size());

  // Example 3: Maximum Spanning Tree
  std::cout << "\n=== Maximum Spanning Tree Example ===\n\n";

  std::vector<Edge> max_edges = {{0, 1, 4}, {1, 2, 8}, {0, 2, 2}};
  std::vector<Edge> max_st;
  auto [max_weight, _] = kruskal(max_edges, max_st, std::greater<int>{});

  std::cout << "Maximum Spanning Tree:\n";
  for (const auto& e : max_st) {
    std::cout << std::format("  ({}, {}, {})\n", e.source_id, e.target_id, e.value);
  }
  std::cout << std::format("Total weight: {}\n", max_weight);

  // Example 4: Prim's algorithm with validation
  std::cout << "\n=== Prim's Algorithm Example ===\n\n";

  using Graph = container::dynamic_graph<int, void, void, uint32_t, false, container::vov_graph_traits<int>>;

  // Create undirected weighted graph
  Graph g({{0, 1, 4}, {1, 0, 4}, {1, 2, 8}, {2, 1, 8}, {2, 0, 11}, {0, 2, 11}, {0, 2, 2}, {2, 0, 2}});

  size_t                n = num_vertices(g);
  std::vector<uint32_t> predecessor(n);
  std::vector<int>      weight(n);

  try {
    auto total_wt = prim(g, predecessor, weight, 0);

    std::cout << "MST from vertex 0:\n";
    std::cout << std::format("  Total weight: {}\n", total_wt);
    std::cout << "\n  MST edges (predecessor -> vertex: weight):\n";
    for (size_t v = 0; v < n; ++v) {
      if (v != 0 && predecessor[v] != v) {
        std::cout << std::format("    {} -> {}: {}\n", predecessor[v], v, weight[v]);
      }
    }
  } catch (const std::out_of_range& e) {
    std::cout << "Error: " << e.what() << "\n";
  }

  // Example 5: Demonstrate input validation
  std::cout << "\n=== Input Validation Example ===\n\n";

  try {
    std::vector<uint32_t> small_pred(2); // Too small!
    std::vector<int>      small_wt(2);
    prim(g, small_pred, small_wt, 0);
  } catch (const std::out_of_range& e) {
    std::cout << "Caught expected error:\n  " << e.what() << "\n";
  }

  try {
    std::vector<uint32_t> pred(n);
    std::vector<int>      wt(n);
    prim(g, pred, wt, 999); // Invalid seed
  } catch (const std::out_of_range& e) {
    std::cout << "\nCaught expected error:\n  " << e.what() << "\n";
  }

  return 0;
}
