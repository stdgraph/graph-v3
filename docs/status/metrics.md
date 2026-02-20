# Canonical Metrics

> **Single source of truth for all user-facing counts.**
> All documentation (README, docs/index.md, etc.) must reference these values
> rather than maintaining independent numbers. Re-run `ctest` and update when
> the implementation surface changes.

| Metric | Value | Source |
|--------|-------|--------|
| Algorithms | 13 | `include/graph/algorithm/` (excluding `traversal_common.hpp`) |
| Views | 7 | vertexlist, edgelist, incidence, neighbors, bfs, dfs, topological_sort |
| Containers | 3 | `dynamic_graph`, `compressed_graph`, `undirected_adjacency_list` |
| Trait combinations | 26 | `include/graph/container/traits/` |
| Test count | 4261 | `ctest --preset linux-gcc-debug` (100% pass, 2026-02-19) |
| C++ standard | C++20 | `CMakeLists.txt` line 26 |
| License | BSL-1.0 | `LICENSE` |
| CMake minimum | 3.20 | `CMakeLists.txt` line 1 |
| Consumer target | `graph::graph3` | `cmake/InstallConfig.cmake` (`find_package(graph3)`) |
