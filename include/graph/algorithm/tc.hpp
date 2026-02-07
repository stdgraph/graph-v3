/**
 * @file tc.hpp
 * 
 * @brief Triangle counting for undirected graphs.
 * 
 * This file provides an efficient algorithm for counting triangles (3-cliques) in undirected
 * graphs. A triangle consists of three vertices where each pair is connected by an edge.
 * 
 * The algorithm requires sorted adjacency lists for correctness and optimal performance.
 * It uses a merge-based set intersection approach that is more efficient than nested loops
 * or hash-based methods for sparse graphs.
 * 
 * @copyright Copyright (c) 2022
 * 
 * SPDX-License-Identifier: BSL-1.0
 *
 * @authors
 *   Andrew Lumsdaine
 *   Phil Ratzloff
 *   Kevin Deweese
 */

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include <ranges>

#ifndef GRAPH_TC_HPP
#  define GRAPH_TC_HPP

namespace graph {

// Using declarations for new namespace structure
using adj_list::index_adjacency_list;
using adj_list::vertex_id_t;

/**
 * @brief Concept for graphs with sorted adjacency lists.
 * 
 * A graph satisfies ordered_edges if the adjacency list for each vertex is sorted by
 * target vertex ID in ascending order. This property enables efficient set intersection
 * algorithms using linear merge operations.
 * 
 * @tparam G Graph type
 * 
 * @note Required for triangle_count algorithm correctness. Graphs with unsorted adjacency
 *       lists will produce incorrect results.
 */
template <class G>
concept ordered_edges = adjacency_list<G> && requires(G& g, vertex_id_t<G> u) {
  requires std::forward_iterator<decltype(std::ranges::begin(edges(g, u)))>;
  // Note: This is a semantic requirement that cannot be fully checked at compile time.
  // The algorithm assumes adjacency lists are sorted by target_id in ascending order.
  // Graph types using std::set, std::map, or similar ordered containers satisfy this.
};

/**
 * @ingroup graph_algorithms
 * @brief Count triangles in an undirected graph with sorted adjacency lists.
 * 
 * A triangle is a set of three vertices {u, v, w} where edges (u,v), (v,w), and (u,w) all exist.
 * This algorithm efficiently counts triangles by iterating over edges and finding common neighbors
 * using a merge-based set intersection approach.
 * 
 * ## Algorithm Overview
 * 
 * For each edge (u,v) where u < v:
 * 1. Get sorted adjacency lists for u and v
 * 2. Use merge to find common neighbors w where w > v
 * 3. Each common neighbor forms exactly one triangle {u, v, w}
 * 
 * The ordering constraints (u < v and w > v) ensure each triangle is counted exactly once.
 * 
 * ## Complexity Analysis
 * 
 * | Case | Complexity | Notes |
 * |------|-----------|-------|
 * | Best case | O(V + E) | Graphs with no triangles (e.g., trees, bipartite) |
 * | Average case | O(m^(3/2)) | Sparse graphs where m = E |
 * | Worst case | O(V * d_max²) | d_max = maximum vertex degree |
 * 
 * For dense graphs (E ≈ V²), worst case approaches O(V³).
 * 
 * **Space Complexity:** O(1) auxiliary space (excluding graph storage and output).
 * 
 * ## Supported Graph Properties
 * 
 * ### Directedness
 * - ✅ Undirected graphs (each edge stored bidirectionally)
 * - ⚠️ Directed graphs: Results may not be meaningful; counts directed 3-cycles
 * 
 * ### Edge Properties
 * - ✅ Unweighted edges
 * - ✅ Weighted edges (weights ignored)
 * - ⚠️ Multi-edges: Each parallel edge contributes to triangle count
 * - ✅ Self-loops: Ignored (cannot form triangles)
 * 
 * ### Graph Structure
 * - ✅ Connected graphs
 * - ✅ Disconnected graphs
 * - ✅ May contain cycles (triangles are 3-cycles)
 * 
 * ### Container Requirements
 * - **Required:** `index_adjacency_list` concept
 * - **Required:** `forward_range<vertex_range_t<G>>`
 * - **Required:** `integral<vertex_id_t<G>>`
 * - **Required:** `ordered_edges<G>` - adjacency lists must be sorted by target ID
 * - **Works with:** `vos`, `uos`, `dos` graph types (vector/map + set edges)
 * - **Not compatible:** `vov`, `vous`, `mous` (unsorted edge containers)
 * 
 * @tparam G Graph type satisfying index_adjacency_list with ordered edges.
 * 
 * @param g The graph to analyze. Must be undirected with sorted adjacency lists.
 * 
 * @return Total number of triangles in the graph.
 * 
 * @pre Graph must store undirected edges bidirectionally (both (u,v) and (v,u))
 * @pre Adjacency lists must be sorted by target_id in ascending order
 * @pre Vertex IDs must be in range [0, num_vertices(g))
 * 
 * @post Return value is non-negative
 * @post For empty graphs or graphs with < 3 vertices, returns 0
 * 
 * @par Example:
 * @code
 * #include <graph/algorithm/tc.hpp>
 * #include <graph/container/dynamic_graph.hpp>
 * #include <graph/container/traits/vos_graph_traits.hpp>
 * 
 * using Graph = graph::container::dynamic_graph<void, void, void, uint32_t, false,
 *                                                 graph::container::vos_graph_traits<...>>;
 * 
 * // Create triangle: vertices {0, 1, 2} with edges (0,1), (1,2), (0,2)
 * Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0}});
 * 
 * size_t count = triangle_count(g);
 * // count == 1 (one triangle: {0, 1, 2})
 * 
 * // Add fourth vertex forming complete graph K4 (4 triangles total)
 * g.create_edge(0, 3);
 * g.create_edge(3, 0);
 * g.create_edge(1, 3);
 * g.create_edge(3, 1);
 * g.create_edge(2, 3);
 * g.create_edge(3, 2);
 * 
 * count = triangle_count(g);
 * // count == 4 (triangles: {0,1,2}, {0,1,3}, {0,2,3}, {1,2,3})
 * @endcode
 * 
 * @note This algorithm is optimized for sparse graphs. For very dense graphs, consider
 *       matrix multiplication approaches.
 * 
 * @see https://en.wikipedia.org/wiki/Clique_(graph_theory)
 * @see "Finding, Counting and Listing all Triangles in Large Graphs" by Schank & Wagner (2005)
 */
template <index_adjacency_list G>
requires forward_range<vertex_range_t<G>> && 
         integral<vertex_id_t<G>> &&
         ordered_edges<G>
size_t triangle_count(G&& g) {
  const size_t vertex_count = size(vertices(g));
  size_t triangles = 0;

  // Iterate over all vertices as potential triangle "roots"
  for (vertex_id_t<G> uid = 0; uid < vertex_count; ++uid) {
    auto u_edges = edges(g, uid);
    auto u_it = std::ranges::begin(u_edges);
    auto u_end = std::ranges::end(u_edges);
    
    // Iterate over neighbors of u
    while (u_it != u_end) {
      auto vid = target_id(g, *u_it);
      
      // Only process edge (u,v) where u < v to avoid double-counting
      if (uid < vid) {
        auto v_edges = edges(g, vid);
        auto v_it = std::ranges::begin(v_edges);
        auto v_end = std::ranges::end(v_edges);
        
        // Start from next neighbor of u (after v) to find common neighbors
        auto u_remaining = std::next(u_it);
        
        // Merge-based intersection: find common neighbors w of both u and v
        // where w > v (ensures each triangle counted exactly once)
        while (u_remaining != u_end && v_it != v_end) {
          auto wid_from_u = target_id(g, *u_remaining);
          auto wid_from_v = target_id(g, *v_it);
          
          if (wid_from_u < wid_from_v) {
            ++u_remaining;
          } else if (wid_from_v < wid_from_u) {
            ++v_it;
          } else {
            // Common neighbor found: triangle {uid, vid, wid_from_u}
            // Only count if wid > vid (triangle ordering: uid < vid < wid)
            if (wid_from_u > vid) {
              ++triangles;
            }
            ++u_remaining;
            ++v_it;
          }
        }
      }
      ++u_it;
    }
  }
  
  return triangles;
}
} // namespace graph

#endif //GRAPH_TC_HPP
