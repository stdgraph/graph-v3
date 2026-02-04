/**
 * @file dijkstra.hpp
 * 
 * @brief Dijkstra's single-source shortest paths algorithm.
 * 
 * @copyright Copyright (c) 2024
 * 
 * SPDX-License-Identifier: BSL-1.0
 *
 * @authors
 *   Andrew Lumsdaine
 *   Phil Ratzloff
 */

#pragma once

#include "graph/graph.hpp"
#include "graph/algorithm/common_shortest_paths.hpp"
#include <queue>
#include <vector>
#include <ranges>
#include <limits>
#include <functional>

#ifndef GRAPH_DIJKSTRA_HPP
#  define GRAPH_DIJKSTRA_HPP

namespace graph {

/**
 * @ingroup graph_algorithms
 * @brief Dijkstra's single-source shortest paths algorithm.
 * 
 * Finds the shortest paths from a source vertex to all other vertices in a weighted graph
 * with non-negative edge weights.
 * 
 * @tparam G           Graph type (must satisfy adjacency_list concept)
 * @tparam Distance    Random access range type for distances
 * @tparam Predecessor Random access range type for predecessors
 * @tparam WF          Edge weight function type
 * 
 * @param g           The graph
 * @param source      Source vertex ID
 * @param distances   Output: distances[v] = shortest distance from source to v
 * @param predecessors Output: predecessors[v] = predecessor of v in shortest path tree
 * @param weight      Edge weight function (default returns 1 for unweighted)
 * 
 * @complexity Time: O((V + E) log V), Space: O(V)
 * 
 * @pre source < num_vertices(g) for vector-based containers
 * @pre distances.size() >= num_vertices(g)
 * @pre predecessors.size() >= num_vertices(g) (if not _null_predecessors)
 * @pre All edge weights must be non-negative
 * 
 * @post distances[source] == 0
 * @post For all reachable vertices v: distances[v] = shortest path length
 * @post For unreachable vertices v: distances[v] = numeric_limits<>::max()
 */
template <adjacency_list      G,
          random_access_range  Distance,
          random_access_range  Predecessor,
          class WF = function<range_value_t<Distance>(edge_t<G>)>>
requires forward_range<vertex_range_t<G>> &&
         integral<vertex_id_t<G>> &&
         is_arithmetic_v<range_value_t<Distance>> &&
         convertible_to<vertex_id_t<G>, range_value_t<Predecessor>> &&
         edge_weight_function<G, WF, range_value_t<Distance>>
void dijkstra(
      G&&            g,
      vertex_id_t<G> source,
      Distance&      distances,
      Predecessor&   predecessors,
      WF&&           weight = [](const edge_t<G>& uv) { return range_value_t<Distance>(1); })
{
  using id_type     = vertex_id_t<G>;
  using weight_type = invoke_result_t<WF, edge_t<G>>;
  using distance_type = range_value_t<Distance>;

  const size_t N = num_vertices(g);
  
  // Initialize distances to infinity
  std::ranges::fill(distances, std::numeric_limits<weight_type>::max());
  distances[source] = 0;

  // Priority queue: (distance, vertex_id)
  struct weighted_vertex {
    id_type     vertex_id = id_type();
    weight_type weight    = weight_type();
  };

  auto qcompare = [](const weighted_vertex& a, const weighted_vertex& b) { 
    return a.weight > b.weight; 
  };
  std::priority_queue<weighted_vertex, std::vector<weighted_vertex>, decltype(qcompare)> Q(qcompare);

  // Start with source vertex
  Q.push({source, distances[source]});

  while (!Q.empty()) {
    auto [uid, d_u] = Q.top();
    Q.pop();

    // Skip if we've already found a better path
    if (d_u > distances[uid])
      continue;

    // Examine all outgoing edges
    for (auto&& [uv, w] : views::incidence(g, uid, weight)) {
      const id_type vid = target_id(g, uv);
      const distance_type new_distance = distances[uid] + w;
      
      if (new_distance < distances[vid]) {
        distances[vid] = new_distance;
        
        // Update predecessor if tracking paths
        if constexpr (!is_same_v<Predecessor, _null_range_type>) {
          predecessors[vid] = uid;
        }
        
        Q.push({vid, new_distance});
      }
    }
  }
}

/**
 * @ingroup graph_algorithms
 * @brief Dijkstra's algorithm - distances only (no predecessor tracking).
 * 
 * Convenience overload that only computes distances, not paths.
 * 
 * @see dijkstra() for full documentation
 */
template <adjacency_list      G,
          random_access_range  Distance,
          class WF = function<range_value_t<Distance>(edge_t<G>)>>
requires forward_range<vertex_range_t<G>> &&
         integral<vertex_id_t<G>> &&
         is_arithmetic_v<range_value_t<Distance>> &&
         edge_weight_function<G, WF, range_value_t<Distance>>
void dijkstra(
      G&&            g,
      vertex_id_t<G> source,
      Distance&      distances,
      WF&&           weight = [](const edge_t<G>& uv) { return range_value_t<Distance>(1); })
{
  dijkstra(g, source, distances, _null_predecessors, forward<WF>(weight));
}

} // namespace graph

#endif // GRAPH_DIJKSTRA_HPP
