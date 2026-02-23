# Code Coverage Report

> **Generated:** 2026-02-23 | **Compiler:** GCC 15.1.0 | **Preset:** `linux-gcc-coverage`
> **Tests:** 4343 passed, 0 failed (100% pass rate)
> **Overall line coverage:** 96.3% (3624 / 3764 lines)
> **Overall function coverage:** 91.8% (29030 / 31639 functions)

---

## Summary by Category

| Category | Lines Hit | Lines Total | Line Coverage |
|----------|-----------|-------------|---------------|
| Adjacency list infrastructure | 375 | 375 | **100.0%** |
| Algorithms | 795 | 833 | **95.4%** |
| Containers | 938 | 1000 | **93.8%** |
| Detail / CPOs | 24 | 24 | **100.0%** |
| Edge list | 15 | 16 | **93.8%** |
| Views | 1477 | 1514 | **97.6%** |

---

## Detailed File Coverage

### Adjacency List Infrastructure (`adj_list/`)

All adjacency list descriptor and CPO support headers reach **100% line coverage**.

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `descriptor_traits.hpp` | 10 / 10 | 100.0% | 8 / 8 | 100.0% |
| `detail/graph_cpo.hpp` | 179 / 179 | 100.0% | 4500 / 4503 | 99.9% |
| `edge_descriptor.hpp` | 84 / 84 | 100.0% | 1293 / 1293 | 100.0% |
| `edge_descriptor_view.hpp` | 37 / 37 | 100.0% | 2453 / 2454 | 100.0% |
| `vertex_descriptor.hpp` | 35 / 35 | 100.0% | 1379 / 1380 | 99.9% |
| `vertex_descriptor_view.hpp` | 30 / 30 | 100.0% | 2410 / 2481 | 97.1% |

### Algorithms (`algorithm/`)

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `articulation_points.hpp` | 53 / 53 | 100.0% | 2 / 2 | 100.0% |
| `bellman_ford_shortest_paths.hpp` | 54 / 63 | 85.7% | 31 / 31 | 100.0% |
| `biconnected_components.hpp` | 68 / 68 | 100.0% | 4 / 4 | 100.0% |
| `breadth_first_search.hpp` | 29 / 29 | 100.0% | 12 / 12 | 100.0% |
| `connected_components.hpp` | 168 / 182 | 92.3% | 18 / 18 | 100.0% |
| `depth_first_search.hpp` | 38 / 38 | 100.0% | 5 / 5 | 100.0% |
| `dijkstra_shortest_paths.hpp` | 65 / 65 | 100.0% | 36 / 36 | 100.0% |
| `jaccard.hpp` | 24 / 24 | 100.0% | 5 / 5 | 100.0% |
| `label_propagation.hpp` | 66 / 69 | 95.7% | 3 / 3 | 100.0% |
| `mis.hpp` | 25 / 28 | 89.3% | 1 / 1 | 100.0% |
| `mst.hpp` | 117 / 126 | 92.9% | 24 / 24 | 100.0% |
| `tc.hpp` | 27 / 27 | 100.0% | 2 / 2 | 100.0% |
| `topological_sort.hpp` | 52 / 52 | 100.0% | 7 / 7 | 100.0% |
| `traversal_common.hpp` | 9 / 9 | 100.0% | 3 / 3 | 100.0% |

### Containers (`container/`)

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `compressed_graph.hpp` | 198 / 223 | 88.8% | 580 / 606 | 95.7% |
| `container_utility.hpp` | 6 / 6 | 100.0% | 393 / 393 | 100.0% |
| `detail/undirected_adjacency_list_impl.hpp` | 312 / 321 | 97.2% | 158 / 163 | 96.9% |
| `dynamic_graph.hpp` | 316 / 343 | 92.1% | 11986 / 14415 | 83.1% |
| `undirected_adjacency_list.hpp` | 106 / 107 | 99.1% | 132 / 134 | 98.5% |

### Views (`views/`)

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `adaptors.hpp` | 146 / 146 | 100.0% | 113 / 113 | 100.0% |
| `bfs.hpp` | 218 / 234 | 93.2% | 193 / 204 | 94.6% |
| `dfs.hpp` | 232 / 246 | 94.3% | 316 / 327 | 96.6% |
| `edge_accessor.hpp` | 12 / 12 | 100.0% | 55 / 55 | 100.0% |
| `edgelist.hpp` | 219 / 221 | 99.1% | 556 / 578 | 96.2% |
| `incidence.hpp` | 124 / 124 | 100.0% | 613 / 619 | 99.0% |
| `neighbors.hpp` | 134 / 134 | 100.0% | 386 / 390 | 99.0% |
| `search_base.hpp` | 5 / 5 | 100.0% | 11 / 11 | 100.0% |
| `topological_sort.hpp` | 252 / 257 | 98.1% | 296 / 305 | 97.0% |
| `transpose.hpp` | 23 / 23 | 100.0% | 22 / 22 | 100.0% |
| `vertexlist.hpp` | 112 / 112 | 100.0% | 507 / 509 | 99.6% |

### Other

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `detail/edge_cpo.hpp` | 24 / 24 | 100.0% | 500 / 500 | 100.0% |
| `edge_list/edge_list_descriptor.hpp` | 15 / 16 | 93.8% | 21 / 21 | 100.0% |
| `graph_info.hpp` | 0 / 2 | 0.0% | 0 / 2 | 0.0% |

---

## Coverage Gaps

Files below 90% line coverage, ranked by gap size:

| File | Line % | Lines Missed | Notes |
|------|--------|--------------|-------|
| `graph_info.hpp` | 0.0% | 2 | Metadata-only header; no executable paths exercised |
| `bellman_ford_shortest_paths.hpp` | 85.7% | 9 | Negative-cycle detection path not fully exercised |
| `compressed_graph.hpp` | 88.8% | 25 | Some construction / accessor overloads untested |
| `mis.hpp` | 89.3% | 3 | Some branches not fully exercised |

---

## How to Reproduce

```bash
# Configure with coverage preset
cmake --preset linux-gcc-coverage

# Build
cmake --build --preset linux-gcc-coverage

# Run tests
ctest --preset linux-gcc-coverage

# Generate HTML report (output in build/linux-gcc-coverage/coverage_html/)
cd build/linux-gcc-coverage
lcov --directory . --capture --output-file coverage.info --ignore-errors mismatch
lcov --remove coverage.info '/usr/*' '/opt/*' '*/tests/*' '*/build/*' '*/_deps/*' \
     --output-file coverage.info --ignore-errors mismatch
genhtml coverage.info --output-directory coverage_html --ignore-errors mismatch
```

Open `build/linux-gcc-coverage/coverage_html/index.html` in a browser for the full interactive report.
