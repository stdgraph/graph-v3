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
 * - Requires: `index_adjacency_list<G>` concept
 * - Requires: `integral<vertex_id_t<G>>`
 * - Requires: `std::output_iterator<OutputIterator, vertex_id_t<G>>`
 * - Works with: All `dynamic_graph` container combinations
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
 * - **Color array:** `std::vector<Color>` for O(1) vertex state lookup
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
#include "graph/views/incidence.hpp"

#include <vector>
#include <stack>
#include <ranges>

#ifndef GRAPH_TOPOSORT_ALGORITHM_HPP
#  define GRAPH_TOPOSORT_ALGORITHM_HPP

namespace graph {

// Using declarations for new namespace structure
using adj_list::index_adjacency_list;
using adj_list::vertex_id_t;
using adj_list::vertices;
using adj_list::vertex_id;
using adj_list::target_id;

namespace detail {

  /**
 * @brief Helper function for DFS visit during topological sort.
 * 
 * Performs iterative DFS from a source vertex, collecting finish order and detecting cycles.
 * 
 * @tparam G Graph type
 * @tparam Color Enum type for vertex colors
 * @param g The graph
 * @param source Starting vertex ID
 * @param color Color array for tracking vertex state
 * @param finish_order Vector to collect vertices in finish order
 * @param has_cycle Flag set to true if cycle detected
 */
  template <index_adjacency_list G, typename Color>
  void topological_sort_dfs_visit(const G&                     g,
                                  vertex_id_t<G>               source,
                                  std::vector<Color>&          color,
                                  std::vector<vertex_id_t<G>>& finish_order,
                                  bool&                        has_cycle) {

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

    std::stack<StackFrame> S;
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

      if (color[vid] == Color::White) {
        // Tree edge: discover target and push its frame
        color[vid] = Color::Gray;
        auto inc   = basic_incidence(g, vid);
        S.push({vid, std::ranges::begin(inc), std::ranges::end(inc)});
      } else if (color[vid] == Color::Gray) {
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
 * This is the most general form. Use when you need the dependency ordering for building
 * multiple targets, processing only the union of their dependency subgraphs. The algorithm
 * maintains a shared color array to prevent duplicate vertex visits across sources.
 * 
 * **Parameters:**
 * 
 * @tparam G Graph type satisfying `index_adjacency_list` concept
 *   - **Requires:** `index_adjacency_list<G>`
 *   - **Requires:** `integral<vertex_id_t<G>>`
 *   - **Description:** The directed graph type to operate on
 * 
 * @tparam Sources Input range of source vertex IDs
 *   - **Requires:** `std::ranges::input_range<Sources>`
 *   - **Requires:** `std::convertible_to<std::ranges::range_value_t<Sources>, vertex_id_t<G>>`
 *   - **Description:** Range providing source vertex IDs to start traversal from
 *   - **Typical:** `std::vector<vertex_id_t<G>>`, `std::array<vertex_id_t<G>, N>`, `std::span`, etc.
 * 
 * @tparam OutputIterator Iterator type for writing vertex IDs in topological order
 *   - **Requires:** `std::output_iterator<OutputIterator, vertex_id_t<G>>`
 *   - **Description:** Output iterator that accepts vertex IDs
 * 
 * @param g The directed graph to sort
 *   - **Precondition:** Graph must be directed
 *   - **Postcondition:** Graph remains unmodified
 * 
 * @param sources Range of starting vertex IDs for traversal
 *   - **Type:** `const Sources&`
 *   - **Precondition:** All vertex IDs must be valid (in range for graph)
 *   - **Precondition:** `sources` can be empty (returns true with no output)
 *   - **Description:** Collection of source vertices from which to start DFS
 * 
 * @param result Output iterator where vertex IDs are written in topological order
 *   - **Postcondition:** If returns true, contains all reachable vertices exactly once
 *   - **Postcondition:** If returns false, may contain partial results
 * 
 * **Return Value:**
 * - **Type:** `bool`
 * - **Returns:** `true` if reachable subgraph is acyclic and ordering is valid
 * - **Returns:** `false` if cycle is detected in reachable subgraph
 * 
 * **Preconditions:**
 * 1. Graph `g` must be directed
 * 2. All vertex IDs in `sources` must be valid
 * 3. Reachable subgraph should be a DAG for successful result
 * 
 * **Postconditions:**
 * 1. If returns `true`:
 *    - For every edge (u,v) where both u,v are reachable from any source, u appears before v
 *    - Only vertices reachable from any source are written to output
 *    - Each reachable vertex appears exactly once (even if reachable from multiple sources)
 *    - Vertices from different sources may be interleaved in output
 * 2. If returns `false`:
 *    - A cycle was detected in the reachable subgraph
 *    - Output iterator may contain partial results
 * 3. If `sources` is empty:
 *    - Returns `true` with no output (trivially valid)
 * 
 * **Complexity:**
 * - **Time:** O(V_r + E_r) where V_r = vertices reachable from any source, E_r = reachable edges
 * - **Space:** O(V) for color array (full graph size), O(V_r) for finish order
 * 
 * **Exception Safety:**
 * - **Basic guarantee:** If exception thrown, graph remains unchanged
 * - **Throws:** May throw `std::bad_alloc` if memory allocation fails
 * - **Throws:** May propagate exceptions from `sources` range iteration (if any)
 * 
 * **Example:**
 * @code
 * using Graph = vov_void;
 * // Graph: 0->2, 1->2, 2->3, 4->5 (vertices 4,5 unreachable from 0,1)
 * Graph g({{0,2}, {1,2}, {2,3}, {4,5}});
 * 
 * std::vector<uint32_t> sources = {0, 1};
 * std::vector<uint32_t> order;
 * 
 * if (topological_sort(g, sources, std::back_inserter(order))) {
 *     // order contains: [0, 1, 2, 3] or [1, 0, 2, 3]
 *     // Both are valid - vertex 2 comes after both 0 and 1
 *     // Vertices 4, 5 are NOT included (unreachable)
 *     std::cout << "Build order for targets 0 and 1: ";
 *     for (auto v : order) {
 *         std::cout << v << " ";
 *     }
 * }
 * @endcode
 * 
 * **Use Cases:**
 * - Building multiple targets in a build system
 * - Resolving dependencies for installing multiple packages
 * - Finding prerequisites for completing multiple courses
 * - Processing disconnected components with explicit starting points
 * - Incremental computation: process only affected subgraph after changes
 * 
 * **Notes:**
 * - Redundant sources (where one is reachable from another) are handled efficiently
 * - Sources can be provided in any order
 * - If sources span disconnected components, all components are processed
 * - Empty sources range is valid (returns true, no output)
 * 
 * @see topological_sort(const G&, OutputIterator) for full-graph variant
 * @see topological_sort(const G&, vertex_id_t<G>, OutputIterator) for single-source variant
 */
template <index_adjacency_list G, std::ranges::input_range Sources, class OutputIterator>
requires std::convertible_to<std::ranges::range_value_t<Sources>, vertex_id_t<G>> &&
         std::output_iterator<OutputIterator, vertex_id_t<G>>
bool topological_sort(const G& g, const Sources& sources, OutputIterator result) {
  using id_type = vertex_id_t<G>;

  // Vertex color states for DFS
  enum class Color : uint8_t {
    White, // Undiscovered
    Gray,  // Discovered but not finished (on stack)
    Black  // Finished
  };

  std::vector<Color>   color(num_vertices(g), Color::White);
  std::vector<id_type> finish_order;
  finish_order.reserve(num_vertices(g));

  bool has_cycle = false;

  // Run DFS from each source (skipping already-visited vertices)
  for (auto source : sources) {
    if (color[source] == Color::White) {
      detail::topological_sort_dfs_visit(g, source, color, finish_order, has_cycle);
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
 * Performs topological sort starting from a single source vertex using depth-first 
 * search. Outputs only vertices reachable from the source in reverse finish-time order.
 * Returns false if a cycle is detected in the reachable subgraph.
 * 
 * Use this when you need the dependency ordering for building a specific target,
 * ignoring unreachable parts of the graph. This is more efficient than full-graph
 * topological sort when only a subset of the graph needs to be processed.
 * 
 * **Parameters:**
 * 
 * @tparam G Graph type satisfying `index_adjacency_list` concept
 *   - **Requires:** `index_adjacency_list<G>`
 *   - **Requires:** `integral<vertex_id_t<G>>`
 *   - **Description:** The directed graph type to operate on
 * 
 * @tparam OutputIterator Iterator type for writing vertex IDs in topological order
 *   - **Requires:** `std::output_iterator<OutputIterator, vertex_id_t<G>>`
 *   - **Description:** Output iterator that accepts vertex IDs
 * 
 * @param g The directed graph to sort
 *   - **Precondition:** Graph must be directed
 *   - **Postcondition:** Graph remains unmodified
 * 
 * @param source Starting vertex ID for traversal
 *   - **Type:** `vertex_id_t<G>`
 *   - **Precondition:** `source < num_vertices(g)` (for vector-based containers)
 *   - **Precondition:** `contains_vertex(g, source)` (for map-based containers)
 *   - **Description:** The source vertex from which to start DFS traversal
 * 
 * @param result Output iterator where vertex IDs are written in topological order
 *   - **Postcondition:** If returns true, contains reachable vertices in valid topological order
 *   - **Postcondition:** If returns false, may contain partial results
 * 
 * **Return Value:**
 * - **Type:** `bool`
 * - **Returns:** `true` if reachable subgraph is acyclic and ordering is valid
 * - **Returns:** `false` if cycle is detected in reachable subgraph
 * 
 * **Preconditions:**
 * 1. Graph `g` must be directed
 * 2. `source` must be a valid vertex ID in the graph
 * 3. Reachable subgraph should be a DAG for successful result
 * 
 * **Postconditions:**
 * 1. If returns `true`:
 *    - For every edge (u,v) where both u,v are reachable from source, u appears before v
 *    - Only vertices reachable from source are written to output
 *    - Each reachable vertex appears exactly once
 *    - Source vertex always appears in output (unless self-loop detected)
 * 2. If returns `false`:
 *    - A cycle was detected in the reachable subgraph
 *    - Output iterator may contain partial results
 * 
 * **Complexity:**
 * - **Time:** O(V_r + E_r) where V_r = reachable vertices, E_r = reachable edges
 * - **Space:** O(V) for color array (full graph size), O(V_r) for finish order
 * 
 * **Exception Safety:**
 * - **Basic guarantee:** If exception thrown, graph remains unchanged
 * - **Throws:** May throw `std::bad_alloc` if memory allocation fails
 * 
 * **Example:**
 * @code
 * using Graph = vov_void;
 * // Graph: 0->1->3, 2->3 (vertex 2 is not reachable from 0)
 * Graph g({{0,1}, {1,3}, {2,3}});
 * 
 * std::vector<uint32_t> order;
 * if (topological_sort(g, 0, std::back_inserter(order))) {
 *     // order contains: [0, 1, 3]
 *     // Vertex 2 is not included (unreachable from source 0)
 *     std::cout << "Build order: ";
 *     for (auto v : order) {
 *         std::cout << v << " ";
 *     }
 * }
 * @endcode
 * 
 * **Use Cases:**
 * - Computing build dependencies for a specific target
 * - Finding prerequisite courses to complete a specific course
 * - Resolving package dependencies for installing one package
 * - Processing only the relevant portion of a large dependency graph
 * 
 * @see topological_sort(const G&, OutputIterator) for full-graph variant
 * @see topological_sort(const G&, const Sources&, OutputIterator) for multi-source variant
 */
template <index_adjacency_list G, class OutputIterator>
requires std::output_iterator<OutputIterator, vertex_id_t<G>>
bool topological_sort(const G& g, vertex_id_t<G> source, OutputIterator result) {
  // Delegate to multi-source version with single source
  std::array<vertex_id_t<G>, 1> sources = {source};
  return topological_sort(g, sources, result);
}

/**
 * @brief Compute topological ordering of all vertices in a directed acyclic graph (DAG).
 * 
 * Performs topological sort of the entire graph using depth-first search. Outputs 
 * all vertices in reverse finish-time order, which is a valid topological ordering 
 * if the graph is acyclic. Returns false if a cycle is detected (back edge found).
 * 
 * This is the most common topological sort use case: ordering all vertices in the graph
 * such that for every directed edge (u,v), vertex u appears before vertex v.
 * 
 * **Parameters:**
 * 
 * @tparam G Graph type satisfying `index_adjacency_list` concept
 *   - **Requires:** `index_adjacency_list<G>`
 *   - **Requires:** `integral<vertex_id_t<G>>`
 *   - **Description:** The directed graph type to operate on
 * 
 * @tparam OutputIterator Iterator type for writing vertex IDs in topological order
 *   - **Requires:** `std::output_iterator<OutputIterator, vertex_id_t<G>>`
 *   - **Description:** Output iterator that accepts vertex IDs
 *   - **Typical:** `std::back_inserter(vector)`, `std::ostream_iterator`, etc.
 * 
 * @param g The directed graph to sort
 *   - **Precondition:** Graph must be directed (undirected graphs not supported)
 *   - **Postcondition:** Graph remains unmodified
 * 
 * @param result Output iterator where vertex IDs are written in topological order
 *   - **Postcondition:** If returns true, contains all vertices in valid topological order
 *   - **Postcondition:** If returns false, may contain partial results (indeterminate)
 * 
 * **Return Value:**
 * - **Type:** `bool`
 * - **Returns:** `true` if graph is acyclic and ordering is valid
 * - **Returns:** `false` if cycle is detected (graph is not a DAG)
 * 
 * **Preconditions:**
 * 1. Graph `g` must be directed
 * 2. Graph should be a DAG (no cycles) for successful result
 * 
 * **Postconditions:**
 * 1. If returns `true`:
 *    - For every directed edge (u,v), u appears before v in output
 *    - All vertices in graph are written to output exactly once
 *    - Output represents a valid topological ordering
 * 2. If returns `false`:
 *    - A cycle was detected during DFS traversal
 *    - Output iterator may contain partial results
 *    - No guarantee about output content
 * 
 * **Complexity:**
 * - **Time:** O(V + E) where V = number of vertices, E = number of edges
 * - **Space:** O(V) for color array, finish order vector, and DFS stack
 * 
 * **Exception Safety:**
 * - **Basic guarantee:** If exception thrown, graph remains unchanged
 * - **Throws:** May throw `std::bad_alloc` if memory allocation fails
 * - **Note:** Output iterator state is indeterminate after exception
 * 
 * **Example:**
 * @code
 * using Graph = vov_void;
 * Graph g({{0,1}, {0,2}, {1,3}, {2,3}});
 * 
 * std::vector<uint32_t> order;
 * if (topological_sort(g, std::back_inserter(order))) {
 *     // order contains: [0, 1, 2, 3] or [0, 2, 1, 3]
 *     // Both are valid topological orderings
 *     for (auto v : order) {
 *         std::cout << v << " ";
 *     }
 * } else {
 *     std::cout << "Graph contains a cycle!\n";
 * }
 * @endcode
 * 
 * @see topological_sort(const G&, vertex_id_t<G>, OutputIterator) for single-source variant
 * @see topological_sort(const G&, const Sources&, OutputIterator) for multi-source variant
 */
template <index_adjacency_list G, class OutputIterator>
requires std::output_iterator<OutputIterator, vertex_id_t<G>>
bool topological_sort(const G& g, OutputIterator result) {
  using id_type = vertex_id_t<G>;

  // Vertex color states for DFS
  enum class Color : uint8_t {
    White, // Undiscovered
    Gray,  // Discovered but not finished (on stack)
    Black  // Finished
  };

  std::vector<Color>   color(num_vertices(g), Color::White);
  std::vector<id_type> finish_order;
  finish_order.reserve(num_vertices(g));

  bool has_cycle = false;

  // Run DFS from each unvisited vertex
  for (auto v : vertices(g)) {
    id_type vid = vertex_id(g, v);
    if (color[vid] == Color::White) {
      detail::topological_sort_dfs_visit(g, vid, color, finish_order, has_cycle);
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
