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
#include "graph/adj_list/vertex_property_map.hpp"

#ifndef GRAPH_MIS_HPP
#  define GRAPH_MIS_HPP

namespace graph {

// Using declarations for new namespace structure
using adj_list::adjacency_list;
using adj_list::vertex_id_t;
using adj_list::vertices;
using adj_list::edges;
using adj_list::target_id;
using adj_list::vertex_id;
using adj_list::find_vertex;
using adj_list::num_vertices;

/**
 * @ingroup graph_algorithms
 * @brief Find a maximal independent set of vertices in a graph.
 * 
 * An independent set is a set of vertices where no two vertices are adjacent.
 * A maximal independent set (MIS) is one that cannot be extended by adding any
 * other vertex. This greedy algorithm is order-dependent and produces a maximal
 * set, not necessarily a maximum (largest possible) set.
 * 
 * @tparam G          The graph type. Must satisfy adjacency_list concept.
 * @tparam Iter       The output iterator type. Must be output_iterator<vertex_id_t<G>>.
 * 
 * @param g           The graph.
 * @param mis         The output iterator where selected vertex IDs will be written.
 * @param seed        The seed vertex ID to start from (default: 0).
 * 
 * @return The number of vertices in the maximal independent set.
 * 
 * **Mandates:**
 * - G must satisfy adjacency_list (index or mapped vertex containers)
 * - Iter must satisfy std::output_iterator<vertex_id_t<G>>
 * 
 * **Preconditions:**
 * - seed must be a valid vertex ID in the graph
 * 
 * **Effects:**
 * - Writes selected vertex IDs to mis output iterator
 * - Does not modify the graph g
 * 
 * **Postconditions:**
 * - The returned set is independent: no two vertices in output are adjacent
 * - The returned set is maximal: no additional vertex can be added
 * - For empty graphs, returns 0 with no output
 * 
 * **Returns:**
 * - Number of vertices in the maximal independent set (size_t)
 * - Attribute: [[nodiscard]]
 * 
 * **Throws:**
 * - std::bad_alloc if internal vector allocation fails
 * - Exception guarantee: Basic. Graph g remains unchanged; output may be partially written.
 * 
 * **Complexity:**
 * - Time: O(V + E) — visits each vertex once and examines all edges
 * - Space: O(V) for the uint8_t array tracking removed vertices
 * 
 * **Remarks:**
 * - Vertices with self-loops are automatically excluded from MIS
 * - Deterministic for a given seed but different seeds produce different results
 * - This finds maximal (cannot extend), not maximum (NP-complete) independent set
 * 
 * **Supported Graph Properties:**
 *
 * Directedness:
 * - ✅ Undirected graphs (recommended — each edge stored bidirectionally)
 * - ⚠️ Directed graphs (treats edges as directed; result may not be valid for underlying undirected graph)
 *
 * Edge Properties:
 * - ✅ Unweighted edges
 * - ✅ Weighted edges (weights ignored)
 * - ✅ Multi-edges (all edges considered when marking neighbors)
 * - ✅ Self-loops (vertices with self-loops excluded from MIS)
 *
 * Graph Structure:
 * - ✅ Connected graphs
 * - ✅ Disconnected graphs (processes all components)
 * - ✅ Empty graphs (returns 0)
 *
 * ## Example Usage
 *
 * ```cpp
 * #include <graph/graph.hpp>
 * #include <graph/algorithm/mis.hpp>
 * #include <vector>
 *
 * using namespace graph;
 *
 * // Path: 0-1-2-3-4 (bidirectional)
 * std::vector<vertex_id_t<Graph>> mis_result;
 * size_t mis_size = maximal_independent_set(g, std::back_inserter(mis_result));
 * // Possible output: mis_size=3, vertices: {0, 2, 4}
 * ```
 */

template <adjacency_list G, class Iter>
requires output_iterator<Iter, vertex_id_t<G>>
size_t maximal_independent_set(G&&                   g,       // graph
                               Iter                  mis,     // out: maximal independent set
                               const vertex_id_t<G>& seed = 0 // seed vtx
) {
  size_t N = num_vertices(g);
  if (N == 0) {
    return 0;
  }

  auto seed_vit = find_vertex(g, seed);
  assert(seed_vit != std::ranges::end(vertices(g)));

  size_t count            = 0;
  auto   removed_vertices = make_vertex_property_map<G, uint8_t>(g, uint8_t{0});

  // Mark seed vertex as removed
  removed_vertices[seed] = 1;

  // Check if seed vertex has a self-loop
  bool seed_has_self_loop = false;
  for (auto uv : edges(g, *seed_vit)) {
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
    for (auto uv : edges(g, *seed_vit)) {
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
