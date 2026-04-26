/**
 * @file benchmark_dijkstra.cpp
 * @brief Google Benchmark suite for dijkstra_shortest_distances.
 *
 * Covers four graph topologies × two containers (CSR and vov) across a
 * scale sweep of V ∈ {1 000, 10 000, 100 000}.  Graph construction and
 * distance-vector initialisation are excluded from the timed region via
 * state.PauseTiming() / state.ResumeTiming() — only the Dijkstra call
 * itself is measured.
 *
 * Benchmark naming convention:
 *   BM_Dijkstra_<Container>_<Topology>
 *   Container : CSR  (compressed_graph)
 *               VoV  (dynamic_graph / vov)
 *   Topology  : ER_Sparse   Erdős–Rényi, E/V ≈ 8
 *               Grid        2D grid (bidirectional, E/V ≈ 4)
 *               BA          Barabási–Albert, m=4, E/V ≈ 8
 *               Path        Path graph, E/V = 1 (minimum decrease-key)
 *
 * Compile-time macro DIJKSTRA_BENCH_LARGE enables the 1 000 000-vertex
 * tier (disabled by default to keep CI times reasonable).
 *
 * Results are saved by the Phase 0.4 script to
 *   agents/indexed_dary_heap_baseline.md
 */

#include <benchmark/benchmark.h>

#include <graph/algorithm/dijkstra_shortest_paths.hpp>
#include <graph/algorithm/traversal_common.hpp>
#include <graph/graph.hpp>

#include "dijkstra_fixtures.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

namespace {

/// Resize and initialise the distance vector to +∞.  Called once before
/// the benchmark loop; the per-iteration reset uses std::fill (cheaper
/// than reallocation) inside the paused region.
template <class G>
void init_dist(const G& g, std::vector<double>& dist) {
  const std::size_t n = graph::num_vertices(g);
  dist.assign(n, std::numeric_limits<double>::max());
}

/// Weight function: return the edge value stored in the graph.
constexpr auto weight_fn = [](const auto& g, const auto& uv) {
  return graph::edge_value(g, uv);
};

} // namespace

// ---------------------------------------------------------------------------
// Macro: define a Dijkstra benchmark for a given container and graph builder.
//
// Parameters:
//   NAME       — benchmark function name  (e.g. BM_Dijkstra_CSR_ER_Sparse)
//   GRAPH_T    — graph container type
//   MAKE_FN    — graph::benchmark::make_csr or make_vov
//   EDGE_EXPR  — expression producing edge_list, may use vertex_id_t n
//   N_EXPR     — expression for num_vertices to pass to MAKE_FN (usually n)
// ---------------------------------------------------------------------------

#define DEFINE_DIJKSTRA_BM(NAME, GRAPH_T, MAKE_FN, EDGE_EXPR, N_EXPR)               \
  static void NAME(benchmark::State& state) {                                        \
    const auto n = static_cast<graph::benchmark::vertex_id_t>(state.range(0));       \
    /* Build graph outside the timed loop */                                         \
    const auto edges = (EDGE_EXPR);                                                  \
    GRAPH_T    g     = graph::benchmark::MAKE_FN(edges, (N_EXPR));                   \
    std::vector<double> dist;                                                         \
    init_dist(g, dist);                                                              \
    for (auto _ : state) {                                                           \
      /* Exclude distance-reset from the measurement */                              \
      state.PauseTiming();                                                           \
      std::fill(dist.begin(), dist.end(), std::numeric_limits<double>::max());       \
      state.ResumeTiming();                                                          \
      graph::dijkstra_shortest_distances(                                            \
            g, graph::benchmark::vertex_id_t{0}, graph::container_value_fn(dist),   \
            weight_fn);                                                              \
      benchmark::DoNotOptimize(dist.data());                                        \
    }                                                                                \
    state.SetComplexityN(state.range(0));                                            \
  }

// ---------------------------------------------------------------------------
// Erdős–Rényi, E/V ≈ 8  (p = 8/n)
// ---------------------------------------------------------------------------

DEFINE_DIJKSTRA_BM(
      BM_Dijkstra_CSR_ER_Sparse,
      graph::benchmark::csr_graph_t,
      make_csr,
      graph::benchmark::erdos_renyi(n, 8.0 / n),
      n)

DEFINE_DIJKSTRA_BM(
      BM_Dijkstra_VoV_ER_Sparse,
      graph::benchmark::vov_graph_t,
      make_vov,
      graph::benchmark::erdos_renyi(n, 8.0 / n),
      n)

BENCHMARK(BM_Dijkstra_CSR_ER_Sparse)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_VoV_ER_Sparse)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();

// ---------------------------------------------------------------------------
// 2D grid  (rows = cols = sqrt(n), E/V ≈ 4)
// ---------------------------------------------------------------------------

DEFINE_DIJKSTRA_BM(
      BM_Dijkstra_CSR_Grid,
      graph::benchmark::csr_graph_t,
      make_csr,
      graph::benchmark::grid_2d(
            static_cast<graph::benchmark::vertex_id_t>(std::sqrt(static_cast<double>(n))),
            static_cast<graph::benchmark::vertex_id_t>(std::sqrt(static_cast<double>(n)))),
      static_cast<graph::benchmark::vertex_id_t>(std::sqrt(static_cast<double>(n))) *
            static_cast<graph::benchmark::vertex_id_t>(std::sqrt(static_cast<double>(n))))

DEFINE_DIJKSTRA_BM(
      BM_Dijkstra_VoV_Grid,
      graph::benchmark::vov_graph_t,
      make_vov,
      graph::benchmark::grid_2d(
            static_cast<graph::benchmark::vertex_id_t>(std::sqrt(static_cast<double>(n))),
            static_cast<graph::benchmark::vertex_id_t>(std::sqrt(static_cast<double>(n)))),
      static_cast<graph::benchmark::vertex_id_t>(std::sqrt(static_cast<double>(n))) *
            static_cast<graph::benchmark::vertex_id_t>(std::sqrt(static_cast<double>(n))))

BENCHMARK(BM_Dijkstra_CSR_Grid)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_VoV_Grid)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();

// ---------------------------------------------------------------------------
// Barabási–Albert, m=4  (E/V ≈ 8, heavy hub traffic)
// ---------------------------------------------------------------------------

DEFINE_DIJKSTRA_BM(
      BM_Dijkstra_CSR_BA,
      graph::benchmark::csr_graph_t,
      make_csr,
      graph::benchmark::barabasi_albert(n, 4),
      n)

DEFINE_DIJKSTRA_BM(
      BM_Dijkstra_VoV_BA,
      graph::benchmark::vov_graph_t,
      make_vov,
      graph::benchmark::barabasi_albert(n, 4),
      n)

BENCHMARK(BM_Dijkstra_CSR_BA)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_VoV_BA)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();

// ---------------------------------------------------------------------------
// Path graph  (E/V = 1, minimum decrease-key)
// ---------------------------------------------------------------------------

DEFINE_DIJKSTRA_BM(
      BM_Dijkstra_CSR_Path,
      graph::benchmark::csr_graph_t,
      make_csr,
      graph::benchmark::path_graph(n),
      n)

DEFINE_DIJKSTRA_BM(
      BM_Dijkstra_VoV_Path,
      graph::benchmark::vov_graph_t,
      make_vov,
      graph::benchmark::path_graph(n),
      n)

BENCHMARK(BM_Dijkstra_CSR_Path)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_VoV_Path)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();

// ---------------------------------------------------------------------------
// Optional large-scale tier  (V = 1 000 000)
// Enable with: cmake -DDIJKSTRA_BENCH_LARGE=ON ...
// ---------------------------------------------------------------------------

#ifdef DIJKSTRA_BENCH_LARGE
BENCHMARK(BM_Dijkstra_CSR_ER_Sparse)->Arg(1'000'000)->Complexity();
BENCHMARK(BM_Dijkstra_VoV_ER_Sparse)->Arg(1'000'000)->Complexity();
BENCHMARK(BM_Dijkstra_CSR_BA)->Arg(1'000'000)->Complexity();
BENCHMARK(BM_Dijkstra_VoV_BA)->Arg(1'000'000)->Complexity();
#endif

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

BENCHMARK_MAIN();
