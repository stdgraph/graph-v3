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
#include "graph/algorithm/traversal_common.hpp"
#include "graph/adj_list/vertex_property_map.hpp"

#include <queue>
#include <ranges>
#include <limits>

#ifndef GRAPH_BREADTH_FIRST_SEARCH_HPP
#  define GRAPH_BREADTH_FIRST_SEARCH_HPP

namespace graph {

// Using declarations for new namespace structure
using adj_list::adjacency_list;
using adj_list::vertex_id_t;
using adj_list::find_vertex;

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
 * @tparam G Graph type satisfying adjacency_list concept
 * @tparam Sources Input range of source vertex IDs
 * @tparam Visitor Visitor type with optional callback methods
 * 
 * @param g The graph to traverse (forwarding reference)
 * @param sources Range of starting vertex IDs
 * @param visitor Visitor object to receive traversal events (default: empty_visitor)
 * 
 * @return void. Results delivered via visitor callbacks.
 * 
 * **Mandates:**
 * - G must satisfy adjacency_list (index or mapped vertex containers)
 * - Sources must be input_range with values convertible to vertex_id_t<G>
 * - Visitor callbacks (if present) must accept appropriate parameters
 * 
 * **Preconditions:**
 * - g must not be modified during traversal
 * - All vertex IDs in sources must be valid vertex IDs in g
 * - Visitor methods must not modify graph structure
 * 
 * **Effects:**
 * - Does not modify the graph g
 * - Invokes visitor callbacks in BFS traversal order
 * - Vertices are visited in level-order (distance from sources)
 * 
 * **Postconditions:**
 * - All vertices reachable from any source are visited exactly once
 * - Visitor callbacks invoked in BFS order
 * - Graph g is unchanged
 * 
 * **Throws:**
 * - std::bad_alloc if visited array or queue cannot allocate memory
 * - May propagate exceptions from visitor callbacks
 * - May propagate exceptions from container operations
 * - Exception guarantee: Basic. If an exception is thrown, graph g remains unchanged;
 *   visitor state depends on implementation; partial traversal may have occurred.
 * 
 * **Complexity:**
 * - Time: O(V + E) — each vertex visited once, each edge examined once
 * - Space: O(V) for visited array and queue
 * 
 * **Remarks:**
 * - Uses std::queue for FIFO vertex processing
 * - Visited tracking: vector<bool> for index graphs, unordered_map for mapped graphs
 * - No distance tracking (use BFS views for distances)
 * - Multi-source as primary interface: single-source is a special case with no overhead
 * 
 * **Visitor Callbacks:**
 * - on_initialize_vertex(vertex_id): Called when vertex is added to initial sources
 * - on_discover_vertex(vertex_id): Called when vertex is first encountered
 * - on_examine_vertex(vertex_id): Called when vertex is dequeued for processing
 * - on_examine_edge(edge): Called for each outgoing edge examined
 * - on_finish_vertex(vertex_id): Called after all edges examined
 * All callbacks are optional via SFINAE (has_on_* concept checks).
 * 
 * **Supported Graph Properties:**
 *
 * Directedness:
 * - ✅ Directed graphs
 * - ✅ Undirected graphs
 *
 * Edge Properties:
 * - ✅ Unweighted edges (BFS finds shortest paths)
 * - ✅ Weighted edges (weights ignored, treats as unweighted)
 * - ✅ Multi-edges (all edges examined, vertices visited once)
 * - ✅ Self-loops (examined but don't affect traversal)
 * - ✅ Cycles (visited tracking prevents infinite loops)
 *
 * Graph Structure:
 * - ✅ Connected graphs
 * - ✅ Disconnected graphs (visits reachable component)
 * - ✅ Empty graphs (returns immediately)
 *
 * ## Example Usage
 *
 * ```cpp
 * #include <graph/graph.hpp>
 * #include <graph/algorithm/breadth_first_search.hpp>
 *
 * using namespace graph;
 *
 * // Basic traversal:
 * Graph g({{0,1}, {1,2}, {2,3}});
 * std::vector<uint32_t> sources = {0};
 * breadth_first_search(g, sources); // Traverses 0->1->2->3
 *
 * // With custom visitor:
 * struct PrintVisitor {
 *     void on_discover_vertex(auto& g, auto v) {
 *         std::cout << "Discovered: " << v << "\n";
 *     }
 * };
 * PrintVisitor visitor;
 * breadth_first_search(g, sources, visitor);
 *
 * // Multi-source BFS:
 * std::vector<uint32_t> multi_sources = {0, 5, 10};
 * breadth_first_search(g, multi_sources); // Explores from all simultaneously
 * ```
 *
 * @see breadth_first_search(G&&, vertex_id_t<G>, Visitor&&) Single-source convenience wrapper
 * @see views::vertices_bfs BFS view for range-based traversal
 * @see connected_components For component detection using BFS
 */
template <adjacency_list G, std::ranges::input_range Sources, class Visitor = empty_visitor>
requires std::convertible_to<std::ranges::range_value_t<Sources>, vertex_id_t<G>>
void breadth_first_search(G&&            g, // graph
                          const Sources& sources,
                          Visitor&&      visitor = empty_visitor()) {
  using id_type = vertex_id_t<G>;

  // Initialize BFS data structures
  std::queue<id_type> Q;                                    // FIFO queue for level-order traversal
  auto                visited = make_vertex_property_map<G, bool>(g, false); // Track visited vertices to prevent cycles

  // Initialize all source vertices
  for (auto uid : sources) {
    // Notify visitor of initialization
    if constexpr (has_on_initialize_vertex<G, Visitor>) {
      visitor.on_initialize_vertex(g, *find_vertex(g, uid));
    } else if constexpr (has_on_initialize_vertex_id<G, Visitor>) {
      visitor.on_initialize_vertex(g, uid);
    }
    if constexpr (has_on_discover_vertex<G, Visitor>) {
      visitor.on_discover_vertex(g, *find_vertex(g, uid));
    } else if constexpr (has_on_discover_vertex_id<G, Visitor>) {
      visitor.on_discover_vertex(g, uid);
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
      visitor.on_examine_vertex(g, *find_vertex(g, uid));
    } else if constexpr (has_on_examine_vertex_id<G, Visitor>) {
      visitor.on_examine_vertex(g, uid);
    }

    // Explore all edges from current vertex
    for (auto&& [vid, uv] : views::incidence(g, *find_vertex(g, uid))) {
      // Notify visitor about this edge
      if constexpr (has_on_examine_edge<G, Visitor>) {
        visitor.on_examine_edge(g, uv);
      }

      // If target vertex not yet visited, discover it
      if (!visited[vid]) {
        visited[vid] = true; // Mark as visited before queueing
        if constexpr (has_on_discover_vertex<G, Visitor>) {
          visitor.on_discover_vertex(g, *find_vertex(g, vid));
        } else if constexpr (has_on_discover_vertex_id<G, Visitor>) {
          visitor.on_discover_vertex(g, vid);
        }
        Q.push(vid); // Add to queue for later examination
      }
    }

    // Notify visitor that we've finished examining all edges from this vertex
    if constexpr (has_on_finish_vertex<G, Visitor>) {
      visitor.on_finish_vertex(g, *find_vertex(g, uid));
    } else if constexpr (has_on_finish_vertex_id<G, Visitor>) {
      visitor.on_finish_vertex(g, uid);
    }
  }
}

/**
 * @brief Single-source breadth-first search with visitor pattern.
 * 
 * Convenience wrapper for BFS starting from a single source vertex.
 * Delegates to the multi-source version by wrapping the source in a std::array.
 * 
 * @tparam G Graph type satisfying adjacency_list concept
 * @tparam Visitor Visitor type with optional callback methods
 * 
 * @param g The graph to traverse (forwarding reference)
 * @param source Starting vertex ID
 * @param visitor Visitor object to receive traversal events (default: empty_visitor)
 * 
 * @return void. Results delivered via visitor callbacks.
 * 
 * **Preconditions:**
 * - source must be a valid vertex ID in g
 * 
 * **Postconditions:**
 * - All vertices reachable from source are visited exactly once
 * 
 * **Throws:**
 * - Exception guarantee: Basic (same as multi-source version).
 * 
 * **Complexity:**
 * - Time: O(V + E)
 * - Space: O(V)
 * - Identical to multi-source version; delegation overhead is negligible.
 *
 * ## Example Usage
 *
 * ```cpp
 * Graph g({{0,1}, {1,2}, {2,3}});
 * breadth_first_search(g, 0); // Start from vertex 0
 * ```
 *
 * @see breadth_first_search(G&&, Sources&&, Visitor&&) Multi-source version
 * @see views::vertices_bfs BFS view for range-based traversal
 */
template <adjacency_list G, class Visitor = empty_visitor>
void breadth_first_search(G&&                      g,      // graph
                          const vertex_id_t<G>&    source, // starting vertex_id
                          Visitor&&                visitor = empty_visitor()) {
  // Wrap single source in array and delegate to multi-source version
  std::array<vertex_id_t<G>, 1> sources{source};
  breadth_first_search(std::forward<G>(g), sources, std::forward<Visitor>(visitor));
}

} // namespace graph

#endif // GRAPH_BREADTH_FIRST_SEARCH_HPP
