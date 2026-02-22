# Architecture Guide

> How graph-v3 is structured, and the design principles behind it.

---

## Directory Structure

```
graph-v3/
├── include/graph/                     # Public headers (header-only library)
│   ├── graph.hpp                      # Main include — pulls in CPOs and core types
│   ├── graph_concepts.hpp             # Graph concepts (adjacency_list, etc.)
│   ├── graph_info.hpp                 # Graph introspection utilities
│   ├── algorithms.hpp                 # Convenience include for all algorithms
│   ├── views.hpp                      # Convenience include for all views
│   ├── adj_list/                      # Adjacency list descriptor system
│   │   ├── adjacency_list_concepts.hpp
│   │   ├── adjacency_list_traits.hpp
│   │   ├── descriptor.hpp
│   │   ├── descriptor_traits.hpp
│   │   ├── vertex_descriptor.hpp
│   │   ├── vertex_descriptor_view.hpp
│   │   ├── edge_descriptor.hpp
│   │   ├── edge_descriptor_view.hpp
│   │   ├── graph_utility.hpp
│   │   └── detail/                    # Implementation details
│   ├── algorithm/                     # Algorithm implementations
│   │   ├── dijkstra_shortest_paths.hpp
│   │   ├── bellman_ford_shortest_paths.hpp
│   │   ├── breadth_first_search.hpp
│   │   ├── depth_first_search.hpp
│   │   ├── topological_sort.hpp
│   │   ├── connected_components.hpp
│   │   ├── articulation_points.hpp
│   │   ├── biconnected_components.hpp
│   │   ├── mst.hpp
│   │   ├── tc.hpp                     # Triangle counting
│   │   ├── mis.hpp                    # Maximal independent set
│   │   ├── label_propagation.hpp
│   │   ├── jaccard.hpp
│   │   └── traversal_common.hpp
│   ├── container/                     # Library-provided containers
│   │   ├── dynamic_graph.hpp          # 27 trait combinations
│   │   ├── compressed_graph.hpp       # CSR format
│   │   ├── undirected_adjacency_list.hpp
│   │   ├── container_utility.hpp
│   │   ├── traits/
│   │   └── detail/
│   ├── detail/                        # Core CPO machinery
│   │   ├── cpo_common.hpp             # _Choice_t, shared helpers
│   │   ├── edge_cpo.hpp              # Edge CPO implementations
│   │   └── graph_using.hpp           # Namespace-level using declarations
│   ├── edge_list/                     # Edge list model
│   │   ├── edge_list.hpp
│   │   ├── edge_list_descriptor.hpp
│   │   └── edge_list_traits.hpp
│   └── views/                         # Lazy range views
│       ├── vertexlist.hpp
│       ├── edgelist.hpp
│       ├── incidence.hpp
│       ├── neighbors.hpp
│       ├── bfs.hpp
│       ├── dfs.hpp
│       ├── topological_sort.hpp
│       ├── basic_views.hpp
│       ├── search_base.hpp
│       ├── view_concepts.hpp
│       └── adaptors.hpp
├── tests/                             # Catch2 test suite (4261+ tests)
│   ├── test_main.cpp
│   ├── CMakeLists.txt
│   ├── adj_list/                      # Descriptor and CPO tests
│   ├── algorithms/                    # One file per algorithm
│   ├── container/                     # Container conformance tests
│   ├── common/                        # Shared test utilities
│   ├── edge_list/                     # Edge list tests
│   └── views/                         # View tests
├── examples/                          # Compilable example programs
├── benchmark/                         # Google Benchmark micro-benchmarks
├── docs/                              # Documentation
│   ├── index.md                       # Documentation hub
│   ├── user-guide/                    # Tutorials and user-facing docs
│   ├── reference/                     # API reference pages
│   └── contributing/                  # Contributor guides (this directory)
├── cmake/                             # CMake modules and tooling
├── CMakeLists.txt                     # Root build file
└── CMakePresets.json                  # Build presets for all platforms
```

---

## Design Principles

### 1. Works with Your Containers

Users should be able to use standard containers — `std::vector<std::vector<int>>`, `std::map<int, std::vector<std::pair<int,double>>>`, etc. — as graphs *without modification*. The library provides containers (`dynamic_graph`, `compressed_graph`) for users who want richer features, but they are never required.

### 2. Range-of-Ranges Model

An adjacency list is a *range of ranges*:

```
graph  =  [  vertex₀:[edge₀₀, edge₀₁],  vertex₁:[edge₁₀],  vertex₂:[] ]
           ──────── outer range ────────
                     ── inner range ──
```

- The **outer range** (`vertices(g)`) yields vertex descriptors.
- Each vertex's **inner range** (`edges(g, u)`) yields edge descriptors.
- Edge elements can be bare integers (target ID), pairs `(target, weight)`, tuples, or user-defined structs.

This model maps directly onto `std::vector<std::vector<T>>`, `std::map<K, std::list<E>>`, and similar nested containers.

### 3. Descriptor Wrapping

Raw container iterators and indices are wrapped in lightweight **descriptors** (`vertex_descriptor`, `edge_descriptor`) that provide a uniform interface:

- `vertex_descriptor<Iter>` — wraps a vertex iterator, exposes `vertex_id()` and `inner_value(g)`
- `edge_descriptor<EdgeIter, VertexIter>` — wraps an edge iterator, exposes `target_id()`, `source_id()`, and `inner_value(g)`
- `vertex_descriptor_view` / `edge_descriptor_view` — range adaptors that automatically wrap raw iterators

The descriptor layer means algorithms and views never deal with raw iterators; they work with a stable abstraction regardless of the underlying container.

### 4. CPO-Based Interface

All graph operations go through **Customization Point Objects** (CPOs) — function objects that provide a stable API while allowing per-type customization:

```cpp
auto verts = graph::vertices(g);       // Works for any graph type
auto eid   = graph::target_id(g, uv);  // Automatically selects best implementation
```

CPOs check for three customization tiers in priority order:
1. **Member function** — `g.vertices()`, `uv.target_id()`
2. **ADL free function** — `vertices(g)`, `target_id(g, uv)`
3. **Default implementation** — built-in fallback (e.g., index-based vertex IDs for vectors)

The library uses the **MSVC-style `_Choice_t` pattern** with `if constexpr` dispatch for cross-compiler reliability. See [CPO Implementation Guide](cpo-implementation.md) for details.

### 5. Header-Only

The entire library is header-only. There is nothing to compile or link — just add `include/` to your include path and `#include <graph/graph.hpp>`.

### 6. C++20 Throughout

graph-v3 leverages C++20 features pervasively:
- **Concepts** for type constraints (`adjacency_list<G>`, `has_vertex_value<G>`)
- **Ranges / views** for lazy iteration (all 7 views are `std::ranges`-compatible)
- **`if constexpr`** for zero-cost dispatch inside CPOs
- **`consteval`** for compile-time path selection
- **Structured bindings** for ergonomic iteration: `for (auto [uid, vid, w] : graph::edgelist(g, evf))`

---

## Namespace Organization

| Namespace | Purpose |
|-----------|---------|
| `graph::` | Core CPOs (`vertices`, `edges`, `target_id`, …), concepts, type aliases |
| `graph::views::` | View factory functions (`vertexlist`, `incidence`, `bfs`, …) |
| `graph::views::adaptors::` | Pipe-style adaptors (`g \| vertexlist()`) |
| `graph::adj_list::` | Adjacency list descriptors, traits, concepts |
| `graph::edge_list::` | Edge list descriptors, traits |
| `graph::container::` | Library-provided containers (`dynamic_graph`, `compressed_graph`) |
| `graph::_cpo::` | Internal CPO implementation details (not public API) |

---

## Build System

### CMake Presets

The project uses CMake Presets (version 6) with Ninja generator. All presets require **C++20**.

| Preset | Compiler | Build Type | Notes |
|--------|----------|------------|-------|
| `linux-gcc-debug` | GCC 13+ | Debug | Primary Linux dev preset |
| `linux-gcc-release` | GCC 13+ | Release | Optimized builds |
| `linux-clang-debug` | Clang 10+ | Debug | |
| `linux-clang-release` | Clang 10+ | Release | |
| `windows-msvc-debug` | MSVC 2022+ | Debug | |
| `windows-msvc-release` | MSVC 2022+ | Release | |
| `macos-debug` | AppleClang | Debug | |
| `macos-release` | AppleClang | Release | |

### Building and Testing

```bash
# Configure + build
cmake --preset linux-gcc-debug
cmake --build build/linux-gcc-debug

# Run tests
ctest --preset linux-gcc-debug

# Or run specific test binaries directly
./build/linux-gcc-debug/tests/graph_tests
```

### Test Organization

Tests mirror the source structure:

| Test directory | What it covers |
|----------------|---------------|
| `tests/adj_list/` | Descriptor construction, CPO dispatch, type aliases |
| `tests/algorithms/` | One test file per algorithm (Dijkstra, BFS, DFS, …) |
| `tests/container/` | Container conformance — all 27 trait combinations |
| `tests/edge_list/` | Edge list model tests |
| `tests/views/` | View iteration and composition |
| `tests/common/` | Shared test utilities and helpers |

---

## Two ADTs — Adjacency Lists and Edge Lists

### Adjacency Lists

The primary graph representation. An adjacency list stores per-vertex edge lists and supports fast neighbor traversal.

**Namespace:** `graph::adj_list::`

**Key abstractions:**
- `vertex_descriptor<Iter>` — identity + access to vertex data
- `edge_descriptor<EdgeIter, VertexIter>` — identity + access to edge data
- `vertex_descriptor_view` / `edge_descriptor_view` — range adaptors

### Edge Lists

A flat sequence of `(source, target [, value])` tuples. Ideal for algorithms that scan all edges (Kruskal MST, bulk loading, serialization).

**Namespace:** `graph::edge_list::`

**Key abstractions:**
- `edge_list_descriptor` — wraps an edge iterator
- Source/target extraction via CPOs: `source_id(e)`, `target_id(e)`

Both ADTs share the same CPO interface, so algorithms can be written generically when possible.

---

## Inner Value Priority Pattern

A recurring design pattern in graph-v3's CPOs. When a CPO operates on a descriptor (e.g., `vertex_id(g, u)`), it resolves the implementation in this order:

1. **Inner value member** — `u.inner_value(g).vertex_id(g)` (the stored data defines behavior)
2. **ADL with inner value** — `vertex_id(g, u.inner_value(g))` (free function on stored data)
3. **ADL with descriptor** — `vertex_id(g, u)` (free function on the wrapper)
4. **Descriptor default** — `u.vertex_id()` (built-in fallback, e.g., index-based ID)

This ensures user-defined element types always take precedence over the library's default behavior. See [CPO Implementation Guide](cpo-implementation.md) for implementation details.

---

## See Also

- [CPO Implementation Guide](cpo-implementation.md) — how to add or modify a CPO
- [CPO Order](cpo-order.md) — canonical implementation order and dependencies
- [Coding Guidelines](coding-guidelines.md) — naming conventions and style rules
