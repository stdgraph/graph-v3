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
 *   BM_Dijkstra_<Container>_<Topology>           — default heap (priority_queue)
 *   BM_Dijkstra_<Container>_<Topology>_Idx<D>    — indexed d-ary heap, arity D
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
 * Phase 0.4 baseline results: agents/indexed_dary_heap_baseline.md
 * Phase 4.1 comparative results: agents/indexed_dary_heap_results.md
 */

#include <benchmark/benchmark.h>

#include <graph/algorithm/dijkstra_shortest_paths.hpp>
#include <graph/algorithm/traversal_common.hpp>
#include <graph/graph.hpp>

#include "dijkstra_fixtures.hpp"

#ifdef BENCH_BGL
#  include "bgl_dijkstra_fixtures.hpp"
#endif

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
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
// Macro: define a Dijkstra benchmark for a given container, graph builder,
//        and heap tag.
//
// Parameters:
//   NAME       — benchmark function name  (e.g. BM_Dijkstra_CSR_ER_Sparse)
//   GRAPH_T    — graph container type
//   MAKE_FN    — graph::benchmark::make_csr or make_vov
//   EDGE_EXPR  — expression producing edge_list, may use vertex_id_t n
//   N_EXPR     — expression for num_vertices to pass to MAKE_FN (usually n)
//   HEAP_TAG   — use_default_heap{} or use_indexed_dary_heap<D>{}
// ---------------------------------------------------------------------------

#define DEFINE_DIJKSTRA_BM(NAME, GRAPH_T, MAKE_FN, EDGE_EXPR, N_EXPR, HEAP_TAG)     \
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
            weight_fn, graph::empty_visitor{},                                       \
            std::less<double>{}, std::plus<double>{},                                \
            HEAP_TAG, std::allocator<std::byte>{});                                  \
      benchmark::DoNotOptimize(dist.data());                                        \
    }                                                                                \
    state.SetComplexityN(state.range(0));                                            \
  }

// Convenience shorthands for the four heap variants.
#define DEF_BM_DEFAULT(NAME, GT, MK, EE, NE)  DEFINE_DIJKSTRA_BM(NAME, GT, MK, EE, NE, graph::use_default_heap{})
#define DEF_BM_IDX2(NAME, GT, MK, EE, NE)     DEFINE_DIJKSTRA_BM(NAME, GT, MK, EE, NE, graph::use_indexed_dary_heap<2>{})
#define DEF_BM_IDX4(NAME, GT, MK, EE, NE)     DEFINE_DIJKSTRA_BM(NAME, GT, MK, EE, NE, graph::use_indexed_dary_heap<4>{})
#define DEF_BM_IDX8(NAME, GT, MK, EE, NE)     DEFINE_DIJKSTRA_BM(NAME, GT, MK, EE, NE, graph::use_indexed_dary_heap<8>{})

// ---------------------------------------------------------------------------
// Erdős–Rényi, E/V ≈ 8  (p = 8/n)
// ---------------------------------------------------------------------------

#define ER_EDGES(n)   graph::benchmark::erdos_renyi(n, 8.0 / n)
#define GRID_SQRT(n)  static_cast<graph::benchmark::vertex_id_t>(std::sqrt(static_cast<double>(n)))

DEF_BM_DEFAULT(BM_Dijkstra_CSR_ER_Sparse,       graph::benchmark::csr_graph_t, make_csr, ER_EDGES(n), n)
DEF_BM_IDX2   (BM_Dijkstra_CSR_ER_Sparse_Idx2,  graph::benchmark::csr_graph_t, make_csr, ER_EDGES(n), n)
DEF_BM_IDX4   (BM_Dijkstra_CSR_ER_Sparse_Idx4,  graph::benchmark::csr_graph_t, make_csr, ER_EDGES(n), n)
DEF_BM_IDX8   (BM_Dijkstra_CSR_ER_Sparse_Idx8,  graph::benchmark::csr_graph_t, make_csr, ER_EDGES(n), n)

DEF_BM_DEFAULT(BM_Dijkstra_VoV_ER_Sparse,       graph::benchmark::vov_graph_t, make_vov, ER_EDGES(n), n)
DEF_BM_IDX4   (BM_Dijkstra_VoV_ER_Sparse_Idx4,  graph::benchmark::vov_graph_t, make_vov, ER_EDGES(n), n)

BENCHMARK(BM_Dijkstra_CSR_ER_Sparse)      ->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_CSR_ER_Sparse_Idx2)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_CSR_ER_Sparse_Idx4)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_CSR_ER_Sparse_Idx8)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_VoV_ER_Sparse)     ->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_VoV_ER_Sparse_Idx4)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();

// ---------------------------------------------------------------------------
// 2D grid  (rows = cols = sqrt(n), E/V ≈ 4)
// ---------------------------------------------------------------------------

DEF_BM_DEFAULT(BM_Dijkstra_CSR_Grid,      graph::benchmark::csr_graph_t, make_csr,
               graph::benchmark::grid_2d(GRID_SQRT(n), GRID_SQRT(n)), GRID_SQRT(n) * GRID_SQRT(n))
DEF_BM_IDX2   (BM_Dijkstra_CSR_Grid_Idx2, graph::benchmark::csr_graph_t, make_csr,
               graph::benchmark::grid_2d(GRID_SQRT(n), GRID_SQRT(n)), GRID_SQRT(n) * GRID_SQRT(n))
DEF_BM_IDX4   (BM_Dijkstra_CSR_Grid_Idx4, graph::benchmark::csr_graph_t, make_csr,
               graph::benchmark::grid_2d(GRID_SQRT(n), GRID_SQRT(n)), GRID_SQRT(n) * GRID_SQRT(n))
DEF_BM_IDX8   (BM_Dijkstra_CSR_Grid_Idx8, graph::benchmark::csr_graph_t, make_csr,
               graph::benchmark::grid_2d(GRID_SQRT(n), GRID_SQRT(n)), GRID_SQRT(n) * GRID_SQRT(n))

DEF_BM_DEFAULT(BM_Dijkstra_VoV_Grid,      graph::benchmark::vov_graph_t, make_vov,
               graph::benchmark::grid_2d(GRID_SQRT(n), GRID_SQRT(n)), GRID_SQRT(n) * GRID_SQRT(n))
DEF_BM_IDX4   (BM_Dijkstra_VoV_Grid_Idx4, graph::benchmark::vov_graph_t, make_vov,
               graph::benchmark::grid_2d(GRID_SQRT(n), GRID_SQRT(n)), GRID_SQRT(n) * GRID_SQRT(n))

BENCHMARK(BM_Dijkstra_CSR_Grid)      ->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_CSR_Grid_Idx2)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_CSR_Grid_Idx4)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_CSR_Grid_Idx8)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_VoV_Grid)     ->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_VoV_Grid_Idx4)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();

// ---------------------------------------------------------------------------
// Barabási–Albert, m=4  (E/V ≈ 8, heavy hub traffic)
// ---------------------------------------------------------------------------

DEF_BM_DEFAULT(BM_Dijkstra_CSR_BA,      graph::benchmark::csr_graph_t, make_csr, graph::benchmark::barabasi_albert(n, 4), n)
DEF_BM_IDX2   (BM_Dijkstra_CSR_BA_Idx2, graph::benchmark::csr_graph_t, make_csr, graph::benchmark::barabasi_albert(n, 4), n)
DEF_BM_IDX4   (BM_Dijkstra_CSR_BA_Idx4, graph::benchmark::csr_graph_t, make_csr, graph::benchmark::barabasi_albert(n, 4), n)
DEF_BM_IDX8   (BM_Dijkstra_CSR_BA_Idx8, graph::benchmark::csr_graph_t, make_csr, graph::benchmark::barabasi_albert(n, 4), n)

DEF_BM_DEFAULT(BM_Dijkstra_VoV_BA,      graph::benchmark::vov_graph_t, make_vov, graph::benchmark::barabasi_albert(n, 4), n)
DEF_BM_IDX4   (BM_Dijkstra_VoV_BA_Idx4, graph::benchmark::vov_graph_t, make_vov, graph::benchmark::barabasi_albert(n, 4), n)

BENCHMARK(BM_Dijkstra_CSR_BA)      ->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_CSR_BA_Idx2)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_CSR_BA_Idx4)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_CSR_BA_Idx8)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_VoV_BA)     ->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_VoV_BA_Idx4)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();

// ---------------------------------------------------------------------------
// Path graph  (E/V = 1, minimum decrease-key)
// ---------------------------------------------------------------------------

DEF_BM_DEFAULT(BM_Dijkstra_CSR_Path,      graph::benchmark::csr_graph_t, make_csr, graph::benchmark::path_graph(n), n)
DEF_BM_IDX2   (BM_Dijkstra_CSR_Path_Idx2, graph::benchmark::csr_graph_t, make_csr, graph::benchmark::path_graph(n), n)
DEF_BM_IDX4   (BM_Dijkstra_CSR_Path_Idx4, graph::benchmark::csr_graph_t, make_csr, graph::benchmark::path_graph(n), n)
DEF_BM_IDX8   (BM_Dijkstra_CSR_Path_Idx8, graph::benchmark::csr_graph_t, make_csr, graph::benchmark::path_graph(n), n)

DEF_BM_DEFAULT(BM_Dijkstra_VoV_Path,      graph::benchmark::vov_graph_t, make_vov, graph::benchmark::path_graph(n), n)
DEF_BM_IDX4   (BM_Dijkstra_VoV_Path_Idx4, graph::benchmark::vov_graph_t, make_vov, graph::benchmark::path_graph(n), n)

BENCHMARK(BM_Dijkstra_CSR_Path)      ->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_CSR_Path_Idx2)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_CSR_Path_Idx4)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_CSR_Path_Idx8)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_VoV_Path)     ->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_VoV_Path_Idx4)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();

// ---------------------------------------------------------------------------
// Optional large-scale tier  (V = 1 000 000)
// Enable with: cmake -DDIJKSTRA_BENCH_LARGE=ON ...
// ---------------------------------------------------------------------------

#ifdef DIJKSTRA_BENCH_LARGE
BENCHMARK(BM_Dijkstra_CSR_ER_Sparse)      ->Arg(1'000'000)->Complexity();
BENCHMARK(BM_Dijkstra_CSR_ER_Sparse_Idx4)->Arg(1'000'000)->Complexity();
BENCHMARK(BM_Dijkstra_VoV_ER_Sparse)     ->Arg(1'000'000)->Complexity();
BENCHMARK(BM_Dijkstra_VoV_ER_Sparse_Idx4)->Arg(1'000'000)->Complexity();
BENCHMARK(BM_Dijkstra_CSR_BA)      ->Arg(1'000'000)->Complexity();
BENCHMARK(BM_Dijkstra_CSR_BA_Idx4)->Arg(1'000'000)->Complexity();
#endif

// ---------------------------------------------------------------------------
// BGL comparison benchmarks (Phase 4.3)
//
// Enabled with -DDIJKSTRA_BENCH_BGL=ON -DBGL_INCLUDE_DIR=/path/to/boost.
// Topologically identical graphs are built from the same edge_list, so the
// BGL and graph-v3 numbers can be compared directly.
//
// BGL uses dijkstra_shortest_paths_no_color_map_no_init for fairness:
// caller pre-initialises distances; no color-map allocation inside the
// timed region (matches graph-v3's no-init semantics).
// ---------------------------------------------------------------------------

#ifdef BENCH_BGL

#define DEFINE_BGL_DIJKSTRA_BM(NAME, GRAPH_T, MAKE_FN, EDGE_EXPR, N_EXPR)            \
  static void NAME(benchmark::State& state) {                                        \
    const auto n = static_cast<graph::benchmark::vertex_id_t>(state.range(0));       \
    const auto edges = (EDGE_EXPR);                                                  \
    GRAPH_T    g     = graph::benchmark::MAKE_FN(edges, (N_EXPR));                   \
    std::vector<double> dist(boost::num_vertices(g),                                 \
                              std::numeric_limits<double>::max());                   \
    for (auto _ : state) {                                                           \
      state.PauseTiming();                                                           \
      std::fill(dist.begin(), dist.end(), std::numeric_limits<double>::max());       \
      dist[0] = 0.0;                                                                  \
      state.ResumeTiming();                                                          \
      graph::benchmark::run_bgl_dijkstra(g, 0u, dist);                                \
      benchmark::DoNotOptimize(dist.data());                                         \
    }                                                                                \
    state.SetComplexityN(state.range(0));                                            \
  }

DEFINE_BGL_DIJKSTRA_BM(BM_Dijkstra_BGL_CSR_ER_Sparse,
                       graph::benchmark::bgl_csr_graph_t, make_bgl_csr,
                       ER_EDGES(n), n)
DEFINE_BGL_DIJKSTRA_BM(BM_Dijkstra_BGL_CSR_Grid,
                       graph::benchmark::bgl_csr_graph_t, make_bgl_csr,
                       graph::benchmark::grid_2d(GRID_SQRT(n), GRID_SQRT(n)),
                       GRID_SQRT(n) * GRID_SQRT(n))
DEFINE_BGL_DIJKSTRA_BM(BM_Dijkstra_BGL_CSR_BA,
                       graph::benchmark::bgl_csr_graph_t, make_bgl_csr,
                       graph::benchmark::barabasi_albert(n, 4), n)
DEFINE_BGL_DIJKSTRA_BM(BM_Dijkstra_BGL_CSR_Path,
                       graph::benchmark::bgl_csr_graph_t, make_bgl_csr,
                       graph::benchmark::path_graph(n), n)

DEFINE_BGL_DIJKSTRA_BM(BM_Dijkstra_BGL_Adj_ER_Sparse,
                       graph::benchmark::bgl_adj_graph_t, make_bgl_adj,
                       ER_EDGES(n), n)
DEFINE_BGL_DIJKSTRA_BM(BM_Dijkstra_BGL_Adj_Grid,
                       graph::benchmark::bgl_adj_graph_t, make_bgl_adj,
                       graph::benchmark::grid_2d(GRID_SQRT(n), GRID_SQRT(n)),
                       GRID_SQRT(n) * GRID_SQRT(n))
DEFINE_BGL_DIJKSTRA_BM(BM_Dijkstra_BGL_Adj_BA,
                       graph::benchmark::bgl_adj_graph_t, make_bgl_adj,
                       graph::benchmark::barabasi_albert(n, 4), n)
DEFINE_BGL_DIJKSTRA_BM(BM_Dijkstra_BGL_Adj_Path,
                       graph::benchmark::bgl_adj_graph_t, make_bgl_adj,
                       graph::benchmark::path_graph(n), n)

BENCHMARK(BM_Dijkstra_BGL_CSR_ER_Sparse)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_BGL_CSR_Grid)     ->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_BGL_CSR_BA)       ->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_BGL_CSR_Path)     ->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_BGL_Adj_ER_Sparse)->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_BGL_Adj_Grid)     ->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_BGL_Adj_BA)       ->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();
BENCHMARK(BM_Dijkstra_BGL_Adj_Path)     ->RangeMultiplier(10)->Range(1'000, 100'000)->Complexity();

// ---------------------------------------------------------------------------
// Cross-library distance correctness check.
//
// Runs once at startup before benchmarks begin. For each topology, builds
// the same edge list with both libraries, runs Dijkstra from vertex 0,
// and asserts the resulting distance vectors match. Differences indicate
// a wrapper or property-map bug rather than a real performance signal.
// ---------------------------------------------------------------------------

namespace {

bool check_bgl_distance_parity() {
  using namespace graph::benchmark;
  constexpr vertex_id_t n = 1024;

  auto check = [&](const auto& edges) {
    const std::size_t nv = static_cast<std::size_t>(n);

    // graph-v3
    auto g3 = make_csr(edges, n);
    std::vector<double> d3(nv, std::numeric_limits<double>::max());
    graph::dijkstra_shortest_distances(
          g3, vertex_id_t{0}, graph::container_value_fn(d3),
          weight_fn, graph::empty_visitor{},
          std::less<double>{}, std::plus<double>{},
          graph::use_default_heap{}, std::allocator<std::byte>{});

    // BGL
    auto g_bgl = make_bgl_csr(edges, n);
    std::vector<double> d_bgl(nv, std::numeric_limits<double>::max());
    d_bgl[0] = 0.0;
    run_bgl_dijkstra(g_bgl, vertex_id_t{0}, d_bgl);

    if (d3.size() != d_bgl.size()) return false;
    for (std::size_t i = 0; i < nv; ++i) {
      // Both write infinity for unreachable; otherwise distances must agree
      // exactly (same edge weights, same compare/combine).
      if (d3[i] != d_bgl[i]) return false;
    }
    return true;
  };

  bool ok = true;
  ok &= check(erdos_renyi(n, 8.0 / static_cast<double>(n)));
  ok &= check(barabasi_albert(n, 4));
  ok &= check(path_graph(n));
  return ok;
}

const bool kBglParityChecked = [] {
  if (!check_bgl_distance_parity()) {
    std::fprintf(stderr,
                 "FATAL: BGL vs graph-v3 distance parity check failed; "
                 "benchmarks would compare incorrect results.\n");
    std::abort();
  }
  return true;
}();

} // namespace

#endif // BENCH_BGL

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

BENCHMARK_MAIN();
