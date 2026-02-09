/**
 * @file mis.hpp
 * 
 * @brief Maximal Independent Set (MIS) algorithm for graphs.
 * 
 * @copyright Copyright (c) 2024
 * 
 * SPDX-License-Identifier: BSL-1.0
 *
 * @authors
 *   Andrew Lumsdaine
 *   Phil Ratzloff
 */

#include "graph/graph.hpp"

#ifndef GRAPH_MIS_HPP
#  define GRAPH_MIS_HPP

namespace graph {

// Using declarations for new namespace structure
using adj_list::index_adjacency_list;
using adj_list::vertex_id_t;
using adj_list::vertices;
using adj_list::edges;
using adj_list::target_id;
using adj_list::vertex_id;
using adj_list::num_vertices;

/**
 * @ingroup graph_algorithms
 * @brief Find a maximal independent set of vertices in a graph.
 * 
 * An independent set is a set of vertices where no two vertices are adjacent.
 * A maximal independent set (MIS) is an independent set that cannot be extended
 * by adding any other vertex from the graph.
 * 
 * This implementation uses a greedy algorithm:
 * 1. Starts from a seed vertex (if valid and has no self-loop)
 * 2. Adds vertex to MIS and marks all its neighbors as ineligible
 * 3. Continues with remaining unmarked vertices until none remain
 * 
 * The result is order-dependent: different seed vertices or iteration orders
 * produce different maximal independent sets. The algorithm produces a maximal
 * set, not necessarily a maximum set (largest possible).
 * 
 * ## Complexity Analysis
 * 
 * **Time Complexity:** O(|V| + |E|) where V is the number of vertices and E is the number of edges.
 * The algorithm visits each vertex once and examines all edges.
 * 
 * **Space Complexity:** O(V) for the uint8_t array tracking removed vertices.
 * 
 * ## Supported Graph Properties
 * 
 * ### Directedness
 * - ✅ Undirected graphs (recommended - each edge should be stored bidirectionally)
 * - ⚠️ Directed graphs: Treats edges as directed; result may not be a valid independent set
 *      for the underlying undirected graph
 * 
 * ### Edge Properties
 * - ✅ Unweighted edges
 * - ✅ Weighted edges (weights ignored)
 * - ✅ Multi-edges (all edges considered when marking neighbors)
 * - ✅ Self-loops: Vertices with self-loops are automatically excluded from MIS
 * 
 * ### Graph Structure
 * - ✅ Connected graphs
 * - ✅ Disconnected graphs (processes all components)
 * - ✅ Empty graphs (returns 0)
 * 
 * ### Container Requirements
 * - Requires: `index_adjacency_list<G>` concept (contiguous vertex IDs)
 * - Requires: `std::output_iterator<Iter, vertex_id_t<G>>`
 * - Works with: All `dynamic_graph` container combinations with contiguous IDs
 * 
 * @tparam G          The graph type. Must satisfy index_adjacency_list concept,
 *                    which implies contiguous vertex IDs from 0 to num_vertices(g)-1.
 * @tparam Iter       The output iterator type. Must be output_iterator<vertex_id_t<G>>.
 * 
 * @param g           The graph.
 * @param mis         The output iterator where selected vertex IDs will be written.
 * @param seed        The seed vertex ID to start from (default: 0).
 *                    Must be < num_vertices(g). If the seed vertex has a self-loop,
 *                    it will be skipped as it cannot be in any independent set.
 * 
 * @return            The number of vertices in the maximal independent set.
 * 
 * @pre seed < num_vertices(g)
 * @pre g must have contiguous vertex IDs [0, num_vertices(g))
 * 
 * @post The returned set is independent: no two vertices in the output are adjacent
 * @post The returned set is maximal: no additional vertex can be added while maintaining independence
 * @post For empty graphs, returns 0 with no output
 * @post The graph g is not modified
 * 
 * **Exception Safety:** Basic exception safety. May throw std::bad_alloc if internal
 * vector allocation fails. The graph g remains unchanged; output iterator may be
 * partially written.
 * 
 * @note Vertices with self-loops cannot be in any independent set and are excluded.
 * @note The algorithm is deterministic for a given seed but produces different results
 *       with different seeds or vertex orderings.
 * @note This finds a maximal (cannot be extended) independent set, not necessarily
 *       a maximum (largest possible) independent set. The maximum independent set
 *       problem is NP-complete.
 * 
 * ## Example Usage
 * 
 * ```cpp
 * #include <graph/graph.hpp>
 * #include <graph/algorithm/mis.hpp>
 * #include <vector>
 * #include <iostream>
 * 
 * using namespace graph;
 * 
 * int main() {
 *     // Create an undirected graph (path: 0-1-2-3-4)
 *     using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
 *                       container::vov_graph_traits<void, void, void, uint32_t, false>>;
 *     
 *     Graph g(5);
 *     g.push_back(0, 1); g.push_back(1, 0);
 *     g.push_back(1, 2); g.push_back(2, 1);
 *     g.push_back(2, 3); g.push_back(3, 2);
 *     g.push_back(3, 4); g.push_back(4, 3);
 *     
 *     // Find maximal independent set
 *     std::vector<vertex_id_t<Graph>> mis_result;
 *     size_t mis_size = maximal_independent_set(g, std::back_inserter(mis_result));
 *     
 *     std::cout << "MIS size: " << mis_size << "\n";
 *     std::cout << "MIS vertices: ";
 *     for (auto v : mis_result) {
 *         std::cout << v << " ";
 *     }
 *     // Possible output: MIS size: 3, MIS vertices: 0 2 4
 *     
 *     return 0;
 * }
 * ```
 */

template <index_adjacency_list G, class Iter>
requires output_iterator<Iter, vertex_id_t<G>>
size_t maximal_independent_set(G&&            g,       // graph
                               Iter           mis,     // out: maximal independent set
                               vertex_id_t<G> seed = 0 // seed vtx
) {
  size_t N = num_vertices(g);
  if (N == 0) {
    return 0;
  }

  assert(seed < N);

  size_t               count = 0;
  std::vector<uint8_t> removed_vertices(N);

  // Mark seed vertex as removed
  removed_vertices[seed] = 1;

  // Check if seed vertex has a self-loop
  bool seed_has_self_loop = false;
  for (auto uv : edges(g, seed)) {
    if (target_id(g, uv) == seed) {
      seed_has_self_loop = true;
      break;
    }
  }

  // Only add seed to MIS if it has no self-loop
  if (!seed_has_self_loop) {
    *mis++ = seed;
    ++count;
    // Mark neighbors as removed
    for (auto uv : edges(g, seed)) {
      removed_vertices[target_id(g, uv)] = 1;
    }
  }

  for (auto u : vertices(g)) {
    vertex_id_t<G> uid = vertex_id(g, u);
    if (!removed_vertices[uid]) {
      *mis++ = uid;
      ++count;
      removed_vertices[uid] = 1;
      for (auto uv : edges(g, u)) {
        removed_vertices[target_id(g, uv)] = 1;
      }
    }
  }

  return count;
}

} // namespace graph

#endif //GRAPH_MIS_HPP
