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

#include <ranges>
#include <optional>
#include <stdexcept>
#include <format>

#ifndef GRAPH_BELLMAN_SHORTEST_PATHS_HPP
#  define GRAPH_BELLMAN_SHORTEST_PATHS_HPP

namespace graph {

// Using declarations for CPO-based architecture
using adj_list::index_adjacency_list;
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
template <index_adjacency_list G, forward_range Predecessors, class OutputIterator>
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
      uid          = predecessor[uid];
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
 * @tparam G            The graph type. Must satisfy index_adjacency_list concept.
 * @tparam Sources      Input range of source vertex IDs.
 * @tparam Distances    Random access range for storing distances. Value type must be arithmetic.
 * @tparam Predecessors Random access range for storing predecessor information. Can use _null_predecessors
 *                      if path reconstruction is not needed.
 * @tparam WF           Edge weight function. Defaults to returning 1 for all edges (unweighted).
 * @tparam Visitor      Visitor type with callbacks for algorithm events. Defaults to empty_visitor.
 *                      Visitor calls are optimized away if not used.
 * @tparam Compare      Comparison function for distance values. Defaults to less<>.
 * @tparam Combine      Function to combine distances and weights. Defaults to plus<>.
 * 
 * @param g            The graph to process.
 * @param sources      Range of source vertex IDs to start from.
 * @param distances    [out] Shortest distances from sources. Must be sized >= num_vertices(g).
 * @param predecessor  [out] Predecessor information for path reconstruction. Must be sized >= num_vertices(g).
 * @param weight       Edge weight function: (const edge_t<G>&) -> Distance.
 * @param visitor      Visitor for algorithm events (examine, relax, not_relaxed, minimized, not_minimized).
 * @param compare      Distance comparison function: (Distance, Distance) -> bool.
 * @param combine      Distance combination function: (Distance, Weight) -> Distance.
 * 
 * @return optional<vertex_id_t<G>>. Returns empty if no negative cycle detected. Returns a vertex ID
 *         in the negative cycle if one exists. Use find_negative_cycle() to extract all cycle vertices.
 * 
 * **Complexity:**
 * - Time: O(V * E) - iterates over all edges V times
 * - Space: O(1) auxiliary space (excluding output parameters)
 * 
 * **Mandates:**
 * - G must satisfy index_adjacency_list (integral vertex IDs)
 * - Sources must be input_range with values convertible to vertex_id_t<G>
 * - Distances must be random_access_range with arithmetic value type
 * - Predecessors must be random_access_range with values convertible from vertex_id_t<G>
 * - WF must satisfy basic_edge_weight_function
 * 
 * **Preconditions:**
 * - All source vertices must be valid: source < num_vertices(g) for vector-based containers
 * - distances.size() >= num_vertices(g)
 * - predecessor.size() >= num_vertices(g) (unless using _null_predecessors)
 * - Weight function must not throw or modify graph state
 * 
 * **Postconditions:**
 * - distances[s] == 0 for all sources s
 * - If no negative cycle: For reachable v, distances[v] contains shortest distance from nearest source
 * - If no negative cycle: For reachable v, predecessor[v] contains predecessor in shortest path tree
 * - If negative cycle detected: distances and predecessor may contain intermediate values
 * - For unreachable vertices v: distances[v] == numeric_limits<Distance>::max()
 * 
 * **Effects:**
 * - Modifies distances: Sets distances[v] for all vertices v
 * - Modifies predecessor: Sets predecessor[v] for all processed edges
 * - Does not modify the graph g
 * 
 * **Exception Safety:**
 * Basic guarantee. If an exception is thrown:
 * - Graph g remains unchanged
 * - distances and predecessor may be partially modified (indeterminate state)
 * 
 * **Throws:**
 * - std::out_of_range if a source vertex ID is out of range
 * - std::out_of_range if distances or predecessor are undersized
 * 
 * **Remarks:**
 * - Use Bellman-Ford when: graph has negative weights, need cycle detection, or edges processed sequentially
 * - Use Dijkstra when: all weights non-negative and need better performance O((V+E) log V)
 * - Negative cycle detection: Algorithm performs V iterations. If any edge relaxes on iteration V, a
 *   negative cycle exists. The returned vertex ID can be used with find_negative_cycle() to extract
 *   all vertices in the cycle.
 * - Based on Boost.Graph bellman_ford_shortest_paths implementation
 * 
 * @see find_negative_cycle() to extract vertices in detected negative cycle
 * @see dijkstra_shortest_paths() for faster algorithm with non-negative weights
 */
template <index_adjacency_list G,
          input_range          Sources,
          random_access_range  Distances,
          random_access_range  Predecessors,
          class WF      = function<range_value_t<Distances>(const edge_t<G>&)>,
          class Visitor = empty_visitor,
          class Compare = less<range_value_t<Distances>>,
          class Combine = plus<range_value_t<Distances>>>
requires convertible_to<range_value_t<Sources>, vertex_id_t<G>> &&      //
         is_arithmetic_v<range_value_t<Distances>> &&                   //
         convertible_to<vertex_id_t<G>, range_value_t<Predecessors>> && //
         sized_range<Distances> &&                                      //
         sized_range<Predecessors> &&                                   //
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
[[nodiscard]] constexpr optional<vertex_id_t<G>> bellman_ford_shortest_paths(
      G&&            g,
      const Sources& sources,
      Distances&     distances,
      Predecessors&  predecessor,
      WF&&      weight  = [](const edge_t<G>& uv) { return range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<range_value_t<Distances>>(),
      Combine&& combine = plus<range_value_t<Distances>>()) {
  using id_type       = vertex_id_t<G>;
  using DistanceValue = range_value_t<Distances>;
  using weight_type   = invoke_result_t<WF, edge_t<G>>;
  using return_type   = optional<vertex_id_t<G>>;

  // relaxing the target is the function of reducing the distance from the source to the target
  auto relax_target = [&g, &predecessor, &distances, &compare, &combine] //
        (const edge_t<G>& e, vertex_id_t<G> uid, const weight_type& w_e) -> bool {
    id_type             vid = target_id(g, e);
    const DistanceValue d_u = distances[static_cast<size_t>(uid)];
    const DistanceValue d_v = distances[static_cast<size_t>(vid)];

    if (compare(combine(d_u, w_e), d_v)) {
      distances[static_cast<size_t>(vid)] = combine(d_u, w_e);
      if constexpr (!is_same_v<Predecessors, _null_range_type>) {
        predecessor[static_cast<size_t>(vid)] = uid;
      }
      return true;
    }
    return false;
  };

  if (size(distances) < size(vertices(g))) {
    throw std::out_of_range(
          std::format("bellman_ford_shortest_paths: size of distances of {} is less than the number of vertices {}",
                      size(distances), size(vertices(g))));
  }

  if constexpr (!is_same_v<Predecessors, _null_range_type>) {
    if (size(predecessor) < size(vertices(g))) {
      throw std::out_of_range(
            std::format("bellman_ford_shortest_paths: size of predecessor of {} is less than the number of vertices {}",
                        size(predecessor), size(vertices(g))));
    }
  }

  constexpr auto zero     = shortest_path_zero<DistanceValue>();
  constexpr auto infinite = shortest_path_infinite_distance<DistanceValue>();

  const id_type N = static_cast<id_type>(num_vertices(g));

  // Seed the queue with the initial vertice(s)
  for (auto&& source : sources) {
    if (source >= N || source < 0) {
      throw std::out_of_range(
            std::format("bellman_ford_shortest_paths: source vertex id '{}' is out of range", source));
    }
    distances[static_cast<size_t>(source)] = zero; // mark source as discovered
    if constexpr (has_on_discover_vertex<G, Visitor>) {
      visitor.on_discover_vertex(g, *find_vertex(g, source));
    }
  }

  // Evaluate the shortest paths
  bool at_least_one_edge_relaxed = false;
  for (id_type k = 0; k < N; ++k) {
    at_least_one_edge_relaxed = false;
    for (auto&& [uv, w] : views::edgelist(g, weight)) {
      id_type uid = source_id(g, uv);
      id_type vid = target_id(g, uv);
      if constexpr (has_on_examine_edge<G, Visitor>) {
        visitor.on_examine_edge(g, uv);
      }
      if (relax_target(uv, uid, w)) {
        at_least_one_edge_relaxed = true;
        if constexpr (has_on_edge_relaxed<G, Visitor>) {
          visitor.on_edge_relaxed(g, uv);
        }
      } else if constexpr (has_on_edge_not_relaxed<G, Visitor>) {
        visitor.on_edge_not_relaxed(g, uv);
      }
    }
    if (!at_least_one_edge_relaxed)
      break;
  }

  // Check for negative weight cycles
  if (at_least_one_edge_relaxed) {
    for (auto&& [uv, w] : views::edgelist(g, weight)) {
      id_type uid = source_id(g, uv);
      id_type vid = target_id(g, uv);
      if (compare(combine(distances[uid], w), distances[vid])) {
        if constexpr (!is_same_v<Predecessors, _null_range_type>) {
          predecessor[vid] = uid; // close the cycle
        }
        if constexpr (has_on_edge_not_minimized<G, Visitor>) {
          visitor.on_edge_not_minimized(g, uv);
        }
        return return_type(uid);
      } else {
        if constexpr (has_on_edge_minimized<G, Visitor>) {
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
template <index_adjacency_list G,
          random_access_range  Distances,
          random_access_range  Predecessors,
          class WF      = function<range_value_t<Distances>(const edge_t<G>&)>,
          class Visitor = empty_visitor,
          class Compare = less<range_value_t<Distances>>,
          class Combine = plus<range_value_t<Distances>>>
requires is_arithmetic_v<range_value_t<Distances>> &&                   //
         convertible_to<vertex_id_t<G>, range_value_t<Predecessors>> && //
         sized_range<Distances> &&                                      //
         sized_range<Predecessors> &&                                   //
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
[[nodiscard]] constexpr optional<vertex_id_t<G>> bellman_ford_shortest_paths(
      G&&            g,
      vertex_id_t<G> source,
      Distances&     distances,
      Predecessors&  predecessor,
      WF&&      weight  = [](const edge_t<G>& uv) { return range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<range_value_t<Distances>>(),
      Combine&& combine = plus<range_value_t<Distances>>()) {
  return bellman_ford_shortest_paths(g, subrange(&source, (&source + 1)), distances, predecessor, weight,
                                     forward<Visitor>(visitor), forward<Compare>(compare), forward<Combine>(combine));
}


/**
 * @brief Multi-source shortest distances using Bellman-Ford algorithm (no predecessor tracking).
 * 
 * Computes shortest distances without tracking predecessor information. More efficient when
 * path reconstruction is not needed. Can detect negative weight cycles.
 * 
 * @tparam G            The graph type. Must satisfy index_adjacency_list concept.
 * @tparam Sources      Input range of source vertex IDs.
 * @tparam Distances    Random access range for storing distances. Value type must be arithmetic.
 * @tparam WF           Edge weight function. Defaults to returning 1 for all edges (unweighted).
 * @tparam Visitor      Visitor type with callbacks for algorithm events. Defaults to empty_visitor.
 * @tparam Compare      Comparison function for distance values. Defaults to less<>.
 * @tparam Combine      Function to combine distances and weights. Defaults to plus<>.
 * 
 * @param g            The graph to process.
 * @param sources      Range of source vertex IDs to start from.
 * @param distances    [out] Shortest distances from sources. Must be sized >= num_vertices(g).
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
template <index_adjacency_list G,
          input_range          Sources,
          random_access_range  Distances,
          class WF      = function<range_value_t<Distances>(const edge_t<G>&)>,
          class Visitor = empty_visitor,
          class Compare = less<range_value_t<Distances>>,
          class Combine = plus<range_value_t<Distances>>>
requires convertible_to<range_value_t<Sources>, vertex_id_t<G>> && //
         is_arithmetic_v<range_value_t<Distances>> &&              //
         sized_range<Distances> &&                                 //
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
[[nodiscard]] constexpr optional<vertex_id_t<G>> bellman_ford_shortest_distances(
      G&&            g,
      const Sources& sources,
      Distances&     distances,
      WF&&      weight  = [](const edge_t<G>& uv) { return range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<range_value_t<Distances>>(),
      Combine&& combine = plus<range_value_t<Distances>>()) {
  return bellman_ford_shortest_paths(g, sources, distances, _null_predecessors, forward<WF>(weight),
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
template <index_adjacency_list G,
          random_access_range  Distances,
          class WF      = function<range_value_t<Distances>(const edge_t<G>&)>,
          class Visitor = empty_visitor,
          class Compare = less<range_value_t<Distances>>,
          class Combine = plus<range_value_t<Distances>>>
requires is_arithmetic_v<range_value_t<Distances>> && //
         sized_range<Distances> &&                    //
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
[[nodiscard]] constexpr optional<vertex_id_t<G>> bellman_ford_shortest_distances(
      G&&            g,
      vertex_id_t<G> source,
      Distances&     distances,
      WF&&      weight  = [](const edge_t<G>& uv) { return range_value_t<Distances>(1); }, // default weight(uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<range_value_t<Distances>>(),
      Combine&& combine = plus<range_value_t<Distances>>()) {
  return bellman_ford_shortest_paths(g, subrange(&source, (&source + 1)), distances, _null_predecessors,
                                     forward<WF>(weight), forward<Visitor>(visitor), forward<Compare>(compare),
                                     forward<Combine>(combine));
}

} // namespace graph

#endif // GRAPH_BELLMAN_SHORTEST_PATHS_HPP
