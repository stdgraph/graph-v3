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

#ifndef GRAPH_LABEL_PROPAGATION_HPP
#  define GRAPH_LABEL_PROPAGATION_HPP

#  include <algorithm>
#  include <limits>
#  include <numeric>
#  include <random>
#  include <ranges>
#  include <unordered_map>
#  include <vector>

namespace graph {

// Using declarations for new namespace structure
using adj_list::index_adjacency_list;
using adj_list::vertex_id_t;
using adj_list::vertices;
using adj_list::edges;
using adj_list::target_id;
using adj_list::vertex_id;
using adj_list::num_vertices;

/**
 * @ingroup graph_algorithms
 * @brief Propagate vertex labels by majority voting among neighbours.
 *
 * Each iteration shuffles the vertex processing order, then sets every vertex's label
 * to the most popular label among its neighbours. Ties are broken randomly using the
 * supplied random-number generator. The algorithm iterates until no label changes
 * (convergence) or until @p max_iters iterations have been performed.
 *
 * ## Complexity Analysis
 *
 * **Time Complexity:** O(M) per iteration, where M = |E|. The number of iterations
 * required for convergence is typically small relative to graph size.
 *
 * **Space Complexity:** O(V) for the shuffled vertex-ID vector and frequency map.
 *
 * ## Supported Graph Properties
 *
 * ### Directedness
 * - ✅ Directed graphs
 *
 * ### Edge Properties
 * - ✅ Unweighted edges
 * - ✅ Weighted edges (weights ignored)
 * - ✅ Multi-edges (all edges counted in tally)
 * - ✅ Self-loops (counted in tally)
 * - ✅ Cycles
 *
 * ### Container Requirements
 * - Requires: `index_adjacency_list<G>` concept (contiguous vertex IDs)
 * - Requires: `std::ranges::random_access_range<Label>` sized to `num_vertices(g)`
 * - Requires: `std::equality_comparable<std::ranges::range_value_t<Label>>`
 *
 * @tparam G          Graph type satisfying `index_adjacency_list`.
 * @tparam Label      Random-access range whose `value_type` is the label type.
 * @tparam Gen        Uniform random bit generator type (default `std::default_random_engine`).
 * @tparam T          Integral type for the iteration limit (default `size_t`).
 *
 * @param g           The graph.
 * @param label       Random-access range of size `num_vertices(g)` holding the initial
 *                    labels. Modified in place to hold final labels on return.
 * @param rng         Random-number generator used for shuffle and tie-breaking.
 * @param max_iters   Maximum number of iterations (default: unlimited).
 *
 * @pre `label.size() >= num_vertices(g)`
 * @pre `label` contains meaningful initial labels for every vertex.
 *
 * @post `label[uid]` holds the discovered label assignment for vertex @p uid.
 * @post The graph @p g is not modified.
 *
 * **Exception Safety:** Basic. May throw `std::bad_alloc` from internal allocations.
 * The graph is unchanged; the label array may be partially updated.
 *
 * ## Example Usage
 *
 * ```cpp
 * #include <graph/graph.hpp>
 * #include <graph/algorithm/label_propagation.hpp>
 * #include <vector>
 * #include <random>
 *
 * using namespace graph;
 *
 * int main() {
 *     using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
 *                       container::vov_graph_traits<void, void, void, uint32_t, false>>;
 *     Graph g({{0,1},{1,0},{1,2},{2,1},{2,3},{3,2}});
 *
 *     std::vector<int> label = {0, 1, 2, 3};
 *     std::mt19937 rng{42};
 *     label_propagation(g, label, rng);
 *     // label is now converged — neighbouring vertices share a common label
 * }
 * ```
 */
template <index_adjacency_list G,
          std::ranges::random_access_range Label,
          class Gen = std::default_random_engine,
          class T   = size_t>
requires std::equality_comparable<std::ranges::range_value_t<Label>> &&
         std::uniform_random_bit_generator<std::remove_cvref_t<Gen>>
void label_propagation(G&&    g,
                       Label& label,
                       Gen&&  rng       = std::default_random_engine{},
                       T      max_iters = std::numeric_limits<T>::max()) {
  using label_type = std::ranges::range_value_t<Label>;

  const size_t N = num_vertices(g);
  if (N == 0) {
    return;
  }

  // Build shuffleable vector of vertex IDs
  std::vector<vertex_id_t<G>> order(N);
  std::iota(order.begin(), order.end(), vertex_id_t<G>{0});

  for (T iter = 0; iter < max_iters; ++iter) {
    std::shuffle(order.begin(), order.end(), rng);

    bool changed = false;

    for (auto uid : order) {
      // Tally neighbour labels
      std::unordered_map<label_type, size_t> freq;
      for (auto&& uv : edges(g, *find_vertex(g, uid))) {
        auto tid = target_id(g, uv);
        ++freq[label[tid]];
      }

      if (freq.empty()) {
        continue; // isolated vertex — keep current label
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

/**
 * @ingroup graph_algorithms
 * @brief Propagate vertex labels with an empty-label sentinel.
 *
 * Behaves like the primary overload, except vertices whose label equals @p empty_label
 * are treated as unlabelled: they do not vote and are not counted in neighbour tallies.
 * When an unlabelled vertex acquires a label from a neighbour, that acquisition counts
 * as a change for convergence purposes.
 *
 * @copydetails label_propagation(G&&, Label&, Gen&&, T)
 *
 * @param empty_label  Sentinel value representing an unlabelled vertex. Passed by value.
 */
template <index_adjacency_list G,
          std::ranges::random_access_range Label,
          class Gen = std::default_random_engine,
          class T   = size_t>
requires std::equality_comparable<std::ranges::range_value_t<Label>> &&
         std::uniform_random_bit_generator<std::remove_cvref_t<Gen>>
void label_propagation(G&&                                  g,
                       Label&                               label,
                       std::ranges::range_value_t<Label>    empty_label,
                       Gen&&                                rng       = std::default_random_engine{},
                       T                                    max_iters = std::numeric_limits<T>::max()) {
  using label_type = std::ranges::range_value_t<Label>;

  const size_t N = num_vertices(g);
  if (N == 0) {
    return;
  }

  // Build shuffleable vector of vertex IDs
  std::vector<vertex_id_t<G>> order(N);
  std::iota(order.begin(), order.end(), vertex_id_t<G>{0});

  for (T iter = 0; iter < max_iters; ++iter) {
    std::shuffle(order.begin(), order.end(), rng);

    bool changed = false;

    for (auto uid : order) {
      // Tally neighbour labels, skipping neighbours with empty_label
      std::unordered_map<label_type, size_t> freq;
      for (auto&& uv : edges(g, *find_vertex(g, uid))) {
        auto tid = target_id(g, uv);
        if (!(label[tid] == empty_label)) {
          ++freq[label[tid]];
        }
      }

      if (freq.empty()) {
        continue; // no labelled neighbours — keep current label
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

} // namespace graph

#endif // GRAPH_LABEL_PROPAGATION_HPP
