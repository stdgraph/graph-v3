/**
 * @file connected_components.hpp
 * 
 * @brief Connected components algorithms for undirected and directed graphs.
 * 
 * This file provides three algorithms for finding connected components:
 * - kosaraju: Finds strongly connected components in directed graphs (requires transpose)
 * - connected_components: Finds connected components in undirected graphs
 * - afforest: Fast parallel-friendly connected components using neighbor sampling
 * 
 * @copyright Copyright (c) 2024
 * 
 * SPDX-License-Identifier: BSL-1.0
 *
 * @authors
 *   Andrew Lumsdaine
 *   Phil Ratzloff
 *   Kevin Deweese
 */

#include "graph/graph.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/dfs.hpp"
#include "graph/views/bfs.hpp"
#include <stack>
#include <random>
#include <numeric>

#ifndef GRAPH_CC_HPP
#  define GRAPH_CC_HPP

namespace graph {

// Using declarations for new namespace structure
using adj_list::index_adjacency_list;
using adj_list::adjacency_list;
using adj_list::vertex_id_t;
using adj_list::edge_t;
using adj_list::vertex_range_t;
using adj_list::vertices;
using adj_list::edges;
using adj_list::target_id;

//=============================================================================
// kosaraju - Strongly Connected Components (Directed Graphs)
//=============================================================================

/**
 * @brief Finds strongly connected components in a directed graph using Kosaraju's algorithm.
 * 
 * A strongly connected component (SCC) is a maximal set of vertices where every vertex
 * is reachable from every other vertex in the set via directed paths. Kosaraju's algorithm
 * performs two depth-first searches: one on the original graph to determine finish times,
 * and one on the transpose graph to identify components.
 * 
 * @par Complexity Analysis
 * 
 * | Case | Time | Space |
 * |------|------|-------|
 * | All cases | O(V + E) | O(V) |
 * 
 * - Time: Two DFS traversals on graph and transpose
 * - Space: O(V) for visited array, finish order, and component assignment
 * 
 * @par Supported Graph Properties
 * 
 * - ✅ Directed graphs (required)
 * - ❌ Undirected graphs (use connected_components instead)
 * - ✅ Weighted edges (weights ignored)
 * - ✅ Self-loops (handled correctly)
 * - ✅ Multi-edges (treated as single edge)
 * - ✅ Disconnected graphs
 * - ✅ Cyclic graphs
 * 
 * @par Container Requirements
 * 
 * - Requires: `index_adjacency_list<G>`
 * - Requires: `index_adjacency_list<GT>` (transpose)
 * - Requires: `random_access_range<Component>`
 * - Works with: All `dynamic_graph` container combinations
 * 
 * @tparam G Graph type (must satisfy index_adjacency_list concept)
 * @tparam GT Graph transpose type (must satisfy index_adjacency_list concept)
 * @tparam Component Random access range for component IDs
 * 
 * @param g The directed graph to analyze
 * @param g_t The transpose of graph g (edges reversed)
 * @param component Output: component[v] = component ID for vertex v
 * 
 * @pre `component.size() >= num_vertices(g)`
 * @pre `num_vertices(g) == num_vertices(g_t)`
 * @pre `g_t` is the transpose of `g` (all edges reversed)
 * 
 * @post `component[v]` contains the SCC ID for vertex v
 * @post Component IDs are assigned 0, 1, 2, ..., num_components-1
 * @post Vertices in the same SCC have the same component ID
 * 
 * @throws May throw std::bad_alloc if internal allocations fail
 * 
 * @par Example
 * @code
 * // Create directed graph: 0->1->2->0 (cycle), 2->3
 * using Graph = container::dynamic_graph<...>;
 * Graph g({{0,1}, {1,2}, {2,0}, {2,3}});
 * Graph g_t = transpose(g); // Transpose the graph
 * 
 * std::vector<size_t> component(num_vertices(g));
 * kosaraju(g, g_t, component);
 * 
 * // component = {0, 0, 0, 1}  // Vertices 0,1,2 in SCC 0; vertex 3 in SCC 1
 * @endcode
 * 
 * @par Algorithm Overview
 * 
 * 1. **First DFS Pass (on g):**
 *    - Visit all vertices and compute finish times
 *    - Store vertices in post-order (finish time order)
 * 
 * 2. **Second DFS Pass (on g_t):**
 *    - Process vertices in reverse finish time order
 *    - Each DFS tree in this pass is one SCC
 *    - Assign component IDs to vertices as they're visited
 * 
 * @par Implementation Notes
 * 
 * - Uses iterative DFS (stack-based) to avoid recursion stack overflow
 * - Finish times tracked via explicit ordering vector
 * - Uses numeric_limits<CT>::max() as unvisited marker
 * - Component IDs assigned in topological order of SCCs
 * 
 * @see connected_components For undirected graphs
 * @see afforest For faster parallel-friendly alternative
 */
template <index_adjacency_list G,
          index_adjacency_list GT,
          random_access_range  Component>
void kosaraju(G&&        g,        // graph
              GT&&       g_t,      // graph transpose
              Component& component // out: strongly connected component assignment

) {
  size_t            N(size(vertices(g)));
  std::vector<bool> visited(N, false);
  using CT = typename std::decay<decltype(*component.begin())>::type;
  std::fill(component.begin(), component.end(), std::numeric_limits<CT>::max());
  std::vector<vertex_id_t<G>> order;

  // Store a reference to avoid forwarding reference issues in lambda
  auto& g_ref = g;

  // Helper: iterative DFS to compute finish times (post-order)
  auto dfs_finish_order = [&](vertex_id_t<G> start) {
    std::stack<std::pair<vertex_id_t<G>, bool>> stack; // (vertex, children_visited)
    stack.push({start, false});
    visited[start] = true;

    while (!stack.empty()) {
      auto [uid, children_visited] = stack.top();
      stack.pop();

      if (children_visited) {
        // All children have been visited, add to finish order
        order.push_back(uid);
      } else {
        // Mark that we'll process this vertex after its children
        stack.push({uid, true});

        // Push all unvisited neighbors
        auto uid_vertex = *find_vertex(g_ref, uid);
        for (auto&& [e] : views::incidence(g_ref, uid_vertex)) {
          auto vid = target_id(g_ref, e);
          if (!visited[vid]) {
            visited[vid] = true;
            stack.push({vid, false});
          }
        }
      }
    }
  };

  // First pass: compute finish times on original graph
  for (auto&& vinfo : views::vertexlist(g_ref)) {
    auto uid = vertex_id(g_ref, vinfo.vertex);
    if (!visited[uid]) {
      dfs_finish_order(uid);
    }
  }

  size_t                    cid = 0;
  std::ranges::reverse_view reverse{order};
  for (auto& uid : reverse) {
    if (component[uid] == std::numeric_limits<CT>::max()) {
      graph::views::vertices_dfs_view<std::remove_reference_t<GT>> dfs(g_t, uid);
      for (auto&& [v] : dfs) {
        auto vid = vertex_id(g_t, v);
        if (component[vid] != std::numeric_limits<CT>::max()) {
          dfs.cancel(graph::views::cancel_search::cancel_branch);
        } else {
          component[vid] = cid;
        }
      }
      ++cid;
    }
  }
}

//=============================================================================
// connected_components - Connected Components (Undirected Graphs)
//=============================================================================

/**
 * @brief Finds connected components in an undirected graph using iterative DFS.
 * 
 * A connected component is a maximal set of vertices where there exists a path
 * between any pair of vertices in the set. This algorithm uses depth-first search
 * with an explicit stack to identify all connected components in the graph.
 * 
 * @par Complexity Analysis
 * 
 * | Case | Time | Space |
 * |------|------|-------|
 * | All cases | O(V + E) | O(V) |
 * 
 * - Time: Single traversal visiting each vertex and edge once
 * - Space: O(V) for component assignment and DFS stack
 * 
 * @par Supported Graph Properties
 * 
 * - ✅ Undirected graphs (treats directed graphs as undirected)
 * - ✅ Directed graphs (ignores edge direction)
 * - ✅ Weighted edges (weights ignored)
 * - ✅ Self-loops (handled correctly, counted as component)
 * - ✅ Multi-edges (treated as single edge)
 * - ✅ Disconnected graphs (primary use case)
 * - ✅ Acyclic graphs
 * - ✅ Cyclic graphs
 * 
 * @par Container Requirements
 * 
 * - Requires: `index_adjacency_list<G>` (vertex IDs are indices)
 * - Requires: `random_access_range<Component>`
 * - Works with: All `dynamic_graph` container combinations
 * - Works with: Vector-based containers (vov, vol, vofl, etc.)
 * 
 * @tparam G Graph type (must satisfy index_adjacency_list concept)
 * @tparam Component Random access range for component IDs
 * 
 * @param g The graph to analyze (treated as undirected)
 * @param component Output: component[v] = component ID for vertex v
 * 
 * @return Number of connected components found
 * 
 * @pre `component.size() >= num_vertices(g)`
 * 
 * @post `component[v]` contains the component ID for vertex v
 * @post Component IDs are assigned 0, 1, 2, ..., num_components-1
 * @post Vertices in the same component have the same component ID
 * @post Return value equals the number of distinct component IDs
 * @post Isolated vertices (no edges) are assigned unique component IDs
 * 
 * @throws May throw std::bad_alloc if internal allocations fail
 * 
 * @par Example
 * @code
 * // Create undirected graph with 2 components: {0,1,2} and {3,4}
 * using Graph = container::dynamic_graph<...>;
 * Graph g(5);
 * // Component 1: 0-1-2
 * g.add_edge(0, 1);
 * g.add_edge(1, 2);
 * // Component 2: 3-4
 * g.add_edge(3, 4);
 * 
 * std::vector<size_t> component(num_vertices(g));
 * size_t num_components = connected_components(g, component);
 * 
 * // num_components = 2
 * // component = {0, 0, 0, 1, 1}
 * @endcode
 * 
 * @par Algorithm Overview
 * 
 * 1. Initialize all components to unvisited (numeric_limits::max)
 * 2. For each unvisited vertex:
 *    - Start new component with unique ID
 *    - Use DFS to visit all reachable vertices
 *    - Assign component ID to all visited vertices
 * 3. Return total number of components found
 * 
 * @par Implementation Notes
 * 
 * - Uses iterative DFS with explicit stack (no recursion)
 * - Isolated vertices (degree 0) get unique component IDs
 * - Handles vertices with no edges specially for efficiency
 * - Uses numeric_limits<CT>::max() as unvisited marker
 * 
 * @par Special Cases
 * 
 * - **Isolated vertices:** Each gets its own component ID
 * - **Empty graph:** Returns 0 (no components)
 * - **Single vertex:** Returns 1 (one component)
 * - **Fully connected:** Returns 1 (one component)
 * 
 * @see kosaraju For strongly connected components in directed graphs
 * @see afforest For faster parallel-friendly alternative
 */
template <index_adjacency_list G,
          random_access_range  Component>
size_t connected_components(G&&        g,        // graph
                            Component& component // out: connected component assignment
) {
  size_t N(size(vertices(g)));
  using CT = typename std::decay<decltype(*component.begin())>::type;
  std::fill(component.begin(), component.end(), std::numeric_limits<CT>::max());

  std::stack<vertex_id_t<G>> S;
  CT                         cid = 0;
  for (vertex_id_t<G> uid = 0; uid < N; ++uid) {
    if (component[uid] < std::numeric_limits<CT>::max()) {
      continue;
    }

    if (!size(edges(g, uid))) {
      component[uid] = cid++;
      continue;
    }

    component[uid] = cid;
    S.push(uid);
    while (!S.empty()) {
      auto vid = S.top();
      S.pop();
      for (auto&& einfo : views::incidence(g, vid)) {
        auto wid = target_id(g, einfo.edge);
        if (component[wid] == std::numeric_limits<CT>::max()) {
          component[wid] = cid;
          S.push(wid);
        }
      }
    }
    ++cid;
  }
  return cid;
}

//=============================================================================
// Helper Functions for afforest Algorithm
//=============================================================================

/**
 * @brief Links two vertices into the same component using union-find.
 * 
 * Internal helper for afforest algorithm. Performs path compression while
 * linking two components together.
 * 
 * @tparam vertex_id_t Vertex ID type
 * @tparam Component Random access range for component IDs
 * @param u First vertex ID
 * @param v Second vertex ID  
 * @param component Component assignment array (modified in-place)
 */
template <typename vertex_id_t, random_access_range Component>
static void link(vertex_id_t u, vertex_id_t v, Component& component) {
  vertex_id_t p1 = component[u];
  vertex_id_t p2 = component[v];

  while (p1 != p2) {
    vertex_id_t high   = std::max(p1, p2);
    vertex_id_t low    = p1 + (p2 - high);
    vertex_id_t p_high = component[high];
    if (p_high == low)
      break;
    if (p_high == high) {
      if (component[high] == high) {
        component[high] = low;
        break;
      } else {
        high = low;
      }
    }
    p1 = component[p_high];
    p2 = component[low];
  }
}

/**
 * @brief Compresses component paths for improved query performance.
 * 
 * Internal helper for afforest algorithm. Performs path compression to
 * flatten the component tree structure.
 * 
 * @tparam Component Random access range for component IDs
 * @param component Component assignment array (modified in-place)
 */
template <random_access_range Component>
static void compress(Component& component) {
  for (size_t i = 0; i < component.size(); ++i) {
    if (component[i] != component[component[i]]) {
      component[i] = component[component[i]];
    }
  }
}

/**
 * @brief Samples the most frequent component ID using random sampling.
 * 
 * Internal helper for afforest algorithm. Uses reservoir sampling to identify
 * the largest component without full traversal.
 * 
 * @tparam vertex_id_t Vertex ID type
 * @tparam Component Random access range for component IDs
 * @param component Component assignment array
 * @param num_samples Number of random samples to take (default: 1024)
 * @return The most frequently occurring component ID in the sample
 */
template <typename vertex_id_t, random_access_range Component>
static vertex_id_t sample_frequent_element(Component& component, size_t num_samples = 1024) {
  std::unordered_map<vertex_id_t, int>       counts(32);
  std::mt19937                               gen;
  std::uniform_int_distribution<vertex_id_t> distribution(0, component.size() - 1);

  for (size_t i = 0; i < num_samples; ++i) {
    vertex_id_t sample = distribution(gen);
    counts[component[sample]]++;
  }

  auto&& [num, count] = *std::max_element(counts.begin(), counts.end(),
                                          [](auto&& a, auto&& b) { return std::get<1>(a) < std::get<1>(b); });
  return num;
}

//=============================================================================
// afforest - Fast Parallel-Friendly Connected Components
//=============================================================================

/**
 * @brief Finds connected components using the Afforest algorithm (neighbor sampling).
 * 
 * Afforest is a fast, parallel-friendly algorithm for finding connected components that
 * uses neighbor sampling and union-find with path compression. It processes edges in
 * rounds, linking vertices through their first few neighbors, then samples to identify
 * the largest component before processing remaining edges. This approach is particularly
 * effective for large graphs and can be parallelized efficiently.
 * 
 * @par Complexity Analysis
 * 
 * | Case | Time | Space |
 * |------|------|-------|
 * | Best case | O(V) | O(V) |
 * | Average case | O(V + E·α(V)) | O(V) |
 * | Worst case | O(V + E·α(V)) | O(V) |
 * 
 * Where α(V) is the inverse Ackermann function (effectively constant).
 * 
 * - Time: Nearly linear due to union-find with path compression
 * - Space: O(V) for component array only (no additional structures)
 * - Practical performance: Often faster than DFS-based algorithms for large graphs
 * 
 * @par Supported Graph Properties
 * 
 * - ✅ Undirected graphs (primary use case)
 * - ✅ Directed graphs (treats as undirected)
 * - ✅ Weighted edges (weights ignored)
 * - ✅ Self-loops (handled correctly)
 * - ✅ Multi-edges (all edges processed)
 * - ✅ Disconnected graphs
 * - ✅ Large-scale graphs (designed for performance)
 * - ✅ Parallel execution friendly (this implementation is serial)
 * 
 * @par Container Requirements
 * 
 * - Requires: `index_adjacency_list<G>` (vertex IDs are indices)
 * - Requires: `random_access_range<Component>`
 * - Requires: Bidirectional conversion between vertex_id_t<G> and Component value type
 * - Works with: All `dynamic_graph` container combinations
 * - Works with: Vector-based containers for best performance
 * 
 * @tparam G Graph type (must satisfy index_adjacency_list concept)
 * @tparam Component Random access range for component IDs
 * 
 * @param g The graph to analyze (treated as undirected)
 * @param component Output: component[v] = component ID for vertex v
 * @param neighbor_rounds Number of neighbor sampling rounds (default: 2)
 * 
 * @pre `component.size() >= num_vertices(g)`
 * @pre `neighbor_rounds >= 0`
 * 
 * @post `component[v]` contains the component ID for vertex v
 * @post Vertices in the same component have the same component ID
 * @post Component IDs form a union-find forest (may need compression for queries)
 * 
 * @throws May throw std::bad_alloc if internal allocations fail
 * 
 * @par Example
 * @code
 * using Graph = container::dynamic_graph<...>;
 * Graph g({{0,1}, {1,2}, {3,4}, {4,5}});  // Two components
 * 
 * std::vector<size_t> component(num_vertices(g));
 * afforest(g, component);
 * 
 * // Compress to get canonical component IDs
 * compress(component);
 * @endcode
 * 
 * @par Algorithm Overview
 * 
 * 1. **Initialization:** Each vertex is its own component
 * 2. **Neighbor Rounds:** For r = 0 to neighbor_rounds-1:
 *    - Link each vertex to its r-th neighbor
 *    - Compress paths
 * 3. **Sampling:** Identify most frequent component (largest)
 * 4. **Remaining Edges:** Process edges beyond neighbor_rounds for non-largest components
 * 5. **Final Compression:** Flatten union-find structure
 * 
 * @par Implementation Notes
 * 
 * - Uses union-find with path compression for near-constant time operations
 * - Neighbor sampling reduces total edge processing for many graphs
 * - Sampling step identifies largest component to skip redundant work
 * - More efficient than DFS for graphs with large components
 * - Serial implementation; parallel version would use atomic operations
 * 
 * @par Performance Tuning
 * 
 * - `neighbor_rounds=1`: Fastest, good for dense graphs
 * - `neighbor_rounds=2`: Default, balanced performance
 * - `neighbor_rounds>2`: More thorough initial linking, diminishing returns
 * - For sparse graphs: Lower values perform better
 * - For dense graphs: Higher values may improve early component formation
 * 
 * @see connected_components For simpler DFS-based alternative
 * @see kosaraju For directed graph strongly connected components
 * 
 * @par References
 * 
 * - Sutton et al. (2018). "Afforest: A Fast Parallel Connected Components Algorithm"
 *   International Conference on Parallel Processing (ICPP)
 */
template <index_adjacency_list G, random_access_range Component>
requires std::convertible_to<range_value_t<Component>, vertex_id_t<G>> &&
         std::convertible_to<vertex_id_t<G>, range_value_t<Component>>
void afforest(G&&          g,         // graph
              Component&   component, // out: connected component assignment
              const size_t neighbor_rounds = 2) {
  size_t N(size(vertices(g)));
  std::iota(component.begin(), component.end(), 0);

  for (size_t r = 0; r < neighbor_rounds; ++r) {
    for (auto&& [u] : views::vertexlist(g)) {
      auto uid = vertex_id(g, u);
      if (r < size(edges(g, u))) {
        auto it = edges(g, u).begin();
        std::advance(it, r);
        link(static_cast<vertex_id_t<G>>(uid), static_cast<vertex_id_t<G>>(target_id(g, *it)), component);
      }
    }
    compress(component);
  }

  vertex_id_t<G> c = sample_frequent_element<vertex_id_t<G>>(component);

  for (auto&& vinfo : views::vertexlist(g)) {
    auto uid = vertex_id(g, vinfo.vertex);
    if (component[uid] == c) {
      continue;
    }
    if (neighbor_rounds < edges(g, uid).size()) {
      auto it = edges(g, vinfo.vertex).begin();
      std::advance(it, neighbor_rounds);
      for (; it != edges(g, vinfo.vertex).end(); ++it) {
        link(static_cast<vertex_id_t<G>>(uid), static_cast<vertex_id_t<G>>(target_id(g, *it)), component);
      }
    }
  }

  compress(component);
}

/**
 * @brief Finds connected components using Afforest with bidirectional edge processing.
 * 
 * This overload of afforest processes edges in both directions (forward and reverse)
 * by accepting both the original graph and its transpose. This can improve convergence
 * for directed graphs when treating them as undirected, and may find components faster
 * in some graph structures.
 * 
 * @par Complexity Analysis
 * 
 * Same as single-graph afforest, but processes edges in both directions:
 * - Time: O(V + (E + E_t)·α(V)) where E_t is edges in transpose
 * - Space: O(V) (transpose not counted)
 * 
 * @par Additional Requirements
 * 
 * All requirements from single-graph afforest, plus:
 * - `g_t` must be transpose of `g` (edges reversed) OR contain additional edges
 * - Can be used for bidirectional edge processing in undirected graphs represented as directed
 * 
 * @tparam G Graph type (must satisfy index_adjacency_list concept)
 * @tparam GT Graph transpose type (must satisfy adjacency_list concept)
 * @tparam Component Random access range for component IDs
 * 
 * @param g The graph to analyze
 * @param g_t The transpose of g (or additional edges to process)
 * @param component Output: component[v] = component ID for vertex v
 * @param neighbor_rounds Number of neighbor sampling rounds (default: 2)
 * 
 * @pre All preconditions from single-graph afforest
 * @pre `num_vertices(g) == num_vertices(g_t)`
 * 
 * @post Same postconditions as single-graph afforest
 * 
 * @par Example
 * @code
 * using Graph = container::dynamic_graph<...>;
 * Graph g({{0,1}, {2,3}});
 * Graph g_t = transpose(g);  // g_t: {{1,0}, {3,2}}
 * 
 * std::vector<size_t> component(num_vertices(g));
 * afforest(g, g_t, component);  // Process edges in both directions
 * 
 * compress(component);
 * @endcode
 * 
 * @par Algorithm Differences
 * 
 * Same as single-graph afforest, with additional step:
 * - After processing remaining edges from g, also processes all edges from g_t
 * - This ensures bidirectional reachability for vertices not in largest component
 * 
 * @par Use Cases
 * 
 * - Directed graphs represented as undirected (process both edge directions)
 * - Graphs where transpose is already available
 * - Improving convergence speed for certain graph topologies
 * 
 * @see afforest(G&&, Component&, size_t) For single-graph version
 */
template <index_adjacency_list G, adjacency_list GT, random_access_range Component>
requires std::convertible_to<range_value_t<Component>, vertex_id_t<G>> &&
         std::convertible_to<vertex_id_t<G>, range_value_t<Component>>
void afforest(G&&          g,         // graph
              GT&&         g_t,       // graph transpose
              Component&   component, // out: connected component assignment
              const size_t neighbor_rounds = 2) {
  size_t N(size(vertices(g)));
  std::iota(component.begin(), component.end(), 0);

  for (size_t r = 0; r < neighbor_rounds; ++r) {
    for (auto&& [u] : views::vertexlist(g)) {
      auto uid = vertex_id(g, u);
      if (r < size(edges(g, u))) {
        auto it = edges(g, u).begin();
        std::advance(it, r);
        link(static_cast<vertex_id_t<G>>(uid), static_cast<vertex_id_t<G>>(target_id(g, *it)), component);
      }
    }
    compress(component);
  }

  vertex_id_t<G> c = sample_frequent_element<vertex_id_t<G>>(component);

  for (auto&& [u] : views::vertexlist(g)) {
    auto uid = vertex_id(g, u);
    if (component[uid] == c) {
      continue;
    }
    if (neighbor_rounds < edges(g, uid).size()) {
      auto it = edges(g, u).begin();
      std::advance(it, neighbor_rounds);
      for (; it != edges(g, u).end(); ++it) {
        link(static_cast<vertex_id_t<G>>(uid), static_cast<vertex_id_t<G>>(target_id(g, *it)), component);
      }
    }
    for (auto it2 = edges(g_t, u).begin(); it2 != edges(g_t, u).end(); ++it2) {
      link(static_cast<vertex_id_t<G>>(uid), static_cast<vertex_id_t<G>>(target_id(g_t, *it2)), component);
    }
  }

  compress(component);
}

} // namespace graph

#endif //GRAPH_CC_HPP
