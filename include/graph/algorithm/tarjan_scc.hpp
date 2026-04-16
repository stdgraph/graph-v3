/**
 * @file tarjan_scc.hpp
 *
 * @brief Tarjan's Strongly Connected Components algorithm for directed graphs.
 *
 * Finds all strongly connected components (SCCs) in a directed graph using
 * Tarjan's single-pass DFS algorithm with low-link values.
 *
 * @copyright Copyright (c) 2024
 *
 * SPDX-License-Identifier: BSL-1.0
 *
 * @authors
 *   Phil Ratzloff
 */

#include "graph/graph.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/adj_list/vertex_property_map.hpp"
#include "graph/algorithm/traversal_common.hpp"

#ifndef GRAPH_TARJAN_SCC_HPP
#  define GRAPH_TARJAN_SCC_HPP

#  include <limits>
#  include <stack>

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
 * @brief Find strongly connected components using Tarjan's algorithm.
 *
 * A strongly connected component (SCC) is a maximal set of vertices where every
 * vertex is reachable from every other vertex via directed paths. Tarjan's
 * algorithm discovers all SCCs in a single depth-first search using discovery
 * times and low-link values.
 *
 * @tparam G           The graph type. Must satisfy adjacency_list concept.
 * @tparam ComponentFn Callable providing per-vertex component ID access:
 *                     (const G&, vertex_id_t<G>) -> ComponentID&. Must satisfy
 *                     vertex_property_fn_for<ComponentFn, G>.
 * @tparam Alloc       Allocator type for the internal SCC stack and DFS stack storage.
 *                     Defaults to std::allocator<std::byte>.
 *
 * @param g         The directed graph to analyze
 * @param component Callable providing per-vertex component access: component(g, uid) -> ComponentID&.
 *                  For containers: wrap with container_value_fn(component).
 * @param alloc     Allocator instance used for the internal SCC stack and DFS stack (default: Alloc())
 *
 * @return Number of strongly connected components found
 *
 * **Mandates:**
 * - G must satisfy adjacency_list
 * - ComponentFn must satisfy vertex_property_fn_for<ComponentFn, G>
 *
 * **Preconditions:**
 * - component must contain an entry for each vertex of g
 *
 * **Effects:**
 * - Sets component(g, uid) for all vertices via the component function
 * - Does not modify the graph g
 *
 * **Postconditions:**
 * - component(g, uid) contains the SCC ID for vertex uid
 * - Component IDs are assigned 0, 1, 2, ... , num_components-1
 * - Vertices in the same SCC have the same component ID
 * - Return value equals the number of distinct component IDs
 *
 * **Throws:**
 * - std::bad_alloc if internal allocations fail
 * - Exception guarantee: Basic.
 *
 * **Complexity:**
 * - Time: O(V + E) — single DFS traversal visiting each vertex and edge once
 * - Space: O(V) for discovery time, low-link, on-stack flag, DFS stack, and SCC stack
 *
 * **Remarks:**
 * - Uses iterative DFS with explicit stack to avoid recursion-depth limits
 * - Single-pass: requires only one DFS (vs Kosaraju's two passes)
 * - Does not require a transpose graph
 * - Low-link values track the earliest reachable ancestor in each subtree
 * - An SCC root is identified when disc[u] == low[u] after processing all edges
 * - Component IDs are assigned in reverse topological order of the SCC DAG
 *
 * **Supported Graph Properties:**
 *
 * Directedness:
 * - ✅ Directed graphs (required)
 * - ❌ Undirected graphs (use connected_components instead)
 *
 * Edge Properties:
 * - ✅ Weighted edges (weights ignored)
 * - ✅ Self-loops (handled correctly)
 * - ✅ Multi-edges (treated as single edge)
 * - ✅ Cycles
 *
 * Graph Structure:
 * - ✅ Connected graphs
 * - ✅ Disconnected graphs
 * - ✅ Empty graphs (returns 0)
 *
 * ## Example Usage
 *
 * ```cpp
 * #include <graph/algorithm/tarjan_scc.hpp>
 * #include <vector>
 *
 * using namespace graph;
 *
 * // Create directed graph: 0->1->2->0 (cycle), 2->3
 * Graph g({{0,1}, {1,2}, {2,0}, {2,3}});
 *
 * std::vector<size_t> component(num_vertices(g));
 * size_t num = tarjan_scc(g, container_value_fn(component));
 * // num = 2, component: vertices {0,1,2} share one ID, vertex 3 has another
 * ```
 *
 * @see kosaraju For two-pass SCC using transpose graph
 * @see connected_components For undirected graphs
 */
template <adjacency_list G,
          class          ComponentFn,
          class          Alloc = std::allocator<std::byte>>
requires vertex_property_fn_for<ComponentFn, G>
size_t tarjan_scc(G&&           g,         // graph
                  ComponentFn&& component, // out: strongly connected component assignment
                  const Alloc&  alloc = Alloc()
) {
  using vid_t = vertex_id_t<G>;
  using CT    = vertex_fn_value_t<ComponentFn, G>;

  const size_t N = num_vertices(g);
  if (N == 0) {
    return 0;
  }

  constexpr size_t UNVISITED = std::numeric_limits<size_t>::max();

  auto disc     = make_vertex_property_map<std::remove_reference_t<G>, size_t>(g, UNVISITED);
  auto low      = make_vertex_property_map<std::remove_reference_t<G>, size_t>(g, UNVISITED);
  auto on_stack = make_vertex_property_map<std::remove_reference_t<G>, bool>(g, false);

  // Initialize all components as unvisited
  for (auto&& [uid, u] : views::vertexlist(g)) {
    component(g, uid) = std::numeric_limits<CT>::max();
  }

  size_t timer = 0;
  size_t cid   = 0;

  // Tarjan's stack: vertices in the current DFS path and pending SCC assignment
  using VidAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<vid_t>;
  std::stack<vid_t, std::deque<vid_t, VidAlloc>> scc_stack{std::deque<vid_t, VidAlloc>(VidAlloc(alloc))};

  // Iterative DFS: store edge iterators per frame to avoid re-scanning adjacency lists
  using edge_iter_t = std::ranges::iterator_t<decltype(edges(g, std::declval<const vid_t&>()))>;

  struct dfs_frame {
    vid_t       uid;
    edge_iter_t it;
    edge_iter_t it_end;
  };

  using FrameAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<dfs_frame>;
  std::stack<dfs_frame, std::deque<dfs_frame, FrameAlloc>> dfs{std::deque<dfs_frame, FrameAlloc>(FrameAlloc(alloc))};

  // Outer loop: handle disconnected graphs
  for (auto [start] : views::basic_vertexlist(g)) {
    if (disc[start] != UNVISITED) {
      continue;
    }

    disc[start] = low[start] = timer++;
    on_stack[start] = true;
    scc_stack.push(start);

    auto start_edges = edges(g, start);
    dfs.push({start, std::ranges::begin(start_edges), std::ranges::end(start_edges)});

    while (!dfs.empty()) {
      auto& [uid, it, it_end] = dfs.top();

      if (it == it_end) {
        // All edges processed — check if uid is an SCC root
        if (disc[uid] == low[uid]) {
          // Pop all vertices in this SCC from the Tarjan stack
          vid_t w;
          do {
            w = scc_stack.top();
            scc_stack.pop();
            on_stack[w]    = false;
            component(g, w) = static_cast<CT>(cid);
          } while (w != uid);
          ++cid;
        }

        // Backtrack: update parent's low-link
        dfs.pop();
        if (!dfs.empty()) {
          auto& [par_uid, par_it, par_it_end] = dfs.top();
          if (low[uid] < low[par_uid]) {
            low[par_uid] = low[uid];
          }
        }
        continue;
      }

      vid_t vid = target_id(g, *it);
      ++it; // advance stored iterator for next resume

      if (disc[vid] == UNVISITED) {
        // Tree edge: push new DFS frame
        disc[vid] = low[vid] = timer++;
        on_stack[vid] = true;
        scc_stack.push(vid);

        auto vid_edges = edges(g, vid);
        dfs.push({vid, std::ranges::begin(vid_edges), std::ranges::end(vid_edges)});
      } else if (on_stack[vid]) {
        // Back/cross edge to vertex still on SCC stack: update low-link
        if (disc[vid] < low[uid]) {
          low[uid] = disc[vid];
        }
      }
      // If vid is already assigned to a completed SCC (on_stack[vid] == false),
      // it's a cross edge to a finished SCC — do not update low-link.
    }
  }

  return cid;
}

} // namespace graph

#endif // GRAPH_TARJAN_SCC_HPP
