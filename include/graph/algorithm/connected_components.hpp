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
#include "graph/adj_list/vertex_property_map.hpp"
#include <stack>
#include <random>
#include <numeric>
#include "graph/algorithm/traversal_common.hpp"

#ifndef GRAPH_CC_HPP
#  define GRAPH_CC_HPP

namespace graph {

// Using declarations for new namespace structure
using adj_list::index_adjacency_list;
using adj_list::index_bidirectional_adjacency_list;
using adj_list::adjacency_list;
using adj_list::index_vertex_range;
using adj_list::vertex_id_t;
using adj_list::vertex_t;
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
 * @tparam G Graph type (must satisfy index_adjacency_list concept)
 * @tparam GT Graph transpose type (must satisfy index_adjacency_list concept)
 * @tparam ComponentFn Callable providing per-vertex component ID access:
 *                     (const G&, vertex_id_t<G>) -> ComponentID&. Must satisfy
 *                     vertex_property_fn_for<ComponentFn, G>.
 * 
 * @param g The directed graph to analyze
 * @param g_t The transpose of graph g (edges reversed)
 * @param component Callable providing per-vertex component access: component(g, uid) -> ComponentID&.
 *                  For containers: wrap with container_value_fn(c).
 * 
 * @return void. Results are stored in the component output parameter.
 * 
 * **Mandates:**
 * - G must satisfy adjacency_list
 * - GT must satisfy adjacency_list
 * - ComponentFn must satisfy vertex_property_fn_for<ComponentFn, G>
 * 
 * **Preconditions:**
 * - component must contain an entry for each vertex of g
 * - num_vertices(g) == num_vertices(g_t)
 * - g_t is the transpose of g (all edges reversed)
 * 
 * **Effects:**
 * - Sets component(g, uid) for all vertices via the component function
 * - Does not modify graphs g or g_t
 * 
 * **Postconditions:**
 * - component(g, uid) contains the SCC ID for vertex uid
 * - Component IDs are assigned 0, 1, 2, ..., num_components-1
 * - Vertices in the same SCC have the same component ID
 * 
 * **Throws:**
 * - std::bad_alloc if internal allocations fail
 * - Exception guarantee: Basic.
 * 
 * **Complexity:**
 * - Time: O(V + E) — two DFS traversals on graph and transpose
 * - Space: O(V) for visited array, finish order, and component assignment
 * 
 * **Remarks:**
 * - Uses iterative DFS (stack-based) to avoid recursion stack overflow
 * - Finish times tracked via explicit ordering vector
 * - Uses numeric_limits<CT>::max() as unvisited marker
 * - Component IDs assigned in topological order of SCCs
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
 * - ✅ Empty graphs
 *
 * ## Example Usage
 *
 * ```cpp
 * // Create directed graph: 0->1->2->0 (cycle), 2->3
 * Graph g({{0,1}, {1,2}, {2,0}, {2,3}});
 * Graph g_t = transpose(g);
 *
 * std::vector<size_t> component(num_vertices(g));
 * kosaraju(g, g_t, container_value_fn(component));
 * // component = {0, 0, 0, 1}
 * ```
 *
 * @see connected_components For undirected graphs
 * @see afforest For faster parallel-friendly alternative
 */
template <adjacency_list G,
          adjacency_list GT,
          class          ComponentFn>
requires vertex_property_fn_for<ComponentFn, G>
void kosaraju(G&&           g,         // graph
              GT&&          g_t,       // graph transpose
              ComponentFn&& component  // out: strongly connected component assignment
) {
  using CT = vertex_fn_value_t<ComponentFn, G>;
  auto visited = make_vertex_property_map<std::remove_reference_t<G>, bool>(g, false);
  // Initialize all components as unvisited
  for (auto&& [uid, u] : views::vertexlist(g)) {
    component(g, uid) = std::numeric_limits<CT>::max();
  }
  // Order stores vertex IDs (not descriptors) because the second pass
  // operates on g_t which has different descriptors than g.
  std::vector<vertex_id_t<G>> order;

  // Store a reference to avoid forwarding reference issues in lambda
  auto& g_ref = g;

  // Helper: iterative DFS to compute finish times (post-order)
  // This creates reverse topological ordering for SCC discovery
  // Stack stores vertex descriptors — 8-byte iterators, no string copies,
  // and O(1) vertex_id extraction via vertex_id(g, descriptor).
  using vertex_desc = vertex_t<std::remove_reference_t<G>>;
  auto dfs_finish_order = [&](vertex_desc start) {
    std::stack<std::pair<vertex_desc, bool>> stack; // (vertex, children_visited)
    stack.push({start, false});
    visited[vertex_id(g_ref, start)] = true;

    while (!stack.empty()) {
      auto [u, children_visited] = stack.top();
      stack.pop();

      if (children_visited) {
        // All children have been visited, add to finish order (post-order)
        order.push_back(vertex_id(g_ref, u));
      } else {
        // Re-push with children_visited=true to record finish time later
        stack.push({u, true});

        // Push all unvisited neighbors onto stack
        for (auto&& [vid, e] : views::incidence(g_ref, u)) {
          if (!visited[vid]) {
            visited[vid] = true;
            stack.push({*find_vertex(g_ref, vid), false});
          }
        }
      }
    }
  };

  // First pass: compute finish times on original graph
  for (auto&& [uid, u] : views::vertexlist(g_ref)) {
    if (!visited[uid]) {
      dfs_finish_order(u);
    }
  }

  // Second pass: DFS on transpose graph in reverse finish order
  // Each DFS tree in this pass corresponds to exactly one SCC
  using gt_vertex_desc = vertex_t<std::remove_reference_t<GT>>;
  size_t                    cid = 0;
  std::ranges::reverse_view reverse{order};
  for (auto& uid : reverse) {
    if (component(g, uid) == std::numeric_limits<CT>::max()) {
      // Manual iterative DFS on transpose graph using descriptors
      std::stack<gt_vertex_desc> dfs_stack;
      dfs_stack.push(*find_vertex(g_t, uid));
      component(g, uid) = cid;

      while (!dfs_stack.empty()) {
        auto current = dfs_stack.top();
        dfs_stack.pop();

        for (auto&& [vid, e] : views::incidence(g_t, current)) {
          if (component(g, vid) == std::numeric_limits<CT>::max()) {
            component(g, vid) = cid; // Assign to current SCC
            dfs_stack.push(*find_vertex(g_t, vid));
          }
        }
      }
      ++cid; // Move to next SCC
    }
  }
}

//=============================================================================
// kosaraju (bidirectional) - Single-graph SCC using in_edges
//=============================================================================

/**
 * @brief Finds strongly connected components using in_edges (no transpose needed).
 * 
 * When the graph satisfies `bidirectional_adjacency_list`, the second DFS
 * pass can traverse incoming edges directly instead of requiring a separate
 * transpose graph.  This eliminates the O(V + E) cost of constructing and
 * storing the transpose.  Works with both index and mapped bidirectional graphs.
 * 
 * @tparam G Graph type (must satisfy bidirectional_adjacency_list concept)
 * @tparam ComponentFn Callable providing per-vertex component ID access:
 *                     (const G&, vertex_id_t<G>) -> ComponentID&. Must satisfy
 *                     vertex_property_fn_for<ComponentFn, G>.
 * 
 * @param g The directed bidirectional graph to analyze
 * @param component Callable providing per-vertex component access: component(g, uid) -> ComponentID&.
 *                  For containers: wrap with container_value_fn(c).
 * 
 * @return void. Results are stored in the component output parameter.
 * 
 * **Mandates:**
 * - G must satisfy bidirectional_adjacency_list
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
 * - Component IDs are assigned 0, 1, 2, ..., num_components-1
 * - Vertices in the same SCC have the same component ID
 * 
 * **Throws:**
 * - std::bad_alloc if internal allocations fail
 * - Exception guarantee: Basic.
 * 
 * **Complexity:**
 * - Time: O(V + E) — same asymptotic cost as two-graph overload
 * - Space: O(V) — lower constant factor (no transpose construction)
 *
 * ## Example Usage
 *
 * ```cpp
 * using Traits = container::vov_graph_traits<int, void, void, size_t, false, true>;
 * container::dynamic_graph<Traits> g({{0,1}, {1,2}, {2,0}, {2,3}});
 *
 * std::vector<size_t> component(num_vertices(g));
 * kosaraju(g, container_value_fn(component));  // No transpose needed!
 * ```
 *
 * @see kosaraju(G&&, GT&&, Component&) For non-bidirectional graphs
 */
template <bidirectional_adjacency_list G,
          class                        ComponentFn>
requires vertex_property_fn_for<ComponentFn, G>
void kosaraju(G&&           g,         // bidirectional graph
              ComponentFn&& component  // out: strongly connected component assignment
) {
  using CT = vertex_fn_value_t<ComponentFn, G>;
  auto visited = make_vertex_property_map<std::remove_reference_t<G>, bool>(g, false);
  // Initialize all components as unvisited
  for (auto&& [uid, u] : views::vertexlist(g)) {
    component(g, uid) = std::numeric_limits<CT>::max();
  }
  // Both passes use the same graph, so descriptors are valid throughout.
  using vertex_desc = vertex_t<std::remove_reference_t<G>>;
  std::vector<vertex_desc> order;

  auto& g_ref = g;

  // First pass: iterative DFS to compute finish times (same as two-graph version)
  // Stack stores vertex descriptors — lightweight, no string copies.
  auto dfs_finish_order = [&](vertex_desc start) {
    std::stack<std::pair<vertex_desc, bool>> stack;
    stack.push({start, false});
    visited[vertex_id(g_ref, start)] = true;

    while (!stack.empty()) {
      auto [u, children_visited] = stack.top();
      stack.pop();

      if (children_visited) {
        order.push_back(u);
      } else {
        stack.push({u, true});

        for (auto&& [vid, e] : views::incidence(g_ref, u)) {
          if (!visited[vid]) {
            visited[vid] = true;
            stack.push({*find_vertex(g_ref, vid), false});
          }
        }
      }
    }
  };

  for (auto&& [uid, u] : views::vertexlist(g_ref)) {
    if (!visited[uid]) {
      dfs_finish_order(u);
    }
  }

  // Second pass: DFS on reverse edges (via in_edges) in reverse finish order.
  // Each DFS tree corresponds to exactly one SCC.
  size_t                    cid = 0;
  std::ranges::reverse_view reverse{order};
  for (auto& u : reverse) {
    auto uid = vertex_id(g_ref, u);
    if (component(g, uid) == std::numeric_limits<CT>::max()) {
      // Manual iterative DFS using in_edges + source_id, storing descriptors
      std::stack<vertex_desc> dfs_stack;
      dfs_stack.push(u);
      component(g, uid) = cid;

      while (!dfs_stack.empty()) {
        auto current = dfs_stack.top();
        dfs_stack.pop();

        for (auto&& ie : adj_list::in_edges(g_ref, current)) {
          auto src = adj_list::source_id(g_ref, ie);
          if (component(g, src) == std::numeric_limits<CT>::max()) {
            component(g, src) = cid;
            dfs_stack.push(*adj_list::find_vertex(g_ref, src));
          }
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
 * @tparam G Graph type (must satisfy index_adjacency_list concept)
 * @tparam ComponentFn Callable providing per-vertex component ID access:
 *                     (const G&, vertex_id_t<G>) -> ComponentID&. Must satisfy
 *                     vertex_property_fn_for<ComponentFn, G>.
 * 
 * @param g The graph to analyze (treated as undirected)
 * @param component Callable providing per-vertex component access: component(g, uid) -> ComponentID&.
 *                  For containers: wrap with container_value_fn(c).
 * 
 * @return Number of connected components found
 * 
 * **Mandates:**
 * - G must satisfy adjacency_list
 * - ComponentFn must satisfy vertex_property_fn_for<ComponentFn, G>
 * 
 * **Preconditions:**
 * - component must contain an entry for each vertex of g
 * 
 * **Effects:**
 * - Modifies component: Sets component(g, uid) for all vertices
 * - Does not modify the graph g
 * 
 * **Postconditions:**
 * - component(g, uid) contains the component ID for vertex uid
 * - Component IDs are assigned 0, 1, 2, ..., num_components-1
 * - Vertices in the same component have the same component ID
 * - Return value equals the number of distinct component IDs
 * - Isolated vertices (no edges) are assigned unique component IDs
 * 
 * **Returns:**
 * - Number of connected components found (size_t)
 * 
 * **Throws:**
 * - std::bad_alloc if internal allocations fail
 * - Exception guarantee: Basic.
 * 
 * **Complexity:**
 * - Time: O(V + E) — single traversal visiting each vertex and edge once
 * - Space: O(V) for component assignment and DFS stack
 * 
 * **Remarks:**
 * - Uses iterative DFS with explicit stack (no recursion)
 * - Isolated vertices (degree 0) get unique component IDs
 * - Uses numeric_limits<CT>::max() as unvisited marker
 * - Special cases: empty graph returns 0, single vertex returns 1
 * 
 * **Supported Graph Properties:**
 *
 * Directedness:
 * - ✅ Undirected graphs (primary use case)
 * - ✅ Directed graphs (ignores edge direction)
 *
 * Edge Properties:
 * - ✅ Weighted edges (weights ignored)
 * - ✅ Self-loops (handled correctly)
 * - ✅ Multi-edges (treated as single edge)
 * - ✅ Cycles
 *
 * Graph Structure:
 * - ✅ Connected graphs
 * - ✅ Disconnected graphs (primary use case)
 * - ✅ Empty graphs (returns 0)
 *
 * ## Example Usage
 *
 * ```cpp
 * Graph g(5);
 * g.add_edge(0, 1); g.add_edge(1, 2); // Component 1: {0,1,2}
 * g.add_edge(3, 4);                     // Component 2: {3,4}
 *
 * std::vector<size_t> component(num_vertices(g));
 * size_t num = connected_components(g, container_value_fn(component));
 * // num = 2, component = {0, 0, 0, 1, 1}
 * ```
 *
 * @see kosaraju For strongly connected components in directed graphs
 * @see afforest For faster parallel-friendly alternative
 */
template <adjacency_list G,
          class          ComponentFn>
requires vertex_property_fn_for<ComponentFn, G>
size_t connected_components(G&&           g,         // graph
                            ComponentFn&& component  // out: connected component assignment
) {
  using CT = vertex_fn_value_t<ComponentFn, G>;
  // Initialize all components as unvisited
  for (auto&& [uid, u] : views::vertexlist(g)) {
    component(g, uid) = std::numeric_limits<CT>::max();
  }

  // Stack of vertex descriptors — lightweight (8 bytes), avoids string copies,
  // and lets us call views::incidence(g, descriptor) without find_vertex on pop.
  using vertex_desc = vertex_t<std::remove_reference_t<G>>;
  std::stack<vertex_desc> S;
  CT                      cid = 0; // Current component ID
  for (auto&& [uid, u] : views::vertexlist(g)) {
    if (component(g, uid) < std::numeric_limits<CT>::max()) {
      continue; // Already assigned to a component
    }

    // Handle isolated vertices (no edges)
    if (!num_edges(g, uid)) {
      component(g, uid) = cid++;
      continue;
    }

    // Start DFS for new component
    component(g, uid) = cid;
    S.push(u);
    while (!S.empty()) {
      auto v = S.top();
      S.pop();
      // Visit all unvisited neighbors and add to same component
      for (auto&& [wid, e] : views::incidence(g, v)) {
        if (component(g, wid) == std::numeric_limits<CT>::max()) {
          component(g, wid) = cid; // Same component as parent
          S.push(*find_vertex(g, wid));
        }
      }
    }
    ++cid; // Move to next component
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
 * @tparam Component Subscriptable container for component IDs (vector or unordered_map)
 * @param u First vertex ID
 * @param v Second vertex ID  
 * @param component Component assignment array (modified in-place)
 */
template <typename vertex_id_t, class Component>
static void link(vertex_id_t u, vertex_id_t v, Component& component) {
  vertex_id_t p1 = component[u]; // Parent of u
  vertex_id_t p2 = component[v]; // Parent of v

  // Follow parent pointers with path compression until roots converge
  while (p1 != p2) {
    vertex_id_t high   = std::max(p1, p2); // Higher ID
    vertex_id_t low    = p1 + (p2 - high); // Lower ID (clever: avoids branch)
    vertex_id_t p_high = component[high];  // Parent of higher ID

    // Already linked: high points to low
    if (p_high == low)
      break;

    if (p_high == high) {
      // high is a root (points to itself)
      if (component[high] == high) {
        component[high] = low; // Link high root to low
        break;
      } else {
        // Race condition: another thread changed it; retry with low
        high = low;
      }
    }

    // Path compression: follow parent links and try again
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
 * @tparam Component Subscriptable container for component IDs (vector or unordered_map)
 * @param component Component assignment array (modified in-place)
 */
template <class Component>
static void compress(Component& component) {
  // Two-pass path compression: point each node to its grandparent
  // This flattens the union-find tree structure for faster queries
  // Note: Does not fully compress to root, but significantly reduces depth
  if constexpr (std::ranges::random_access_range<Component>) {
    for (size_t i = 0; i < component.size(); ++i) {
      if (component[i] != component[component[i]]) {
        component[i] = component[component[i]]; // Point to grandparent
      }
    }
  } else {
    // Map-based component: iterate over key-value pairs
    for (auto& [key, val] : component) {
      auto parent = component[val];
      if (val != parent) {
        val = parent; // Point to grandparent
      }
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
 * @tparam Component Subscriptable container for component IDs (vector or unordered_map)
 * @param component Component assignment array
 * @param num_samples Number of random samples to take (default: 1024)
 * @return The most frequently occurring component ID in the sample
 */
template <typename vertex_id_t, class Component>
static vertex_id_t sample_frequent_element(Component& component, size_t num_samples = 1024) {
  // Use random sampling to find the most common component ID
  // This is faster than scanning all vertices for large graphs
  // The largest component is likely to be sampled frequently
  std::unordered_map<vertex_id_t, int> counts(32);
  std::mt19937                         gen;

  if constexpr (std::ranges::random_access_range<Component>) {
    std::uniform_int_distribution<vertex_id_t> distribution(0, component.size() - 1);
    // Take random samples and count occurrences of each component ID
    for (size_t i = 0; i < num_samples; ++i) {
      vertex_id_t sample = distribution(gen);
      counts[component[sample]]++;
    }
  } else {
    // Map-based: collect keys and sample from them
    std::vector<vertex_id_t> keys;
    keys.reserve(component.size());
    for (auto& [k, v] : component) {
      keys.push_back(k);
    }
    std::uniform_int_distribution<size_t> distribution(0, keys.size() - 1);
    for (size_t i = 0; i < num_samples; ++i) {
      auto& key = keys[distribution(gen)];
      counts[component[key]]++;
    }
  }

  // Return the component ID with highest count
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
 * the largest component before processing remaining edges.
 * 
 * @tparam G Graph type (must satisfy index_adjacency_list concept)
 * @tparam Component Random access range for component IDs
 * 
 * @param g The graph to analyze (treated as undirected)
 * @param component Output: component[v] = component ID for vertex v
 * @param neighbor_rounds Number of neighbor sampling rounds (default: 2)
 * 
 * @return void. Results are stored in the component output parameter.
 * 
 * **Mandates:**
 * - G must satisfy adjacency_list
 * - Component must satisfy vertex_property_map_for<Component, G>
 * - Bidirectional conversion between vertex_id_t<G> and vertex_property_map_value_t<Component>
 * 
 * **Preconditions:**
 * - component must contain an entry for each vertex of g
 * - neighbor_rounds >= 0
 * 
 * **Effects:**
 * - Modifies component: Sets component[v] for all vertices v
 * - Does not modify the graph g
 * 
 * **Postconditions:**
 * - component[v] contains the component ID for vertex v
 * - Vertices in the same component have the same component ID
 * - Component IDs form a union-find forest (compressed at end)
 * 
 * **Throws:**
 * - std::bad_alloc if internal allocations fail
 * - Exception guarantee: Basic.
 * 
 * **Complexity:**
 * - Time: O(V + E·α(V)) where α is inverse Ackermann (effectively constant)
 * - Space: O(V) for component array only
 * - Often faster than DFS-based algorithms for large graphs
 * 
 * **Remarks:**
 * - Uses union-find with path compression for near-constant time operations
 * - Neighbor sampling reduces total edge processing for many graphs
 * - Serial implementation; designed for parallelization (see Sutton et al., 2018)
 * - Performance tuning: neighbor_rounds=1 for dense, 2 for balanced, >2 diminishing returns
 * 
 * **Supported Graph Properties:**
 *
 * Directedness:
 * - ✅ Undirected graphs (primary use case)
 * - ✅ Directed graphs (treats as undirected)
 *
 * Edge Properties:
 * - ✅ Weighted edges (weights ignored)
 * - ✅ Self-loops (handled correctly)
 * - ✅ Multi-edges (all edges processed)
 * - ✅ Cycles
 *
 * Graph Structure:
 * - ✅ Connected graphs
 * - ✅ Disconnected graphs
 * - ✅ Empty graphs
 *
 * ## Example Usage
 *
 * ```cpp
 * Graph g({{0,1}, {1,2}, {3,4}, {4,5}});  // Two components
 *
 * std::vector<size_t> component(num_vertices(g));
 * afforest(g, component);
 * compress(component); // Get canonical component IDs
 * ```
 *
 * @see connected_components For simpler DFS-based alternative
 * @see kosaraju For directed graph strongly connected components
 */
template <adjacency_list G, class Component>
requires vertex_property_map_for<Component, G> &&
         std::convertible_to<vertex_property_map_value_t<Component>, vertex_id_t<G>> &&
         std::convertible_to<vertex_id_t<G>, vertex_property_map_value_t<Component>>
void afforest(G&&          g,         // graph
              Component&   component, // out: connected component assignment
              const size_t neighbor_rounds = 2) {
  // Initialize: each vertex is its own component
  if constexpr (std::ranges::random_access_range<Component>) {
    std::iota(component.begin(), component.end(), vertex_property_map_value_t<Component>(0));
  } else {
    for (auto&& [uid, u] : views::vertexlist(g)) {
      component[uid] = static_cast<vertex_property_map_value_t<Component>>(uid);
    }
  }

  using vid_t = vertex_id_t<G>;

  // Phase 1: Neighbor sampling - link vertices through first few neighbors
  // This quickly forms large components without processing all edges
  for (size_t r = 0; r < neighbor_rounds; ++r) {
    for (auto&& [uid, u] : views::vertexlist(g)) {
      if (r < size(edges(g, u))) {
        auto it = edges(g, u).begin();
        std::advance(it, r); // Get r-th neighbor
        link(static_cast<vid_t>(uid), static_cast<vid_t>(target_id(g, *it)), component);
      }
    }
    compress(component); // Flatten union-find tree after each round
  }

  // Phase 2: Identify largest component via sampling
  // Skip processing edges within largest component (optimization)
  vid_t c = sample_frequent_element<vid_t>(component);

  // Phase 3: Process remaining edges for vertices not in largest component
  // Start from neighbor_rounds to avoid re-processing sampled neighbors
  for (auto&& [uid, u] : views::vertexlist(g)) {
    if (component[uid] == c) {
      continue; // Skip vertices in largest component
    }
    if (neighbor_rounds < edges(g, u).size()) {
      auto it = edges(g, u).begin();
      std::advance(it, neighbor_rounds); // Skip already-processed neighbors
      for (; it != edges(g, u).end(); ++it) {
        link(static_cast<vid_t>(uid), static_cast<vid_t>(target_id(g, *it)), component);
      }
    }
  }

  compress(component); // Final compression for query efficiency
}

/**
 * @brief Finds connected components using Afforest with bidirectional edge processing.
 * 
 * This overload processes edges in both directions (forward and reverse) by accepting
 * both the original graph and its transpose. This can improve convergence for directed
 * graphs when treating them as undirected.
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
 * @return void. Results are stored in the component output parameter.
 * 
 * **Mandates:**
 * - All mandates from single-graph afforest, plus:
 * - GT must satisfy adjacency_list
 * 
 * **Preconditions:**
 * - All preconditions from single-graph afforest
 * - num_vertices(g) == num_vertices(g_t)
 * - g_t must be transpose of g (edges reversed) or contain additional edges
 * 
 * **Effects:**
 * - Modifies component: Sets component[v] for all vertices v
 * - Does not modify graphs g or g_t
 * 
 * **Postconditions:**
 * - Same postconditions as single-graph afforest
 * 
 * **Throws:**
 * - std::bad_alloc if internal allocations fail
 * - Exception guarantee: Basic.
 * 
 * **Complexity:**
 * - Time: O(V + (E + E_t)·α(V)) where E_t is edges in transpose
 * - Space: O(V) (transpose not counted)
 *
 * ## Example Usage
 *
 * ```cpp
 * Graph g({{0,1}, {2,3}});
 * Graph g_t = transpose(g);
 *
 * std::vector<size_t> component(num_vertices(g));
 * afforest(g, g_t, component);
 * compress(component);
 * ```
 *
 * @see afforest(G&&, Component&, size_t) For single-graph version
 */
template <adjacency_list G, adjacency_list GT, class Component>
requires vertex_property_map_for<Component, G> &&
         std::convertible_to<vertex_property_map_value_t<Component>, vertex_id_t<G>> &&
         std::convertible_to<vertex_id_t<G>, vertex_property_map_value_t<Component>>
void afforest(G&&          g,         // graph
              GT&&         g_t,       // graph transpose
              Component&   component, // out: connected component assignment
              const size_t neighbor_rounds = 2) {
  // Initialize: each vertex is its own component
  if constexpr (std::ranges::random_access_range<Component>) {
    std::iota(component.begin(), component.end(), vertex_property_map_value_t<Component>(0));
  } else {
    for (auto&& [uid, u] : views::vertexlist(g)) {
      component[uid] = static_cast<vertex_property_map_value_t<Component>>(uid);
    }
  }

  using vid_t = vertex_id_t<G>;

  // Phase 1: Neighbor sampling (same as single-graph version)
  for (size_t r = 0; r < neighbor_rounds; ++r) {
    for (auto&& [uid, u] : views::vertexlist(g)) {
      if (r < size(edges(g, u))) {
        auto it = edges(g, u).begin();
        std::advance(it, r); // Get r-th neighbor
        link(static_cast<vid_t>(uid), static_cast<vid_t>(target_id(g, *it)), component);
      }
    }
    compress(component); // Flatten union-find tree
  }

  // Phase 2: Identify largest component via sampling
  vid_t c = sample_frequent_element<vid_t>(component);

  // Phase 3: Process remaining edges in both directions
  for (auto&& [uid, u] : views::vertexlist(g)) {
    if (component[uid] == c) {
      continue; // Skip largest component
    }
    // Process remaining forward edges (from g)
    if (neighbor_rounds < edges(g, u).size()) {
      auto it = edges(g, u).begin();
      std::advance(it, neighbor_rounds); // Skip sampled neighbors
      for (; it != edges(g, u).end(); ++it) {
        link(static_cast<vid_t>(uid), static_cast<vid_t>(target_id(g, *it)), component);
      }
    }
    // Process all backward edges (from transpose g_t)
    // This ensures bidirectional reachability for undirected graphs
    for (auto it2 = edges(g_t, u).begin(); it2 != edges(g_t, u).end(); ++it2) {
      link(static_cast<vid_t>(uid), static_cast<vid_t>(target_id(g_t, *it2)), component);
    }
  }

  compress(component); // Final compression
}

} // namespace graph

#endif //GRAPH_CC_HPP
