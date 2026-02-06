/**
 * @file breadth_first_search.hpp
 * 
 * @brief Breadth-first search traversal algorithms for graphs.
 * 
 * Breadth-first search (BFS) is a fundamental graph traversal algorithm that explores
 * vertices in order of their distance from the source vertex(es). It visits all vertices
 * at distance k before visiting any vertex at distance k+1, making it ideal for finding
 * unweighted shortest paths, level-order traversal, and testing graph connectivity.
 * 
 * This implementation provides both single-source and multi-source variants with
 * customizable visitor callbacks for tracking traversal events.
 * 
 * @copyright Copyright (c) 2024
 * 
 * SPDX-License-Identifier: BSL-1.0
 *
 * @authors Andrew Lumsdaine, Phil Ratzloff
 */

#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include "graph/algorithm/common_shortest_paths.hpp"

#include <vector>
#include <queue>
#include <ranges>
#include <limits>

#ifndef GRAPH_BREADTH_FIRST_SEARCH_HPP
#  define GRAPH_BREADTH_FIRST_SEARCH_HPP

namespace graph {

// Using declarations for new namespace structure
using adj_list::index_adjacency_list;
using adj_list::vertex_id_t;

/**
 * @brief Multi-source breadth-first search with visitor pattern.
 * 
 * Performs breadth-first traversal starting from multiple source vertices simultaneously,
 * calling visitor methods at key points during traversal. This is the fundamental BFS
 * implementation that supports custom event callbacks for tracking algorithm progress.
 * 
 * BFS explores vertices in waves: all vertices at distance k from any source are visited
 * before any vertex at distance k+1. When multiple sources are provided, vertices reachable
 * from any source are discovered in the first wave, making this useful for multi-source
 * shortest path problems and parallel/concurrent reachability analysis.
 * 
 * @par Complexity Analysis
 * 
 * | Case | Time | Space |
 * |------|------|-------|
 * | All cases | O(V + E) | O(V) |
 * 
 * Where V = number of vertices, E = number of edges
 * 
 * **Time Complexity:**
 * - Each vertex is visited exactly once: O(V)
 * - Each edge is examined exactly once: O(E)
 * - Queue operations (push/pop) are O(1) each
 * - Total: O(V + E) for all cases
 * 
 * **Space Complexity:**
 * - Visited array: O(V)
 * - Queue: O(V) worst case (all vertices at same level)
 * - Total auxiliary space: O(V)
 * 
 * @par Supported Graph Properties
 * 
 * **Directedness:**
 * - ✅ Directed graphs
 * - ✅ Undirected graphs
 * - ✅ Mixed (with edge direction semantics)
 * 
 * **Edge Properties:**
 * - ✅ Unweighted edges (BFS finds shortest paths)
 * - ✅ Weighted edges (weights ignored, treats as unweighted)
 * - ✅ Uniform weights (optimal shortest path algorithm)
 * - ✅ Multi-edges: All edges examined, vertices visited once
 * - ✅ Self-loops: Examined but don't affect traversal
 * 
 * **Graph Structure:**
 * - ✅ Connected graphs
 * - ✅ Disconnected graphs (visits reachable component)
 * - ✅ Acyclic graphs (DAG)
 * - ✅ Cyclic graphs (visited tracking prevents infinite loops)
 * - ✅ Trees (optimal level-order traversal)
 * 
 * **Container Requirements:**
 * - Requires: `index_adjacency_list<G>` (vertex IDs are indices)
 * - Requires: `input_range<Sources>` with convertible elements
 * - Works with: All `dynamic_graph` container combinations
 * - Works with: Vector-based containers (vov, vol, vofl, etc.)
 * - Limitations: Requires contiguous vertex IDs for visited tracking
 * 
 * @tparam G Graph type satisfying index_adjacency_list concept
 * @tparam Sources Input range of source vertex IDs
 * @tparam Visitor Visitor type with optional callback methods
 * 
 * @param g The graph to traverse (forwarding reference)
 * @param sources Range of starting vertex IDs
 * @param visitor Visitor object to receive traversal events (default: empty_visitor)
 * 
 * @pre `g` must not be modified during traversal
 * @pre All vertex IDs in `sources` must be valid: `source < num_vertices(g)`
 * @pre `Visitor` methods must not modify graph structure
 * 
 * @post All vertices reachable from any source are visited exactly once
 * @post `visitor` callbacks invoked in BFS order
 * @post Graph `g` is unchanged
 * 
 * @par Exception Safety
 * 
 * **Guarantee:** Basic exception safety
 * 
 * **Throws:**
 * - May throw `std::bad_alloc` if visited array or queue cannot allocate memory
 * - May propagate exceptions from visitor callbacks
 * - May propagate exceptions from container operations
 * 
 * **State after exception:**
 * - Graph `g` remains unchanged
 * - Visitor state depends on implementation
 * - Partial traversal may have occurred
 * 
 * @par Visitor Callbacks
 * 
 * The visitor can optionally implement any of these methods:
 * 
 * - `on_initialize_vertex(vertex_id)`: Called when vertex is added to initial sources
 * - `on_discover_vertex(vertex_id)`: Called when vertex is first encountered
 * - `on_examine_vertex(vertex_id)`: Called when vertex is dequeued for processing
 * - `on_examine_edge(edge)`: Called for each outgoing edge examined
 * - `on_finish_vertex(vertex_id)`: Called after all edges examined
 * 
 * All callbacks are optional via SFINAE (`has_on_*` concept checks).
 * 
 * @par Example Usage
 * 
 * **Basic traversal:**
 * @code
 * using Graph = container::dynamic_graph<...>;
 * Graph g({{0,1}, {1,2}, {2,3}});
 * 
 * std::vector<uint32_t> sources = {0};
 * breadth_first_search(g, sources); // Traverses 0->1->2->3
 * @endcode
 * 
 * **With custom visitor:**
 * @code
 * struct PrintVisitor {
 *     void on_discover_vertex(auto v) {
 *         std::cout << "Discovered: " << v << "\n";
 *     }
 * };
 * 
 * PrintVisitor visitor;
 * std::vector<uint32_t> sources = {0};
 * breadth_first_search(g, sources, visitor);
 * @endcode
 * 
 * **Multi-source BFS:**
 * @code
 * std::vector<uint32_t> sources = {0, 5, 10}; // Start from 3 vertices
 * breadth_first_search(g, sources); // Explores from all simultaneously
 * @endcode
 * 
 * @par Implementation Notes
 * 
 * **Data Structures:**
 * - Queue: `std::queue` for FIFO vertex processing
 * - Visited: `std::vector<bool>` for O(1) lookup (space-efficient)
 * - No distance tracking (use BFS views for distances)
 * 
 * **Design Decisions:**
 * 1. **Why visitor pattern?**
 *    - Flexibility: Clients customize behavior without modifying algorithm
 *    - Performance: Callbacks inlined via template, zero overhead
 *    - Extensibility: Easy to add tracking, statistics, early termination
 * 
 * 2. **Why multi-source as primary interface?**
 *    - Generality: Single-source is special case
 *    - Efficiency: No overhead vs separate single-source implementation
 *    - Use cases: Multi-source shortest paths, reachability from sets
 * 
 * 3. **Why std::vector<bool> for visited tracking?**
 *    - Space efficiency: 1 bit per vertex (8x smaller than std::vector<char>)
 *    - Performance: Modern std::vector<bool> optimizations competitive with bitset
 *    - Flexibility: Size determined at runtime
 * 
 * **Optimization Opportunities:**
 * - For small graphs: Use std::bitset if vertex count known at compile time
 * - For sparse visitation: Use std::unordered_set<vertex_id_t> (map-based containers)
 * - For parallel BFS: Use concurrent queue and atomic visited flags
 * 
 * @see breadth_first_search(G&&, vertex_id_t<G>, Visitor&&) Single-source convenience wrapper
 * @see views::vertices_bfs BFS view for range-based traversal
 * @see connected_components For component detection using BFS
 * 
 * @par References
 * 
 * - Moore, E. F. (1959). "The shortest path through a maze". *Proceedings of the International Symposium on the Theory of Switching*. Harvard University Press.
 * - Cormen et al. (2009). *Introduction to Algorithms* (3rd ed.). MIT Press. Section 22.2.
 */
template <index_adjacency_list G, std::ranges::input_range Sources, class Visitor = empty_visitor>
requires std::convertible_to<std::ranges::range_value_t<Sources>, vertex_id_t<G>>
void breadth_first_search(G&&            g, // graph
                          const Sources& sources,
                          Visitor&&      visitor = empty_visitor()) {
  using id_type = vertex_id_t<G>;

  // Initialize BFS data structures
  std::queue<id_type> Q;                                              // FIFO queue for level-order traversal
  std::vector<bool>   visited(std::ranges::size(vertices(g)), false); // Track visited vertices to prevent cycles

  // Initialize all source vertices
  for (auto uid : sources) {
    // Notify visitor of initialization
    if constexpr (has_on_initialize_vertex<G, Visitor>) {
      visitor.on_initialize_vertex(uid);
    }
    if constexpr (has_on_discover_vertex<G, Visitor>) {
      visitor.on_discover_vertex(uid);
    }
    // Mark source as visited and add to queue
    visited[uid] = true;
    Q.push(uid);
  }

  // Main BFS loop: process vertices in level-order
  while (!Q.empty()) {
    // Dequeue next vertex to examine
    id_type uid = Q.front();
    Q.pop();

    // Notify visitor that we're examining this vertex
    if constexpr (has_on_examine_vertex<G, Visitor>) {
      visitor.on_examine_vertex(uid);
    }

    // Explore all edges from current vertex
    for (auto&& [uv] : views::incidence(g, uid)) {
      id_type vid = target_id(g, uv); // Get target vertex ID

      // Notify visitor about this edge
      if constexpr (has_on_examine_edge<G, Visitor>) {
        visitor.on_examine_edge(uv);
      }

      // If target vertex not yet visited, discover it
      if (!visited[vid]) {
        visited[vid] = true; // Mark as visited before queueing
        if constexpr (has_on_discover_vertex<G, Visitor>) {
          visitor.on_discover_vertex(vid);
        }
        Q.push(vid); // Add to queue for later examination
      }
    }

    // Notify visitor that we've finished examining all edges from this vertex
    if constexpr (has_on_finish_vertex<G, Visitor>) {
      visitor.on_finish_vertex(uid);
    }
  }
}

/**
 * @brief Single-source breadth-first search with visitor pattern.
 * 
 * Convenience wrapper for BFS starting from a single source vertex.
 * This function delegates to the multi-source version by wrapping the
 * source in a std::array, providing the same visitor pattern capabilities
 * with simpler API for the common single-source case.
 * 
 * @par Complexity Analysis
 * 
 * | Case | Time | Space |
 * |------|------|-------|
 * | All cases | O(V + E) | O(V) |
 * 
 * Identical to multi-source version since delegation overhead is negligible.
 * 
 * @tparam G Graph type satisfying index_adjacency_list concept
 * @tparam Visitor Visitor type with optional callback methods
 * 
 * @param g The graph to traverse (forwarding reference)
 * @param source Starting vertex ID
 * @param visitor Visitor object to receive traversal events (default: empty_visitor)
 * 
 * @pre `source` must be valid: `source < num_vertices(g)`
 * @post All vertices reachable from `source` are visited exactly once
 * 
 * @par Exception Safety
 * Basic exception safety (same as multi-source version)
 * 
 * @par Example Usage
 * 
 * **Simple traversal:**
 * @code
 * using Graph = container::dynamic_graph<...>;
 * Graph g({{0,1}, {1,2}, {2,3}});
 * breadth_first_search(g, 0); // Start from vertex 0
 * @endcode
 * 
 * **With visitor:**
 * @code
 * struct DepthTracker {
 *     std::unordered_map<uint32_t, int> depths;
 *     int current_depth = 0;
 *     
 *     void on_discover_vertex(auto v) {
 *         depths[v] = current_depth;
 *     }
 * };
 * 
 * DepthTracker tracker;
 * breadth_first_search(g, 0, tracker);
 * // tracker.depths now contains BFS depths from vertex 0
 * @endcode
 * 
 * @see breadth_first_search(G&&, Sources&&, Visitor&&) Multi-source version (implementation)
 * @see views::vertices_bfs BFS view for range-based traversal
 */
template <index_adjacency_list G, class Visitor = empty_visitor>
void breadth_first_search(G&&            g,      // graph
                          vertex_id_t<G> source, // starting vertex_id
                          Visitor&&      visitor = empty_visitor()) {
  // Wrap single source in array and delegate to multi-source version
  std::array<vertex_id_t<G>, 1> sources{source};
  breadth_first_search(std::forward<G>(g), sources, std::forward<Visitor>(visitor));
}

} // namespace graph

#endif // GRAPH_BREADTH_FIRST_SEARCH_HPP
