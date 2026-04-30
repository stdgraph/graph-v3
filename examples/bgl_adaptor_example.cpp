/**
 * @file bgl_adaptor_example.cpp
 * @brief Example: Using graph-v3 algorithms on a Boost.Graph adjacency_list.
 *
 * This example demonstrates how to:
 *   1. Build a weighted directed graph using BGL's adjacency_list
 *   2. Wrap it with graph::bgl::graph_adaptor (one line)
 *   3. Bridge BGL property maps via make_bgl_edge_weight_fn
 *   4. Run graph-v3's dijkstra_shortest_paths
 *   5. Iterate results using graph-v3's vertexlist view
 */

#include <graph/graph.hpp>
#include <graph/algorithm/dijkstra_shortest_paths.hpp>
#include <graph/adaptors/bgl/graph_adaptor.hpp>
#include <graph/adaptors/bgl/property_bridge.hpp>

#include <boost/graph/adjacency_list.hpp>

#include <iostream>
#include <limits>
#include <vector>

// ── BGL graph definition ────────────────────────────────────────────────────

struct EdgeWeight {
  double weight;
};

using BGL_Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                         boost::no_property, EdgeWeight>;

int main() {
  // 1. Build a BGL graph (CLRS Dijkstra example)
  //
  //        0
  //       /|\
  //     10  5  2
  //     /   |   \
  //    1    2    4
  //    \   /|   /
  //     1 3  2  3
  //      \|   \/
  //       3
  //
  BGL_Graph bgl_g(5);
  boost::add_edge(0, 1, EdgeWeight{10.0}, bgl_g);
  boost::add_edge(0, 2, EdgeWeight{5.0}, bgl_g);
  boost::add_edge(0, 4, EdgeWeight{2.0}, bgl_g);
  boost::add_edge(1, 3, EdgeWeight{1.0}, bgl_g);
  boost::add_edge(2, 1, EdgeWeight{3.0}, bgl_g);
  boost::add_edge(2, 3, EdgeWeight{9.0}, bgl_g);
  boost::add_edge(2, 4, EdgeWeight{2.0}, bgl_g);
  boost::add_edge(3, 4, EdgeWeight{7.0}, bgl_g);
  boost::add_edge(4, 3, EdgeWeight{3.0}, bgl_g);

  // 2. Wrap with graph_adaptor — one line, non-owning
  auto g = graph::bgl::graph_adaptor(bgl_g);

  // 3. Set up distance and predecessor storage
  const auto n = graph::num_vertices(g);
  std::vector<double>      dist(n, std::numeric_limits<double>::max());
  std::vector<std::size_t> pred(n);
  for (std::size_t i = 0; i < n; ++i)
    pred[i] = i;

  // Bridge BGL property map → graph-v3 edge weight function
  auto weight_fn = graph::bgl::make_bgl_edge_weight_fn(
      boost::get(&EdgeWeight::weight, bgl_g));

  // Wrap vectors as graph-v3 property functions
  auto dist_fn = graph::bgl::make_vertex_id_property_fn(dist);
  auto pred_fn = graph::bgl::make_vertex_id_property_fn(pred);

  // 4. Run Dijkstra from vertex 0
  dist_fn(g, std::size_t{0}) = 0.0;
  std::vector<std::size_t> sources = {0};
  graph::dijkstra_shortest_paths(g, sources, dist_fn, pred_fn, weight_fn);

  // 5. Print results
  std::cout << "Shortest paths from vertex 0:\n";
  for (std::size_t v = 0; v < n; ++v) {
    std::cout << "  0 -> " << v << " : distance = " << dist[v]
              << ", predecessor = " << pred[v] << "\n";
  }

  // Expected output:
  //   0 -> 0 : distance = 0, predecessor = 0
  //   0 -> 1 : distance = 8, predecessor = 2
  //   0 -> 2 : distance = 5, predecessor = 0
  //   0 -> 3 : distance = 5, predecessor = 4
  //   0 -> 4 : distance = 2, predecessor = 0

  return 0;
}
