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

# Optional Boost.Graph (BGL) comparison benchmarks (Phase 4.3).
# Set BGL_INCLUDE_DIR to a Boost include directory containing
# <boost/graph/dijkstra_shortest_paths.hpp> (header-only; no linking).
#   cmake -DDIJKSTRA_BENCH_BGL=ON -DBGL_INCLUDE_DIR=/path/to/boost
option(DIJKSTRA_BENCH_BGL "Enable BGL comparison benchmarks (requires BGL_INCLUDE_DIR)" OFF)
set(BGL_INCLUDE_DIR "" CACHE PATH "Boost include directory for BGL benchmarks")
if(DIJKSTRA_BENCH_BGL)
    if(NOT BGL_INCLUDE_DIR OR NOT EXISTS "${BGL_INCLUDE_DIR}/boost/graph/dijkstra_shortest_paths.hpp")
        message(FATAL_ERROR
            "DIJKSTRA_BENCH_BGL=ON but BGL_INCLUDE_DIR='${BGL_INCLUDE_DIR}' does not "
            "contain boost/graph/dijkstra_shortest_paths.hpp. Set -DBGL_INCLUDE_DIR=/path/to/boost.")
    endif()
    target_include_directories(benchmark_dijkstra SYSTEM PRIVATE "${BGL_INCLUDE_DIR}")
    target_compile_definitions(benchmark_dijkstra PRIVATE BENCH_BGL)
    message(STATUS "BGL comparison benchmarks enabled (using ${BGL_INCLUDE_DIR})")
endif()

# Register with CTest using a short minimum time so CI stays fast.
# For proper baseline capture use: ./benchmark_dijkstra --benchmark_min_time=1.0
add_test(NAME benchmark_dijkstra
    COMMAND benchmark_dijkstra --benchmark_min_time=0.1)
