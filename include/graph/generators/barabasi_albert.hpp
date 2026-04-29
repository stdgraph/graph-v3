/**
 * @file barabasi_albert.hpp
 * @brief Barabási–Albert preferential attachment generator — scale-free / power-law.
 *
 * Starts with a fully-connected seed of m0 = max(m, 2) vertices, then
 * adds each subsequent vertex w by selecting m existing targets with
 * probability proportional to their current degree ("urn" method).
 * Both w→t and t→w directed edges are added so the graph is undirected
 * in terms of reachability, which maximises relaxation traffic from hubs.
 * The returned list is sorted by source_id.
 */

#pragma once

#include <graph/generators/common.hpp>

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

namespace graph::generators {

/// Generate a Barabási–Albert preferential-attachment graph.
///
/// @tparam VId  Vertex id type (default: uint32_t).
/// @param n     Number of vertices.
/// @param m     Edges per new vertex (attaches to m existing targets).
/// @param seed  RNG seed for reproducibility.
/// @param wdist Weight distribution family.
/// @return Sorted edge list (sorted ascending by source_id).
template <class VId = uint32_t>
edge_list<VId> barabasi_albert(VId n, VId m, uint64_t seed = 42,
                               weight_dist wdist = weight_dist::uniform) {
  std::mt19937_64 rng(seed);

  // "Urn" stores one entry per endpoint per edge, giving degree-proportional
  // selection at O(1) per pick (trade memory for simplicity).
  std::vector<VId> urn;
  urn.reserve(2 * static_cast<size_t>(n) * m);

  edge_list<VId> edges;
  edges.reserve(2 * static_cast<size_t>(n) * m);

  // Seed: fully-connected clique of m0 vertices
  const VId m0 = std::max(m, VId{2});
  for (VId u = 0; u < m0; ++u) {
    for (VId v = u + 1; v < m0; ++v) {
      edges.push_back({u, v, sample_weight(rng, wdist)});
      edges.push_back({v, u, sample_weight(rng, wdist)});
      urn.push_back(u);
      urn.push_back(v);
    }
  }

  for (VId w = m0; w < n; ++w) {
    std::vector<VId> chosen;
    chosen.reserve(m);

    while (chosen.size() < m) {
      std::uniform_int_distribution<size_t> pick(0, urn.size() - 1);
      VId t = urn[pick(rng)];
      // chosen.size() ≤ m ≤ ~8 so linear scan is fine
      bool already = (t == w);
      for (auto x : chosen)
        already |= (x == t);
      if (!already) {
        chosen.push_back(t);
        edges.push_back({w, t, sample_weight(rng, wdist)});
        edges.push_back({t, w, sample_weight(rng, wdist)});
        urn.push_back(w);
        urn.push_back(t);
      }
    }
  }

  std::stable_sort(edges.begin(), edges.end(),
                   [](const auto& a, const auto& b) { return a.source_id < b.source_id; });
  return edges;
}

} // namespace graph::generators
