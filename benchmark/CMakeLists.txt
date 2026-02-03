# Benchmark Configuration
# Performance benchmarks for graph library components

include(FetchContent)

# Fetch Google Benchmark
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.1
    GIT_SHALLOW TRUE
)

# Don't build benchmark tests
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(benchmark)

# Benchmark executables
add_executable(graph_benchmarks
    benchmark_main.cpp
    benchmark_vertex_access.cpp
    benchmark_views.cpp
)

target_link_libraries(graph_benchmarks
    PRIVATE
        graph::graph3
        benchmark::benchmark
)

# Register benchmarks with CTest
add_test(NAME graph_benchmarks 
    COMMAND graph_benchmarks --benchmark_min_time=0.1)

# Add algorithm benchmarks subdirectory
add_subdirectory(algorithms)
