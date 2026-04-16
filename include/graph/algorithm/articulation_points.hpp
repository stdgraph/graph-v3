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
#include "graph/adj_list/vertex_property_map.hpp"

#ifndef GRAPH_ARTICULATION_POINTS_HPP
#  define GRAPH_ARTICULATION_POINTS_HPP

#  include <limits>
#  include <optional>
#  include <stack>
#  include <vector>

namespace graph {

// Using declarations for new namespace structure
using adj_list::adjacency_list;
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
 * @tparam G          The graph type. Must satisfy adjacency_list concept.
 * @tparam Iter       The output iterator type. Must be output_iterator<vertex_id_t<G>>.
 * @tparam Alloc      Allocator type for internal DFS stack storage. Defaults to std::allocator<std::byte>.
 *
 * @param g           The graph. Callers must supply both directions of each undirected edge.
 * @param cut_vertices The output iterator where articulation point vertex IDs will be written.
 * @param alloc       Allocator instance used for the internal DFS stack (default: Alloc())
 *
 * @return void. Articulation point IDs are written to the output iterator.
 *
 * **Mandates:**
 * - G must satisfy adjacency_list (index or mapped vertex containers)
 * - Iter must satisfy std::output_iterator<vertex_id_t<G>>
 *
 * **Preconditions:**
 * - For undirected semantics, each edge {u,v} must be stored as both (u,v) and (v,u)
 *
 * **Effects:**
 * - Writes articulation point vertex IDs to cut_vertices output iterator
 * - Does not modify the graph g
 *
 * **Postconditions:**
 * - Output contains all articulation points, each emitted exactly once
 * - No ordering guarantee on the emitted vertices
 *
 * **Throws:**
 * - std::bad_alloc if internal vector allocation fails
 * - Exception guarantee: Basic. Graph g remains unchanged; output iterator may be
 *   partially written.
 *
 * **Complexity:**
 * - Time: O(V + E) — each vertex and edge visited exactly once during the DFS
 * - Space: O(V) for discovery time, low-link, parent, child count, emitted arrays, and DFS stack
 *
 * **Remarks:**
 * - Uses iterative DFS with explicit stack to avoid recursion-depth limits
 * - Root rule: root is articulation point if it has 2+ DFS children
 * - Non-root rule: u is articulation point if child v has low[v] >= disc[u]
 *
 * **Supported Graph Properties:**
 *
 * Directedness:
 * - ✅ Directed graphs (caller must store both {u,v} and {v,u} for undirected semantics)
 *
 * Edge Properties:
 * - ✅ Unweighted edges
 * - ✅ Weighted edges (weights ignored)
 * - ✅ Multi-edges (first reverse edge to parent skipped; additional parallel edges treated as back-edges)
 * - ✅ Self-loops (ignored)
 * - ✅ Cycles
 *
 * Graph Structure:
 * - ✅ Connected graphs
 * - ✅ Disconnected graphs (processes all components via outer loop)
 * - ✅ Empty graphs (returns immediately)
 *
 * ## Example Usage
 *
 * ```cpp
 * #include <graph/graph.hpp>
 * #include <graph/algorithm/articulation_points.hpp>
 * #include <vector>
 *
 * using namespace graph;
 *
 * // Path graph: 0 - 1 - 2 - 3 (bidirectional)
 * Graph g({{0,1},{1,0},{1,2},{2,1},{2,3},{3,2}});
 *
 * std::vector<vertex_id_t<Graph>> result;
 * articulation_points(g, std::back_inserter(result));
 * // result contains {1, 2} (in some order)
 * ```
 */
template <adjacency_list G, class Iter, class Alloc = std::allocator<std::byte>>
requires std::output_iterator<Iter, vertex_id_t<G>>
void articulation_points(G&& g, Iter cut_vertices, const Alloc& alloc = Alloc()) {
  using vid_t = vertex_id_t<G>;

  const size_t N = num_vertices(g);
  if (N == 0) {
    return;
  }

  constexpr size_t UNVISITED = std::numeric_limits<size_t>::max();

  auto disc        = make_vertex_property_map<G, size_t>(g, UNVISITED);
  auto low         = make_vertex_property_map<G, size_t>(g, UNVISITED);
  auto parent      = make_vertex_property_map<G, std::optional<vid_t>>(g, std::nullopt);
  auto child_count = make_vertex_property_map<G, size_t>(g, size_t{0});
  auto emitted     = make_vertex_property_map<G, bool>(g, false);

  size_t timer = 0;

  // Deduce the iterator type for edge ranges returned by edges(g, uid).
  // edge_descriptor_view iterators store the underlying edge_storage (an
  // index for RA containers, an iterator for map-based containers) and
  // do not reference the view, so they safely outlive it and can be
  // stored on the DFS stack.
  using edge_iter_t = std::ranges::iterator_t<decltype(edges(g, std::declval<const vid_t&>()))>;

  // Frame for iterative DFS: (vertex_id, edge_iterator, edge_end, parent_edge_skipped)
  // Storing iterators avoids the O(degree) re-scan that an index-based
  // approach would require each time we resume a stack frame.
  // parent_edge_skipped ensures only the first reverse edge to the DFS parent
  // is treated as the tree edge; subsequent parallel edges update low-link.
  struct dfs_frame {
    vid_t       uid;
    edge_iter_t it;
    edge_iter_t it_end;
    bool        parent_edge_skipped;
  };

  using FrameAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<dfs_frame>;
  std::stack<dfs_frame, std::deque<dfs_frame, FrameAlloc>> stk{std::deque<dfs_frame, FrameAlloc>(FrameAlloc(alloc))};

  // Outer loop: handle disconnected graphs
  for (auto [start] : views::basic_vertexlist(g)) {
    if (disc[start] != UNVISITED) {
      continue;
    }

    disc[start] = low[start] = timer++;
    auto start_edges = edges(g, start);
    stk.push({start, std::ranges::begin(start_edges),
              std::ranges::end(start_edges), false});

    while (!stk.empty()) {
      auto& [uid, it, it_end, parent_skipped] = stk.top();

      if (it == it_end) {
        // All edges processed — backtrack
        stk.pop();
        if (!stk.empty()) {
          auto& [par_uid, par_it, par_it_end, par_skipped] = stk.top();
          // Update low-link of parent
          if (low[uid] < low[par_uid]) {
            low[par_uid] = low[uid];
          }

          // Check articulation point condition for non-root
          if (parent[par_uid].has_value()) {
            // Non-root rule: child v has low[v] >= disc[u]
            if (low[uid] >= disc[par_uid] && !emitted[par_uid]) {
              *cut_vertices++  = par_uid;
              emitted[par_uid] = true;
            }
          }
        }
        continue;
      }

      vid_t vid = target_id(g, *it);
      ++it; // advance stored iterator for next resume

      // Skip self-loops
      if (vid == uid) {
        continue;
      }

      if (disc[vid] == UNVISITED) {
        // Tree edge: vid is a new DFS child of uid
        parent[vid] = uid;
        child_count[uid]++;
        disc[vid] = low[vid] = timer++;
        auto vid_edges = edges(g, vid);
        stk.push({vid, std::ranges::begin(vid_edges),
                  std::ranges::end(vid_edges), false});
      } else if (parent[uid].has_value() && *parent[uid] == vid && !parent_skipped) {
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
