# Algorithm Benchmarks CMakeLists.txt
# Performance benchmarks for graph algorithms

# ---------------------------------------------------------------------------
# Dijkstra benchmark (Phase 0 — baseline capture)
# ---------------------------------------------------------------------------

add_executable(benchmark_dijkstra
    benchmark_dijkstra.cpp
)

target_link_libraries(benchmark_dijkstra
    PRIVATE
        graph::graph3
        benchmark::benchmark
)

target_include_directories(benchmark_dijkstra
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Optional large-scale tier (V = 1 000 000): cmake -DDIJKSTRA_BENCH_LARGE=ON
option(DIJKSTRA_BENCH_LARGE "Enable V=1000000 benchmark tier" OFF)
if(DIJKSTRA_BENCH_LARGE)
    target_compile_definitions(benchmark_dijkstra PRIVATE DIJKSTRA_BENCH_LARGE)
endif()

# Register with CTest using a short minimum time so CI stays fast.
# For proper baseline capture use: ./benchmark_dijkstra --benchmark_min_time=1.0
add_test(NAME benchmark_dijkstra
    COMMAND benchmark_dijkstra --benchmark_min_time=0.1)
