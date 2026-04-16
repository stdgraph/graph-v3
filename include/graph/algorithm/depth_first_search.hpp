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
#include "graph/adj_list/vertex_property_map.hpp"

#include <stack>
#include <ranges>
#include <limits>

#ifndef GRAPH_DFS_ALGORITHM_HPP
#  define GRAPH_DFS_ALGORITHM_HPP

namespace graph {

// Using declarations for new namespace structure
using adj_list::adjacency_list;
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
 * @tparam G       Graph type satisfying adjacency_list concept
 * @tparam Visitor Visitor type with optional callback methods
 * @tparam Alloc   Allocator type for internal stack storage. Defaults to std::allocator<std::byte>.
 * 
 * @param g       The graph to traverse (forwarding reference)
 * @param source  Starting vertex ID
 * @param visitor Visitor object to receive traversal events (default: empty_visitor)
 * @param alloc   Allocator instance used for the internal DFS stack (default: Alloc())
 * 
 * @return void. Results delivered via visitor callbacks.
 * 
 * **Mandates:**
 * - G must satisfy adjacency_list (index or mapped vertex containers)
 * - Visitor callbacks (if present) must accept the appropriate graph and vertex/edge parameters
 * 
 * **Preconditions:**
 * - source must be a valid vertex ID in g
 * - g must not be modified during traversal
 * - Visitor methods must not modify graph structure
 * 
 * **Effects:**
 * - Does not modify the graph g
 * - Invokes visitor callbacks in DFS traversal order
 * - Classifies every edge as tree, back, or forward/cross
 * 
 * **Postconditions:**
 * - All vertices reachable from source are visited exactly once
 * - on_finish_vertex called in reverse topological order for DAGs
 * - Graph g is unchanged
 * 
 * **Throws:**
 * - std::bad_alloc if color array or stack cannot allocate memory
 * - May propagate exceptions from visitor callbacks or container operations
 * - Exception guarantee: Basic. If an exception is thrown, graph g remains unchanged;
 *   visitor state depends on implementation; partial traversal may have occurred.
 * 
 * **Complexity:**
 * - Time: O(V + E) — each vertex discovered/finished once, each edge examined once
 * - Space: O(V) for color array and stack (linear chain worst case)
 * 
 * **Remarks:**
 * - Uses iterative DFS with explicit stack to avoid recursion-depth limits
 * - Edge iterators stored in stack frames for O(1) resume after backtracking
 * - Three-color scheme (White/Gray/Black) enables precise edge classification:
 *   Tree edge (v is White), Back edge (v is Gray, indicates cycle),
 *   Forward/Cross edge (v is Black)
 * 
 * **Visitor Callbacks:**
 * - on_initialize_vertex(g, vertex): Called for the source vertex before traversal
 * - on_start_vertex(g, vertex): Called when traversal begins from the source
 * - on_discover_vertex(g, vertex): Called when a vertex is first discovered (colored Gray)
 * - on_examine_edge(g, edge): Called for each outgoing edge examined
 * - on_tree_edge(g, edge): Called when edge leads to undiscovered (White) vertex
 * - on_back_edge(g, edge): Called when edge leads to ancestor (Gray) vertex — indicates cycle
 * - on_forward_or_cross_edge(g, edge): Called when edge leads to finished (Black) vertex
 * - on_finish_edge(g, edge): Called after an edge is fully classified
 * - on_finish_vertex(g, vertex): Called when all edges processed (colored Black)
 * All callbacks are optional via SFINAE (has_on_* concept checks).
 * 
 * **Supported Graph Properties:**
 *
 * Directedness:
 * - ✅ Directed graphs (full edge classification: tree/back/forward/cross)
 * - ✅ Undirected graphs (tree/back edges only)
 *
 * Edge Properties:
 * - ✅ Unweighted edges
 * - ✅ Weighted edges (weights ignored during traversal)
 * - ✅ Multi-edges (all edges examined, classified independently)
 * - ✅ Self-loops (classified as back edges)
 * - ✅ Cycles (back edges indicate cycles)
 *
 * Graph Structure:
 * - ✅ Connected graphs (visits all vertices)
 * - ✅ Disconnected graphs (visits reachable component from source)
 * - ✅ Empty graphs (returns immediately)
 *
 * ## Example Usage
 *
 * ```cpp
 * #include <graph/graph.hpp>
 * #include <graph/algorithm/depth_first_search.hpp>
 *
 * using namespace graph;
 *
 * // Basic traversal:
 * Graph g({{0,1}, {1,2}, {2,3}});
 * depth_first_search(g, 0); // DFS from vertex 0
 *
 * // Cycle detection with visitor:
 * struct CycleDetector {
 *     bool has_cycle = false;
 *     void on_back_edge(auto& g, auto& edge) { has_cycle = true; }
 * };
 * CycleDetector detector;
 * depth_first_search(g, 0, detector);
 *
 * // Topological sort via finish ordering:
 * struct TopoVisitor {
 *     std::vector<uint32_t> topo_order;
 *     void on_finish_vertex(auto& g, auto v) { topo_order.push_back(v); }
 * };
 * TopoVisitor topo;
 * depth_first_search(g, 0, topo);
 * std::ranges::reverse(topo.topo_order);
 * ```
 *
 * @see views::vertices_dfs DFS view for range-based traversal
 * @see breadth_first_search BFS algorithm for shortest-path traversal
 */

template <adjacency_list G, class Visitor = empty_visitor, class Alloc = std::allocator<std::byte>>
void depth_first_search(G&&                   g,      // graph
                        const vertex_id_t<G>& source, // starting vertex_id
                        Visitor&&             visitor = empty_visitor(),
                        const Alloc&          alloc   = Alloc()) {
  using id_type = vertex_id_t<G>;

  // Vertex color states for DFS
  enum class Color : uint8_t {
    White, // Undiscovered
    Gray,  // Discovered but not finished
    Black  // Finished
  };

  // Lazy init: index graphs get a sized vector (value-init → White=0),
  // mapped graphs get an empty reserved map (absent key → White via get_color).
  auto color = make_vertex_property_map<std::remove_reference_t<G>, Color>(g);

  // Read helper: returns White for absent keys (mapped graphs, unvisited vertices).
  auto get_color = [&color](const id_type& uid) -> Color {
    return vertex_property_map_get(color, uid, Color::White);
  };

  // Initialize source vertex
  if constexpr (has_on_initialize_vertex<G, Visitor>) {
    visitor.on_initialize_vertex(g, *find_vertex(g, source));
  } else if constexpr (has_on_initialize_vertex_id<G, Visitor>) {
    visitor.on_initialize_vertex(g, source);
  }

  // Notify visitor that we're starting from this source
  if constexpr (has_on_start_vertex<G, Visitor>) {
    auto src_vertex = *find_vertex(g, source);
    visitor.on_start_vertex(g, src_vertex);
  } else if constexpr (has_on_start_vertex_id<G, Visitor>) {
    visitor.on_start_vertex(g, source);
  }

  // Each stack frame stores a vertex and iterators into its incidence range,
  // simulating the call stack of recursive DFS for correct edge classification.

  using inc_range_t    = decltype(views::incidence(g, *find_vertex(g, source)));
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
  } else if constexpr (has_on_discover_vertex_id<G, Visitor>) {
    visitor.on_discover_vertex(g, source);
  }

  using FrameAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<StackFrame>;
  std::stack<StackFrame, std::deque<StackFrame, FrameAlloc>> S{std::deque<StackFrame, FrameAlloc>(FrameAlloc(alloc))};
  {
    auto inc = views::incidence(g, *find_vertex(g, source));
    S.push({source, std::ranges::begin(inc), std::ranges::end(inc)});
  }

  while (!S.empty()) {
    auto& frame = S.top();

    if (frame.it == frame.end) {
      // All edges exhausted: mark vertex finished (Black) and pop
      color[frame.vertex_id] = Color::Black;
      if constexpr (has_on_finish_vertex<G, Visitor>) {
        visitor.on_finish_vertex(g, *find_vertex(g, frame.vertex_id));
      } else if constexpr (has_on_finish_vertex_id<G, Visitor>) {
        visitor.on_finish_vertex(g, frame.vertex_id);
      }
      S.pop();
      continue;
    }

    // Process next edge from this vertex
    auto&& [vid, uv] = *frame.it; // structured binding extracts target_id and edge
    ++frame.it;                   // advance iterator before potential push (simulates recursion past this edge)

    if constexpr (has_on_examine_edge<G, Visitor>) {
      visitor.on_examine_edge(g, uv);
    }

    const Color target_color = get_color(vid);

    if (target_color == Color::White) {
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
      } else if constexpr (has_on_discover_vertex_id<G, Visitor>) {
        visitor.on_discover_vertex(g, vid);
      }
      auto inc = views::incidence(g, *find_vertex(g, vid));
      S.push({vid, std::ranges::begin(inc), std::ranges::end(inc)});
    } else if (target_color == Color::Gray) {
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
