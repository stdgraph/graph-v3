#include <benchmark/benchmark.h>
#include <vector>
#include <map>
#include <graph/adj_list/vertex_descriptor.hpp>
#include <graph/adj_list/vertex_descriptor_view.hpp>

using namespace std;
using namespace graph;

// Benchmark vertex_descriptor creation with vector
static void BM_VertexDescriptor_Vector_Creation(benchmark::State& state) {
    using VertexIter = vector<int>::iterator;
    
    for (auto _ : state) {
        auto desc = vertex_descriptor<VertexIter>(0);
        benchmark::DoNotOptimize(desc);
    }
}
BENCHMARK(BM_VertexDescriptor_Vector_Creation);

// Benchmark vertex_descriptor_view iteration with vector
static void BM_VertexDescriptorView_Vector_Iteration(benchmark::State& state) {
    vector<int> vertices;
    for (int i = 0; i < state.range(0); ++i) {
        vertices.push_back(i);
    }
    
    for (auto _ : state) {
        size_t count = 0;
        for (auto desc : vertex_descriptor_view(vertices)) {
            benchmark::DoNotOptimize(desc);
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_VertexDescriptorView_Vector_Iteration)
    ->RangeMultiplier(10)
    ->Range(10, 10000)
    ->Complexity();

// Benchmark vertex_descriptor_view iteration with map
static void BM_VertexDescriptorView_Map_Iteration(benchmark::State& state) {
    map<int, string> vertices;
    for (int i = 0; i < state.range(0); ++i) {
        vertices[i] = "vertex_" + to_string(i);
    }
    
    for (auto _ : state) {
        size_t count = 0;
        for (auto desc : vertex_descriptor_view(vertices)) {
            benchmark::DoNotOptimize(desc);
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_VertexDescriptorView_Map_Iteration)
    ->RangeMultiplier(10)
    ->Range(10, 10000)
    ->Complexity();
