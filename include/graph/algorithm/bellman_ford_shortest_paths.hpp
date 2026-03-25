/**
 * @file bellman_ford_shortest_paths.hpp
 * 
 * @brief Single-Source Shortest paths and shortest distances algorithms using Bellman-Ford's algorithm.
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
#include "graph/views/edgelist.hpp"
#include "graph/algorithm/traversal_common.hpp"
#include "graph/adj_list/vertex_property_map.hpp"

#include <ranges>
#include <optional>
#include <stdexcept>
#include <format>

#ifndef GRAPH_BELLMAN_SHORTEST_PATHS_HPP
#  define GRAPH_BELLMAN_SHORTEST_PATHS_HPP

namespace graph {

// Using declarations for CPO-based architecture
using adj_list::index_adjacency_list;
using adj_list::adjacency_list;
using adj_list::index_vertex_range;
using adj_list::vertex_id_t;
using adj_list::edge_t;
using adj_list::num_vertices;
using adj_list::vertices;
using adj_list::find_vertex;
using adj_list::source_id;
using adj_list::target_id;

/**
 * @brief Get the vertex ids in a negative weight cycle.
 * 
 * If a negative weight cycle exists, the vertex ids in the cycle are output to the output iterator.
 * If no negative weight cycle exists, the output iterator is not modified.
 * 
 * @tparam G            The graph type.
 * @tparam Predecessors The predecessor range type.
 * @tparam OutputIterator The output iterator type.
 * 
 * @param g              The graph.
 * @param predecessor    The predecessor range.
 * @param cycle_vertex_id A vertex id in the negative weight cycle. If no negative weight cycle exists 
 *                       then there will be no vertex id defined.
 * @param out_cycle      The output iterator that the vertex ids in the cycle are output to.
 */
template <adjacency_list G, class Predecessors, class OutputIterator>
requires output_iterator<OutputIterator, vertex_id_t<G>>
void find_negative_cycle(G&                              g,
                         const Predecessors&             predecessor,
                         const optional<vertex_id_t<G>>& cycle_vertex_id,
                         OutputIterator                  out_cycle) {
  // Does a negative weight cycle exist?
  if (cycle_vertex_id.has_value()) {
    vertex_id_t<G> uid = cycle_vertex_id.value();
    do {
      *out_cycle++ = uid;
      if constexpr (std::ranges::random_access_range<Predecessors>) {
        uid = predecessor[uid];
      } else {
        uid = predecessor.at(uid);
      }
    } while (uid != cycle_vertex_id.value());
  }
}


/**
 * @brief Multi-source shortest paths using Bellman-Ford algorithm.
 * 
 * Finds shortest paths from one or more source vertices to all other vertices in a weighted graph.
 * Unlike Dijkstra's algorithm, Bellman-Ford can handle negative edge weights and detects negative
 * weight cycles. Returns an optional vertex ID indicating whether a negative cycle was detected.
 * 
 * @tparam G            The graph type. Must satisfy adjacency_list concept (index or mapped).
 * @tparam Sources      Input range of source vertex IDs.
 * @tparam Distances    Vertex property map satisfying vertex_property_map_for<Distances,G>. Value type must be arithmetic.
 * @tparam Predecessors Vertex property map satisfying vertex_property_map_for<Predecessors,G>. Can use null_predecessors
 *                      if path reconstruction is not needed.
 * @tparam WF           Edge weight function. Defaults to returning 1 for all edges (unweighted).
 * @tparam Visitor      Visitor type with callbacks for algorithm events. Defaults to empty_visitor.
 *                      Visitor calls are optimized away if not used.
 * @tparam Compare      Comparison function for distance values. Defaults to less<>.
 * @tparam Combine      Function to combine distances and weights. Defaults to plus<>.
 * 
 * @param g            The graph to process.
 * @param sources      Range of source vertex IDs to start from.
 * @param distances    [out] Shortest distances from sources. Must be a vertex property map for G.
 * @param predecessor  [out] Predecessor information for path reconstruction. Must be a vertex property map for G.
 * @param weight       Edge weight function: (const edge_t<G>&) -> Distance.
 * @param visitor      Visitor for algorithm events (examine, relax, not_relaxed, minimized, not_minimized).
 * @param compare      Distance comparison function: (Distance, Distance) -> bool.
 * @param combine      Distance combination function: (Distance, Weight) -> Distance.
 * 
 * @return optional<vertex_id_t<G>>. Returns empty if no negative cycle detected. Returns a vertex ID
 *         in the negative cycle if one exists. Use find_negative_cycle() to extract all cycle vertices.
 * 
 * **Mandates:**
 * - G must satisfy adjacency_list (index or mapped graphs supported)
 * - Sources must be input_range with values convertible to vertex_id_t<G>
 * - Distances must satisfy vertex_property_map_for<Distances,G> with arithmetic value type
 * - Predecessors must satisfy vertex_property_map_for<Predecessors,G> (or null_predecessors)
 * - WF must satisfy basic_edge_weight_function
 * 
 * **Preconditions:**
 * - All source vertices must be valid vertex IDs in vertices(g)
 * - distances must contain an entry for each vertex of g
 * - predecessor must contain an entry for each vertex of g (unless using null_predecessors)
 * - Weight function must not throw or modify graph state
 * 
 * **Effects:**
 * - Modifies distances: Sets distances[v] for all vertices v
 * - Modifies predecessor: Sets predecessor[v] for all processed edges
 * - Does not modify the graph g
 * 
 * **Postconditions:**
 * - distances[s] == 0 for all sources s
 * - If no negative cycle: For reachable v, distances[v] contains shortest distance from nearest source
 * - If no negative cycle: For reachable v, predecessor[v] contains predecessor in shortest path tree
 * - If negative cycle detected: distances and predecessor may contain intermediate values
 * - For unreachable vertices v: distances[v] == numeric_limits<Distance>::max()
 * 
 * **Returns:**
 * - optional<vertex_id_t<G>>: empty if no negative cycle detected; contains a vertex ID in the
 *   negative cycle if one exists. Use find_negative_cycle() to extract all cycle vertices.
 * - Attribute: [[nodiscard]]
 * 
 * **Throws:**
 * - std::out_of_range if a source vertex ID is out of range
 * - std::out_of_range if distances or predecessor are undersized
 * - Exception guarantee: Basic. If an exception is thrown, graph g remains unchanged;
 *   distances and predecessor may be partially modified (indeterminate state).
 * 
 * **Complexity:**
 * - Time: O(V * E) - iterates over all edges V times
 * - Space: O(1) auxiliary space (excluding output parameters)
 * 
 * **Remarks:**
 * - Use Bellman-Ford when: graph has negative weights, need cycle detection, or edges processed sequentially
 * - Use Dijkstra when: all weights non-negative and need better performance O((V+E) log V)
 * - Negative cycle detection: Algorithm performs V iterations. If any edge relaxes on iteration V, a
 *   negative cycle exists. The returned vertex ID can be used with find_negative_cycle() to extract
 *   all vertices in the cycle.
 * - Based on Boost.Graph bellman_ford_shortest_paths implementation
 *
 * **Supported Graph Properties:**
 *
 * Directedness:
 * - ✅ Directed graphs
 *
 * Edge Properties:
 * - ✅ Weighted edges (including negative weights)
 * - ✅ Unweighted edges (default weight function returns 1)
 * - ✅ Multi-edges (all edges considered during relaxation)
 * - ✅ Self-loops (relaxation has no effect since distance cannot decrease)
 * - ✅ Cycles
 * - ✅ Negative weight cycles (detected and reported via return value)
 *
 * Graph Structure:
 * - ✅ Connected graphs
 * - ✅ Disconnected graphs (unreachable vertices retain infinite distance)
 * - ✅ Empty graphs (returns immediately)
 *
 * ## Example Usage
 *
 * ```cpp
 * #include <graph/graph.hpp>
 * #include <graph/algorithm/bellman_ford_shortest_paths.hpp>
 * #include <vector>
 * #include <limits>
 *
 * using namespace graph;
 *
 * int main() {
 *     using Graph = container::dynamic_graph<void, void, double, uint32_t, false,
 *                       container::vol_graph_traits<void, void, double, uint32_t, false>>;
 *
 *     // Weighted directed graph with a negative edge: 0 --(4.0)--> 1 --(-2.0)--> 2 --(3.0)--> 3
 *     Graph g({{0,1,4.0},{1,2,-2.0},{2,3,3.0}});
 *
 *     constexpr auto INF = std::numeric_limits<double>::max();
 *     std::vector<double>   dist(num_vertices(g), INF);
 *     std::vector<uint32_t> pred(num_vertices(g), 0);
 *
 *     auto cycle = bellman_ford_shortest_paths(g, 0u, dist, pred);
 *     // cycle is empty (no negative cycle)
 *     // dist == {0.0, 4.0, 2.0, 5.0}
 * }
 * ```
 * 
 * @see find_negative_cycle() to extract vertices in detected negative cycle
 * @see dijkstra_shortest_paths() for faster algorithm with non-negative weights
 */
// Note on std::remove_reference_t<G>:
// These templates declare G&& (forwarding reference), so for lvalue arguments G deduces as a
// reference type (e.g. vector<…>&). Writing "const G&" when G is already a reference triggers
// reference collapsing: const (vector<…>&) & → vector<…>& — the const is silently discarded.
// We use std::remove_reference_t<G> in WF default types, invoke_result_t, and concept
// constraints so that "const std::remove_reference_t<G>&" always means a true const ref.
// Default lambdas use "const auto&" instead of "const G&" to sidestep the issue entirely.
template <
      adjacency_list G,
      input_range    Sources,
      class DistanceFn,
      class PredecessorFn,
      class WF = function<distance_fn_value_t<DistanceFn, G>(const std::remove_reference_t<G>&, const edge_t<G>&)>,
      class Visitor = empty_visitor,
      class Compare = less<distance_fn_value_t<DistanceFn, G>>,
      class Combine = plus<distance_fn_value_t<DistanceFn, G>>>
requires distance_function_for<DistanceFn, G> &&                                //
         predecessor_function_for<PredecessorFn, G> &&                          //
         convertible_to<range_value_t<Sources>, vertex_id_t<G>> &&              //
         basic_edge_weight_function<G, WF, distance_fn_value_t<DistanceFn, G>, Compare, Combine>
[[nodiscard]] constexpr optional<vertex_id_t<G>> bellman_ford_shortest_paths(
      G&&             g,
      const Sources&  sources,
      DistanceFn&&    distance,
      PredecessorFn&& predecessor,
      WF&&            weight =
            [](const auto&, const edge_t<G>& uv) {
              return distance_fn_value_t<DistanceFn, G>(1);
            }, // default weight(g, uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<distance_fn_value_t<DistanceFn, G>>(),
      Combine&& combine = plus<distance_fn_value_t<DistanceFn, G>>()) {
  using graph_type    = std::remove_reference_t<G>;
  using id_type       = vertex_id_t<graph_type>;
  using DistanceValue = distance_fn_value_t<DistanceFn, G>;
  using weight_type   = invoke_result_t<WF, const graph_type&, edge_t<graph_type>>;
  using return_type   = optional<vertex_id_t<graph_type>>;

  constexpr auto zero     = shortest_path_zero<DistanceValue>();
  constexpr auto infinite = shortest_path_infinite_distance<DistanceValue>();

  // relaxing the target is the function of reducing the distance from the source to the target
  auto relax_target = [&g, &predecessor, &distance, &compare, &combine] //
        (const edge_t<graph_type>& e, const vertex_id_t<graph_type>& uid, const weight_type& w_e) -> bool {
    const id_type       vid = target_id(g, e);
    const DistanceValue d_u = distance(g, uid);
    if (d_u == infinite)
      return false; // Cannot relax via unreachable vertex (also guards against overflow)
    const DistanceValue d_v = distance(g, vid);

    if (compare(combine(d_u, w_e), d_v)) {
      distance(g, vid) = combine(d_u, w_e);
      if constexpr (!is_null_predecessor_fn_v<PredecessorFn>) {
        predecessor(g, vid) = uid;
      }
      return true;
    }
    return false;
  };

  // Seed the queue with the initial vertices
  for (auto&& seed_id : sources) {
    auto seed_it = find_vertex(g, seed_id);
    if (seed_it == std::ranges::end(vertices(g))) {
      throw std::out_of_range(
            std::format("bellman_ford_shortest_paths: source vertex id '{}' is out of range", seed_id));
    }

    distance(g, seed_id) = zero; // mark source as discovered
    if constexpr (has_on_discover_vertex<graph_type, Visitor>) {
      visitor.on_discover_vertex(g, *seed_it);
    } else if constexpr (has_on_discover_vertex_id<graph_type, Visitor>) {
      visitor.on_discover_vertex(g, seed_id);
    }
  }

  // Evaluate the shortest paths
  const size_t N                         = num_vertices(g);
  bool         at_least_one_edge_relaxed = false;
  for (size_t k = 0; k < N; ++k) {
    at_least_one_edge_relaxed = false;
    for (auto&& [uid, vid, uv, uv_w] : views::edgelist(g, weight)) {
      if constexpr (has_on_examine_edge<graph_type, Visitor>) {
        visitor.on_examine_edge(g, uv);
      }
      if (relax_target(uv, uid, uv_w)) {
        at_least_one_edge_relaxed = true;
        if constexpr (has_on_edge_relaxed<graph_type, Visitor>) {
          visitor.on_edge_relaxed(g, uv);
        }
      } else if constexpr (has_on_edge_not_relaxed<graph_type, Visitor>) {
        visitor.on_edge_not_relaxed(g, uv);
      }
    }
    if (!at_least_one_edge_relaxed)
      break;
  }

  // Check for negative weight cycles
  if (at_least_one_edge_relaxed) {
    for (auto&& [uid, vid, uv, uv_w] : views::edgelist(g, weight)) {
      if (compare(combine(distance(g, uid), uv_w), distance(g, vid))) {
        if constexpr (!is_null_predecessor_fn_v<PredecessorFn>) {
          predecessor(g, vid) = uid; // close the cycle
        }
        if constexpr (has_on_edge_not_minimized<graph_type, Visitor>) {
          visitor.on_edge_not_minimized(g, uv);
        }
        return return_type(uid);
      } else {
        if constexpr (has_on_edge_minimized<graph_type, Visitor>) {
          visitor.on_edge_minimized(g, uv);
        }
      }
    }
  }

  return return_type();
}

/**
 * @brief Single-source shortest paths using Bellman-Ford algorithm.
 * 
 * Convenience overload for single source vertex. See multi-source version for full documentation.
 * 
 * @param source Single source vertex ID instead of range.
 * 
 * @return optional<vertex_id_t<G>> indicating negative cycle detection.
 * 
 * @see bellman_ford_shortest_paths(G&&, const Sources&, Distances&, Predecessors&, WF&&, Visitor&&, Compare&&, Combine&&)
 */
template <
      adjacency_list G,
      class DistanceFn,
      class PredecessorFn,
      class WF = function<distance_fn_value_t<DistanceFn, G>(const std::remove_reference_t<G>&, const edge_t<G>&)>,
      class Visitor = empty_visitor,
      class Compare = less<distance_fn_value_t<DistanceFn, G>>,
      class Combine = plus<distance_fn_value_t<DistanceFn, G>>>
requires distance_function_for<DistanceFn, G> &&                                //
         predecessor_function_for<PredecessorFn, G> &&                          //
         basic_edge_weight_function<G, WF, distance_fn_value_t<DistanceFn, G>, Compare, Combine>
[[nodiscard]] constexpr optional<vertex_id_t<G>> bellman_ford_shortest_paths(
      G&&                   g,
      const vertex_id_t<G>& source,
      DistanceFn&&          distance,
      PredecessorFn&&       predecessor,
      WF&&                  weight =
            [](const auto&, const edge_t<G>& uv) {
              return distance_fn_value_t<DistanceFn, G>(1);
            }, // default weight(g, uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<distance_fn_value_t<DistanceFn, G>>(),
      Combine&& combine = plus<distance_fn_value_t<DistanceFn, G>>()) {
  return bellman_ford_shortest_paths(g, subrange(&source, (&source + 1)), distance, predecessor, weight,
                                     forward<Visitor>(visitor), forward<Compare>(compare), forward<Combine>(combine));
}


/**
 * @brief Multi-source shortest distances using Bellman-Ford algorithm (no predecessor tracking).
 * 
 * Computes shortest distances without tracking predecessor information. More efficient when
 * path reconstruction is not needed. Can detect negative weight cycles.
 * 
 * @tparam G            The graph type. Must satisfy adjacency_list concept (index or mapped).
 * @tparam Sources      Input range of source vertex IDs.
 * @tparam Distances    Vertex property map satisfying vertex_property_map_for<Distances,G>. Value type must be arithmetic.
 * @tparam WF           Edge weight function. Defaults to returning 1 for all edges (unweighted).
 * @tparam Visitor      Visitor type with callbacks for algorithm events. Defaults to empty_visitor.
 * @tparam Compare      Comparison function for distance values. Defaults to less<>.
 * @tparam Combine      Function to combine distances and weights. Defaults to plus<>.
 * 
 * @param g            The graph to process.
 * @param sources      Range of source vertex IDs to start from.
 * @param distances    [out] Shortest distances from sources. Must be a vertex property map for G.
 * @param weight       Edge weight function: (const edge_t<G>&) -> Distance.
 * @param visitor      Visitor for algorithm events.
 * @param compare      Distance comparison function: (Distance, Distance) -> bool.
 * @param combine      Distance combination function: (Distance, Weight) -> Distance.
 * 
 * @return optional<vertex_id_t<G>>. Returns empty if no negative cycle. Returns vertex ID in cycle if detected.
 * 
 * **Effects:**
 * - Modifies distances: Sets distances[v] for all vertices v
 * - Does not modify the graph g
 * - Internally uses _null_predecessors to skip predecessor tracking
 * 
 * @see bellman_ford_shortest_paths() for full documentation and complexity analysis.
 * @see find_negative_cycle() to extract cycle vertices (requires predecessor tracking version).
 */
template <
      adjacency_list G,
      input_range    Sources,
      class DistanceFn,
      class WF = function<distance_fn_value_t<DistanceFn, G>(const std::remove_reference_t<G>&, const edge_t<G>&)>,
      class Visitor = empty_visitor,
      class Compare = less<distance_fn_value_t<DistanceFn, G>>,
      class Combine = plus<distance_fn_value_t<DistanceFn, G>>>
requires distance_function_for<DistanceFn, G> &&                                //
         convertible_to<range_value_t<Sources>, vertex_id_t<G>> &&              //
         basic_edge_weight_function<G, WF, distance_fn_value_t<DistanceFn, G>, Compare, Combine>
[[nodiscard]] constexpr optional<vertex_id_t<G>> bellman_ford_shortest_distances(
      G&&            g,
      const Sources& sources,
      DistanceFn&&   distance,
      WF&&           weight =
            [](const auto&, const edge_t<G>& uv) {
              return distance_fn_value_t<DistanceFn, G>(1);
            }, // default weight(g, uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<distance_fn_value_t<DistanceFn, G>>(),
      Combine&& combine = plus<distance_fn_value_t<DistanceFn, G>>()) {
  return bellman_ford_shortest_paths(g, sources, distance, _null_predecessor, forward<WF>(weight),
                                     forward<Visitor>(visitor), forward<Compare>(compare), forward<Combine>(combine));
}

/**
 * @brief Single-source shortest distances using Bellman-Ford algorithm (no predecessor tracking).
 * 
 * Convenience overload for single source vertex without predecessor tracking.
 * 
 * @param source Single source vertex ID instead of range.
 * 
 * @return optional<vertex_id_t<G>> indicating negative cycle detection.
 * 
 * @see bellman_ford_shortest_distances(G&&, const Sources&, Distances&, WF&&, Visitor&&, Compare&&, Combine&&)
 */
template <
      adjacency_list G,
      class DistanceFn,
      class WF = function<distance_fn_value_t<DistanceFn, G>(const std::remove_reference_t<G>&, const edge_t<G>&)>,
      class Visitor = empty_visitor,
      class Compare = less<distance_fn_value_t<DistanceFn, G>>,
      class Combine = plus<distance_fn_value_t<DistanceFn, G>>>
requires distance_function_for<DistanceFn, G> &&                                //
         basic_edge_weight_function<G, WF, distance_fn_value_t<DistanceFn, G>, Compare, Combine>
[[nodiscard]] constexpr optional<vertex_id_t<G>> bellman_ford_shortest_distances(
      G&&                   g,
      const vertex_id_t<G>& source,
      DistanceFn&&          distance,
      WF&&                  weight =
            [](const auto&, const edge_t<G>& uv) {
              return distance_fn_value_t<DistanceFn, G>(1);
            }, // default weight(g, uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<distance_fn_value_t<DistanceFn, G>>(),
      Combine&& combine = plus<distance_fn_value_t<DistanceFn, G>>()) {
  return bellman_ford_shortest_paths(g, subrange(&source, (&source + 1)), distance, _null_predecessor,
                                     forward<WF>(weight), forward<Visitor>(visitor), forward<Compare>(compare),
                                     forward<Combine>(combine));
}

} // namespace graph

#endif // GRAPH_BELLMAN_SHORTEST_PATHS_HPP
