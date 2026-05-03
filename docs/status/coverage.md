<table><tr>
<td><img src="../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Code Coverage Report

</td>
</tr></table>

> **Generated:** 2026-05-03 | **Compiler:** GCC 15.1.0 | **Preset:** `linux-gcc-coverage`

> **Tests:** 4873 passed, 0 failed (100% pass rate)
> **Overall line coverage:** 95.6% (4456 / 4663 lines)
> **Overall function coverage:** 92.4% (27107 / 29333 functions)

---

## Summary by Category

| Category | Lines Hit | Lines Total | Line % | Funcs Hit | Funcs Total | Func % |
|----------|-----------|-------------|--------|-----------|-------------|--------|
| Adjacency list infrastructure (`adj_list/`) | 360 | 362 | **99.4%** | 10255 | 10331 | **99.3%** |
| Algorithms (`algorithm/`) | 922 | 943 | **97.8%** | 955 | 992 | **96.3%** |
| Containers (`container/`) | 931 | 994 | **93.7%** | 11002 | 12967 | **84.8%** |
| Detail / internal helpers (`detail/`) | 114 | 114 | **100.0%** | 820 | 837 | **97.9%** |
| Edge list (`edge_list/`) | 15 | 16 | **93.8%** | 21 | 21 | **100.0%** |
| Generators (`generators/`) | 83 | 86 | **96.5%** | 13 | 13 | **100.0%** |
| I/O (`io/`) | 484 | 560 | **86.4%** | 35 | 36 | **97.2%** |
| Views (`views/`) | 1501 | 1538 | **97.6%** | 3951 | 4073 | **97.0%** |
| Adaptors (`adaptors/`) | 46 | 48 | **95.8%** | 55 | 61 | **90.2%** |

---

## Detailed File Coverage

### Adjacency List Infrastructure (`adj_list/`)

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `descriptor_traits.hpp` | 10 / 10 | 100.0% | 8 / 8 | 100.0% |
| `detail/graph_cpo.hpp` | 180 / 180 | 100.0% | 3993 / 3996 | 99.9% |
| `edge_descriptor.hpp` | 51 / 51 | 100.0% | 996 / 996 | 100.0% |
| `edge_descriptor_view.hpp` | 33 / 33 | 100.0% | 1886 / 1887 | 99.9% |
| `vertex_descriptor.hpp` | 37 / 37 | 100.0% | 1155 / 1156 | 99.9% |
| `vertex_descriptor_view.hpp` | 30 / 30 | 100.0% | 2082 / 2153 | 96.7% |
| `vertex_property_map.hpp` | 19 / 21 | 90.5% | 135 / 135 | 100.0% |

> **Not measured** (compile-time only, no executable lines): `adjacency_list_concepts.hpp`,
> `adjacency_list_traits.hpp`, `descriptor.hpp`, `graph_utility.hpp`.

### Algorithms (`algorithm/`)

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `articulation_points.hpp` | 52 / 52 | 100.0% | 8 / 8 | 100.0% |
| `bellman_ford_shortest_paths.hpp` | 58 / 59 | 98.3% | 151 / 157 | 96.2% |
| `biconnected_components.hpp` | 65 / 65 | 100.0% | 16 / 16 | 100.0% |
| `breadth_first_search.hpp` | 29 / 29 | 100.0% | 54 / 54 | 100.0% |
| `connected_components.hpp` | 182 / 195 | 93.3% | 50 / 50 | 100.0% |
| `depth_first_search.hpp` | 41 / 41 | 100.0% | 46 / 46 | 100.0% |
| `dijkstra_shortest_paths.hpp` | 106 / 106 | 100.0% | 430 / 461 | 93.3% |
| `jaccard.hpp` | 22 / 22 | 100.0% | 11 / 11 | 100.0% |
| `label_propagation.hpp` | 43 / 43 | 100.0% | 23 / 23 | 100.0% |
| `mis.hpp` | 26 / 29 | 89.7% | 7 / 7 | 100.0% |
| `mst.hpp` | 121 / 125 | 96.8% | 66 / 66 | 100.0% |
| `tarjan_scc.hpp` | 52 / 52 | 100.0% | 3 / 3 | 100.0% |
| `tc.hpp` | 54 / 54 | 100.0% | 5 / 5 | 100.0% |
| `topological_sort.hpp` | 53 / 53 | 100.0% | 35 / 35 | 100.0% |
| `traversal_common.hpp` | 18 / 18 | 100.0% | 50 / 50 | 100.0% |

### Containers (`container/`)

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `compressed_graph.hpp` | 194 / 220 | 88.2% | 578 / 604 | 95.7% |
| `container_utility.hpp` | 6 / 6 | 100.0% | 301 / 301 | 100.0% |
| `detail/undirected_adjacency_list_impl.hpp` | 312 / 321 | 97.2% | 158 / 163 | 96.9% |
| `dynamic_graph.hpp` | 313 / 340 | 92.1% | 9833 / 11765 | 83.6% |
| `undirected_adjacency_list.hpp` | 106 / 107 | 99.1% | 132 / 134 | 98.5% |

> **Not measured** (compile-time type aliases): `container/traits/*.hpp` (18 graph-traits headers),
> `detail/undirected_adjacency_list_api.hpp`.

### Detail / Internal Helpers (`detail/`)

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `detail/edge_cpo.hpp` | 23 / 23 | 100.0% | 515 / 515 | 100.0% |
| `detail/heap_position_map.hpp` | 16 / 16 | 100.0% | 11 / 11 | 100.0% |
| `detail/indexed_dary_heap.hpp` | 75 / 75 | 100.0% | 294 / 311 | 94.5% |

> **Not measured** (compile-time only): `detail/cpo_common.hpp`, `detail/graph_using.hpp`.

### Edge List (`edge_list/`)

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `edge_list/edge_list_descriptor.hpp` | 15 / 16 | 93.8% | 21 / 21 | 100.0% |

> **Not measured** (type aliases / umbrella includes): `edge_list/edge_list.hpp`, `edge_list/edge_list_traits.hpp`.

### Generators (`generators/`)

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `generators/barabasi_albert.hpp` | 32 / 32 | 100.0% | 4 / 4 | 100.0% |
| `generators/common.hpp` | 10 / 10 | 100.0% | 1 / 1 | 100.0% |
| `generators/erdos_renyi.hpp` | 15 / 16 | 93.8% | 2 / 2 | 100.0% |
| `generators/grid.hpp` | 19 / 20 | 95.0% | 4 / 4 | 100.0% |
| `generators/path.hpp` | 7 / 8 | 87.5% | 2 / 2 | 100.0% |

### I/O (`io/`)

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `io/dot.hpp` | 145 / 158 | 91.8% | 11 / 11 | 100.0% |
| `io/graphml.hpp` | 189 / 210 | 90.0% | 9 / 9 | 100.0% |
| `io/json.hpp` | 150 / 192 | 78.1% | 15 / 16 | 93.8% |

> **Not measured**: `io/detail/common.hpp` (internal helper included transitively).

### Views (`views/`)

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `views/adaptors.hpp` | 146 / 146 | 100.0% | 113 / 113 | 100.0% |
| `views/bfs.hpp` | 222 / 238 | 93.3% | 242 / 253 | 95.7% |
| `views/dfs.hpp` | 232 / 246 | 94.3% | 340 / 351 | 96.9% |
| `views/edge_accessor.hpp` | 12 / 12 | 100.0% | 89 / 89 | 100.0% |
| `views/edgelist.hpp` | 219 / 221 | 99.1% | 898 / 974 | 92.2% |
| `views/incidence.hpp` | 125 / 125 | 100.0% | 715 / 721 | 99.2% |
| `views/neighbors.hpp` | 134 / 134 | 100.0% | 386 / 390 | 99.0% |
| `views/search_base.hpp` | 24 / 24 | 100.0% | 54 / 56 | 96.4% |
| `views/topological_sort.hpp` | 251 / 256 | 98.0% | 330 / 340 | 97.1% |
| `views/transpose.hpp` | 23 / 23 | 100.0% | 21 / 21 | 100.0% |
| `views/vertexlist.hpp` | 113 / 113 | 100.0% | 763 / 765 | 99.7% |

> **Not measured** (compile-time only): `views/view_concepts.hpp`, `views/basic_views.hpp`.

### Adaptors (`adaptors/`)

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `adaptors/filtered_graph.hpp` | 46 / 48 | 95.8% | 55 / 61 | 90.2% |
| `adaptors/bgl/graph_adaptor.hpp` | — | — | — | — |
| `adaptors/bgl/bgl_edge_iterator.hpp` | — | — | — | — |
| `adaptors/bgl/property_bridge.hpp` | — | — | — | — |

> BGL adaptor headers require an external Boost installation and are excluded from automated coverage collection.

### Other

| File | Lines Hit / Total | Line % | Funcs Hit / Total | Func % |
|------|-------------------|--------|-------------------|--------|
| `graph_data.hpp` | 0 / 2 | 0.0% | 0 / 2 | 0.0% |

> `graph_data.hpp` contains only metadata type aliases with no executable paths; the 0% figure is expected.

---

## Coverage Gaps

Files below 90% line coverage, ranked by gap size:

| File | Line % | Lines Missed | Notes |
|------|--------|--------------|-------|
| `io/json.hpp` | 78.1% | 42 | Error-handling and edge-case serialisation paths not fully exercised |
| `graph_data.hpp` | 0.0% | 2 | Metadata-only header; no executable paths (expected) |
| `generators/path.hpp` | 87.5% | 1 | One edge-case branch not exercised |
| `compressed_graph.hpp` | 88.2% | 26 | Some construction / accessor overloads untested |
| `mis.hpp` | 89.7% | 3 | Some branches in the randomised MIS variant not exercised |
| `io/graphml.hpp` | 90.0% | 21 | Attribute parsing edge cases not fully covered |

---

## Changes Since Last Report (2026-02-23)

| Area | Change |
|------|--------|
| Tests | +530 tests (4343 → 4873), 100% pass rate maintained |
| New files measured | `vertex_property_map.hpp`, `tarjan_scc.hpp`, `heap_position_map.hpp`, `indexed_dary_heap.hpp` |
| New sections | Generators (`generators/`), I/O (`io/`), Adaptors now has real data |
| `bellman_ford_shortest_paths.hpp` | Improved 85.7% → 98.3% (+12.6 pp) |
| `label_propagation.hpp` | Improved 95.7% → 100.0% |
| `mst.hpp` | Improved 92.9% → 96.8% |
| Total instrumented lines | +899 lines (3764 → 4663) from new files entering coverage |

---

## How to Reproduce

```bash
# Configure with coverage preset
cmake --preset linux-gcc-coverage

# Build
cmake --build build/linux-gcc-coverage -j $(nproc)

# Run tests
ctest --test-dir build/linux-gcc-coverage --output-on-failure -j $(nproc)

# Generate HTML report (output in build/linux-gcc-coverage/coverage_html/)
cd build/linux-gcc-coverage
lcov --directory . --capture --output-file coverage.info --ignore-errors mismatch,source
lcov --remove coverage.info '/usr/*' '/opt/*' '*/tests/*' '*/build/*' '*/_deps/*' \
     --output-file coverage_filtered.info --ignore-errors mismatch,source
genhtml coverage_filtered.info --output-directory coverage_html --ignore-errors mismatch,source
```

Open `build/linux-gcc-coverage/coverage_html/index.html` in a browser for the full interactive report.
