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
#
# BGL is header-only, so we only need an include directory containing
# <boost/graph/dijkstra_shortest_paths.hpp>. No linking required.
#
# Resolution order for the include directory (first match wins):
#   1. Explicit cache var:        cmake -DBGL_INCLUDE_DIR=/path/to/boost
#   2. Environment variable:      $env:BGL_INCLUDE_DIR or $BGL_INCLUDE_DIR
#   3. Environment variable:      $env:BOOST_ROOT     or $BOOST_ROOT
#   4. Per-platform defaults (see DIJKSTRA_BENCH_BGL_DEFAULT_PATHS below)
#
# Typical invocations:
#   cmake -DDIJKSTRA_BENCH_BGL=ON                                  # auto-discover
#   cmake -DDIJKSTRA_BENCH_BGL=ON -DBGL_INCLUDE_DIR=D:/dev_graph/boost
#   $env:BGL_INCLUDE_DIR = "D:/dev_graph/boost"; cmake -DDIJKSTRA_BENCH_BGL=ON
option(DIJKSTRA_BENCH_BGL "Enable BGL comparison benchmarks (auto-discovers Boost; override with BGL_INCLUDE_DIR)" OFF)
set(BGL_INCLUDE_DIR "" CACHE PATH "Boost include directory for BGL benchmarks (overrides auto-discovery)")

# Per-platform default search paths. Customise for new environments by either
# setting BGL_INCLUDE_DIR / BOOST_ROOT in the environment or by appending here.
if(WIN32)
    set(DIJKSTRA_BENCH_BGL_DEFAULT_PATHS
        "D:/dev_graph/boost"
        "C:/dev_graph/boost"
        "C:/boost"
        "C:/local/boost"
    )
elseif(APPLE)
    set(DIJKSTRA_BENCH_BGL_DEFAULT_PATHS
        "$ENV{HOME}/dev_graph/boost"
        "/opt/homebrew/include"
        "/usr/local/include"
    )
else() # Linux / other Unix
    set(DIJKSTRA_BENCH_BGL_DEFAULT_PATHS
        "$ENV{HOME}/dev_graph/boost"
        "/usr/include"
        "/usr/local/include"
    )
endif()

if(DIJKSTRA_BENCH_BGL)
    set(_bgl_resolved "")
    set(_bgl_marker  "boost/graph/dijkstra_shortest_paths.hpp")

    # Build candidate list in priority order.
    set(_bgl_candidates "")
    if(BGL_INCLUDE_DIR)
        list(APPEND _bgl_candidates "${BGL_INCLUDE_DIR}")
    endif()
    if(DEFINED ENV{BGL_INCLUDE_DIR} AND NOT "$ENV{BGL_INCLUDE_DIR}" STREQUAL "")
        list(APPEND _bgl_candidates "$ENV{BGL_INCLUDE_DIR}")
    endif()
    if(DEFINED ENV{BOOST_ROOT} AND NOT "$ENV{BOOST_ROOT}" STREQUAL "")
        list(APPEND _bgl_candidates "$ENV{BOOST_ROOT}")
    endif()
    list(APPEND _bgl_candidates ${DIJKSTRA_BENCH_BGL_DEFAULT_PATHS})

    foreach(_cand IN LISTS _bgl_candidates)
        if(_cand AND EXISTS "${_cand}/${_bgl_marker}")
            set(_bgl_resolved "${_cand}")
            break()
        endif()
    endforeach()

    if(NOT _bgl_resolved)
        message(FATAL_ERROR
            "DIJKSTRA_BENCH_BGL=ON but no Boost include directory was found containing "
            "${_bgl_marker}.\n"
            "Tried (in order): ${_bgl_candidates}\n"
            "Set -DBGL_INCLUDE_DIR=/path/to/boost, $env:BGL_INCLUDE_DIR, or $env:BOOST_ROOT, "
            "or append to DIJKSTRA_BENCH_BGL_DEFAULT_PATHS in benchmark/algorithms/CMakeLists.txt.")
    endif()

    target_include_directories(benchmark_dijkstra SYSTEM PRIVATE "${_bgl_resolved}")
    target_compile_definitions(benchmark_dijkstra PRIVATE BENCH_BGL)
    message(STATUS "BGL comparison benchmarks enabled (using ${_bgl_resolved})")
endif()

# Register with CTest using a short minimum time so CI stays fast.
# For proper baseline capture use: ./benchmark_dijkstra --benchmark_min_time=1.0
add_test(NAME benchmark_dijkstra
    COMMAND benchmark_dijkstra --benchmark_min_time=0.1)
