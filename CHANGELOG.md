# Changelog

## [Unreleased]

### Added
- **Indexed d-ary heap for Dijkstra** (`detail/indexed_dary_heap.hpp`) — opt-in O(V)-bounded heap with true decrease-key, parameterized by arity. Selected via the new `use_indexed_dary_heap<Arity>` heap-selector tag on `dijkstra_shortest_paths` / `dijkstra_shortest_distances`. Supports both dense graphs (via `vector_position_map`) and mapped / hashable-vertex-id graphs (via `assoc_position_map`).
- `use_default_heap` and `use_indexed_dary_heap<Arity>` heap-selector tags. **`use_default_heap` remains the default** — it wins on grid (E/V≈4) and path (E/V=1) workloads. Use `use_indexed_dary_heap<8>` for high-E/V random / scale-free graphs on `compressed_graph`, where Phase 4 benchmarks measured −25% (Erdős–Rényi) and −17% (Barabási–Albert) at 100K vertices vs. the default. See `agents/indexed_dary_heap_results.md` for full numbers.
- `vector_position_map` / `assoc_position_map` adapters (`detail/heap_position_map.hpp`) used by the indexed heap.
- **Tarjan's SCC algorithm** (`tarjan_scc.hpp`) — single-pass O(V+E) strongly connected components using iterative DFS with low-link values; no transpose graph needed
- 17 new Tarjan SCC tests (`test_tarjan_scc.cpp`)
- **Mapped (sparse) graph algorithm support** — all 14 algorithms now accept `adjacency_list<G>` (both index and map-based containers)
- `mapped_vertex_range`, `mapped_adjacency_list`, `mapped_bidirectional_adjacency_list` concepts
- `vertex_property_map<G, T>` type alias and `make_vertex_property_map` factory (vector for index graphs, unordered_map for mapped)
- `vertex_property_map_for<M, G>` concept for algorithm parameter constraints
- `vertex_property_map_value_t<Container>` trait for extracting per-vertex value types
- `is_sparse_vertex_container_v<G>` trait for compile-time graph type dispatch
- Map-based graph test fixtures (`map_graph_fixtures.hpp`) with sparse vertex IDs
- 456 new algorithm tests for sparse graph types (4343 → 4799)

### Changed
- **`edge_descriptor` simplified to iterator-only storage** — removed the `conditional_t<random_access_iterator, size_t, EdgeIter>` dual-storage path; edges always store the iterator directly since edges always have physical containers. Eliminates 38 `if constexpr` branches across 6 files (~500 lines removed).
- **`compressed_graph::vertices(g)` returns `iota_view`** — simplified to `std::ranges::iota_view<size_t, size_t>(0, num_vertices())`, which the `vertices` CPO wraps automatically via `_wrap_if_needed`.
- **`vertex_descriptor_view` CTAD deduction guides** — updated from `Container::iterator`/`const_iterator` to `std::ranges::iterator_t<>` for compatibility with views like `iota_view`.
- **`edge_descriptor_view` forward_list compatibility** — fixed constructor to use `if constexpr` for `sized_range` check so `std::ranges::size()` is not compiled for non-sized ranges like `forward_list`.
- All algorithms relaxed from `index_adjacency_list<G>` to `adjacency_list<G>`
- Algorithm internal arrays use `make_vertex_property_map` (vector or unordered_map depending on graph type)
- User-facing `Distances`, `Predecessors`, `Weight`, `Component`, `Label` parameters accept vertex property maps
- Index-based for-loops replaced with `views::basic_vertexlist(g)` iteration
- Validation uses `if constexpr (index_vertex_range<G>)` for size checks, `find_vertex` for mapped graphs

---

## [0.5.0] - 2026-03-01 *(Initial Beta)*

### Added
- **Bidirectional edge access** — `in_edges`, `in_degree`, `find_in_edge`, `contains_in_edge` CPOs
- `bidirectional_adjacency_list` concept
- `in_edge_range_t<G>`, `in_edge_iterator_t<G>`, `in_edge_t<G>` type aliases
- `out_edge_accessor` / `in_edge_accessor` stateless edge-access policies (`edge_accessor.hpp`)
- `in_incidence` and `in_neighbors` views with pipe-syntax adaptors
- Accessor template parameter on BFS, DFS, and topological sort views for reverse traversal
- `dynamic_graph` `Bidirectional` template parameter for incoming-edge lists
- `undirected_adjacency_list` bidirectional support (incoming = outgoing)
- Kosaraju SCC algorithm and `transpose_graph` view
- 144 new tests (4261 → 4405)
- New documentation: bidirectional access tutorial, updated views/concepts/CPO/container docs

### Documentation
- Complete documentation reorganization: user guide, reference, contributor docs
- New README with badges, compiler table, feature highlights
- Separated adjacency list and edge list documentation
- Full container documentation including 26 dynamic_graph trait combinations
- Consolidated CPO implementation guide (from 3140 lines → ~970)
- Created FAQ, migration guide, getting started, and examples pages
- Fixed algorithms.hpp umbrella header (6 broken includes)
- Archived stale design documents
- Added canonical metrics tracking (docs/status/metrics.md)
