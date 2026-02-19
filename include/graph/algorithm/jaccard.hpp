/**
 * @file jaccard.hpp
 *
 * @brief Jaccard Coefficient algorithm for graphs.
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
#include "graph/views/incidence.hpp"

#ifndef GRAPH_JACCARD_HPP
#  define GRAPH_JACCARD_HPP

#  include <functional>
#  include <unordered_set>
#  include <vector>

namespace graph {

// Using declarations for new namespace structure
using adj_list::index_adjacency_list;
using adj_list::vertex_id_t;
using adj_list::edge_t;
using adj_list::vertices;
using adj_list::vertex_id;
using adj_list::num_vertices;

/**
 * @ingroup graph_algorithms
 * @brief Calculate the Jaccard coefficient for every edge in a graph.
 *
 * For each directed edge (u, v) in the graph, the Jaccard coefficient is:
 *
 *     J(u,v) = |N(u) ∩ N(v)| / |N(u) ∪ N(v)|
 *
 * where N(x) is the open neighborhood of vertex x (the set of all vertices
 * adjacent to x, excluding x itself). The coefficient lies in [0, 1] and
 * measures the similarity of two vertices based on their shared neighbors.
 *
 * The callback `out` is invoked once per directed edge with the two endpoint
 * IDs, a reference to the edge, and the computed coefficient. For an undirected
 * graph stored bidirectionally, `out` is called for both (u,v) and (v,u).
 *
 * ## Complexity Analysis
 *
 * **Time Complexity:** O(V + E × d_min) where d_min is the minimum degree of
 * the two endpoints per edge. Worst case O(|V|³) when the graph is dense.
 * The precomputation of neighbor sets costs O(V + E).
 *
 * **Space Complexity:** O(V + E) for the precomputed neighbor sets.
 *
 * ## Supported Graph Properties
 *
 * ### Directedness
 * - ✅ Directed graphs
 * - ✅ Undirected graphs (stored bidirectionally — callback fires for both directions)
 *
 * ### Edge Properties
 * - ✅ Unweighted edges
 * - ✅ Weighted edges (weights ignored)
 * - ✅ Multi-edges (deduplicated into neighbor sets; callers should prefer simple graphs)
 * - ❌ Self-loops (skipped — do not affect Jaccard computation)
 *
 * ### Graph Structure
 * - ✅ Connected graphs
 * - ✅ Disconnected graphs (processes all components; isolated vertices produce no callbacks)
 * - ✅ Empty graphs (returns immediately)
 *
 * ### Container Requirements
 * - Requires: `index_adjacency_list<G>` concept (contiguous vertex IDs)
 * - Works with: All `dynamic_graph` container combinations with contiguous IDs
 *
 * @tparam G      The graph type. Must satisfy index_adjacency_list concept.
 * @tparam OutOp  Callback invoked as `out(uid, vid, uv, val)` for each directed edge.
 * @tparam T      Floating-point type for the coefficient (default: double).
 *
 * @param g   The graph.
 * @param out Callback receiving (vertex_id_t<G> uid, vertex_id_t<G> vid,
 *            edge_t<G>& uv, T val) for every directed edge.
 *
 * @pre g must have contiguous vertex IDs [0, num_vertices(g)).
 * @pre For undirected semantics, each edge {u,v} must be stored as both (u,v) and (v,u).
 *
 * @post `out` is called exactly once per directed edge in the graph.
 * @post All reported coefficient values lie in [0.0, 1.0].
 * @post The graph g is not modified.
 *
 * **Exception Safety:** Basic exception safety. May throw std::bad_alloc if internal
 * container allocation fails. The graph g remains unchanged; `out` may have been
 * partially invoked.
 *
 * @note T = double is the recommended default. Using integral types will truncate
 *       results to 0 or 1.
 *
 * ## Example Usage
 *
 * ```cpp
 * #include <graph/graph.hpp>
 * #include <graph/algorithm/jaccard.hpp>
 * #include <iostream>
 *
 * using namespace graph;
 *
 * int main() {
 *     using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
 *                       container::vov_graph_traits<void, void, void, uint32_t, false>>;
 *
 *     // Triangle: 0-1-2 (bidirectional)
 *     Graph g({{0,1},{1,0},{1,2},{2,1},{0,2},{2,0}});
 *
 *     jaccard_coefficient(g, [](auto uid, auto vid, auto& uv, double val) {
 *         std::cout << uid << " - " << vid << " : " << val << "\n";
 *     });
 *     // Each edge prints J ≈ 0.333 (1 shared neighbor out of 3 total)
 * }
 * ```
 */
template <index_adjacency_list G, typename OutOp, typename T = double>
requires std::invocable<OutOp, vertex_id_t<G>, vertex_id_t<G>, edge_t<G>&, T>
void jaccard_coefficient(G&& g, OutOp out) {
  using vid_t = vertex_id_t<G>;

  const size_t N = num_vertices(g);
  if (N == 0) {
    return;
  }

  // ============================================================================
  // Phase 1: Build neighbor sets for every vertex (self-loops excluded)
  // ============================================================================
  std::vector<std::unordered_set<vid_t>> nbrs(N);

  for (auto&& u : vertices(g)) {
    vid_t uid = vertex_id(g, u);
    for (auto [tid] : views::basic_incidence(g, uid)) {
      if (tid != uid) { // skip self-loops
        nbrs[uid].insert(tid);
      }
    }
  }

  // ============================================================================
  // Phase 2: For every directed edge, compute and report the Jaccard coefficient
  // ============================================================================
  for (auto&& u : vertices(g)) {
    vid_t uid = vertex_id(g, u);
    for (auto&& [vid, uv] : views::incidence(g, uid)) {
      // Skip self-loops
      if (vid == uid) {
        continue;
      }

      // Compute |N(u) ∩ N(v)| by iterating the smaller set and probing the larger
      const auto& set_a = (nbrs[uid].size() <= nbrs[vid].size()) ? nbrs[uid] : nbrs[vid];
      const auto& set_b = (nbrs[uid].size() <= nbrs[vid].size()) ? nbrs[vid] : nbrs[uid];

      size_t intersect_size = 0;
      for (auto x : set_a) {
        if (set_b.count(x)) {
          ++intersect_size;
        }
      }

      // |N(u) ∪ N(v)| = |N(u)| + |N(v)| - |N(u) ∩ N(v)|
      size_t union_size = nbrs[uid].size() + nbrs[vid].size() - intersect_size;

      T val = (union_size == 0) ? T{0}
                                : static_cast<T>(intersect_size) / static_cast<T>(union_size);

      out(uid, vid, uv, val);
    }
  }
}

} // namespace graph

#endif // GRAPH_JACCARD_HPP
