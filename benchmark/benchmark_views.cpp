/**
 * @file benchmark_views.cpp
 * @brief Performance benchmarks for graph views
 * 
 * Benchmarks measure iteration performance for all view types:
 * - Basic views (vertexlist, incidence, neighbors, edgelist)
 * - Search views (DFS, BFS, topological sort)
 * - Comparison with manual iteration where applicable
 */

#include <benchmark/benchmark.h>
#include <graph/graph.hpp>
#include <graph/views.hpp>
#include <vector>
#include <random>

using namespace graph;
using namespace graph::views::adaptors;

// Test graph type: vector-of-vectors (adjacency list)
using TestGraph = std::vector<std::vector<int>>;

// Create a random directed graph
TestGraph create_random_graph(size_t num_vertices, size_t avg_degree) {
  TestGraph    g(num_vertices);
  std::mt19937 rng(42); // Fixed seed for reproducibility

  std::uniform_int_distribution<size_t> dist(0, num_vertices - 1);

  for (size_t u = 0; u < num_vertices; ++u) {
    size_t degree = std::poisson_distribution<size_t>(avg_degree)(rng);
    for (size_t i = 0; i < degree; ++i) {
      size_t v = dist(rng);
      if (v != u) { // Avoid self-loops
        g[u].push_back(static_cast<int>(v));
      }
    }
  }

  return g;
}

// Create a path graph (0 -> 1 -> 2 -> ... -> n-1)
TestGraph create_path_graph(size_t num_vertices) {
  TestGraph g(num_vertices);
  for (size_t i = 0; i + 1 < num_vertices; ++i) {
    g[i].push_back(static_cast<int>(i + 1));
  }
  return g;
}

// Create a complete graph (all vertices connected)
TestGraph create_complete_graph(size_t num_vertices) {
  TestGraph g(num_vertices);
  for (size_t u = 0; u < num_vertices; ++u) {
    for (size_t v = 0; v < num_vertices; ++v) {
      if (u != v) {
        g[u].push_back(static_cast<int>(v));
      }
    }
  }
  return g;
}

// Create a DAG (directed acyclic graph) for topological sort
TestGraph create_dag(size_t num_vertices) {
  TestGraph g(num_vertices);
  for (size_t u = 0; u < num_vertices; ++u) {
    // Connect to next few vertices only (maintains DAG property)
    for (size_t v = u + 1; v < std::min(u + 5, num_vertices); ++v) {
      g[u].push_back(static_cast<int>(v));
    }
  }
  return g;
}

//=============================================================================
// Basic Views Benchmarks
//=============================================================================

// Benchmark: vertexlist iteration
static void BM_Vertexlist_Iteration(benchmark::State& state) {
  auto g = create_random_graph(state.range(0), 5);

  for (auto _ : state) {
    size_t count = 0;
    for (auto [id, v] : g | vertexlist()) {
      benchmark::DoNotOptimize(v);
      ++count;
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Vertexlist_Iteration)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

// Benchmark: vertexlist with value function
static void BM_Vertexlist_WithValueFunction(benchmark::State& state) {
  auto g   = create_random_graph(state.range(0), 5);
  auto vvf = [](const auto& g, auto v) { return vertex_id(g, v); };

  for (auto _ : state) {
    size_t sum = 0;
    for (auto [vid, v, id] : g | vertexlist(vvf)) {
      benchmark::DoNotOptimize(v);
      sum += id;
    }
    benchmark::DoNotOptimize(sum);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Vertexlist_WithValueFunction)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

// Benchmark: incidence iteration (all vertices)
static void BM_Incidence_AllVertices(benchmark::State& state) {
  auto g = create_random_graph(state.range(0), 5);

  for (auto _ : state) {
    size_t count = 0;
    for (size_t u = 0; u < g.size(); ++u) {
      for (auto [tid, e] : g | incidence(u)) {
        benchmark::DoNotOptimize(e);
        ++count;
      }
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Incidence_AllVertices)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

// Benchmark: neighbors iteration (all vertices)
static void BM_Neighbors_AllVertices(benchmark::State& state) {
  auto g = create_random_graph(state.range(0), 5);

  for (auto _ : state) {
    size_t count = 0;
    for (size_t u = 0; u < g.size(); ++u) {
      for (auto [tid, n] : g | neighbors(u)) {
        benchmark::DoNotOptimize(n);
        ++count;
      }
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Neighbors_AllVertices)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

// Benchmark: edgelist iteration
static void BM_Edgelist_Iteration(benchmark::State& state) {
  auto g = create_random_graph(state.range(0), 5);

  for (auto _ : state) {
    size_t count = 0;
    for (auto [sid, tid, e] : g | edgelist()) {
      benchmark::DoNotOptimize(e);
      ++count;
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Edgelist_Iteration)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

//=============================================================================
// Search Views Benchmarks
//=============================================================================

// Benchmark: DFS vertices traversal
static void BM_DFS_Vertices(benchmark::State& state) {
  auto g = create_random_graph(state.range(0), 5);

  for (auto _ : state) {
    size_t count = 0;
    for (auto [v] : g | vertices_dfs(0)) {
      benchmark::DoNotOptimize(v);
      ++count;
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_DFS_Vertices)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

// Benchmark: DFS edges traversal
static void BM_DFS_Edges(benchmark::State& state) {
  auto g = create_random_graph(state.range(0), 5);

  for (auto _ : state) {
    size_t count = 0;
    for (auto [e] : g | edges_dfs(0)) {
      benchmark::DoNotOptimize(e);
      ++count;
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_DFS_Edges)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

// Benchmark: BFS vertices traversal
static void BM_BFS_Vertices(benchmark::State& state) {
  auto g = create_random_graph(state.range(0), 5);

  for (auto _ : state) {
    size_t count = 0;
    for (auto [v] : g | vertices_bfs(0)) {
      benchmark::DoNotOptimize(v);
      ++count;
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_BFS_Vertices)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

// Benchmark: BFS edges traversal
static void BM_BFS_Edges(benchmark::State& state) {
  auto g = create_random_graph(state.range(0), 5);

  for (auto _ : state) {
    size_t count = 0;
    for (auto [e] : g | edges_bfs(0)) {
      benchmark::DoNotOptimize(e);
      ++count;
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_BFS_Edges)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

// Benchmark: Topological sort vertices
static void BM_TopoSort_Vertices(benchmark::State& state) {
  auto g = create_dag(state.range(0));

  for (auto _ : state) {
    size_t count = 0;
    for (auto [v] : g | vertices_topological_sort()) {
      benchmark::DoNotOptimize(v);
      ++count;
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_TopoSort_Vertices)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

// Benchmark: Topological sort edges
static void BM_TopoSort_Edges(benchmark::State& state) {
  auto g = create_dag(state.range(0));

  for (auto _ : state) {
    size_t count = 0;
    for (auto [e] : g | edges_topological_sort()) {
      benchmark::DoNotOptimize(e);
      ++count;
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_TopoSort_Edges)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

//=============================================================================
// Comparison Benchmarks (View vs Manual)
//=============================================================================

// Benchmark: Manual vertex iteration (baseline)
static void BM_Manual_Vertices(benchmark::State& state) {
  auto g = create_random_graph(state.range(0), 5);

  for (auto _ : state) {
    size_t count = 0;
    for (size_t i = 0; i < g.size(); ++i) {
      benchmark::DoNotOptimize(i);
      ++count;
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Manual_Vertices)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

// Benchmark: Manual edge iteration (baseline)
static void BM_Manual_Edges(benchmark::State& state) {
  auto g = create_random_graph(state.range(0), 5);

  for (auto _ : state) {
    size_t count = 0;
    for (size_t u = 0; u < g.size(); ++u) {
      for (const auto& v : g[u]) {
        benchmark::DoNotOptimize(u);
        benchmark::DoNotOptimize(v);
        ++count;
      }
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Manual_Edges)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

//=============================================================================
// Chaining Benchmarks
//=============================================================================

// Benchmark: View chaining with std::views::filter
static void BM_Chaining_Filter(benchmark::State& state) {
  auto g = create_random_graph(state.range(0), 5);

  for (auto _ : state) {
    size_t count    = 0;
    auto   filtered = g | vertexlist() | std::views::filter([&g](auto info) {
                      auto [id, v] = info;
                      return id % 2 == 0;
                    });

    for (auto info : filtered) {
      benchmark::DoNotOptimize(info);
      ++count;
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Chaining_Filter)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

// Benchmark: View chaining with std::views::transform
static void BM_Chaining_Transform(benchmark::State& state) {
  auto g = create_random_graph(state.range(0), 5);

  for (auto _ : state) {
    size_t sum         = 0;
    auto   transformed = g | vertexlist() | std::views::transform([&g](auto info) {
                         auto [id, v] = info;
                         return id;
                       });

    for (auto id : transformed) {
      sum += id;
    }
    benchmark::DoNotOptimize(sum);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Chaining_Transform)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

// Benchmark: View chaining with std::views::take
static void BM_Chaining_Take(benchmark::State& state) {
  auto g = create_random_graph(state.range(0), 5);

  for (auto _ : state) {
    size_t count   = 0;
    auto   limited = g | vertexlist() | std::views::take(100); // Take first 100

    for (auto info : limited) {
      benchmark::DoNotOptimize(info);
      ++count;
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Chaining_Take)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

//=============================================================================
// Graph Type Benchmarks
//=============================================================================

// Benchmark: DFS on path graph (worst case - linear structure)
static void BM_DFS_PathGraph(benchmark::State& state) {
  auto g = create_path_graph(state.range(0));

  for (auto _ : state) {
    size_t count = 0;
    for (auto [v] : g | vertices_dfs(0)) {
      benchmark::DoNotOptimize(v);
      ++count;
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_DFS_PathGraph)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

// Benchmark: BFS on path graph
static void BM_BFS_PathGraph(benchmark::State& state) {
  auto g = create_path_graph(state.range(0));

  for (auto _ : state) {
    size_t count = 0;
    for (auto [v] : g | vertices_bfs(0)) {
      benchmark::DoNotOptimize(v);
      ++count;
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_BFS_PathGraph)->RangeMultiplier(2)->Range(100, 10000)->Complexity();

// Benchmark: DFS on complete graph (dense)
static void BM_DFS_CompleteGraph(benchmark::State& state) {
  auto g = create_complete_graph(state.range(0));

  for (auto _ : state) {
    size_t count = 0;
    for (auto [v] : g | vertices_dfs(0)) {
      benchmark::DoNotOptimize(v);
      ++count;
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_DFS_CompleteGraph)
      ->RangeMultiplier(2)
      ->Range(10, 100) // Smaller range for complete graphs
      ->Complexity();

// Benchmark: BFS on complete graph (dense)
static void BM_BFS_CompleteGraph(benchmark::State& state) {
  auto g = create_complete_graph(state.range(0));

  for (auto _ : state) {
    size_t count = 0;
    for (auto [v] : g | vertices_bfs(0)) {
      benchmark::DoNotOptimize(v);
      ++count;
    }
    benchmark::DoNotOptimize(count);
  }

  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_BFS_CompleteGraph)
      ->RangeMultiplier(2)
      ->Range(10, 100) // Smaller range for complete graphs
      ->Complexity();
