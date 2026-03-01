# Changelog

## [Unreleased]

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
