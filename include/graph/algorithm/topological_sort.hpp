/**
 * @file topological_sort.hpp
 * 
 * @brief Topological sorting algorithm for directed acyclic graphs (DAGs).
 * 
 * Topological sort produces a linear ordering of vertices such that for every directed 
 * edge (u,v), vertex u appears before vertex v in the ordering. This is essential for:
 * - Task scheduling with dependencies
 * - Build system ordering (makefiles, compilers)
 * - Course prerequisite planning
 * - Package dependency resolution
 * 
 * The algorithm uses depth-first search to compute finish times for all vertices, then 
 * outputs vertices in reverse finish-time order. This produces a valid topological 
 * ordering if and only if the graph is acyclic (a DAG). If a cycle is detected via a 
 * back edge during DFS, the algorithm returns false.
 * 
 * Three variants are provided:
 * 1. Full-graph: topological_sort(g, result) - sorts all vertices
 * 2. Single-source: topological_sort(g, source, result) - sorts vertices reachable from one vertex
 * 3. Multi-source: topological_sort(g, sources, result) - sorts vertices reachable from multiple vertices
 * 
 * **Complexity Analysis:**
 * 
 * **Time Complexity:**
 * | Case | Complexity | Notes |
 * |------|-----------|-------|
 * | Best case | O(V + E) | Always linear in graph size |
 * | Average case | O(V + E) | DFS visits each vertex and edge once |
 * | Worst case | O(V + E) | No pathological cases |
 * 
 * Where:
 * - V = number of vertices (or V_r = reachable vertices for source variants)
 * - E = number of edges (or E_r = reachable edges for source variants)
 * 
 * **Space Complexity:**
 * | Component | Space | Purpose |
 * |-----------|-------|---------|
 * | Color array | O(V) | Track vertex state (White/Gray/Black) |
 * | Finish order | O(V) | Collect vertices in finish order |
 * | DFS stack | O(V) | Store stack frames for iterative DFS |
 * | **Total** | **O(V)** | Auxiliary space, excluding graph |
 * 
 * **Supported Graph Properties:**
 * 
 * **Directedness:**
 * - ✅ Directed graphs (required)
 * - ❌ Undirected graphs (topological sort only defined for directed graphs)
 * 
 * **Edge Properties:**
 * - ✅ Unweighted edges (edge weights ignored)
 * - ✅ Weighted edges (weights ignored for ordering)
 * - ✅ Multi-edges (all edges traversed, may affect DFS tree)
 * - ✅ Self-loops (detected as cycles, returns false)
 * 
 * **Graph Structure:**
 * - ✅ Connected graphs
 * - ✅ Disconnected graphs (processes all components in full-graph variant)
 * - ✅ Must be acyclic (DAG) - returns false if cycle detected
 * - ❌ Cyclic graphs (algorithm detects and returns false)
 * 
 * **Container Requirements:**
 * - Requires: `adjacency_list<G>` concept
 * - Requires: `std::output_iterator<OutputIterator, vertex_id_t<G>>`
 * - Works with: All `dynamic_graph` container combinations (index and map-based)
 * - Works with: Any graph type satisfying the concept
 * 
 * **Implementation Notes:**
 * 
 * **Algorithm Overview:**
 * 1. Initialize color array: all vertices White (undiscovered)
 * 2. For each unvisited vertex (or specified sources):
 *    - Perform iterative DFS using explicit stack
 *    - Mark vertices Gray when discovered (on stack)
 *    - Detect cycles: back edge to Gray vertex
 *    - Mark vertices Black when finished (all descendants processed)
 *    - Record finish time by appending to finish_order
 * 3. Output vertices in reverse finish order (topological order)
 * 
 * **Three-Color Scheme:**
 * - **White:** Vertex not yet discovered
 * - **Gray:** Vertex discovered but not finished (currently on DFS stack)
 * - **Black:** Vertex finished (all descendants processed)
 * 
 * **Cycle Detection:**
 * - **Back edge:** Edge to Gray vertex → cycle detected → return false
 * - **Tree edge:** Edge to White vertex → continue DFS
 * - **Forward/Cross edge:** Edge to Black vertex → ignore (already processed)
 * 
 * **Data Structures:**
 * - **Color array:** `vertex_property_map<G, Color>` — lazy init, absent keys default to White
 * - **Finish order:** `std::vector<vertex_id_t<G>>` collects vertices as finished
 * - **DFS stack:** `std::stack<StackFrame>` stores (vertex_id, edge_iterator, edge_sentinel)
 * - **Output:** Reversed finish_order produces topological ordering
 * 
 * **Design Decisions:**
 * 1. **Why iterative DFS instead of recursive?**
 *    - Avoids stack overflow on deep graphs
 *    - Allows inspection/modification of stack state
 *    - More portable across platforms with limited stack size
 * 
 * 2. **Why three variants instead of one general function?**
 *    - Full-graph variant is most common use case (optimize for this)
 *    - Single-source avoids creating temporary array wrapper
 *    - Multi-source is most general, others delegate to it
 * 
 * 3. **Why return bool instead of throwing on cycle?**
 *    - Cycles are expected conditions in many applications
 *    - Boolean return is more efficient (no exception overhead)
 *    - Easier to reason about control flow
 * 
 * 4. **Why output to iterator instead of returning vector?**
 *    - Flexibility: can output to any container or adapter
 *    - Avoids allocation if caller already has container
 *    - Composable with other algorithms
 * 
 * **Optimization Opportunities:**
 * - For small graphs: recursive DFS may be slightly faster (avoid stack allocation)
 * - For disconnected graphs: could optimize full-graph to skip isolated vertices
 * - Could add early termination if only partial ordering needed
 * 
 * @copyright Copyright (c) 2024
 * 
 * SPDX-License-Identifier: BSL-1.0
 *
 * @authors Andrew Lumsdaine, Phil Ratzloff
 */

#include "graph/graph.hpp"
#include "graph/algorithm/depth_first_search.hpp"
#include "graph/algorithm/traversal_common.hpp"
#include "graph/adj_list/vertex_property_map.hpp"
#include "graph/views/incidence.hpp"

#include <vector>
#include <stack>
#include <ranges>

#ifndef GRAPH_TOPOSORT_ALGORITHM_HPP
#  define GRAPH_TOPOSORT_ALGORITHM_HPP

namespace graph {

// Using declarations for new namespace structure
using adj_list::adjacency_list;
using adj_list::vertex_id_t;
using adj_list::vertices;
using adj_list::vertex_id;
using adj_list::target_id;

namespace detail {

  // Vertex color states for DFS in topological sort
  enum class TopoColor : uint8_t {
    White, // Undiscovered
    Gray,  // Discovered but not finished (on stack)
    Black  // Finished
  };

  /**
 * @brief Helper function for DFS visit during topological sort.
 * 
 * Performs iterative DFS from a source vertex, collecting finish order and detecting cycles.
 * 
 * @tparam G          Graph type
 * @tparam ColorMap   Vertex property map type for colors
 * @tparam Alloc      Allocator type for the internal DFS stack.
 * @param g           The graph
 * @param source      Starting vertex ID
 * @param color       Color map for tracking vertex state
 * @param finish_order Vector to collect vertices in finish order
 * @param has_cycle   Flag set to true if cycle detected
 * @param alloc       Allocator instance used for the internal DFS stack
 */
  template <adjacency_list G, typename ColorMap, class Alloc>
  void topological_sort_dfs_visit(const G&                     g,
                                  const vertex_id_t<G>&        source,
                                  ColorMap&                    color,
                                  auto&                        finish_order,
                                  bool&                        has_cycle,
                                  const Alloc&                 alloc) {

    using Color = TopoColor;
    using id_type = vertex_id_t<G>;
    using namespace graph::views;
    using inc_range_t    = decltype(basic_incidence(g, source));
    using inc_iterator_t = std::ranges::iterator_t<inc_range_t>;
    using inc_sentinel_t = std::ranges::sentinel_t<inc_range_t>;

    struct StackFrame {
      id_type        vertex_id;
      inc_iterator_t it;
      inc_sentinel_t end;
    };

    // Discover source and push its stack frame
    color[source] = Color::Gray;

    using FrameAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<StackFrame>;
    std::stack<StackFrame, std::deque<StackFrame, FrameAlloc>> S{std::deque<StackFrame, FrameAlloc>(FrameAlloc(alloc))};
    {
      auto inc = basic_incidence(g, source);
      S.push({source, std::ranges::begin(inc), std::ranges::end(inc)});
    }

    while (!S.empty() && !has_cycle) {
      auto& frame = S.top();

      if (frame.it == frame.end) {
        // All edges exhausted: mark vertex finished and record finish time
        color[frame.vertex_id] = Color::Black;
        finish_order.push_back(frame.vertex_id);
        S.pop();
        continue;
      }

      // Process next edge from this vertex
      auto&& [vid] = *frame.it;
      ++frame.it;

      const Color target_color = vertex_property_map_get(color, vid, Color::White);

      if (target_color == Color::White) {
        // Tree edge: discover target and push its frame
        color[vid] = Color::Gray;
        auto inc   = basic_incidence(g, vid);
        S.push({vid, std::ranges::begin(inc), std::ranges::end(inc)});
      } else if (target_color == Color::Gray) {
        // Back edge: cycle detected
        has_cycle = true;
        return;
      }
      // Black vertices (forward/cross edges) are ignored - already processed
    }
  }

} // namespace detail

/**
 * @brief Compute topological ordering of vertices reachable from multiple sources.
 * 
 * Performs topological sort starting from multiple source vertices using depth-first
 * search. Outputs all vertices reachable from any source in reverse finish-time order.
 * Returns false if a cycle is detected in the reachable subgraph.
 * 
 * @tparam G              Graph type satisfying adjacency_list concept.
 * @tparam Sources         Input range of source vertex IDs.
 * @tparam OutputIterator  Output iterator for writing vertex IDs in topological order.
 * @tparam Alloc           Allocator type for the internal finish-order vector and DFS stack.
 *                         Defaults to std::allocator<std::byte>.
 * 
 * @param g       The directed graph to sort.
 * @param sources Range of starting vertex IDs for traversal.
 * @param result  Output iterator where vertex IDs are written in topological order.
 * @param alloc   Allocator instance used for the internal DFS stack and finish-order vector
 *                (default: Alloc())
 * 
 * @return true if reachable subgraph is acyclic, false if cycle detected.
 * 
 * **Mandates:**
 * - G must satisfy adjacency_list
 * - Sources must satisfy std::ranges::input_range with values convertible to vertex_id_t<G>
 * - OutputIterator must satisfy std::output_iterator<vertex_id_t<G>>
 * 
 * **Preconditions:**
 * - Graph g must be directed
 * - All vertex IDs in sources must be valid
 * - Reachable subgraph should be a DAG for successful result
 * 
 * **Effects:**
 * - Performs DFS from each source vertex, recording finish order
 * - Outputs reachable vertices in reverse finish-time order (topological order)
 * - Does not modify graph g
 * 
 * **Postconditions:**
 * - If returns true: for every edge (u,v) where both are reachable, u appears before v.
 *   Each reachable vertex appears exactly once. Unreachable vertices are excluded.
 * - If returns false: cycle was detected; output may contain partial results
 * - If sources is empty: returns true with no output
 * 
 * **Returns:**
 * - true if reachable subgraph is acyclic and ordering is valid (bool)
 * - false if cycle detected in reachable subgraph
 * - Attribute: [[nodiscard]] recommended
 * 
 * **Throws:**
 * - std::bad_alloc if memory allocation fails
 * - May propagate exceptions from sources range iteration
 * - Exception guarantee: Basic. Graph g remains unchanged; output may be partial.
 * 
 * **Complexity:**
 * - Time: O(V_r + E_r) where V_r = reachable vertices, E_r = reachable edges
 * - Space: O(V) for color array, O(V_r) for finish order
 * 
 * **Remarks:**
 * - This is the most general form; single-source and full-graph variants delegate to it
 * - Redundant sources (where one is reachable from another) are handled efficiently
 * - Sources can be in any order; components are processed as encountered
 * 
 * ## Example Usage
 *
 * ```cpp
 * std::vector<uint32_t> sources = {0, 1};
 * std::vector<uint32_t> order;
 * if (topological_sort(g, sources, std::back_inserter(order))) {
 *     // order contains reachable vertices in valid topological order
 * }
 * ```
 * 
 * @see topological_sort(const G&, OutputIterator) for full-graph variant
 * @see topological_sort(const G&, vertex_id_t<G>, OutputIterator) for single-source variant
 */
template <adjacency_list G, std::ranges::input_range Sources, class OutputIterator,
          class Alloc = std::allocator<std::byte>>
requires std::convertible_to<std::ranges::range_value_t<Sources>, vertex_id_t<G>> &&
         std::output_iterator<OutputIterator, vertex_id_t<G>>
bool topological_sort(const G& g, const Sources& sources, OutputIterator result,
                      const Alloc& alloc = Alloc()) {
  using id_type = vertex_id_t<G>;
  using Color   = detail::TopoColor;

  // Lazy init: index graphs get a sized vector (value-init → White=0),
  // mapped graphs get an empty reserved map (absent key → White via get).
  auto color = make_vertex_property_map<std::remove_reference_t<G>, Color>(g);
  using IdAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<id_type>;
  std::vector<id_type, IdAlloc> finish_order{IdAlloc(alloc)};
  finish_order.reserve(num_vertices(g));

  bool has_cycle = false;

  // Run DFS from each source (skipping already-visited vertices)
  for (auto source : sources) {
    if (vertex_property_map_get(color, source, Color::White) == Color::White) {
      detail::topological_sort_dfs_visit(g, source, color, finish_order, has_cycle, alloc);
      if (has_cycle) {
        return false; // Cycle detected
      }
    }
  }

  // Output vertices in reverse finish order (topological order)
  std::ranges::copy(finish_order | std::views::reverse, result);

  return true;
}

/**
 * @brief Compute topological ordering of vertices reachable from a single source.
 * 
 * Performs topological sort starting from a single source vertex. Only vertices
 * reachable from the source are included in the output.
 * 
 * @tparam G              Graph type satisfying adjacency_list concept.
 * @tparam OutputIterator  Output iterator for writing vertex IDs in topological order.
 * @tparam Alloc           Allocator type for internal storage. Defaults to std::allocator<std::byte>.
 * 
 * @param g       The directed graph to sort.
 * @param source  Starting vertex ID for traversal.
 * @param result  Output iterator where vertex IDs are written in topological order.
 * @param alloc   Allocator instance forwarded to the multi-source version (default: Alloc())
 * 
 * @return true if reachable subgraph is acyclic, false if cycle detected.
 * 
 * **Mandates:**
 * - G must satisfy adjacency_list
 * - OutputIterator must satisfy std::output_iterator<vertex_id_t<G>>
 * 
 * **Preconditions:**
 * - Graph g must be directed
 * - source must be a valid vertex ID in the graph
 * 
 * **Effects:**
 * - Delegates to multi-source variant with single-element array
 * - Does not modify graph g
 * 
 * **Postconditions:**
 * - If returns true: reachable vertices in valid topological order, source vertex included
 * - If returns false: cycle detected; output may contain partial results
 * 
 * **Returns:**
 * - true if reachable subgraph is acyclic (bool)
 * - false if cycle detected
 * 
 * **Throws:**
 * - std::bad_alloc if memory allocation fails
 * - Exception guarantee: Basic. Graph g remains unchanged.
 * 
 * **Complexity:**
 * - Time: O(V_r + E_r) where V_r = reachable vertices, E_r = reachable edges
 * - Space: O(V) for color array, O(V_r) for finish order
 * 
 * ## Example Usage
 *
 * ```cpp
 * std::vector<uint32_t> order;
 * if (topological_sort(g, 0, std::back_inserter(order))) {
 *     // order contains vertices reachable from 0 in topological order
 * }
 * ```
 * 
 * @see topological_sort(const G&, OutputIterator) for full-graph variant
 * @see topological_sort(const G&, const Sources&, OutputIterator) for multi-source variant
 */
template <adjacency_list G, class OutputIterator, class Alloc = std::allocator<std::byte>>
requires std::output_iterator<OutputIterator, vertex_id_t<G>>
bool topological_sort(const G& g, const vertex_id_t<G>& source, OutputIterator result,
                      const Alloc& alloc = Alloc()) {
  // Delegate to multi-source version with single source
  std::array<vertex_id_t<G>, 1> sources = {source};
  return topological_sort(g, sources, result, alloc);
}

/**
 * @brief Compute topological ordering of all vertices in a directed acyclic graph (DAG).
 * 
 * Performs topological sort of the entire graph using depth-first search. Outputs all
 * vertices in reverse finish-time order. Returns false if a cycle is detected.
 * 
 * @tparam G              Graph type satisfying adjacency_list concept.
 * @tparam OutputIterator  Output iterator for writing vertex IDs in topological order.
 * @tparam Alloc           Allocator type for internal storage. Defaults to std::allocator<std::byte>.
 * 
 * @param g       The directed graph to sort.
 * @param result  Output iterator where vertex IDs are written in topological order.
 * @param alloc   Allocator instance used for the internal finish-order vector and DFS stack
 *                (default: Alloc())
 * 
 * @return true if graph is acyclic, false if cycle detected.
 * 
 * **Mandates:**
 * - G must satisfy adjacency_list
 * - OutputIterator must satisfy std::output_iterator<vertex_id_t<G>>
 * 
 * **Preconditions:**
 * - Graph g must be directed
 * - Graph should be a DAG for successful result
 * 
 * **Effects:**
 * - Performs DFS from each unvisited vertex, recording finish order
 * - Outputs all vertices in reverse finish-time order (topological order)
 * - Does not modify graph g
 * 
 * **Postconditions:**
 * - If returns true: for every directed edge (u,v), u appears before v in output.
 *   All vertices appear exactly once.
 * - If returns false: cycle detected; output may contain partial results
 * 
 * **Returns:**
 * - true if graph is acyclic and ordering is valid (bool)
 * - false if cycle detected (graph is not a DAG)
 * 
 * **Throws:**
 * - std::bad_alloc if memory allocation fails
 * - Exception guarantee: Basic. Graph g remains unchanged; output may be partial.
 * 
 * **Complexity:**
 * - Time: O(V + E) where V = number of vertices, E = number of edges
 * - Space: O(V) for color array, finish order vector, and DFS stack
 * 
 * ## Example Usage
 *
 * ```cpp
 * std::vector<uint32_t> order;
 * if (topological_sort(g, std::back_inserter(order))) {
 *     // order contains all vertices in valid topological order
 * } else {
 *     // Graph contains a cycle
 * }
 * ```
 * 
 * @see topological_sort(const G&, vertex_id_t<G>, OutputIterator) for single-source variant
 * @see topological_sort(const G&, const Sources&, OutputIterator) for multi-source variant
 */
template <adjacency_list G, class OutputIterator, class Alloc = std::allocator<std::byte>>
requires std::output_iterator<OutputIterator, vertex_id_t<G>>
bool topological_sort(const G& g, OutputIterator result, const Alloc& alloc = Alloc()) {
  using id_type = vertex_id_t<G>;
  using Color   = detail::TopoColor;

  // Lazy init: index graphs get a sized vector (value-init → White=0),
  // mapped graphs get an empty reserved map (absent key → White via get).
  auto color = make_vertex_property_map<std::remove_reference_t<G>, Color>(g);
  using IdAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<id_type>;
  std::vector<id_type, IdAlloc> finish_order{IdAlloc(alloc)};
  finish_order.reserve(num_vertices(g));

  bool has_cycle = false;

  // Run DFS from each unvisited vertex
  for (auto v : vertices(g)) {
    id_type vid = vertex_id(g, v);
    if (vertex_property_map_get(color, vid, Color::White) == Color::White) {
      detail::topological_sort_dfs_visit(g, vid, color, finish_order, has_cycle, alloc);
      if (has_cycle) {
        return false; // Cycle detected
      }
    }
  }

  // Output vertices in reverse finish order (topological order)
  std::ranges::copy(finish_order | std::views::reverse, result);

  return true;
}

} // namespace graph

#endif // GRAPH_TOPOSORT_ALGORITHM_HPP
