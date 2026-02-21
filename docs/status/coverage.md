# Code Coverage Report

> **Generated:** 2026-02-21 | **Compiler:** GCC 15.1.0 | **Preset:** `linux-gcc-coverage`
> **Tests:** 4261 passed, 0 failed (100% pass rate)
> **Overall line coverage:** 95.8% (3300 / 3446 lines)
> **Overall function coverage:** 92.0% (27193 / 29567 functions)

---

## Summary by Category

| Category | Lines Hit | Lines Total | Line Coverage |
|----------|-----------|-------------|---------------|
| Adjacency list infrastructure | 321 | 321 | **100.0%** |
| Algorithms | 739 | 788 | **93.8%** |
| Containers | 902 | 962 | **93.8%** |
| Detail / CPOs | 21 | 21 | **100.0%** |
| Edge list | 15 | 16 | **93.8%** |
| Views | 1302 | 1336 | **97.5%** |

---

## Detailed File Coverage

### Adjacency List Infrastructure (`adj_list/`)

All adjacency list descriptor and CPO support headers reach **100% line coverage**.

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `descriptor_traits.hpp` | 10 / 10 | 100.0% | 8 / 8 | 100.0% |
| `detail/graph_cpo.hpp` | 135 / 135 | 100.0% | 4250 / 4253 | 99.9% |
| `edge_descriptor.hpp` | 74 / 74 | 100.0% | 1205 / 1205 | 100.0% |
| `edge_descriptor_view.hpp` | 37 / 37 | 100.0% | 2291 / 2292 | 100.0% |
| `vertex_descriptor.hpp` | 35 / 35 | 100.0% | 1305 / 1306 | 99.9% |
| `vertex_descriptor_view.hpp` | 30 / 30 | 100.0% | 2321 / 2390 | 97.1% |

### Algorithms (`algorithm/`)

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `articulation_points.hpp` | 53 / 53 | 100.0% | 2 / 2 | 100.0% |
| `bellman_ford_shortest_paths.hpp` | 54 / 63 | 85.7% | 31 / 31 | 100.0% |
| `biconnected_components.hpp` | 68 / 68 | 100.0% | 4 / 4 | 100.0% |
| `breadth_first_search.hpp` | 29 / 29 | 100.0% | 12 / 12 | 100.0% |
| `connected_components.hpp` | 123 / 137 | 89.8% | 10 / 10 | 100.0% |
| `depth_first_search.hpp` | 38 / 38 | 100.0% | 5 / 5 | 100.0% |
| `dijkstra_shortest_paths.hpp` | 54 / 65 | 83.1% | 32 / 36 | 88.9% |
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
| `container_utility.hpp` | 6 / 6 | 100.0% | 382 / 382 | 100.0% |
| `detail/undirected_adjacency_list_impl.hpp` | 312 / 321 | 97.2% | 158 / 163 | 96.9% |
| `dynamic_graph.hpp` | 286 / 311 | 92.0% | 11429 / 13626 | 83.9% |
| `undirected_adjacency_list.hpp` | 100 / 101 | 99.0% | 126 / 128 | 98.4% |

### Views (`views/`)

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `adaptors.hpp` | 109 / 109 | 100.0% | 85 / 85 | 100.0% |
| `bfs.hpp` | 211 / 227 | 93.0% | 146 / 157 | 93.0% |
| `dfs.hpp` | 225 / 239 | 94.1% | 257 / 268 | 95.9% |
| `edgelist.hpp` | 219 / 221 | 99.1% | 556 / 578 | 96.2% |
| `incidence.hpp` | 89 / 89 | 100.0% | 463 / 469 | 98.7% |
| `neighbors.hpp` | 102 / 102 | 100.0% | 282 / 286 | 98.6% |
| `search_base.hpp` | 5 / 5 | 100.0% | 11 / 11 | 100.0% |
| `topological_sort.hpp` | 230 / 232 | 99.1% | 248 / 256 | 96.9% |
| `vertexlist.hpp` | 112 / 112 | 100.0% | 467 / 469 | 99.6% |

### Other

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `detail/edge_cpo.hpp` | 21 / 21 | 100.0% | 461 / 461 | 100.0% |
| `edge_list/edge_list_descriptor.hpp` | 15 / 16 | 93.8% | 21 / 21 | 100.0% |
| `graph_info.hpp` | 0 / 2 | 0.0% | 0 / 2 | 0.0% |

---

## Coverage Gaps

Files below 90% line coverage, ranked by gap size:

| File | Line % | Lines Missed | Notes |
|------|--------|--------------|-------|
| `graph_info.hpp` | 0.0% | 2 | Metadata-only header; no executable paths exercised |
| `dijkstra_shortest_paths.hpp` | 83.1% | 11 | Some overload / error-path branches not reached |
| `bellman_ford_shortest_paths.hpp` | 85.7% | 9 | Negative-cycle detection path not fully exercised |
| `compressed_graph.hpp` | 88.8% | 25 | Some construction / accessor overloads untested |
| `connected_components.hpp` | 89.8% | 14 | Sparse-graph and edge-case branches |
| `mis.hpp` | 89.3% | 3 | Minor branch not reached |

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
