/**
 * @file articulation_points.hpp
 *
 * @brief Articulation Points (cut vertices) algorithm for graphs.
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

#ifndef GRAPH_ARTICULATION_POINTS_HPP
#  define GRAPH_ARTICULATION_POINTS_HPP

#  include <limits>
#  include <stack>
#  include <vector>

namespace graph {

// Using declarations for new namespace structure
using adj_list::index_adjacency_list;
using adj_list::vertex_id_t;
using adj_list::vertices;
using adj_list::edges;
using adj_list::target_id;
using adj_list::vertex_id;
using adj_list::num_vertices;
using adj_list::find_vertex;

/**
 * @ingroup graph_algorithms
 * @brief Find articulation points (cut vertices) of a graph.
 *
 * An articulation point is a vertex whose removal (along with its incident edges)
 * disconnects the graph into two or more connected components. This implementation
 * uses the iterative Hopcroft-Tarjan algorithm based on DFS discovery times and
 * low-link values.
 *
 * The algorithm maintains two arrays:
 * - `disc[v]`: DFS discovery time of vertex `v`.
 * - `low[v]`: minimum discovery time reachable from the subtree rooted at `v`
 *   via back-edges.
 *
 * A vertex `u` is an articulation point if:
 * - **Root rule:** `u` is the root of a DFS tree and has two or more DFS children.
 * - **Non-root rule:** `u` is not a root and has a child `v` with `low[v] >= disc[u]`.
 *
 * ## Complexity Analysis
 *
 * **Time Complexity:** O(|V| + |E|) where V is the number of vertices and E is
 * the number of edges. Each vertex and edge is visited exactly once during the DFS.
 *
 * **Space Complexity:** O(V) for the discovery time, low-link, parent, child count,
 * and emitted arrays, plus O(V) for the DFS stack.
 *
 * ## Supported Graph Properties
 *
 * ### Directedness
 * - ✅ Directed graphs (caller must store both {u,v} and {v,u} for undirected semantics)
 *
 * ### Edge Properties
 * - ✅ Unweighted edges
 * - ✅ Weighted edges (weights ignored)
 * - ✅ Multi-edges (only the first reverse edge to the DFS parent is skipped as the tree edge;
 *      additional parallel edges are treated as back-edges that update low-link values)
 * - ✅ Self-loops (ignored — do not affect articulation point detection)
 * - ✅ Cycles
 *
 * ### Graph Structure
 * - ✅ Connected graphs
 * - ✅ Disconnected graphs (processes all components via outer loop)
 * - ✅ Empty graphs (returns immediately)
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
 * @param g           The graph. Callers must supply both directions of each undirected edge.
 * @param cut_vertices The output iterator where articulation point vertex IDs will be written.
 *                    No ordering guarantee on the emitted vertices.
 *
 * @pre g must have contiguous vertex IDs [0, num_vertices(g))
 * @pre For undirected semantics, each edge {u,v} must be stored as both (u,v) and (v,u).
 *
 * @post Output contains all articulation points, each emitted exactly once.
 * @post The graph g is not modified.
 *
 * **Exception Safety:** Basic exception safety. May throw std::bad_alloc if internal
 * vector allocation fails. The graph g remains unchanged; output iterator may be
 * partially written.
 *
 * ## Example Usage
 *
 * ```cpp
 * #include <graph/graph.hpp>
 * #include <graph/algorithm/articulation_points.hpp>
 * #include <vector>
 * #include <iostream>
 *
 * using namespace graph;
 *
 * int main() {
 *     using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
 *                       container::vov_graph_traits<void, void, void, uint32_t, false>>;
 *
 *     // Path graph: 0 - 1 - 2 - 3 (bidirectional)
 *     Graph g({{0,1},{1,0},{1,2},{2,1},{2,3},{3,2}});
 *
 *     std::vector<vertex_id_t<Graph>> result;
 *     articulation_points(g, std::back_inserter(result));
 *     // result contains {1, 2} (in some order)
 * }
 * ```
 */
template <index_adjacency_list G, class Iter>
requires std::output_iterator<Iter, vertex_id_t<G>>
void articulation_points(G&& g, Iter cut_vertices) {
  using vid_t = vertex_id_t<G>;

  const size_t N = num_vertices(g);
  if (N == 0) {
    return;
  }

  constexpr size_t UNVISITED = std::numeric_limits<size_t>::max();
  const vid_t      NO_PARENT = static_cast<vid_t>(N); // sentinel for "no parent"

  std::vector<size_t> disc(N, UNVISITED);
  std::vector<size_t> low(N, UNVISITED);
  std::vector<vid_t>  parent(N, NO_PARENT);
  std::vector<size_t> child_count(N, 0); // DFS tree children count (for root rule)
  std::vector<bool>   emitted(N, false); // deduplication guard

  size_t timer = 0;

  // Frame for iterative DFS: (vertex_id, edge_index, parent_edge_skipped)
  // edge_index tracks how far we've iterated through edges(g, uid)
  // parent_edge_skipped ensures only the first reverse edge to the DFS parent
  // is treated as the tree edge; subsequent parallel edges update low-link.
  struct dfs_frame {
    vid_t  uid;
    size_t edge_idx;
    bool   parent_edge_skipped;
  };

  std::stack<dfs_frame> stk;

  // Outer loop: handle disconnected graphs
  for (auto sv : vertices(g)) {
    vid_t start = vertex_id(g, sv);
    if (disc[start] != UNVISITED) {
      continue;
    }

    disc[start] = low[start] = timer++;
    stk.push({start, 0, false});

    while (!stk.empty()) {
      auto& [uid, edge_idx, parent_skipped] = stk.top();

      // Collect edges into a temporary to allow indexed access
      // We advance through edges one at a time using edge_idx
      auto edge_range = edges(g, uid);
      auto it         = std::ranges::begin(edge_range);
      auto it_end     = std::ranges::end(edge_range);

      // Advance iterator to edge_idx position
      for (size_t i = 0; i < edge_idx && it != it_end; ++i, ++it) {
      }

      if (it == it_end) {
        // All edges processed — backtrack
        stk.pop();
        if (!stk.empty()) {
          auto& [par_uid, par_edge_idx, par_skipped] = stk.top();
          // Update low-link of parent
          if (low[uid] < low[par_uid]) {
            low[par_uid] = low[uid];
          }

          // Check articulation point condition for non-root
          if (parent[par_uid] != NO_PARENT) {
            // Non-root rule: child v has low[v] >= disc[u]
            if (low[uid] >= disc[par_uid] && !emitted[par_uid]) {
              *cut_vertices++ = par_uid;
              emitted[par_uid] = true;
            }
          }
        }
        continue;
      }

      vid_t vid = target_id(g, *it);
      ++edge_idx; // advance for next iteration

      // Skip self-loops
      if (vid == uid) {
        continue;
      }

      if (disc[vid] == UNVISITED) {
        // Tree edge: vid is a new DFS child of uid
        parent[vid] = uid;
        child_count[uid]++;
        disc[vid] = low[vid] = timer++;
        stk.push({vid, 0, false});
      } else if (vid == parent[uid] && !parent_skipped) {
        // First reverse edge to DFS parent — this is the tree edge; skip it
        parent_skipped = true;
      } else {
        // Back edge (or additional parallel edge to parent): update low-link
        if (disc[vid] < low[uid]) {
          low[uid] = disc[vid];
        }
      }
    }

    // Root rule: root is an articulation point iff it has >= 2 DFS children
    if (child_count[start] >= 2 && !emitted[start]) {
      *cut_vertices++ = start;
      emitted[start]  = true;
    }
  }
}

} // namespace graph

#endif // GRAPH_ARTICULATION_POINTS_HPP
