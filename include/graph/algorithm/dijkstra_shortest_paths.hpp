/**
 * @file dijkstra_shortest_paths.hpp
 * 
 * @brief Single-Source & multi-source shortest paths & shortest distances algorithms using Dijkstra's algorithm.
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
#include "graph/algorithm/traversal_common.hpp"

#include <queue>
#include <vector>
#include <ranges>
#include <format>

#ifndef GRAPH_DIJKSTRA_SHORTEST_PATHS_HPP
#  define GRAPH_DIJKSTRA_SHORTEST_PATHS_HPP

namespace graph {

// Import CPOs and types for use in algorithms
using adj_list::vertices;
using adj_list::num_vertices;
using adj_list::find_vertex;
using adj_list::target_id;
using adj_list::vertex_id_t;
using adj_list::edge_t;
using adj_list::index_adjacency_list;

/**
 * @brief Multi-source shortest paths using Dijkstra's algorithm.
 * 
 * Finds shortest paths from one or more source vertices to all other vertices in a weighted graph
 * with non-negative edge weights. Supports custom weight functions, comparison operators, and 
 * visitor callbacks for algorithm events.
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
 * @param visitor      Visitor for algorithm events (discover, examine, relax, finish).
 * @param compare      Distance comparison function: (Distance, Distance) -> bool.
 * @param combine      Distance combination function: (Distance, Weight) -> Distance.
 * 
 * @return void. Results are stored in the distances and predecessor output parameters.
 * 
 * **Complexity:**
 * - Time: O((V + E) log V) using binary heap priority queue
 * - Space: O(V) for priority queue and internal bookkeeping
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
 * - All edge weights must be non-negative
 * - Weight function must not throw or modify graph state
 * 
 * **Postconditions:**
 * - distances[s] == 0 for all sources s
 * - For reachable vertices v: distances[v] contains shortest distance from nearest source
 * - For reachable vertices v: predecessor[v] contains predecessor in shortest path tree
 * - For unreachable vertices v: distances[v] == numeric_limits<Distance>::max()
 * 
 * **Effects:**
 * - Modifies distances: Sets distances[v] for all vertices v
 * - Modifies predecessor: Sets predecessor[v] for all reachable vertices
 * - Does not modify the graph g
 * 
 * **Mandates:**
 * - G must satisfy index_adjacency_list (integral vertex IDs)
 * - Sources must be input_range with values convertible to vertex_id_t<G>
 * - Distances must be random_access_range with arithmetic value type
 * - Predecessors must be random_access_range with values convertible from vertex_id_t<G>
 * - WF must satisfy basic_edge_weight_function
 * 
 * **Exception Safety:**
 * Basic guarantee. If an exception is thrown:
 * - Graph g remains unchanged
 * - distances and predecessor may be partially modified (indeterminate state)
 * 
 * **Throws:**
 * - std::out_of_range if a source vertex ID is out of range
 * - std::out_of_range if distances or predecessor are undersized
 * - std::out_of_range if a negative edge weight is encountered (for signed weight types)
 * - std::logic_error if internal invariant violation detected
 * 
 * **Remarks:**
 * - Uses std::priority_queue with lazy deletion (vertices can be re-inserted)
 * - For unweighted graphs, use default weight function (equivalent to BFS)
 * - For single target, consider A* with admissible heuristic
 * - Implementation based on Boost.Graph dijkstra_shortest_paths_no_init
 */
// Note on std::remove_reference_t<G>:
// These templates declare G&& (forwarding reference), so for lvalue arguments G deduces as a
// reference type (e.g. vector<…>&). Writing "const G&" when G is already a reference triggers
// reference collapsing: const (vector<…>&) & → vector<…>& — the const is silently discarded.
// We use std::remove_reference_t<G> in WF default types, invoke_result_t, and concept
// constraints so that "const std::remove_reference_t<G>&" always means a true const ref.
// Default lambdas use "const auto&" instead of "const G&" to sidestep the issue entirely.
template <index_adjacency_list G,
          input_range          Sources,
          random_access_range  Distances,
          random_access_range  Predecessors,
          class WF      = function<range_value_t<Distances>(const std::remove_reference_t<G>&, const edge_t<G>&)>,
          class Visitor = empty_visitor,
          class Compare = less<range_value_t<Distances>>,
          class Combine = plus<range_value_t<Distances>>>
requires convertible_to<range_value_t<Sources>, vertex_id_t<G>> && //
         is_arithmetic_v<range_value_t<Distances>> &&              //
         sized_range<Distances> &&                                 //
         sized_range<Predecessors> &&                              //
         convertible_to<vertex_id_t<G>, range_value_t<Predecessors>> &&
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
constexpr void dijkstra_shortest_paths(
      G&&            g,
      const Sources& sources,
      Distances&     distances,
      Predecessors&  predecessor,
      WF&&           weight  = [](const auto&,
                       const edge_t<G>& uv) { return range_value_t<Distances>(1); }, // default weight(g, uv) -> 1
      Visitor&&      visitor = empty_visitor(),
      Compare&&      compare = less<range_value_t<Distances>>(),
      Combine&&      combine = plus<range_value_t<Distances>>()) {
  using id_type       = vertex_id_t<G>;
  using distance_type = range_value_t<Distances>;
  using weight_type   = invoke_result_t<WF, const std::remove_reference_t<G>&, edge_t<G>>;

  // relaxing the target is the function of reducing the distance from the source to the target
  auto relax_target = [&g, &predecessor, &distances, &compare, &combine] //
        (const edge_t<G>& e, vertex_id_t<G> uid, const weight_type& w_e) -> bool {
    const id_type       vid = target_id(g, e);
    const distance_type d_u = distances[static_cast<size_t>(uid)];
    const distance_type d_v = distances[static_cast<size_t>(vid)];

    if (compare(combine(d_u, w_e), d_v)) {
      distances[static_cast<size_t>(vid)] = combine(d_u, w_e);
      if constexpr (!is_same_v<Predecessors, _null_range_type>) {
        predecessor[static_cast<size_t>(vid)] = uid;
      }
      return true;
    }
    return false;
  };

  if (size(distances) < num_vertices(g)) {
    throw std::out_of_range(
          std::format("dijkstra_shortest_paths: size of distances of {} is less than the number of vertices {}",
                      size(distances), num_vertices(g)));
  }
  if constexpr (!is_same_v<Predecessors, _null_range_type>) {
    if (size(predecessor) < num_vertices(g)) {
      throw std::out_of_range(
            std::format("dijkstra_shortest_paths: size of predecessor of {} is less than the number of vertices {}",
                        size(predecessor), num_vertices(g)));
    }
  }

  constexpr auto zero     = shortest_path_zero<distance_type>();
  constexpr auto infinite = shortest_path_infinite_distance<distance_type>();

  const id_type N = static_cast<id_type>(num_vertices(g));

  auto qcompare = [&distances](id_type a, id_type b) {
    return distances[static_cast<size_t>(a)] > distances[static_cast<size_t>(b)];
  };
  using Queue = std::priority_queue<id_type, std::vector<id_type>, decltype(qcompare)>;
  Queue queue(qcompare);

  // (The optimizer removes this loop if on_initialize_vertex() is empty.)
  if constexpr (has_on_initialize_vertex<G, Visitor>) {
    for (id_type uid = 0; uid < N; ++uid) {
      visitor.on_initialize_vertex(g, *find_vertex(g, uid));
    }
  } else if constexpr (has_on_initialize_vertex_id<G, Visitor>) {
    for (id_type uid = 0; uid < N; ++uid) {
      visitor.on_initialize_vertex(g, uid);
    }
  }

  // Seed the queue with the initial vertice(s)
  for (auto&& source : sources) {
    if (source >= N || source < 0) {
      throw std::out_of_range(std::format("dijkstra_shortest_paths: source vertex id '{}' is out of range", source));
    }
    queue.push(source);
    distances[static_cast<size_t>(source)] = zero; // mark source as discovered
    if constexpr (has_on_discover_vertex<G, Visitor>) {
      visitor.on_discover_vertex(g, *find_vertex(g, source));
    } else if constexpr (has_on_discover_vertex_id<G, Visitor>) {
      visitor.on_discover_vertex(g, source);
    }
  }

  // Main loop to process the queue
  while (!queue.empty()) {
    const id_type uid = queue.top();
    queue.pop();
    if constexpr (has_on_examine_vertex<G, Visitor>) {
      visitor.on_examine_vertex(g, *find_vertex(g, uid));
    } else if constexpr (has_on_examine_vertex_id<G, Visitor>) {
      visitor.on_examine_vertex(g, uid);
    }

    // Process all outgoing edges from the current vertex
    for (auto&& [vid, uv, w] : views::incidence(g, *find_vertex(g, uid), weight)) {
      if constexpr (has_on_examine_edge<G, Visitor>) {
        visitor.on_examine_edge(g, uv);
      }

      // Negative weights are not allowed for Dijkstra's algorithm
      if constexpr (is_signed_v<weight_type>) {
        if (w < zero) {
          throw std::out_of_range(
                std::format("dijkstra_shortest_paths: invalid negative edge weight of '{}' encountered", w));
        }
      }

      const bool is_neighbor_undiscovered = (distances[static_cast<size_t>(vid)] == infinite);
      const bool was_edge_relaxed         = relax_target(uv, uid, w);

      if (is_neighbor_undiscovered) {
        // tree_edge
        if (was_edge_relaxed) {
          if constexpr (has_on_edge_relaxed<G, Visitor>) {
            visitor.on_edge_relaxed(g, uv);
          }
          if constexpr (has_on_discover_vertex<G, Visitor>) {
            visitor.on_discover_vertex(g, *find_vertex(g, vid));
          } else if constexpr (has_on_discover_vertex_id<G, Visitor>) {
            visitor.on_discover_vertex(g, vid);
          }
          queue.push(vid);
        } else {
          // This is an indicator of a bug in the algorithm and should be investigated.
          throw std::logic_error(
                "dijkstra_shortest_paths: unexpected state where an edge to a new vertex was not relaxed");
        }
      } else {
        // non-tree edge
        if (was_edge_relaxed) {
          if constexpr (has_on_edge_relaxed<G, Visitor>) {
            visitor.on_edge_relaxed(g, uv);
          }
          queue.push(vid); // re-enqueue vid to re-evaluate its neighbors with a shorter path
        } else {
          if constexpr (has_on_edge_not_relaxed<G, Visitor>) {
            visitor.on_edge_not_relaxed(g, uv);
          }
        }
      }
    }

    // Note: while we *think* we're done with this vertex, we may not be. If the graph is unbalanced
    // and another path to this vertex has a lower accumulated weight, we'll process it again.
    // A consequence is that examine_vertex could be called twice (or more) on the same vertex.
    if constexpr (has_on_finish_vertex<G, Visitor>) {
      visitor.on_finish_vertex(g, *find_vertex(g, uid));
    } else if constexpr (has_on_finish_vertex_id<G, Visitor>) {
      visitor.on_finish_vertex(g, uid);
    }
  } // while(!queue.empty())
}

/**
 * @brief Single-source shortest paths using Dijkstra's algorithm.
 * 
 * Convenience overload for single source vertex. See multi-source version for full documentation.
 * 
 * @param source Single source vertex ID instead of range.
 * 
 * @see dijkstra_shortest_paths(G&&, const Sources&, Distances&, Predecessors&, WF&&, Visitor&&, Compare&&, Combine&&)
 */
template <index_adjacency_list G,
          random_access_range  Distances,
          random_access_range  Predecessors,
          class WF      = function<range_value_t<Distances>(const std::remove_reference_t<G>&, const edge_t<G>&)>,
          class Visitor = empty_visitor,
          class Compare = less<range_value_t<Distances>>,
          class Combine = plus<range_value_t<Distances>>>
requires is_arithmetic_v<range_value_t<Distances>> && //
         sized_range<Distances> &&                    //
         sized_range<Predecessors> &&                 //
         convertible_to<vertex_id_t<G>, range_value_t<Predecessors>> &&
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
constexpr void dijkstra_shortest_paths(
      G&&            g,
      vertex_id_t<G> source,
      Distances&     distances,
      Predecessors&  predecessor,
      WF&&           weight  = [](const auto&,
                       const edge_t<G>& uv) { return range_value_t<Distances>(1); }, // default weight(g, uv) -> 1
      Visitor&&      visitor = empty_visitor(),
      Compare&&      compare = less<range_value_t<Distances>>(),
      Combine&&      combine = plus<range_value_t<Distances>>()) {
  dijkstra_shortest_paths(g, subrange(&source, (&source + 1)), distances, predecessor, weight,
                          forward<Visitor>(visitor), forward<Compare>(compare), forward<Combine>(combine));
}

/**
 * @brief Multi-source shortest distances using Dijkstra's algorithm (no predecessor tracking).
 * 
 * Computes shortest distances without tracking predecessor information. More efficient when
 * path reconstruction is not needed.
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
 * @param visitor      Visitor for algorithm events (discover, examine, relax, finish).
 * @param compare      Distance comparison function: (Distance, Distance) -> bool.
 * @param combine      Distance combination function: (Distance, Weight) -> Distance.
 * 
 * @return void. Results are stored in the distances output parameter.
 * 
 * **Effects:**
 * - Modifies distances: Sets distances[v] for all vertices v
 * - Does not modify the graph g
 * - Internally uses _null_predecessors to skip predecessor tracking
 * 
 * @see dijkstra_shortest_paths() for full documentation and complexity analysis.
 */
template <index_adjacency_list G,
          input_range          Sources,
          random_access_range  Distances,
          class WF      = function<range_value_t<Distances>(const std::remove_reference_t<G>&, const edge_t<G>&)>,
          class Visitor = empty_visitor,
          class Compare = less<range_value_t<Distances>>,
          class Combine = plus<range_value_t<Distances>>>
requires convertible_to<range_value_t<Sources>, vertex_id_t<G>> && //
         sized_range<Distances> &&                                 //
         is_arithmetic_v<range_value_t<Distances>> &&              //
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
constexpr void dijkstra_shortest_distances(
      G&&            g,
      const Sources& sources,
      Distances&     distances,
      WF&&           weight  = [](const auto&,
                       const edge_t<G>& uv) { return range_value_t<Distances>(1); }, // default weight(g, uv) -> 1
      Visitor&&      visitor = empty_visitor(),
      Compare&&      compare = less<range_value_t<Distances>>(),
      Combine&&      combine = plus<range_value_t<Distances>>()) {
  dijkstra_shortest_paths(g, sources, distances, _null_predecessors, forward<WF>(weight), forward<Visitor>(visitor),
                          forward<Compare>(compare), forward<Combine>(combine));
}

/**
 * @brief Single-source shortest distances using Dijkstra's algorithm (no predecessor tracking).
 * 
 * Convenience overload for single source vertex without predecessor tracking.
 * 
 * @param source Single source vertex ID instead of range.
 * 
 * @see dijkstra_shortest_distances(G&&, const Sources&, Distances&, WF&&, Visitor&&, Compare&&, Combine&&)
 */
template <index_adjacency_list G,
          random_access_range  Distances,
          class WF      = function<range_value_t<Distances>(const std::remove_reference_t<G>&, const edge_t<G>&)>,
          class Visitor = empty_visitor,
          class Compare = less<range_value_t<Distances>>,
          class Combine = plus<range_value_t<Distances>>>
requires is_arithmetic_v<range_value_t<Distances>> && //
         sized_range<Distances> &&                    //
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
constexpr void dijkstra_shortest_distances(
      G&&            g,
      vertex_id_t<G> source,
      Distances&     distances,
      WF&&           weight  = [](const auto&,
                       const edge_t<G>& uv) { return range_value_t<Distances>(1); }, // default weight(g, uv) -> 1
      Visitor&&      visitor = empty_visitor(),
      Compare&&      compare = less<range_value_t<Distances>>(),
      Combine&&      combine = plus<range_value_t<Distances>>()) {
  dijkstra_shortest_paths(g, subrange(&source, (&source + 1)), distances, _null_predecessors, forward<WF>(weight),
                          forward<Visitor>(visitor), forward<Compare>(compare), forward<Combine>(combine));
}

} // namespace graph

#endif // GRAPH_DIJKSTRA_SHORTEST_PATHS_HPP
