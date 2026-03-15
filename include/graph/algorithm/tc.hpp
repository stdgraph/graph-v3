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
 * - **Required:** `adjacency_list` concept
 * - **Required:** `ordered_vertex_edges<G>` - adjacency lists must be sorted by target ID
 * - **Works with:** `vos`, `uos`, `dos`, `mos` graph types (vector/map + set edges)
 * - **Not compatible:** `vov`, `vous`, `mous` (unsorted edge containers)
 * 
 * @tparam G Graph type satisfying adjacency_list with ordered edges.
 * 
 * @param g The graph to analyze. Must be undirected with sorted adjacency lists.
 * 
 * @return Total number of triangles in the graph.
 * 
 * ## Mandates (Compile-Time Requirements)
 * 
 * ```cpp
 * requires adjacency_list<G>
 * requires ordered_vertex_edges<G>
 * ```
 * 
 * These constraints are enforced via C++20 concepts and will produce a compilation error
 * if not satisfied. The `adjacency_list` concept ensures forward_range vertex
 * iteration and edge access. The `ordered_vertex_edges` concept requires sorted adjacency
 * lists (semantic requirement verified by graph type's container choice).
 * 
 * ## Preconditions (Runtime Requirements)
 * 
 * 1. Graph must store undirected edges bidirectionally (both (u,v) and (v,u))
 * 2. Adjacency lists must be sorted by target_id in ascending order
 * 3. Vertex IDs must be valid vertex IDs in the graph
 * 
 * ## Postconditions
 * 
 * 1. Return value is non-negative
 * 2. For empty graphs or graphs with < 3 vertices, returns 0
 * 3. Graph `g` remains unmodified
 * 
 * ## Exception Safety
 * 
 * **Guarantee:** Strong exception safety (no-throw guarantee)
 * 
 * **Throws:** Never throws. Uses only non-throwing operations (arithmetic, iteration).
 * 
 * **State after exception:** N/A - function is `noexcept`
 * 
 * ## Implementation Notes
 * 
 * ### Algorithm Overview
 * 
 * 1. For each vertex `u`, iterate through its adjacency list
 * 2. For each edge `(u,v)` where `u < v`:
 *    - Perform merge-based intersection of adjacency lists for `u` and `v`
 *    - Count common neighbors `w` where `w > v`
 * 3. Each common neighbor represents exactly one triangle `{u, v, w}`
 * 
 * ### Data Structures
 * 
 * - **No auxiliary data structures** - operates directly on graph's adjacency lists
 * - **Merge operation:** Linear scan through two sorted ranges (like std::set_intersection)
 * 
 * ### Design Decisions
 * 
 * 1. **Why require ordered_vertex_edges?**
 *    - Enables O(d) intersection instead of O(d²) nested loops
 *    - Critical for performance on high-degree vertices
 *    - Natural for graphs using std::set, std::map edge containers
 * 
 * 2. **Why impose ordering constraints (u < v, w > v)?**
 *    - Ensures each triangle counted exactly once
 *    - Alternative would require duplicate detection (higher overhead)
 * 
 * 3. **Why forward iterators only?**
 *    - Sufficient for merge operation
 *    - Compatible with wider range of containers
 * 
 * ### Optimization Opportunities
 * 
 * - For very dense graphs: Matrix multiplication approach O(V^(ω)) where ω ≈ 2.373
 * - For graphs with high clustering: Vertex ordering heuristics can reduce work
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
 * ## References
 * 
 * ### Academic Papers
 * - Schank, T., & Wagner, D. (2005). "Finding, Counting and Listing all Triangles 
 *   in Large Graphs, an Experimental Study". *International Workshop on Experimental
 *   and Efficient Algorithms* (WEA 2005), LNCS 3503, pp. 606-609.
 * - Latapy, M. (2008). "Main-memory triangle computations for very large (sparse 
 *   (power-law)) graphs". *Theoretical Computer Science*, 407(1-3), 458-473.
 * 
 * ### Textbooks
 * - Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2009). 
 *   *Introduction to Algorithms* (3rd ed.). MIT Press. Chapter 22 (Graph Algorithms).
 * 
 * ### Online Resources
 * - [Wikipedia: Clique (Graph Theory)](https://en.wikipedia.org/wiki/Clique_(graph_theory))
 * - [Graph Triangle Counting Overview](https://en.wikipedia.org/wiki/Triangle_counting)
 * 
 * ### Related Algorithms
 * - **k-Clique Counting:** Generalization to count k-cliques (k > 3)
 * - **Clustering Coefficient:** Uses triangle count to measure graph clustering
 * - **Transitivity Ratio:** Ratio of triangles to connected triples
 * 
 * ## Testing
 * 
 * ### Test Coverage
 * - Correctness: Empty graphs, single triangles, multiple triangles, K4 complete graph
 * - Edge cases: No triangles (trees, bipartite), isolated vertices, disconnected components
 * - Container types: vos (vector+set), uos (unordered_map+set), undirected_adjacency_list
 * - Complex structures: Diamond graphs, wheel graphs, chordal cycles
 * 
 * ### Test File Location
 * - `tests/algorithms/test_triangle_count.cpp` (30 test cases)
 * 
 * @see https://en.wikipedia.org/wiki/Clique_(graph_theory)
 * @see "Finding, Counting and Listing all Triangles in Large Graphs" by Schank & Wagner (2005)
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
 * u->v, v->w, and u->w all exist. Unlike `triangle_count`, this function does not
 * impose ordering constraints on vertex IDs, so it counts every directed 3-cycle
 * exactly once by enumerating over all edges (u,v) and finding common out-neighbors w
 * of both u and v (where w != u and w != v).
 * 
 * ## Algorithm Overview
 * 
 * For each vertex u, for each out-neighbor v (v != u):
 * 1. Get sorted out-neighbor lists for u and v
 * 2. Use merge-based intersection to find common out-neighbors w
 * 3. Skip self-loops (w == u or w == v)
 * 4. Each common out-neighbor forms one directed 3-cycle (u -> v, v -> w, u -> w)
 * 
 * Because every ordered triple (u, v, w) is visited exactly once, each 3-cycle is
 * counted exactly once.
 * 
 * ## Complexity Analysis
 * 
 * Same asymptotic complexity as `triangle_count`:
 * | Case | Complexity |
 * |------|------------|
 * | Average | O(m^(3/2)) for sparse graphs (m = E) |
 * | Worst | O(V * d_max^2) where d_max = max out-degree |
 * 
 * **Space Complexity:** O(1) auxiliary space.
 * 
 * @tparam G Graph type satisfying adjacency_list with ordered edges.
 * @param g The directed graph to analyze. Must have sorted adjacency lists.
 * @return Total number of directed 3-cycles in the graph.
 * 
 * @note For an undirected graph stored with bidirectional edges, this will count
 *       each undirected triangle 6 times (once per permutation of the 3 vertices).
 *       Use `triangle_count` instead for undirected graphs.
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
