/**
 * @file watts_strogatz.hpp
 * @brief Watts–Strogatz small-world graph generator.
 *
 * Builds a ring lattice where each vertex is connected to its k nearest
 * neighbours (k/2 on each side), then rewires each "forward" lattice edge
 * with probability β to a uniformly random target, avoiding self-loops and
 * duplicate edges. Low β yields a regular lattice (high clustering, long
 * paths); β → 1 approaches an Erdős–Rényi random graph. Intermediate β
 * produces the characteristic small-world regime (high clustering, short
 * paths).
 *
 * Edges are emitted in both directions (undirected reachability) and the
 * returned list is sorted by source_id.
 */

#pragma once

#include <graph/generators/common.hpp>

#include <algorithm>
#include <cstdint>
#include <random>
#include <set>
#include <utility>
#include <vector>

namespace graph::generators {

/// Generate a Watts–Strogatz small-world graph.
///
/// @tparam VId  Vertex id type (default: uint32_t).
/// @param n     Number of vertices (must be > k).
/// @param k     Each vertex connects to its k nearest ring neighbours
///              (rounded down to an even number).
/// @param beta  Rewiring probability in [0, 1].
/// @param seed  RNG seed for reproducibility.
/// @param wdist Weight distribution family.
/// @return Sorted edge list (ascending by source_id).
template <class VId = uint32_t>
edge_list<VId> watts_strogatz(VId n, VId k, double beta, uint64_t seed = 42,
                              weight_dist wdist = weight_dist::uniform) {
  std::mt19937_64 rng(seed);
  edge_list<VId>  generated_edges;
  if (n < 2) {
    return generated_edges;
  }

  // Force k even and < n.
  VId half = k / 2;
  if (half == 0) {
    return generated_edges;
  }
  if (2 * half >= n) {
    half = static_cast<VId>((n - 1) / 2);
  }

  // Track undirected pairs to avoid duplicates when rewiring.
  std::set<std::pair<VId, VId>> present;
  auto ordered = [](VId a, VId b) {
    return (a < b) ? std::pair<VId, VId>{a, b} : std::pair<VId, VId>{b, a};
  };

  std::uniform_real_distribution<double> coin(0.0, 1.0);
  std::uniform_int_distribution<VId>     pick(0, n - 1);

  for (VId u = 0; u < n; ++u) {
    for (VId j = 1; j <= half; ++j) {
      VId v = static_cast<VId>((u + j) % n); // forward neighbour on the ring
      if (coin(rng) < beta) {
        // Rewire: choose a new target distinct from u and not already present.
        VId w = pick(rng);
        int guard = 0;
        while ((w == u || present.count(ordered(u, w))) && guard++ < 32) {
          w = pick(rng);
        }
        if (w != u && !present.count(ordered(u, w))) {
          v = w;
        }
      }
      auto key = ordered(u, v);
      if (u != v && present.insert(key).second) {
        generated_edges.push_back({u, v, sample_weight(rng, wdist)});
        generated_edges.push_back({v, u, sample_weight(rng, wdist)});
      }
    }
  }

  std::stable_sort(generated_edges.begin(), generated_edges.end(),
                   [](const auto& a, const auto& b) { return a.source_id < b.source_id; });
  return generated_edges;
}

} // namespace graph::generators
