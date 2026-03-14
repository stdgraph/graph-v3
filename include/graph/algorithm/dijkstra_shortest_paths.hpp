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
#include "graph/adj_list/vertex_property_map.hpp"

#include <queue>
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
using adj_list::vertex_t;
using adj_list::vertex_id;
using adj_list::edge_t;
using adj_list::adjacency_list;
using adj_list::index_vertex_range;

/**
 * @brief Multi-source shortest paths using Dijkstra's algorithm.
 * 
 * Finds shortest paths from one or more source vertices to all other vertices in a weighted graph
 * with non-negative edge weights. Supports custom weight functions, comparison operators, and 
 * visitor callbacks for algorithm events.
 * 
 * @tparam G            The graph type. Must satisfy adjacency_list concept.
 * @tparam Sources      Input range of source vertex IDs.
 * @tparam Distances    Container for storing distances (vector for index graphs, unordered_map for mapped).
 * @tparam Predecessors Container for storing predecessor information. Can use _null_predecessors
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
 * - G must satisfy adjacency_list (index or mapped vertex containers)
 * - Sources must be input_range with values convertible to vertex_id_t<G>
 * - Distances must satisfy vertex_property_map_for<Distances, G> (subscriptable by vertex_id_t<G>) with arithmetic value type
 * - Predecessors must satisfy vertex_property_map_for<Predecessors, G> (or be _null_predecessors)
 * - WF must satisfy basic_edge_weight_function
 * 
 * **Preconditions:**
 * - All source vertices must be valid vertex IDs in the graph
 * - For index graphs: distances.size() >= num_vertices(g)
 * - For index graphs: predecessor.size() >= num_vertices(g) (unless using _null_predecessors)
 * - For mapped graphs: distances and predecessors must be eagerly initialized for all vertices
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
template <
      adjacency_list G,
      input_range    Sources,
      class Distances,
      class Predecessors,
      class WF = function<vertex_property_map_value_t<Distances>(const std::remove_reference_t<G>&, const edge_t<G>&)>,
      class Visitor = empty_visitor,
      class Compare = less<vertex_property_map_value_t<Distances>>,
      class Combine = plus<vertex_property_map_value_t<Distances>>>
requires vertex_property_map_for<Distances, G> &&                                       //
         (is_null_range_v<Predecessors> || vertex_property_map_for<Predecessors, G>) && //
         convertible_to<range_value_t<Sources>, vertex_id_t<G>> &&                      //
         is_arithmetic_v<vertex_property_map_value_t<Distances>> &&                     //
         basic_edge_weight_function<G, WF, vertex_property_map_value_t<Distances>, Compare, Combine>
constexpr void dijkstra_shortest_paths(
      G&&            g,
      const Sources& sources,
      Distances&     distances,
      Predecessors&  predecessor,
      WF&&           weight =
            [](const auto&, const edge_t<G>& uv) {
              return vertex_property_map_value_t<Distances>(1);
            }, // default weight(g, uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<vertex_property_map_value_t<Distances>>(),
      Combine&& combine = plus<vertex_property_map_value_t<Distances>>()) {
  using graph_type    = std::remove_reference_t<G>;
  using id_type       = vertex_id_t<graph_type>;
  using distance_type = vertex_property_map_value_t<Distances>;
  using weight_type   = invoke_result_t<WF, const graph_type&, edge_t<graph_type>>;

  constexpr auto zero     = shortest_path_zero<distance_type>();
  constexpr auto infinite = shortest_path_infinite_distance<distance_type>();

  // relaxing the target is the function of reducing the distance from the source to the target
  auto relax_target = [&g, &predecessor, &distances, &compare, &combine, &weight] //
        (const edge_t<graph_type>& uv, const vertex_id_t<graph_type>& uid) -> bool {
    const id_type       vid  = target_id(g, uv);
    const distance_type d_u  = distances[uid];
    const distance_type d_v  = distances[vid];
    const weight_type   w_uv = weight(g, uv);

    // Negative weights are not allowed for Dijkstra's algorithm
    if constexpr (is_signed_v<weight_type>) {
      if (w_uv < zero) {
        throw std::out_of_range(
              std::format("dijkstra_shortest_paths: invalid negative edge weight of '{}' encountered", w_uv));
      }
    }

    if (compare(combine(d_u, w_uv), d_v)) {
      distances[vid] = combine(d_u, w_uv);
      if constexpr (!is_null_range_v<Predecessors>) {
        predecessor[vid] = uid;
      }
      return true;
    }
    return false;
  };

  // Validate preconditions: source vertices must be valid, distances and predecessor must be sized appropriately
  if constexpr (index_vertex_range<graph_type>) {
    if (size(distances) < num_vertices(g)) {
      throw std::out_of_range(
            std::format("dijkstra_shortest_paths: size of distances of {} is less than the number of vertices {}",
                        size(distances), num_vertices(g)));
    }
    if constexpr (!is_null_range_v<Predecessors>) {
      if (size(predecessor) < num_vertices(g)) {
        throw std::out_of_range(
              std::format("dijkstra_shortest_paths: size of predecessor of {} is less than the number of vertices {}",
                          size(predecessor), num_vertices(g)));
      }
    }
  }

  // Define and initialize the priority queue for Dijkstra's algorithm. We use a min-heap based on distance.
  struct weighted_vertex {
    vertex_t<graph_type> vertex_desc = {};
    distance_type        weight      = distance_type();
  };
  auto qcompare = [&compare](const weighted_vertex& a, const weighted_vertex& b) {
    return compare(b.weight, a.weight); // min-heap: pop lowest weight first
  };
  using Queue = std::priority_queue<weighted_vertex, std::vector<weighted_vertex>, decltype(qcompare)>;
  Queue queue(qcompare);

  // (The optimizer removes this loop if on_initialize_vertex() is empty.)
  if constexpr (has_on_initialize_vertex<graph_type, Visitor> || has_on_initialize_vertex_id<graph_type, Visitor>) {
    for (auto&& [uid, u] : views::vertexlist(g)) {
      if constexpr (has_on_initialize_vertex<graph_type, Visitor>) {
        visitor.on_initialize_vertex(g, u);
      } else if constexpr (has_on_initialize_vertex_id<graph_type, Visitor>) {
        visitor.on_initialize_vertex(g, uid);
      }
    }
  }

  // Seed the queue with the initial vertice(s)
  for (auto&& seed_id : sources) {
    auto seed_it = find_vertex(g, seed_id);
    if (seed_it == std::ranges::end(vertices(g))) {
      throw std::out_of_range(std::format("dijkstra_shortest_paths: source vertex id '{}' is out of range", seed_id));
    }
    vertex_t<graph_type> seed = *seed_it;

    distances[seed_id] = zero; // mark seed_id as discovered
    queue.push({seed, zero});
    if constexpr (has_on_discover_vertex<graph_type, Visitor>) {
      visitor.on_discover_vertex(g, seed);
    } else if constexpr (has_on_discover_vertex_id<graph_type, Visitor>) {
      visitor.on_discover_vertex(g, seed_id);
    }
  }

  // Main loop to process the queue
  while (!queue.empty()) {
    auto [u, w] = queue.top();
    queue.pop();
    const id_type uid = vertex_id(g, u);
    if constexpr (has_on_examine_vertex<graph_type, Visitor>) {
      visitor.on_examine_vertex(g, u);
    } else     if constexpr (has_on_examine_vertex_id<graph_type, Visitor>) {
      visitor.on_examine_vertex(g, uid);
    }

    // Process all outgoing edges from the current vertex
    for (auto&& [vid, uv] : views::incidence(g, u)) {
      if constexpr (has_on_examine_edge<graph_type, Visitor>) {
        visitor.on_examine_edge(g, uv);
      }

      const bool is_neighbor_undiscovered = (distances[vid] == infinite);
      const bool was_edge_relaxed         = relax_target(uv, uid);

      if (is_neighbor_undiscovered) {
        vertex_t<graph_type> v = *find_vertex(g, vid); // find_vertex must succeed since vid is a valid vertex ID
        // tree_edge
        if (was_edge_relaxed) {
          if constexpr (has_on_edge_relaxed<graph_type, Visitor>) {
            visitor.on_edge_relaxed(g, uv);
          }
          if constexpr (has_on_discover_vertex<graph_type, Visitor>) {
            visitor.on_discover_vertex(g, v);
          } else if constexpr (has_on_discover_vertex_id<graph_type, Visitor>) {
            visitor.on_discover_vertex(g, vid);
          }
          queue.push({v, distances[vid]});
        } else {
          // This is an indicator of a bug in the algorithm and should be investigated.
          throw std::logic_error(
                "dijkstra_shortest_paths: unexpected state where an edge to a new vertex was not relaxed");
        }
      } else {
        // non-tree edge
        if (was_edge_relaxed) {
          vertex_t<graph_type> v = *find_vertex(g, vid); // find_vertex must succeed since vid is a valid vertex ID
          if constexpr (has_on_edge_relaxed<graph_type, Visitor>) {
            visitor.on_edge_relaxed(g, uv);
          }
          queue.push({v, distances[vid]}); // re-enqueue with updated distance
        } else {
          if constexpr (has_on_edge_not_relaxed<graph_type, Visitor>) {
            visitor.on_edge_not_relaxed(g, uv);
          }
        }
      }
    }

    // Note: while we *think* we're done with this vertex, we may not be. If the graph is unbalanced
    // and another path to this vertex has a lower accumulated weight, we'll process it again.
    // A consequence is that examine_vertex could be called twice (or more) on the same vertex.
    if constexpr (has_on_finish_vertex<graph_type, Visitor>) {
      visitor.on_finish_vertex(g, u);
    } else if constexpr (has_on_finish_vertex_id<graph_type, Visitor>) {
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
template <
      adjacency_list G,
      class Distances,
      class Predecessors,
      class WF = function<vertex_property_map_value_t<Distances>(const std::remove_reference_t<G>&, const edge_t<G>&)>,
      class Visitor = empty_visitor,
      class Compare = less<vertex_property_map_value_t<Distances>>,
      class Combine = plus<vertex_property_map_value_t<Distances>>>
requires vertex_property_map_for<Distances, G> &&                                       //
         (is_null_range_v<Predecessors> || vertex_property_map_for<Predecessors, G>) && //
         is_arithmetic_v<vertex_property_map_value_t<Distances>> &&                     //
         basic_edge_weight_function<G, WF, vertex_property_map_value_t<Distances>, Compare, Combine>
constexpr void dijkstra_shortest_paths(
      G&&                   g,
      const vertex_id_t<G>& source,
      Distances&            distances,
      Predecessors&         predecessor,
      WF&&                  weight =
            [](const auto&, const edge_t<G>& uv) {
              return vertex_property_map_value_t<Distances>(1);
            }, // default weight(g, uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<vertex_property_map_value_t<Distances>>(),
      Combine&& combine = plus<vertex_property_map_value_t<Distances>>()) {
  dijkstra_shortest_paths(g, subrange(&source, (&source + 1)), distances, predecessor, weight,
                          forward<Visitor>(visitor), forward<Compare>(compare), forward<Combine>(combine));
}

/**
 * @brief Multi-source shortest distances using Dijkstra's algorithm (no predecessor tracking).
 * 
 * Computes shortest distances without tracking predecessor information. More efficient when
 * path reconstruction is not needed.
 * 
 * @tparam G            The graph type. Must satisfy adjacency_list concept.
 * @tparam Sources      Input range of source vertex IDs.
 * @tparam Distances    Container for storing distances. Value type must be arithmetic.
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
template <
      adjacency_list G,
      input_range    Sources,
      class Distances,
      class WF = function<vertex_property_map_value_t<Distances>(const std::remove_reference_t<G>&, const edge_t<G>&)>,
      class Visitor = empty_visitor,
      class Compare = less<vertex_property_map_value_t<Distances>>,
      class Combine = plus<vertex_property_map_value_t<Distances>>>
requires vertex_property_map_for<Distances, G> &&                   //
         convertible_to<range_value_t<Sources>, vertex_id_t<G>> &&  //
         is_arithmetic_v<vertex_property_map_value_t<Distances>> && //
         basic_edge_weight_function<G, WF, vertex_property_map_value_t<Distances>, Compare, Combine>
constexpr void dijkstra_shortest_distances(
      G&&            g,
      const Sources& sources,
      Distances&     distances,
      WF&&           weight =
            [](const auto&, const edge_t<G>& uv) {
              return vertex_property_map_value_t<Distances>(1);
            }, // default weight(g, uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<vertex_property_map_value_t<Distances>>(),
      Combine&& combine = plus<vertex_property_map_value_t<Distances>>()) {
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
template <
      adjacency_list G,
      class Distances,
      class WF = function<vertex_property_map_value_t<Distances>(const std::remove_reference_t<G>&, const edge_t<G>&)>,
      class Visitor = empty_visitor,
      class Compare = less<vertex_property_map_value_t<Distances>>,
      class Combine = plus<vertex_property_map_value_t<Distances>>>
requires vertex_property_map_for<Distances, G> &&                   //
         is_arithmetic_v<vertex_property_map_value_t<Distances>> && //
         basic_edge_weight_function<G, WF, vertex_property_map_value_t<Distances>, Compare, Combine>
constexpr void dijkstra_shortest_distances(
      G&&                   g,
      const vertex_id_t<G>& source,
      Distances&            distances,
      WF&&                  weight =
            [](const auto&, const edge_t<G>& uv) {
              return vertex_property_map_value_t<Distances>(1);
            }, // default weight(g, uv) -> 1
      Visitor&& visitor = empty_visitor(),
      Compare&& compare = less<vertex_property_map_value_t<Distances>>(),
      Combine&& combine = plus<vertex_property_map_value_t<Distances>>()) {
  dijkstra_shortest_paths(g, subrange(&source, (&source + 1)), distances, _null_predecessors, forward<WF>(weight),
                          forward<Visitor>(visitor), forward<Compare>(compare), forward<Combine>(combine));
}

} // namespace graph

#endif // GRAPH_DIJKSTRA_SHORTEST_PATHS_HPP
