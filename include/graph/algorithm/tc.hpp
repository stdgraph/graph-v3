/**
 * @file tc.hpp
 * 
 * @brief Triangle counting for undirected and directed graphs.
 * 
 * This file provides efficient algorithms for counting triangles in graphs:
 * - `triangle_count`: Counts triangles (3-cliques) in undirected graphs.
 * - `directed_triangle_count`: Counts directed 3-cycles in directed graphs.
 * 
 * A triangle consists of three vertices where each pair is connected by an edge.
 * 
 * The algorithms require sorted adjacency lists for correctness and optimal performance.
 * They use a merge-based set intersection approach that is more efficient than nested loops
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
using adj_list::adjacency_list;
using adj_list::ordered_vertex_edges;
using adj_list::vertex_id_t;
using adj_list::vertices;
using adj_list::edges;
using adj_list::target_id;
using adj_list::vertex_id;
using adj_list::num_vertices;

/**
 * @ingroup graph_algorithms
 * @brief Count triangles in an undirected graph with sorted adjacency lists.
 * 
 * A triangle is a set of three vertices {u, v, w} where edges (u,v), (v,w), and (u,w) all exist.
 * For each edge (u,v) where u < v, the algorithm uses merge-based set intersection of sorted
 * adjacency lists to find common neighbors w > v, ensuring each triangle is counted exactly once.
 * 
 * @tparam G Graph type satisfying adjacency_list with ordered edges.
 * 
 * @param g The graph to analyze. Must be undirected with sorted adjacency lists.
 * 
 * @return Total number of triangles in the graph.
 * 
 * **Mandates:**
 * - G must satisfy adjacency_list
 * - G must satisfy ordered_vertex_edges (adjacency lists sorted by target ID)
 * 
 * **Preconditions:**
 * - Graph must store undirected edges bidirectionally (both (u,v) and (v,u))
 * - Adjacency lists must be sorted by target_id in ascending order
 * 
 * **Effects:**
 * - Iterates over all edges and computes triangle count
 * - Does not modify the graph g
 * 
 * **Postconditions:**
 * - Return value is non-negative
 * - For empty graphs or graphs with < 3 vertices, returns 0
 * - Graph g remains unmodified
 * 
 * **Returns:**
 * - Total number of triangles (size_t)
 * - Attribute: [[nodiscard]]
 * 
 * **Throws:**
 * - Never throws (noexcept). Uses only non-throwing operations (arithmetic, iteration).
 * - Exception guarantee: Strong (no-throw guarantee).
 * 
 * **Complexity:**
 * - Time: O(V + E) best case (no triangles); O(m^(3/2)) average (sparse, m = E);
 *   O(V * d_max^2) worst case (d_max = max degree). For dense graphs (E ~ V^2), approaches O(V^3).
 * - Space: O(1) auxiliary (excluding graph storage)
 * 
 * **Remarks:**
 * - Optimized for sparse graphs; for very dense graphs consider matrix multiplication approaches
 * - The ordering constraints (u < v, w > v) ensure each triangle is counted exactly once
 * - Uses merge-based intersection of sorted ranges, similar to std::set_intersection
 * 
 * **Supported Graph Properties:**
 *
 * Directedness:
 * - ✅ Undirected graphs (each edge stored bidirectionally)
 * - ⚠️ Directed graphs: Results may not be meaningful; counts directed 3-cycles
 *
 * Edge Properties:
 * - ✅ Unweighted edges
 * - ✅ Weighted edges (weights ignored)
 * - ⚠️ Multi-edges: Each parallel edge contributes to triangle count
 * - ✅ Self-loops: Ignored (cannot form triangles)
 *
 * Graph Structure:
 * - ✅ Connected graphs
 * - ✅ Disconnected graphs
 * - ✅ May contain cycles (triangles are 3-cycles)
 *
 * ## Example Usage
 *
 * ```cpp
 * #include <graph/algorithm/tc.hpp>
 *
 * // Create triangle: vertices {0, 1, 2} with edges (0,1), (1,2), (0,2) bidirectional
 * Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0}});
 *
 * size_t count = triangle_count(g);
 * // count == 1 (one triangle: {0, 1, 2})
 * ```
 *
 * @see directed_triangle_count
 */
template <adjacency_list G>
requires ordered_vertex_edges<G>
[[nodiscard]] size_t triangle_count(G&& g) noexcept {
  size_t triangles = 0;

  // ============================================================================
  // Main loop: Process each vertex as the "first" vertex in potential triangles
  // ============================================================================
  for (auto u : vertices(g)) {
    auto uid     = vertex_id(g, u);
    auto u_edges = edges(g, u);
    auto u_it    = std::ranges::begin(u_edges);
    auto u_end   = std::ranges::end(u_edges);

    // ==========================================================================
    // For each neighbor v of u, find triangles containing edge (u,v)
    // ==========================================================================
    while (u_it != u_end) {
      auto vid = target_id(g, *u_it);

      // Only process edges where uid < vid to avoid counting the same edge twice
      // (since undirected graphs store both (u,v) and (v,u))
      if (uid < vid) {
        // Get adjacency list for vertex v
        auto v_edges = edges(g, vid);
        auto v_it    = std::ranges::begin(v_edges);
        auto v_end   = std::ranges::end(v_edges);

        // Skip past neighbors we've already processed (uid through vid)
        // Start checking for common neighbors after vid
        auto u_remaining = std::next(u_it);

        // ======================================================================
        // Merge-based intersection: Find vertices adjacent to BOTH u and v
        // This forms triangles {u, v, w} where w is a common neighbor
        // ======================================================================
        while (u_remaining != u_end && v_it != v_end) {
          auto wid_from_u = target_id(g, *u_remaining); // Candidate from u's adjacency list
          auto wid_from_v = target_id(g, *v_it);        // Candidate from v's adjacency list

          if (wid_from_u < wid_from_v) {
            // u's neighbor is smaller - advance u's iterator
            ++u_remaining;
          } else if (wid_from_v < wid_from_u) {
            // v's neighbor is smaller - advance v's iterator
            ++v_it;
          } else {
            // Found common neighbor w: both u and v are adjacent to w
            // This forms a triangle {uid, vid, wid}

            // Only count if wid > vid (ensures ordering: uid < vid < wid)
            // This guarantees each triangle is counted exactly once
            if (wid_from_u > vid) {
              ++triangles;
            }

            // Advance both iterators past this common neighbor
            ++u_remaining;
            ++v_it;
          }
        }
      }
      ++u_it; // Move to next neighbor of u
    }
  }

  return triangles;
}

/**
 * @ingroup graph_algorithms
 * @brief Count directed 3-cycles in a directed graph with sorted adjacency lists.
 * 
 * A directed 3-cycle is a set of three vertices {u, v, w} where directed edges
 * u->v, v->w, and u->w all exist. For each edge (u,v) where v != u, the algorithm
 * finds common out-neighbors w of both u and v (w != u, w != v) using merge-based
 * set intersection. Each ordered triple (u, v, w) is visited exactly once.
 * 
 * @tparam G Graph type satisfying adjacency_list with ordered edges.
 * 
 * @param g The directed graph to analyze. Must have sorted adjacency lists.
 * 
 * @return Total number of directed 3-cycles in the graph.
 * 
 * **Mandates:**
 * - G must satisfy adjacency_list
 * - G must satisfy ordered_vertex_edges (adjacency lists sorted by target ID)
 * 
 * **Preconditions:**
 * - Adjacency lists must be sorted by target_id in ascending order
 * 
 * **Effects:**
 * - Iterates over all edges and computes directed 3-cycle count
 * - Does not modify the graph g
 * 
 * **Postconditions:**
 * - Return value is non-negative
 * - For empty graphs or graphs with < 3 vertices, returns 0
 * - Graph g remains unmodified
 * 
 * **Returns:**
 * - Total number of directed 3-cycles (size_t)
 * - Attribute: [[nodiscard]]
 * 
 * **Throws:**
 * - Never throws (noexcept). Uses only non-throwing operations (arithmetic, iteration).
 * - Exception guarantee: Strong (no-throw guarantee).
 * 
 * **Complexity:**
 * - Time: O(m^(3/2)) average (sparse, m = E); O(V * d_max^2) worst case (d_max = max out-degree)
 * - Space: O(1) auxiliary
 * 
 * **Remarks:**
 * - For undirected graphs stored with bidirectional edges, this counts each undirected
 *   triangle 6 times (once per permutation). Use triangle_count instead.
 * - Self-loops are skipped during enumeration
 * 
 * **Supported Graph Properties:**
 *
 * Directedness:
 * - ✅ Directed graphs (recommended)
 * - ⚠️ Undirected graphs: Each triangle counted 6 times; use triangle_count instead
 *
 * Edge Properties:
 * - ✅ Unweighted edges
 * - ✅ Weighted edges (weights ignored)
 * - ✅ Self-loops: Skipped
 *
 * Graph Structure:
 * - ✅ Connected graphs
 * - ✅ Disconnected graphs
 *
 * ## Example Usage
 *
 * ```cpp
 * #include <graph/algorithm/tc.hpp>
 *
 * // Directed 3-cycle: 0->1, 1->2, 0->2
 * size_t count = directed_triangle_count(g);
 * // count == 1
 * ```
 *
 * @see triangle_count
 */
template <adjacency_list G>
requires ordered_vertex_edges<G>
[[nodiscard]] size_t directed_triangle_count(G&& g) noexcept {
  size_t triangles = 0;

  for (auto u : vertices(g)) {
    auto uid     = vertex_id(g, u);
    auto u_edges = edges(g, u);
    auto u_it    = std::ranges::begin(u_edges);
    auto u_end   = std::ranges::end(u_edges);

    while (u_it != u_end) {
      auto vid = target_id(g, *u_it);

      // Skip self-loops
      if (vid != uid) {
        // Get adjacency list for vertex v
        auto v_edges = edges(g, vid);
        auto v_it    = std::ranges::begin(v_edges);
        auto v_end   = std::ranges::end(v_edges);

        // Scan all of u's out-neighbors (not just those after v)
        auto u_remaining = std::ranges::begin(u_edges);

        // Merge-based intersection of u's and v's out-neighbor lists
        while (u_remaining != u_end && v_it != v_end) {
          auto wid_from_u = target_id(g, *u_remaining);
          auto wid_from_v = target_id(g, *v_it);

          if (wid_from_u < wid_from_v) {
            ++u_remaining;
          } else if (wid_from_v < wid_from_u) {
            ++v_it;
          } else {
            // Common out-neighbor w found; skip if w is u or v (self-loop)
            if (wid_from_u != uid && wid_from_u != vid) {
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
