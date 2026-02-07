/**
 * @file depth_first_search.hpp
 * 
 * @brief Depth-first search traversal algorithm for graphs.
 * 
 * Depth-first search (DFS) is a fundamental graph traversal algorithm that explores
 * vertices by going as deep as possible along each branch before backtracking. It
 * classifies edges into tree, back, forward, and cross edges, making it the basis
 * for cycle detection, topological sorting, strongly connected components, and many
 * other graph algorithms.
 * 
 * This implementation provides a single-source variant with customizable visitor
 * callbacks for tracking traversal events and edge classification.
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

#include <vector>
#include <stack>
#include <ranges>
#include <limits>

#ifndef GRAPH_DFS_ALGORITHM_HPP
#  define GRAPH_DFS_ALGORITHM_HPP

namespace graph {

// Using declarations for new namespace structure
using adj_list::index_adjacency_list;
using adj_list::vertex_id_t;
using adj_list::find_vertex;

/**
 * @brief Single-source depth-first search with visitor pattern.
 * 
 * Performs depth-first traversal starting from a single source vertex, calling 
 * visitor methods at key points during traversal. This implementation precisely 
 * simulates recursive DFS using an explicit stack with stored edge iterators, 
 * ensuring correct edge classification (tree/back/forward/cross) and proper 
 * vertex finish ordering.
 * 
 * DFS explores vertices by following edges as deep as possible before backtracking.
 * Each vertex transitions through three color states: White (undiscovered) -> 
 * Gray (discovered, in progress) -> Black (finished). This three-color scheme 
 * enables precise classification of every edge encountered during traversal.
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
 * - Each vertex is discovered and finished exactly once: O(V)
 * - Each edge is examined exactly once: O(E)
 * - Edge iterator advancement is O(1) per edge (stored iterators)
 * - Total: O(V + E) for all cases
 * 
 * **Space Complexity:**
 * - Color array: O(V)
 * - Stack: O(V) worst case (linear chain graph)
 * - Each stack frame stores a vertex ID and edge iterators
 * - Total auxiliary space: O(V)
 * 
 * @par Supported Graph Properties
 * 
 * **Directedness:**
 * - ✅ Directed graphs (full edge classification: tree/back/forward/cross)
 * - ✅ Undirected graphs (tree/back edges only)
 * 
 * **Edge Properties:**
 * - ✅ Unweighted edges
 * - ✅ Weighted edges (weights ignored during traversal)
 * - ✅ Multi-edges: All edges examined, classified independently
 * - ✅ Self-loops: Classified as back edges (vertex is Gray when revisited)
 * 
 * **Graph Structure:**
 * - ✅ Connected graphs (visits all vertices)
 * - ✅ Disconnected graphs (visits reachable component from source)
 * - ✅ Acyclic graphs (DAGs) - no back edges, yields topological order
 * - ✅ Cyclic graphs - back edges indicate cycles
 * - ✅ Trees - all edges classified as tree edges
 * 
 * **Container Requirements:**
 * - Requires: `index_adjacency_list<G>` (vertex IDs are contiguous indices)
 * - Works with: All `dynamic_graph` container combinations
 * - Works with: Vector-based containers (vov, vol, vofl, etc.)
 * - Limitations: Requires contiguous vertex IDs for color array tracking
 * 
 * @tparam G Graph type satisfying index_adjacency_list concept
 * @tparam Visitor Visitor type with optional callback methods
 * 
 * @param g The graph to traverse (forwarding reference)
 * @param source Starting vertex ID
 * @param visitor Visitor object to receive traversal events (default: empty_visitor)
 * 
 * @pre `source` must be valid: `source < num_vertices(g)`
 * @pre `g` must not be modified during traversal
 * @pre `Visitor` methods must not modify graph structure
 * 
 * @post All vertices reachable from `source` are visited exactly once
 * @post `visitor` callbacks invoked in DFS order
 * @post `on_finish_vertex` called in reverse topological order for DAGs
 * @post Graph `g` is unchanged
 * 
 * @par Exception Safety
 * 
 * **Guarantee:** Basic exception safety
 * 
 * **Throws:**
 * - May throw `std::bad_alloc` if color array or stack cannot allocate memory
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
 * - `on_initialize_vertex(g, vertex)`: Called for the source vertex before traversal begins
 * - `on_start_vertex(g, vertex)`: Called when traversal begins from the source
 * - `on_discover_vertex(g, vertex)`: Called when a vertex is first discovered (colored Gray)
 * - `on_examine_edge(g, edge)`: Called for each outgoing edge examined
 * - `on_tree_edge(g, edge)`: Called when edge leads to an undiscovered (White) vertex
 * - `on_back_edge(g, edge)`: Called when edge leads to an ancestor (Gray) vertex — indicates cycle
 * - `on_forward_or_cross_edge(g, edge)`: Called when edge leads to a finished (Black) vertex
 * - `on_finish_edge(g, edge)`: Called after an edge has been fully classified
 * - `on_finish_vertex(g, vertex)`: Called when all edges from a vertex are processed (colored Black)
 * 
 * All callbacks are optional via SFINAE (`has_on_*` concept checks).
 * 
 * @par Edge Classification
 * 
 * Edge (u, v) is classified by the color of vertex v when the edge is examined:
 * - **Tree edge**: v is White — part of the DFS tree
 * - **Back edge**: v is Gray — v is an ancestor of u (cycle indicator)
 * - **Forward/Cross edge**: v is Black — v was fully processed before this examination
 * 
 * @par Example Usage
 * 
 * **Basic traversal:**
 * @code
 * using Graph = container::dynamic_graph<...>;
 * Graph g({{0,1}, {1,2}, {2,3}});
 * depth_first_search(g, 0); // DFS from vertex 0
 * @endcode
 * 
 * **With visitor for cycle detection:**
 * @code
 * struct CycleDetector {
 *     bool has_cycle = false;
 *     void on_back_edge(auto& g, auto& edge) {
 *         has_cycle = true;
 *     }
 * };
 * 
 * CycleDetector detector;
 * depth_first_search(g, 0, detector);
 * if (detector.has_cycle) { ... // graph contains a cycle }
 * @endcode
 * 
 * **Topological sort via finish ordering:**
 * @code
 * struct TopoVisitor {
 *     std::vector<uint32_t> topo_order;
 *     void on_finish_vertex(auto& g, auto v) {
 *         topo_order.push_back(v);
 *     }
 * };
 * 
 * TopoVisitor topo;
 * depth_first_search(g, 0, topo);
 * std::ranges::reverse(topo.topo_order); // Reverse finish order = topological order
 * @endcode
 * 
 * @par Implementation Notes
 * 
 * **Algorithm Overview:**
 * The algorithm uses an explicit stack to simulate recursive DFS. Each stack frame
 * stores a vertex ID and iterators into its incidence (edge) range, precisely
 * mirroring the state of a recursive DFS call frame. When a frame's edge iterator
 * reaches its sentinel, the vertex is finished and the frame is popped, just as a
 * recursive call would return.
 * 
 * **Data Structures:**
 * - Stack: `std::stack<StackFrame>` where each frame holds `{vertex_id, it, end}`
 * - Color: `std::vector<Color>` with three states (White/Gray/Black) using `uint8_t`
 * - Edge iterators: Stored in stack frames for O(1) resume after backtracking
 * 
 * **Design Decisions:**
 * 1. **Why iterative with explicit stack instead of recursive?**
 *    - Avoids stack overflow on deep graphs (limited system call stack)
 *    - Same asymptotic complexity as recursive version
 *    - Precise control over traversal state for edge classification
 * 
 * 2. **Why store edge iterators in stack frames?**
 *    - Enables O(1) edge advancement when resuming a vertex after backtracking
 *    - Without stored iterators, resuming would require O(degree) re-scanning
 *    - Maintains overall O(V + E) time complexity
 * 
 * 3. **Why three-color scheme instead of two-color (visited/unvisited)?**
 *    - Distinguishes ancestors (Gray) from completed vertices (Black)
 *    - Enables precise edge classification (back vs forward/cross)
 *    - Required for cycle detection (back edge = edge to Gray vertex)
 * 
 * 4. **Why single-source only (no multi-source variant)?**
 *    - DFS from multiple sources can be achieved by calling repeatedly
 *    - Multi-source DFS has less well-defined semantics than multi-source BFS
 *    - Keeps the interface simple for the common case
 * 
 * **Optimization Opportunities:**
 * - For parallel DFS: Requires careful synchronization of color states
 * - For early termination: Add return value or exception-based mechanism to visitor
 * - For iterative deepening: Combine with depth limit for IDDFS
 * 
 * @par References
 * 
 * - Tarjan, R. E. (1972). "Depth-first search and linear graph algorithms". 
 *   *SIAM Journal on Computing*, 1(2), 146-160.
 * - Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2009). 
 *   *Introduction to Algorithms* (3rd ed.). MIT Press. Section 22.3.
 * 
 * @see views::vertices_dfs DFS view for range-based traversal
 * @see breadth_first_search BFS algorithm for shortest-path traversal
 */

template <index_adjacency_list G, class Visitor = empty_visitor>
void depth_first_search(G&&                   g,      // graph
                        const vertex_id_t<G>& source, // starting vertex_id
                        Visitor&&             visitor = empty_visitor()) {
  using id_type = vertex_id_t<G>;

  // Vertex color states for DFS
  enum class Color : uint8_t {
    White, // Undiscovered
    Gray,  // Discovered but not finished
    Black  // Finished
  };

  std::vector<Color> color(std::ranges::size(vertices(g)), Color::White);

  // Initialize source vertex
  if constexpr (has_on_initialize_vertex<G, Visitor>) {
    visitor.on_initialize_vertex(g, *find_vertex(g, source));
  }

  // Notify visitor that we're starting from this source
  if constexpr (has_on_start_vertex<G, Visitor>) {
    auto src_vertex = *find_vertex(g, source);
    visitor.on_start_vertex(g, src_vertex);
  }

  // Each stack frame stores a vertex and iterators into its incidence range,
  // simulating the call stack of recursive DFS for correct edge classification.

  using inc_range_t    = decltype(views::incidence(g, source));
  using inc_iterator_t = std::ranges::iterator_t<inc_range_t>;
  using inc_sentinel_t = std::ranges::sentinel_t<inc_range_t>;

  struct StackFrame {
    id_type        vertex_id;
    inc_iterator_t it;
    inc_sentinel_t end;
  };

  // Discover source and push its stack frame
  color[source] = Color::Gray;
  if constexpr (has_on_discover_vertex<G, Visitor>) {
    visitor.on_discover_vertex(g, *find_vertex(g, source));
  }

  std::stack<StackFrame> S;
  {
    auto inc = views::incidence(g, source);
    S.push({source, std::ranges::begin(inc), std::ranges::end(inc)});
  }

  while (!S.empty()) {
    auto& frame = S.top();
    
    if (frame.it == frame.end) {
      // All edges exhausted: mark vertex finished (Black) and pop
      color[frame.vertex_id] = Color::Black;
      if constexpr (has_on_finish_vertex<G, Visitor>) {
        visitor.on_finish_vertex(g, *find_vertex(g, frame.vertex_id));
      }
      S.pop();
      continue;
    }
    
    // Process next edge from this vertex
    auto&& [uv] = *frame.it;  // structured binding extracts the edge
    id_type vid = target_id(g, uv);
    ++frame.it;  // advance iterator before potential push (simulates recursion past this edge)
    
    if constexpr (has_on_examine_edge<G, Visitor>) {
      visitor.on_examine_edge(g, uv);
    }
    
    if (color[vid] == Color::White) {
      // Tree edge: target is undiscovered
      if constexpr (has_on_tree_edge<G, Visitor>) {
        visitor.on_tree_edge(g, uv);
      }
      // Finish this edge before "recursing" into the target vertex
      if constexpr (has_on_finish_edge<G, Visitor>) {
        visitor.on_finish_edge(g, uv);
      }
      // Discover target and push its frame (equivalent to recursive call)
      color[vid] = Color::Gray;
      if constexpr (has_on_discover_vertex<G, Visitor>) {
        visitor.on_discover_vertex(g, *find_vertex(g, vid));
      }
      auto inc = views::incidence(g, vid);
      S.push({vid, std::ranges::begin(inc), std::ranges::end(inc)});
    } else if (color[vid] == Color::Gray) {
      // Back edge: target is an ancestor still being processed (cycle)
      if constexpr (has_on_back_edge<G, Visitor>) {
        visitor.on_back_edge(g, uv);
      }
      if constexpr (has_on_finish_edge<G, Visitor>) {
        visitor.on_finish_edge(g, uv);
      }
    } else {
      // Forward or cross edge: target is already finished (Black)
      if constexpr (has_on_forward_or_cross_edge<G, Visitor>) {
        visitor.on_forward_or_cross_edge(g, uv);
      }
      if constexpr (has_on_finish_edge<G, Visitor>) {
        visitor.on_finish_edge(g, uv);
      }
    }
  }
} 


} // namespace graph

#endif // GRAPH_DFS_ALGORITHM_HPP
