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
#include "graph/views/vertexlist.hpp"
#include "graph/adj_list/vertex_property_map.hpp"

#ifndef GRAPH_JACCARD_HPP
#  define GRAPH_JACCARD_HPP

#  include <functional>
#  include <unordered_set>
#  include <vector>

namespace graph {

// Using declarations for new namespace structure
using adj_list::adjacency_list;
using adj_list::vertex_id_t;
using adj_list::edge_t;
using adj_list::num_vertices;

/**
 * @ingroup graph_algorithms
 * @brief Calculate the Jaccard coefficient for every edge in a graph.
 *
 * For each directed edge (u, v), the Jaccard coefficient is:
 *     J(u,v) = |N(u) ∩ N(v)| / |N(u) ∪ N(v)|
 * where N(x) is the open neighborhood of vertex x. The coefficient lies in [0, 1].
 *
 * @tparam G      The graph type. Must satisfy adjacency_list concept.
 * @tparam OutOp  Callback invoked as out(uid, vid, uv, val) for each directed edge.
 * @tparam T      Floating-point type for the coefficient (default: double).
 *
 * @param g   The graph.
 * @param out Callback receiving (vertex_id_t<G> uid, vertex_id_t<G> vid,
 *            edge_t<G>& uv, T val) for every directed edge.
 *
 * @return void. Results are delivered via the callback.
 *
 * **Mandates:**
 * - G must satisfy adjacency_list (index or mapped vertex containers)
 * - OutOp must be invocable with (vertex_id_t<G>, vertex_id_t<G>, edge_t<G>&, T)
 *
 * **Preconditions:**
 * - For undirected semantics, each edge {u,v} must be stored as both (u,v) and (v,u)
 *
 * **Effects:**
 * - Invokes out(uid, vid, uv, val) once per directed edge
 * - Does not modify the graph g
 *
 * **Postconditions:**
 * - out is called exactly once per directed edge in the graph
 * - All reported coefficient values lie in [0.0, 1.0]
 *
 * **Throws:**
 * - std::bad_alloc if internal container allocation fails
 * - May propagate exceptions from the user-provided callback out
 * - Exception guarantee: Basic. Graph g remains unchanged; out may have been partially invoked.
 *
 * **Complexity:**
 * - Time: O(V + E × d_min) where d_min is minimum degree per edge; worst case O(V³)
 * - Space: O(V + E) for precomputed neighbor sets
 *
 * **Remarks:**
 * - T = double is recommended. Integral types truncate results to 0 or 1.
 * - Self-loops are skipped and do not affect Jaccard computation
 *
 * **Supported Graph Properties:**
 *
 * Directedness:
 * - ✅ Directed graphs
 * - ✅ Undirected graphs (stored bidirectionally — callback fires for both directions)
 *
 * Edge Properties:
 * - ✅ Unweighted edges
 * - ✅ Weighted edges (weights ignored)
 * - ✅ Multi-edges (deduplicated into neighbor sets)
 * - ❌ Self-loops (skipped)
 *
 * Graph Structure:
 * - ✅ Connected graphs
 * - ✅ Disconnected graphs (processes all components; isolated vertices produce no callbacks)
 * - ✅ Empty graphs (returns immediately)
 *
 * ## Example Usage
 *
 * ```cpp
 * #include <graph/graph.hpp>
 * #include <graph/algorithm/jaccard.hpp>
 *
 * using namespace graph;
 *
 * // Triangle: 0-1-2 (bidirectional)
 * Graph g({{0,1},{1,0},{1,2},{2,1},{0,2},{2,0}});
 *
 * jaccard_coefficient(g, [](auto uid, auto vid, auto& uv, double val) {
 *     std::cout << uid << " - " << vid << " : " << val << "\n";
 * });
 * // Each edge prints J ≈ 0.333
 * ```
 */
template <adjacency_list G, typename OutOp, typename T = double>
requires std::invocable<OutOp, vertex_id_t<G>, vertex_id_t<G>, edge_t<G>&, T>
void jaccard_coefficient(G&& g, OutOp out) {
  using vid_t = vertex_id_t<G>;

  if (num_vertices(g) == 0) {
    return;
  }

  // ============================================================================
  // Phase 1: Build neighbor sets for every vertex (self-loops excluded)
  // ============================================================================
  // vertex_property_map: vector<set> for index graphs, unordered_map<VId, set> for mapped.
  using nbr_set = std::unordered_set<vid_t>;
  auto nbrs     = make_vertex_property_map<std::remove_reference_t<G>, nbr_set>(g, nbr_set{});

  for (auto [uid] : views::basic_vertexlist(g)) {
    for (auto [tid] : views::basic_incidence(g, uid)) {
      if (tid != uid) { // skip self-loops
        nbrs[uid].insert(tid);
      }
    }
  }

  // ============================================================================
  // Phase 2: For every directed edge, compute and report the Jaccard coefficient
  // ============================================================================
  for (auto&& [uid, u] : views::vertexlist(g)) {
    for (auto&& [vid, uv] : views::incidence(g, u)) {
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

      T val = (union_size == 0) ? T{0} : static_cast<T>(intersect_size) / static_cast<T>(union_size);

      out(uid, vid, uv, val);
    }
  }
}

} // namespace graph

#endif // GRAPH_JACCARD_HPP
