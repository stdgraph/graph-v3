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
#include "graph/adj_list/vertex_property_map.hpp"

#ifndef GRAPH_BICONNECTED_COMPONENTS_HPP
#  define GRAPH_BICONNECTED_COMPONENTS_HPP

#  include <limits>
#  include <optional>
#  include <set>
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
 * @brief Find the biconnected components of a graph using the Hopcroft-Tarjan algorithm.
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
 * @tparam G               The graph type. Must satisfy adjacency_list concept.
 * @tparam OuterContainer  A container of containers for the output components.
 *                         Typically std::vector<std::vector<vertex_id_t<G>>>.
 *
 * @param g           The graph to process. Callers must supply both directions of each
 *                    undirected edge.
 * @param components  [out] Output container; one inner container is push_back'd per biconnected
 *                    component found. Articulation-point vertices appear in multiple inner
 *                    containers. No ordering guarantee on the order of components or vertex
 *                    IDs within a component.
 *
 * @return void. Results are stored in the components output parameter.
 *
 * **Mandates:**
 * - G must satisfy adjacency_list (index or mapped vertex containers)
 * - OuterContainer must support push_back with an inner container constructible
 *   from a pair of set iterators
 *
 * **Preconditions:**
 * - For undirected semantics, each edge {u,v} must be stored as both (u,v) and (v,u)
 *
 * **Effects:**
 * - Modifies components: push_back's one inner container per biconnected component
 * - Does not modify the graph g
 *
 * **Postconditions:**
 * - Every vertex appears in at least one component
 * - Articulation-point vertices appear in more than one component
 * - Each component's induced subgraph is biconnected
 *
 * **Throws:**
 * - std::bad_alloc from internal vector, set, or stack allocations
 * - Exception guarantee: Basic. If an exception is thrown, graph g remains unchanged;
 *   components may be partially written (indeterminate state).
 *
 * **Complexity:**
 * - Time: O(V + E) — each vertex and edge is visited exactly once during the DFS
 * - Space: O(V + E) for discovery/low-link arrays (O(V)), DFS stack (O(V)), and
 *   edge stack (O(E))
 *
 * **Remarks:**
 * - Uses iterative DFS with explicit stack to avoid recursion-depth limits
 * - Edge iterators are stored on the DFS stack to avoid O(degree) re-scans on resume
 *
 * **Supported Graph Properties:**
 *
 * Directedness:
 * - ✅ Directed graphs (caller must store both {u,v} and {v,u} for undirected semantics)
 *
 * Edge Properties:
 * - ✅ Unweighted edges
 * - ✅ Weighted edges (weights ignored)
 * - ✅ Multi-edges (only the first reverse edge to the DFS parent is skipped as the tree edge;
 *      additional parallel edges are treated as back-edges)
 * - ✅ Self-loops (ignored — do not affect biconnected component detection)
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
template <adjacency_list G, class OuterContainer>
void biconnected_components(G&& g, OuterContainer& components) {
  using vid_t      = vertex_id_t<G>;
  using inner_type = typename OuterContainer::value_type;

  const size_t N = num_vertices(g);
  if (N == 0) {
    return;
  }

  constexpr size_t UNVISITED = std::numeric_limits<size_t>::max();

  auto disc   = make_vertex_property_map<G, size_t>(g, UNVISITED);
  auto low    = make_vertex_property_map<G, size_t>(g, UNVISITED);
  auto parent = make_vertex_property_map<G, std::optional<vid_t>>(g, std::nullopt);

  size_t timer = 0;

  // Edge stack: stores (source, target) pairs for edges traversed during DFS.
  // When a biconnected component boundary is detected, edges are popped to
  // extract the vertex set of that component.
  using edge_pair = std::pair<vid_t, vid_t>;
  std::stack<edge_pair> edge_stk;

  // Deduce the iterator type for edge ranges returned by edges(g, uid).
  // edge_descriptor_view iterators store the underlying edge_storage (an
  // index for RA containers, an iterator for map-based containers) and
  // do not reference the view, so they safely outlive it and can be
  // stored on the DFS stack.
  using edge_iter_t = std::ranges::iterator_t<decltype(edges(g, std::declval<const vid_t&>()))>;

  // Frame for iterative DFS: (vertex_id, edge_iterator, edge_end, parent_edge_skipped)
  // Storing iterators avoids the O(degree) re-scan that an index-based
  // approach would require each time we resume a stack frame.
  struct dfs_frame {
    vid_t       uid;
    edge_iter_t it;
    edge_iter_t it_end;
    bool        parent_edge_skipped;
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
  for (auto [start] : views::basic_vertexlist(g)) {
    if (disc[start] != UNVISITED) {
      continue;
    }

    // Check for isolated vertex (no edges)
    auto start_edges = edges(g, start);
    if (std::ranges::begin(start_edges) == std::ranges::end(start_edges)) {
      // Isolated vertex — trivial biconnected component
      components.push_back(inner_type{static_cast<typename inner_type::value_type>(start)});
      disc[start] = timer++; // mark as visited
      continue;
    }

    disc[start] = low[start] = timer++;
    stk.push({start, std::ranges::begin(start_edges),
              std::ranges::end(start_edges), false});

    while (!stk.empty()) {
      auto& [uid, it, it_end, parent_skipped] = stk.top();

      if (it == it_end) {
        // All edges processed — backtrack
        vid_t backtrack_uid = uid;
        stk.pop();
        if (!stk.empty()) {
          auto& [par_uid, par_it, par_it_end, par_skipped] = stk.top();
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
      ++it; // advance stored iterator for next resume

      // Skip self-loops
      if (vid == uid) {
        continue;
      }

      if (disc[vid] == UNVISITED) {
        // Tree edge: vid is a new DFS child of uid
        parent[vid] = uid;
        disc[vid] = low[vid] = timer++;
        edge_stk.push({uid, vid});
        auto vid_edges = edges(g, vid);
        stk.push({vid, std::ranges::begin(vid_edges),
                  std::ranges::end(vid_edges), false});
      } else if (parent[uid].has_value() && *parent[uid] == vid && !parent_skipped) {
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
