/**
 * @file biconnected_components.hpp
 *
 * @brief Biconnected Components algorithm for graphs.
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

#ifndef GRAPH_BICONNECTED_COMPONENTS_HPP
#  define GRAPH_BICONNECTED_COMPONENTS_HPP

#  include <limits>
#  include <set>
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
 * @brief Find the biconnected components of a graph.
 *
 * A biconnected component (also called a 2-connected component) is a maximal
 * biconnected subgraph — one that is connected and has no articulation points.
 * Equivalently, any two vertices in a biconnected component lie on a common
 * simple cycle.
 *
 * This implementation uses the iterative Hopcroft-Tarjan algorithm extended
 * with an explicit edge stack. During the DFS, each tree edge and back edge is
 * pushed onto the edge stack. Whenever an articulation-point boundary is
 * detected on backtrack (i.e., `low[v] >= disc[u]` for child v and parent u),
 * the edge stack is popped down to and including the edge (u, v) and the unique
 * vertex IDs from those edges form one biconnected component.
 *
 * Isolated vertices (degree 0) are emitted as trivial single-vertex components.
 * Articulation-point vertices appear in more than one component.
 *
 * ## Complexity Analysis
 *
 * **Time Complexity:** O(|V| + |E|) where V is the number of vertices and E is
 * the number of edges. Each vertex and edge is visited exactly once during the DFS.
 *
 * **Space Complexity:** O(V + E) for the discovery time and low-link arrays (O(V)),
 * the DFS stack (O(V)), and the edge stack (O(E)).
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
 *      additional parallel edges are treated as back-edges)
 * - ✅ Self-loops (ignored — do not affect biconnected component detection)
 * - ✅ Cycles
 *
 * ### Graph Structure
 * - ✅ Connected graphs
 * - ✅ Disconnected graphs (processes all components via outer loop)
 * - ✅ Empty graphs (returns immediately)
 *
 * ### Container Requirements
 * - Requires: `index_adjacency_list<G>` concept (contiguous vertex IDs)
 * - Works with: All `dynamic_graph` container combinations with contiguous IDs
 *
 * @tparam G               The graph type. Must satisfy index_adjacency_list concept,
 *                         which implies contiguous vertex IDs from 0 to num_vertices(g)-1.
 * @tparam OuterContainer  A container of containers for the output components.
 *                         Typically `std::vector<std::vector<vertex_id_t<G>>>`.
 *
 * @param g           The graph. Callers must supply both directions of each undirected edge.
 * @param components  Output container; one inner container is push_back'd per biconnected
 *                    component found. Articulation-point vertices appear in multiple inner
 *                    containers. No ordering guarantee on the order of components or vertex
 *                    IDs within a component.
 *
 * @pre g must have contiguous vertex IDs [0, num_vertices(g))
 * @pre For undirected semantics, each edge {u,v} must be stored as both (u,v) and (v,u).
 *
 * @post Every vertex appears in at least one component.
 * @post Articulation-point vertices appear in more than one component.
 * @post Each component's induced subgraph is biconnected.
 * @post The graph g is not modified.
 *
 * **Exception Safety:** Basic exception safety. May throw std::bad_alloc if internal
 * vector or set allocation fails. The graph g remains unchanged; components may be
 * partially written.
 *
 * ## Example Usage
 *
 * ```cpp
 * #include <graph/graph.hpp>
 * #include <graph/algorithm/biconnected_components.hpp>
 * #include <vector>
 * #include <iostream>
 *
 * using namespace graph;
 *
 * int main() {
 *     using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
 *                       container::vov_graph_traits<void, void, void, uint32_t, false>>;
 *
 *     // Two triangles joined by bridge 2-3 (bidirectional)
 *     Graph g({{0,1},{1,0},{1,2},{2,1},{0,2},{2,0},
 *              {3,4},{4,3},{4,5},{5,4},{3,5},{5,3},
 *              {2,3},{3,2}});
 *
 *     std::vector<std::vector<vertex_id_t<Graph>>> components;
 *     biconnected_components(g, components);
 *     // components contains 3 entries: {0,1,2}, {2,3}, {3,4,5} (in some order)
 * }
 * ```
 */
template <index_adjacency_list G, class OuterContainer>
void biconnected_components(G&& g, OuterContainer& components) {
  using vid_t      = vertex_id_t<G>;
  using inner_type = typename OuterContainer::value_type;

  const size_t N = num_vertices(g);
  if (N == 0) {
    return;
  }

  constexpr size_t UNVISITED = std::numeric_limits<size_t>::max();
  const vid_t      NO_PARENT = static_cast<vid_t>(N); // sentinel for "no parent"

  std::vector<size_t> disc(N, UNVISITED);
  std::vector<size_t> low(N, UNVISITED);
  std::vector<vid_t>  parent(N, NO_PARENT);

  size_t timer = 0;

  // Edge stack: stores (source, target) pairs for edges traversed during DFS.
  // When a biconnected component boundary is detected, edges are popped to
  // extract the vertex set of that component.
  using edge_pair = std::pair<vid_t, vid_t>;
  std::stack<edge_pair> edge_stk;

  // Frame for iterative DFS: (vertex_id, edge_index, parent_edge_skipped)
  struct dfs_frame {
    vid_t  uid;
    size_t edge_idx;
    bool   parent_edge_skipped;
  };

  std::stack<dfs_frame> stk;

  // Helper: pop edges from edge_stk until (u, v) is popped (inclusive).
  // Collect unique vertex IDs and push_back as a new component.
  auto flush_component = [&](vid_t u, vid_t v) {
    std::set<vid_t> vset;
    while (true) {
      auto [eu, ev] = edge_stk.top();
      edge_stk.pop();
      vset.insert(eu);
      vset.insert(ev);
      if (eu == u && ev == v) {
        break;
      }
    }
    inner_type comp(vset.begin(), vset.end());
    components.push_back(std::move(comp));
  };

  // Outer loop: handle disconnected graphs
  for (auto sv : vertices(g)) {
    vid_t start = vertex_id(g, sv);
    if (disc[start] != UNVISITED) {
      continue;
    }

    // Check for isolated vertex (no edges)
    auto start_edges = edges(g, sv);
    if (std::ranges::begin(start_edges) == std::ranges::end(start_edges)) {
      // Isolated vertex — trivial biconnected component
      components.push_back(inner_type{static_cast<typename inner_type::value_type>(start)});
      disc[start] = timer++; // mark as visited
      continue;
    }

    disc[start] = low[start] = timer++;
    stk.push({start, 0, false});

    while (!stk.empty()) {
      auto& [uid, edge_idx, parent_skipped] = stk.top();

      auto edge_range = edges(g, uid);
      auto it         = std::ranges::begin(edge_range);
      auto it_end     = std::ranges::end(edge_range);

      // Advance iterator to edge_idx position
      for (size_t i = 0; i < edge_idx && it != it_end; ++i, ++it) {
      }

      if (it == it_end) {
        // All edges processed — backtrack
        vid_t backtrack_uid = uid;
        stk.pop();
        if (!stk.empty()) {
          auto& [par_uid, par_edge_idx, par_skipped] = stk.top();
          // Update low-link of parent
          if (low[backtrack_uid] < low[par_uid]) {
            low[par_uid] = low[backtrack_uid];
          }

          // Check biconnected component boundary:
          // If low[child] >= disc[parent], then parent is an articulation point
          // (or is the root), and we flush a component.
          if (low[backtrack_uid] >= disc[par_uid]) {
            flush_component(par_uid, backtrack_uid);
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
        disc[vid] = low[vid] = timer++;
        edge_stk.push({uid, vid});
        stk.push({vid, 0, false});
      } else if (vid == parent[uid] && !parent_skipped) {
        // First reverse edge to DFS parent — this is the tree edge; skip it
        parent_skipped = true;
      } else {
        // Back edge (or additional parallel edge to parent): update low-link
        // Only push back edges going to an ancestor (disc[vid] < disc[uid])
        // to avoid pushing forward-direction duplicates.
        if (disc[vid] < disc[uid]) {
          edge_stk.push({uid, vid});
        }
        if (disc[vid] < low[uid]) {
          low[uid] = disc[vid];
        }
      }
    }
  }
}

} // namespace graph

#endif // GRAPH_BICONNECTED_COMPONENTS_HPP
