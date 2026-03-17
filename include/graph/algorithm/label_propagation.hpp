/**
 * @file label_propagation.hpp
 *
 * @brief Label Propagation algorithm for community detection in graphs.
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
#include "graph/adj_list/vertex_property_map.hpp"

#ifndef GRAPH_LABEL_PROPAGATION_HPP
#  define GRAPH_LABEL_PROPAGATION_HPP

#  include <algorithm>
#  include <limits>
#  include <numeric>
#  include <optional>
#  include <random>
#  include <ranges>
#  include <unordered_map>
#  include <vector>

namespace graph {

// Using declarations for new namespace structure
using adj_list::adjacency_list;
using adj_list::vertex_id_t;
using adj_list::vertices;
using adj_list::edges;
using adj_list::target_id;
using adj_list::vertex_id;
using adj_list::find_vertex;
using adj_list::num_vertices;

namespace detail {

template <adjacency_list G,
          class Label,
          class Gen,
          class T>
requires vertex_property_map_for<Label, G> &&
         std::equality_comparable<vertex_property_map_value_t<Label>> &&
         std::uniform_random_bit_generator<std::remove_cvref_t<Gen>>
void label_propagation_impl(G&&                                                     g,
                            Label&                                                   label,
                            std::optional<vertex_property_map_value_t<Label>> const&  empty_label,
                            Gen&&                                                     rng,
                            T                                                         max_iters) {
  using label_type = vertex_property_map_value_t<Label>;

  const size_t N = num_vertices(g);
  if (N == 0) {
    return;
  }

  // Build shuffleable vector of vertex IDs
  std::vector<vertex_id_t<G>> order;
  order.reserve(N);
  for (auto&& u : vertices(g)) {
    order.push_back(vertex_id(g, u));
  }

  for (T iter = 0; iter < max_iters; ++iter) {
    std::shuffle(order.begin(), order.end(), rng);

    bool changed = false;

    for (auto uid : order) {
      // Tally neighbour labels, skipping neighbours with empty_label if provided
      std::unordered_map<label_type, size_t> freq;
      for (auto&& uv : edges(g, *find_vertex(g, uid))) {
        auto tid = target_id(g, uv);
        if (!empty_label || !(label[tid] == *empty_label)) {
          ++freq[label[tid]];
        }
      }

      if (freq.empty()) {
        continue; // isolated vertex or no labelled neighbours — keep current label
      }

      // Find the maximum frequency
      size_t max_count = 0;
      for (auto& [lbl, cnt] : freq) {
        if (cnt > max_count) {
          max_count = cnt;
        }
      }

      // Collect all labels tied at the maximum
      std::vector<label_type> candidates;
      for (auto& [lbl, cnt] : freq) {
        if (cnt == max_count) {
          candidates.push_back(lbl);
        }
      }

      // Pick one of the tied labels (randomly if more than one)
      label_type best;
      if (candidates.size() == 1) {
        best = candidates[0];
      } else {
        std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
        best = candidates[dist(rng)];
      }

      if (!(label[uid] == best)) {
        label[uid] = best;
        changed    = true;
      }
    }

    if (!changed) {
      break; // convergence
    }
  }
}

} // namespace detail

/**
 * @brief Label propagation for community detection by majority voting among neighbours.
 *
 * Each iteration shuffles the vertex processing order, then sets every vertex's label
 * to the most popular label among its neighbours. Ties are broken randomly using the
 * supplied random-number generator. The algorithm iterates until no label changes
 * (convergence) or until @p max_iters iterations have been performed.
 *
 * @tparam G          The graph type. Must satisfy adjacency_list concept.
 * @tparam Label      Vertex property map whose vertex_property_map_value_t is the label type.
 *                    For index graphs: std::vector<T>. For mapped graphs: std::unordered_map<VId, T>.
 * @tparam Gen        Uniform random bit generator type (default std::default_random_engine).
 * @tparam T          Integral type for the iteration limit (default size_t).
 *
 * @param g           The graph to process.
 * @param label       [in,out] Vertex property map holding initial labels for every vertex.
 *                    Modified in place to hold final labels on return.
 * @param rng         Random-number generator used for shuffle and tie-breaking.
 * @param max_iters   Maximum number of iterations (default: unlimited).
 *
 * @return void. Results are stored in the label output parameter.
 *
 * **Mandates:**
 * - G must satisfy adjacency_list (index or mapped vertex containers)
 * - Label must satisfy vertex_property_map_for<Label, G> (subscriptable by vertex_id_t<G>)
 * - vertex_property_map_value_t<Label> must satisfy std::equality_comparable
 * - Gen must satisfy std::uniform_random_bit_generator
 *
 * **Preconditions:**
 * - label contains a meaningful initial label for every vertex in g
 * - For index graphs: label.size() >= num_vertices(g)
 *
 * **Effects:**
 * - Modifies label: Sets label[v] for all vertices v
 * - Does not modify the graph g
 *
 * **Postconditions:**
 * - label[uid] holds the discovered community label for vertex uid
 * - Neighbouring vertices in the same dense community share a common label
 *
 * **Throws:**
 * - std::bad_alloc from internal allocations (frequency map, candidate vector)
 * - Exception guarantee: Basic. If an exception is thrown, graph g remains unchanged;
 *   label may be partially modified (indeterminate state).
 *
 * **Complexity:**
 * - Time: O(E) per iteration; the number of iterations required for convergence is
 *   typically small relative to graph size
 * - Space: O(V) for the shuffled vertex-ID vector and frequency map
 *
 * **Remarks:**
 * - For semi-supervised propagation with unlabelled vertices, use the overload
 *   accepting an empty_label sentinel
 *
 * **Supported Graph Properties:**
 *
 * Directedness:
 * - ✅ Directed graphs
 *
 * Edge Properties:
 * - ✅ Unweighted edges
 * - ✅ Weighted edges (weights ignored)
 * - ✅ Multi-edges (all edges counted in tally)
 * - ✅ Self-loops (counted in tally)
 * - ✅ Cycles
 */
template <adjacency_list G,
          class Label,
          class Gen = std::default_random_engine,
          class T   = size_t>
requires vertex_property_map_for<Label, G> &&
         std::equality_comparable<vertex_property_map_value_t<Label>> &&
         std::uniform_random_bit_generator<std::remove_cvref_t<Gen>>
void label_propagation(G&&    g,
                       Label& label,
                       Gen&&  rng       = std::default_random_engine{},
                       T      max_iters = std::numeric_limits<T>::max()) {
  detail::label_propagation_impl(g, label, std::nullopt, std::forward<Gen>(rng), max_iters);
}

/**
 * @brief Label propagation with an empty-label sentinel for semi-supervised community detection.
 *
 * Behaves like the primary overload, except vertices whose label equals @p empty_label
 * are treated as unlabelled: they do not vote and are not counted in neighbour tallies.
 * When an unlabelled vertex acquires a label from a neighbour, that acquisition counts
 * as a change for convergence purposes.
 *
 * @tparam G          The graph type. Must satisfy adjacency_list concept.
 * @tparam Label      Vertex property map whose vertex_property_map_value_t is the label type.
 * @tparam Gen        Uniform random bit generator type (default std::default_random_engine).
 * @tparam T          Integral type for the iteration limit (default size_t).
 *
 * @param g           The graph to process.
 * @param label       [in,out] Vertex property map holding initial labels for every vertex.
 *                    Modified in place to hold final labels on return.
 * @param empty_label Sentinel value representing an unlabelled vertex. Passed by value.
 * @param rng         Random-number generator used for shuffle and tie-breaking.
 * @param max_iters   Maximum number of iterations (default: unlimited).
 *
 * @return void. Results are stored in the label output parameter.
 *
 * **Mandates:**
 * - G must satisfy adjacency_list (index or mapped vertex containers)
 * - Label must satisfy vertex_property_map_for<Label, G> (subscriptable by vertex_id_t<G>)
 * - vertex_property_map_value_t<Label> must satisfy std::equality_comparable
 * - Gen must satisfy std::uniform_random_bit_generator
 *
 * **Preconditions:**
 * - label contains an initial label (or empty_label) for every vertex in g
 * - For index graphs: label.size() >= num_vertices(g)
 *
 * **Effects:**
 * - Modifies label: Sets label[v] for all vertices v that acquire a label
 * - Does not modify the graph g
 *
 * **Postconditions:**
 * - Vertices reachable from a labelled vertex acquire a non-empty label
 * - Vertices in components with no labelled vertex retain empty_label
 *
 * **Throws:**
 * - std::bad_alloc from internal allocations (frequency map, candidate vector)
 * - Exception guarantee: Basic. If an exception is thrown, graph g remains unchanged;
 *   label may be partially modified (indeterminate state).
 *
 * **Complexity:**
 * - Time: O(E) per iteration; the number of iterations required for convergence is
 *   typically small relative to graph size
 * - Space: O(V) for the shuffled vertex-ID vector and frequency map
 *
 * **Remarks:**
 * - If no vertex in a connected component has a non-empty label, those vertices
 *   remain unlabelled (empty_label) after convergence
 * - When empty_label is not present in label, behaviour is identical to the
 *   primary overload
 *
 * **Supported Graph Properties:**
 *
 * Directedness:
 * - ✅ Directed graphs
 *
 * Edge Properties:
 * - ✅ Unweighted edges
 * - ✅ Weighted edges (weights ignored)
 * - ✅ Multi-edges (all edges counted in tally)
 * - ✅ Self-loops (counted in tally)
 * - ✅ Cycles
 */
template <adjacency_list G,
          class Label,
          class Gen = std::default_random_engine,
          class T   = size_t>
requires vertex_property_map_for<Label, G> &&
         std::equality_comparable<vertex_property_map_value_t<Label>> &&
         std::uniform_random_bit_generator<std::remove_cvref_t<Gen>>
void label_propagation(G&&                                            g,
                       Label&                                         label,
                       vertex_property_map_value_t<Label>              empty_label,
                       Gen&&                                           rng       = std::default_random_engine{},
                       T                                               max_iters = std::numeric_limits<T>::max()) {
  detail::label_propagation_impl(g, label, std::optional(std::move(empty_label)),
                                 std::forward<Gen>(rng), max_iters);
}

} // namespace graph

#endif // GRAPH_LABEL_PROPAGATION_HPP
