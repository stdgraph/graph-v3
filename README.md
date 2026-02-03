# Graph Library

A modern C++20 graph library providing a comprehensive, customizable interface for graph data structures, algorithms, and views.

## Overview

This library provides the foundation for a complete graph library following the P1709 Graph Library proposal. It includes:

- **Descriptor System**: Type-safe, efficient descriptors for vertices and edges that abstract storage strategies
- **Graph Container Interface**: Standardized concepts, traits, and functions for adjacency lists and edgelists
- **Customization Point Objects (CPOs)**: All graph operations can be customized for existing data structures
- **Range-Based Design**: Graphs as ranges of vertices, where each vertex is a range of edges
- **Documentation**: Comprehensive documentation following P1709 conventions

**Current Status**: Phase 9 complete - All graph containers (`dynamic_graph`, `compressed_graph`, `undirected_adjacency_list`) and comprehensive graph views (`vertexlist`, `edgelist`, `neighbors`, `incidence`, `BFS`, `DFS`, `topological_sort`) fully implemented with 3931 tests passing. Core CPOs, value access CPOs, partitioning/sourced edge CPOs, adjacency list concepts, traits, and complete view system are production-ready.

## Features

### Core Descriptor System
- **Type-safe descriptors**: Vertex and edge descriptors are distinct types, preventing accidental misuse
- **Zero-cost abstraction**: No runtime overhead compared to raw indices/iterators
- **Storage flexibility**: Works with random-access containers (vector, deque) and associative containers (map, unordered_map)
- **Descriptor views**: `vertex_descriptor_view` and `edge_descriptor_view` provide range-based iteration
- **Automatic pattern support**: `vertices(g)` and `edges(g, u)` automatically return descriptor views for containers following inner value and edge value patterns
- **Hash support**: Built-in std::hash specializations for use in unordered containers

### Graph Container Interface (GCI)
- **Standardized concepts**: Edge, vertex, and adjacency list concepts for type checking
- **Flexible implementations**: Supports vector-of-lists, CSR, adjacency matrix, and custom graph types
- **CPO-based architecture**: All functions are customization point objects following MSVC style
- **Default implementations**: Reasonable defaults minimize work to adapt existing data structures
- **Pattern recognition**: Automatic support for common edge patterns (tuples, edge_info)
- **Partition support**: Unipartite, bipartite, and multipartite graphs
- **Edgelist interface**: Separate interface for edge lists with source/target ID access

### Documentation
- **Complete naming conventions**: All template parameters, type aliases, and variable names from P1709
- **Container interface specification**: Comprehensive summary of concepts, traits, types, and functions
- **CPO implementation guide**: Detailed patterns for creating customization point objects
- **Common guidelines**: Architectural commitments and project structure requirements

## Requirements

- C++20 compliant compiler (GCC 10+, Clang 10+, MSVC 2019+)
- CMake 3.20 or later
- Ninja (optional, recommended)

## Building

```bash
# Configure
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure
```

## Quick Start

### Automatic Graph Support (No Custom Code Needed!)

```cpp
#include <graph/graph.hpp>
#include <vector>
#include <map>

// Simple adjacency list: vector of vectors of ints
std::vector<std::vector<int>> adj_list = {
    {1, 2},      // Vertex 0 -> edges to 1, 2
    {2, 3},      // Vertex 1 -> edges to 2, 3
    {3},         // Vertex 2 -> edge to 3
    {}           // Vertex 3 -> no edges
};

// Works automatically with vertices(g) and edges(g, u)!
for (auto u : graph::vertices(adj_list)) {
    std::cout << "Vertex " << u.vertex_id() << " connects to: ";
    for (auto e : graph::edges(adj_list, u)) {
        std::cout << e.target_id(adj_list) << " ";
    }
    std::cout << "\n";
}

// Map-based graph also works automatically
std::map<int, std::vector<int>> map_graph = {
    {100, {200, 300}},
    {200, {300}},
    {300, {}}
};

for (auto u : graph::vertices(map_graph)) {
    std::cout << "Vertex " << u.vertex_id() << " connects to: ";
    for (auto e : graph::edges(map_graph, u)) {
        std::cout << e.target_id(map_graph) << " ";
    }
    std::cout << "\n";
}
```

### Vertex Descriptors with Vector Storage

```cpp
#include <graph/vertex_descriptor.hpp>
#include <graph/vertex_descriptor_view.hpp>
#include <vector>

std::vector<int> vertices = {10, 20, 30, 40, 50};

using VectorIter = std::vector<int>::iterator;
using VD = graph::vertex_descriptor<VectorIter>;

// Create descriptor for vertex at index 2
VD vd{2};
std::cout << "Vertex ID: " << vd.vertex_id() << "\n";  // Output: 2

// Iterate over all vertices
graph::vertex_descriptor_view view{vertices};
for (auto desc : view) {
    std::cout << "Vertex " << desc.vertex_id() << "\n";
}
```

### Vertex Descriptors with Map Storage

```cpp
#include <graph/vertex_descriptor.hpp>
#include <graph/vertex_descriptor_view.hpp>
#include <map>

std::map<int, std::string> vertex_map = {
    {100, "Node A"},
    {200, "Node B"},
    {300, "Node C"}
};

using MapIter = std::map<int, std::string>::iterator;
using VD = graph::vertex_descriptor<MapIter>;

// Create descriptor from iterator
auto it = vertex_map.find(200);
VD vd{it};
std::cout << "Vertex ID: " << vd.vertex_id() << "\n";  // Output: 200

// Iterate over all vertices
graph::vertex_descriptor_view map_view{vertex_map};
for (auto desc : map_view) {
    std::cout << "Vertex " << desc.vertex_id() << "\n";
}
```

## Implementation Status

### ✅ Phase 1: Foundation (COMPLETE)
- [x] **Directory Structure**: Standard graph library layout (algorithm/, container/, views/, detail/)
- [x] **Descriptor System**:
  - Core descriptor concepts and traits
  - Vertex descriptors (keyed and direct storage)
  - Edge descriptors with source vertex tracking
  - Descriptor views for range-based access
  - Comprehensive unit tests (75 tests passing for descriptors)
- [x] **Infrastructure**:
  - CPO framework with _Choice_t pattern
  - Common std library imports (detail/graph_using.hpp)
  - Graph information structures (graph_info.hpp)
  - Edge list container (edgelist.hpp)
  - CMake build system with presets
  - Catch2 test framework integration
- [x] **Documentation**:
  - Complete naming conventions from P1709
  - Graph Container Interface specification
  - CPO implementation guide
  - Common graph guidelines
  - Migration documentation

### 🔄 Phase 2: Graph Utility CPOs (IN PROGRESS)
Implement core graph operation CPOs in `graph_cpo.hpp` following the canonical order:

**Phase 1: Core Foundation (Essential)**
- [x] `vertices(g)` - Get vertex range (returns `vertex_descriptor_view`) ✅ **COMPLETE** - 18 tests passing
- [x] Type aliases: `vertex_range_t<G>`, `vertex_iterator_t<G>`, `vertex_t<G>` ✅ **COMPLETE** - 5 tests passing
- [x] `vertex_id(g, u)` - Get vertex ID from descriptor ✅ **COMPLETE** - 15 tests passing
  - Resolution order (inner_value first):
    1. `u.inner_value(g).vertex_id(g)` - Inner value member (highest priority)
    2. `vertex_id(g, u.inner_value(g))` - ADL with inner_value
    3. `vertex_id(g, u)` - ADL with descriptor
    4. `u.vertex_id()` - Descriptor default (lowest priority)
- [x] Type alias: `vertex_id_t<G>` ✅ **COMPLETE** - 2 tests passing
- [x] `find_vertex(g, uid)` - Find vertex by ID ✅ **COMPLETE** - 11 tests passing
  - Resolution order:
    1. `g.find_vertex(uid)` - Member function (highest priority)
    2. `find_vertex(g, uid)` - ADL (medium priority)
    3. `std::ranges::next(std::ranges::begin(vertices(g)), uid)` - Random access default (lowest priority)
  - **Note**: Default implementation requires `sized_range` (works with vector/deque, maps need custom implementation)
- [x] `edges(g, u)` - Get outgoing edges from vertex (returns `edge_descriptor_view`) ✅ **COMPLETE** - 14 tests passing
  - Resolution order:
    1. `g.edges(u)` - Member function (highest priority)
    2. `edges(g, u)` - ADL (medium priority)
    3. `edge_descriptor_view(u.inner_value(g), u)` - Edge value pattern default (lowest priority)
  - **Automatic support** for containers following edge value patterns: simple (int), pair, tuple, custom
- [x] Type aliases: `vertex_edge_range_t<G>`, `vertex_edge_iterator_t<G>`, `edge_t<G>` ✅ **COMPLETE**
- [x] `target_id(g, uv)` - Get target vertex ID from edge ✅ **COMPLETE** - 20 tests passing
  - Resolution order:
    1. `target_id(g, uv)` - ADL (highest priority)
    2. `uv.target_id(edges)` - Edge descriptor default (lowest priority)
  - **Automatic extraction** for edge patterns: simple (int → itself), pair (→ .first), tuple (→ get<0>), custom via ADL
- [x] `target(g, uv)` - Get target vertex descriptor from edge ✅ **COMPLETE** - 28 tests passing
  - Resolution order:
    1. `g.target(uv)` - Member function (highest priority)
    2. `target(g, uv)` - ADL (medium priority)
    3. `*find_vertex(g, target_id(g, uv))` - Default (lowest priority)
  - **Return type flexibility**: Custom implementations (member/ADL) can return either:
    - `vertex_descriptor` (vertex_t<G>) - used as-is
    - `vertex_iterator` (iterator to vertices) - automatically converted to descriptor
  - **Smart type conversion**: Internal `_to_vertex_descriptor` helper properly handles both direct descriptors and iterators from `vertices(g)`
  - **Default implementation**: Combines `find_vertex` and `target_id` for convenience
  - **Performance**: O(1) for random-access graphs, O(log n) or O(1) average for associative graphs

**Phase 2: Query Functions (High Priority)**
- [x] `num_vertices(g)` - Count vertices in graph ✅ **COMPLETE** - 24 tests passing
  - Resolution order:
    1. `g.num_vertices()` - Member function (highest priority)
    2. `num_vertices(g)` - ADL (medium priority)
    3. `std::ranges::size(g)` - Ranges default (lowest priority)
  - **Automatic support** for any sized_range (vector, deque, map, unordered_map, etc.)
- [x] `num_edges(g)` - Count total edges in graph ✅ **COMPLETE** - 31 tests passing
  - Resolution order:
    1. `g.num_edges()` - Member function (highest priority)
    2. `num_edges(g)` - ADL (medium priority)
    3. Iterate vertices and sum edge counts - Default (lowest priority)
  - **Default implementation**: Iterates through all vertices using `vertices(g)`, for each vertex calls `edges(g, u)`, uses `std::ranges::size()` for sized ranges or `std::ranges::distance()` for non-sized ranges
  - **Note**: For directed graphs counts each edge once; for undirected graphs counts each edge twice (once per endpoint)
- [x] `edges(g, uid)` - Get outgoing edges from vertex by ID (convenience wrapper) ✅ **COMPLETE** - 19 tests passing
  - Resolution order:
    1. `g.edges(uid)` - Member function (highest priority)
    2. `edges(g, uid)` - ADL (medium priority)
    3. `edges(g, *find_vertex(g, uid))` - Default using find_vertex (lowest priority)
  - **Note**: Convenience function that combines find_vertex + edges(g,u) to avoid boilerplate
  - **Return type**: Returns `edge_descriptor_view` (same as edges(g,u))
  - **Dual overload pattern**: Uses `requires (!vertex_descriptor_type<VId>)` constraint to differentiate from edges(g,u)
- [x] `degree(g, u)` - Get out-degree of vertex (descriptor version) ✅ **COMPLETE** - 17 tests passing
  - Resolution order:
    1. `g.degree(u)` - Member function (highest priority)
    2. `degree(g, u)` - ADL (medium priority)
    3. `std::ranges::size(edges(g, u))` - Default using edges (lowest priority)
  - **Return type**: Integral type (size_t or similar)
  - **Default implementation**: Uses `std::ranges::size()` on result of `edges(g, u)` when available
- [x] `degree(g, uid)` - Get out-degree of vertex by ID (convenience wrapper) ✅ **COMPLETE** - Included in 17 tests
  - Resolution order:
    1. `g.degree(uid)` - Member function (highest priority)
    2. `degree(g, uid)` - ADL (medium priority)
    3. `degree(g, *find_vertex(g, uid))` - Default using find_vertex (lowest priority)
  - **Note**: Convenience function that combines find_vertex + degree(g,u) to avoid boilerplate
  - **Dual overload pattern**: Uses `requires (!vertex_descriptor_type<VId>)` constraint to differentiate from degree(g,u)

**Phase 3: Edge Queries (Medium Priority)**
- [x] `find_vertex_edge(g, u, v)` - Find edge from u to v ✅ **COMPLETE** - Included in 17 tests
  - Resolution order:
    1. `g.find_vertex_edge(u, v)` - Member function (highest priority)
    2. `find_vertex_edge(g, u, v)` - ADL (medium priority)
    3. Iterates `edges(g, u)` comparing `target_id(g, e)` (lowest priority)
  - **Return type**: Edge descriptor (like iterator), returns end sentinel if not found
  - **Triple overload pattern**: Three versions for different parameter combinations
- [x] `find_vertex_edge(g, u, vid)` - Find edge from u to vid ✅ **COMPLETE** - Included in 17 tests
  - Resolution order:
    1. `g.find_vertex_edge(u, vid)` - Member function (highest priority)
    2. `find_vertex_edge(g, u, vid)` - ADL (medium priority)
    3. Iterates `edges(g, u)` comparing `target_id(g, e) == vid` (lowest priority)
  - **Constraint**: Uses `requires (!vertex_descriptor_type<VId>)` to differentiate from (g,u,v) overload
- [x] `find_vertex_edge(g, uid, vid)` - Find edge from uid to vid ✅ **COMPLETE** - Included in 17 tests
  - Resolution order:
    1. `g.find_vertex_edge(uid, vid)` - Member function (highest priority)
    2. `find_vertex_edge(g, uid, vid)` - ADL (medium priority)
    3. `find_vertex_edge(g, *find_vertex(g, uid), vid)` - Default using find_vertex (lowest priority)
  - **Note**: Convenience function for ID-based edge queries
- [x] `contains_edge(g, u, v)` - Check if edge exists ✅ **COMPLETE** - Included in 21 tests
  - Resolution order:
    1. `g.contains_edge(u, v)` - Member function (highest priority)
    2. `contains_edge(g, u, v)` - ADL (medium priority)
    3. Iterates `edges(g, u)` comparing `target_id(g, e)` with `vertex_id(g, v)` (lowest priority)
  - **Return type**: `bool` - true if edge exists, false otherwise
  - **Note**: More efficient than find_vertex_edge for existence checks
- [x] `contains_edge(g, uid, vid)` - Check if edge exists ✅ **COMPLETE** - Included in 21 tests
  - Resolution order:
    1. `g.contains_edge(uid, vid)` - Member function (highest priority)
    2. `contains_edge(g, uid, vid)` - ADL (medium priority)
    3. `find_vertex(g, uid)` then iterate `edges(g, u)` comparing `target_id(g, e) == vid` (lowest priority)
  - **Dual overload pattern**: Uses `requires (!vertex_descriptor_type<UId> && !vertex_descriptor_type<VId>)` to differentiate
- [x] `has_edge(g)` - Check if graph has any edges ✅ **COMPLETE** - 33 tests passing
  - Resolution order:
    1. `g.has_edge()` - Member function (highest priority)
    2. `has_edge(g)` - ADL (medium priority)
    3. Iterate `vertices(g)` checking if any vertex has non-empty `edges(g, u)` (lowest priority)
  - **Return type**: `bool` - true if graph has at least one edge, false otherwise
  - **Default implementation**: Uses `std::ranges::find_if` to find first vertex with non-empty edge range (short-circuits on first match)
  - **Note**: Efficient early termination - stops at first vertex with edges

**Phase 4: Value Access (Optional)**
- [x] `vertex_value(g, u)` - Get user-defined vertex value ✅ **COMPLETE** - 21 tests passing
  - Resolution order:
    1. `g.vertex_value(u)` - Member function (highest priority)
    2. `vertex_value(g, u)` - ADL (medium priority)
    3. `u.inner_value(g)` - Default using descriptor's inner_value (lowest priority)
  - **Return type**: `decltype(auto)` - Perfect forwarding of references
  - **Default implementation**: Delegates to `u.inner_value(g)` which handles different container types:
    * Random-access (vector): returns `container[index]`
    * Associative (map): returns `.second` value (not the key)
    * Bidirectional: returns dereferenced value
  - **Note**: Provides access to user-defined vertex properties/data stored in the graph
- [x] `edge_value(g, uv)` - Get user-defined edge value ✅ **COMPLETE** - 33 tests passing
  - Resolution order:
    1. `g.edge_value(uv)` - Member function (highest priority)
    2. `edge_value(g, uv)` - ADL (medium priority)
    3. `uv.inner_value(edges)` - Default using edge descriptor's inner_value (lowest priority)
  - **Return type**: `decltype(auto)` - Perfect forwarding of references (supports both by-value and by-reference returns)
  - **Default implementation**: Delegates to `uv.inner_value(uv.source().inner_value(g))` which extracts properties based on edge pattern:
    * Simple (int): returns the value itself (target ID only, no separate property)
    * Pair (target, weight): returns `.second` (the weight/property)
    * Tuple (target, prop1, prop2, ...): returns tuple of elements [1, N) (all properties except target)
    * Custom struct: returns the whole value (user manages property access)
  - **Note**: Honors const-correctness - returns const reference when graph is const, mutable reference when graph is mutable
- [x] `graph_value(g)` - Get user-defined graph value ✅ **COMPLETE** - 27 tests passing
  - Resolution order:
    1. `g.graph_value()` - Member function (highest priority)
    2. `graph_value(g)` - ADL (lowest priority)
  - **Return type**: `decltype(auto)` - Perfect forwarding of references (supports both by-value and by-reference returns)
  - **No default implementation**: Compile-time error if neither member nor ADL is found
  - **Note**: Provides access to user-defined graph-level properties/metadata (e.g., name, version, statistics)
  - **Use cases**: Graph metadata, version tracking, global parameters (weight multipliers), statistics

**Phase 5: Optional Features (Sourced Edges & Partitioning)**
- [x] `source_id(g, uv)` - Get source vertex ID (for sourced edges) ✅ **COMPLETE** - No tests (CPO only)
  - Resolution order:
    1. `g.source_id(uv)` - Member function (highest priority)
    2. `source_id(g, uv)` - ADL (lowest priority)
  - **No default implementation**: Unlike target_id which works for standard adjacency lists, source requires explicit implementation
  - **Use cases**: Bidirectional graphs, edge lists, graphs where edges explicitly track their source vertex
  - **Return type**: Vertex ID (by value)
- [x] `source(g, uv)` - Get source vertex descriptor (for sourced edges) ✅ **COMPLETE** - No tests (CPO only)
  - Resolution order:
    1. `g.source(uv)` - Member function (highest priority)
    2. `source(g, uv)` - ADL (lowest priority)
  - **No default implementation**: Unlike target which has default via find_vertex, source requires explicit implementation
  - **Return type flexibility**: Custom implementations can return either:
    - `vertex_descriptor` (vertex_t<G>) - used as-is
    - `vertex_iterator` (iterator to vertices) - automatically converted to descriptor
  - **Smart type conversion**: Internal `_to_vertex_descriptor` helper properly handles both cases
  - **Use cases**: Bidirectional graphs, edge list graphs where edges are stored independently
- [x] `partition_id(g, u)` - Get partition ID for a vertex ✅ **COMPLETE** - 16 tests passing
  - Resolution order:
    1. `g.partition_id(u)` - Member function (highest priority)
    2. `partition_id(g, u)` - ADL (medium priority)
    3. Default: returns 0 (lowest priority)
  - **Default implementation**: Assumes single partition where all vertices belong to partition 0
  - **Use cases**: Multi-partition distributed graphs, graph coloring, NUMA-aware partitioning, load balancing
  - **Return type**: Integral type (default 0)
  - **Noexcept**: Default implementation is noexcept
- [x] `num_partitions(g)` - Get number of partitions in the graph ✅ **COMPLETE** - 27 tests passing
  - Resolution order:
    1. `g.num_partitions()` - Member function (highest priority)
    2. `num_partitions(g)` - ADL (medium priority)
    3. Default: returns 1 (lowest priority)
  - **Default implementation**: Assumes single partition (all vertices in partition 0)
  - **Relationship to partition_id**: partition_id values should be in range [0, num_partitions)
  - **Use cases**: Distributed graphs, dynamic partition counting, graph coloring results, NUMA-aware partition counts
  - **Return type**: Integral type (default 1)
  - **Noexcept**: Default implementation is noexcept

Unit tests and documentation for each CPO added.

### ✅ Phase 6: Adjacency List Concepts (COMPLETE)
- [x] **Adjacency List Concepts** in `graph/adjacency_list_concepts.hpp` ✅ **COMPLETE** - 26 tests passing
  - Edge concepts:
    * `targeted_edge<G, E>` - Requires target_id(g,e) and target(g,e), plus is_edge_descriptor_v ✅
    * `sourced_edge<G, E>` - Requires source_id(g,e) and source(g,e), plus is_edge_descriptor_v ✅
    * `sourced_targeted_edge<G, E>` - Combines both targeted_edge and sourced_edge ✅
    * `targeted_edge_range<R, G>` - Forward range of targeted_edges ✅
    * `sourced_targeted_edge_range<R, G>` - Forward range of sourced_targeted_edges ✅
  - Vertex concepts:
    * `vertex_range<G>` - Graph with forward range of vertex descriptors (derived from vertex_range_t<G>) ✅
    * `index_vertex_range<G>` - Graph with random access range of vertex descriptors ✅
  - Adjacency list concepts:
    * `adjacency_list<G>` - Graph with vertex_range and targeted_edge_range ✅
    * `index_adjacency_list<G>` - Graph with index_vertex_range and targeted_edge_range ✅
    * `sourced_adjacency_list<G>` - Graph with vertex_range and sourced_targeted_edge_range ✅
    * `index_sourced_adjacency_list<G>` - Graph with index_vertex_range and sourced_targeted_edge_range ✅
  - Design decisions:
    * Parameter order: Range concepts use `<R, G>` to work with C++20's `-> concept<G>` syntax
    * vertex_range concepts refactored to use single graph parameter, deriving range type via vertex_range_t<G>
    * Forward-only iteration: vertex_descriptor_view is forward-only by design (descriptors synthesized on-the-fly)
    * No sized_range requirement: Map-based graphs don't naturally provide O(1) size()
  - Test coverage:
    * 8 test cases for edge concepts with edge_descriptor from vector/deque/pair containers
    * 18 test cases for vertex and graph concepts with vector/map/deque containers
    * Comprehensive integration tests for concept hierarchies

### ✅ Phase 7: Adjacency List Traits (COMPLETE)
- [x] **Adjacency List Traits** in `graph/adjacency_list_traits.hpp` ✅ **COMPLETE** - 19 tests passing
  - `has_degree<G>` - Graph supports degree(g,u) and degree(g,uid) functions ✅
  - `has_find_vertex<G>` - Graph supports find_vertex(g,uid) ✅
  - `has_find_vertex_edge<G>` - Graph supports all three find_vertex_edge overloads ✅
  - `has_contains_edge<G, V>` - Graph supports contains_edge operations ✅
  - `define_unordered_edge<G>` - Overridable trait (defaults to false) for unordered edges ✅
  - `has_basic_queries<G>` - Combined trait (degree + find_vertex + find_vertex_edge) ✅
  - `has_full_queries<G>` - Combined trait (basic queries + contains_edge) ✅
  - Each trait includes convenience variable template (_v)
  - Design decisions:
    * Traits use concept-based detection via requires expressions
    * Individual trait concepts check for specific CPO support
    * Combined traits provide higher-level abstractions
    * define_unordered_edge uses std::false_type/std::true_type pattern
    * All traits are SFINAE-friendly for enable_if patterns
  - Test coverage:
    * 19 test cases covering all trait concepts
    * Tests with vector (random access), map (associative), deque (bidirectional)
    * Runtime verification tests for trait correctness
    * Integration tests with existing CPO framework

### ✅ Phase 8: Graph Container Implementations (COMPLETE)
- [x] **dynamic_graph** - Flexible directed graph with configurable vertex/edge containers ✅
  - Multiple container combinations: vector-of-vector (vov), vector-of-list (vol), vector-of-forward_list (vofl)
  - Partition support for distributed/NUMA-aware graphs
  - Sourced edges optional (track source vertex in edge)
  - Comprehensive test coverage: 1000+ test cases
- [x] **compressed_graph** - Read-only compressed sparse row (CSR) format ✅
  - Optimal for static graph algorithms
  - Minimal memory footprint
  - O(1) vertex access, O(degree) edge iteration
  - Partition support
- [x] **undirected_adjacency_list** - Undirected graph with dual-list design ✅
  - Each edge stored in two linked lists (one per endpoint)
  - O(1) edge removal from both vertices
  - Template defaults aligned with other graphs (VV, EV, GV default to `void`)
  - Modern `eproj` constructor pattern (consistent with dynamic_graph)
  - 94 test cases, 525 assertions
  - Full CPO conformance (47+ functions)

### ✅ Phase 9: Graph Views (COMPLETE)
- [x] **Graph Views** in `include/graph/views/` ✅ **COMPLETE**
  - Basic views:
    * `vertexlist` - Vertex range view with structured bindings (vertex_info)
    * `edgelist` - Edge range view with structured bindings (edge_info)
    * `neighbors` - Neighbor range view (neighbor_info)
    * `incidence` - Incidence edge range view
  - Search views:
    * `vertices_bfs` / `edges_bfs` - Breadth-first search traversal
    * `vertices_dfs` / `edges_dfs` - Depth-first search traversal
    * `vertices_topological_sort` / `edges_topological_sort` - Topological ordering
  - Features:
    * Range adaptor closures for pipe syntax (`g | vertexlist()`)
    * Optional value functions (VVF, EVF) for property access
    * Cancellation support for search views
    * Depth tracking in BFS/DFS
    * Chainable with standard library views
  - Test coverage: Comprehensive view tests in `tests/views/`

### 📋 Phase 10: Algorithms and Advanced Views (PLANNED)
- [ ] Standalone algorithms in `include/graph/algorithm/`:
  - `dijkstra_shortest_paths.hpp` - Single-source shortest paths
  - `bellman_ford.hpp` - Shortest paths with negative weights
  - `minimum_spanning_tree.hpp` - MST algorithms
- [ ] Advanced views:
  - `filtered_graph` - Filtered graph views
  - `reverse_graph` - Reversed graph views
  - Edge list views for edge_list namespace

## Project Structure

```
desc/
├── benchmark/              # Performance benchmarks (future)
├── docs/                   # Documentation
│   ├── common_graph_guidelines.md    # Naming conventions and requirements
│   ├── container_interface.md        # GCI specification summary
│   ├── cpo.md                         # CPO implementation guide
│   └── vertex_storage_concepts.md    # Storage pattern concepts
├── examples/               # Example programs
│   └── basic_usage.cpp    # Basic descriptor usage
├── include/                # Public headers
│   └── graph/
│       ├── algorithm/     # Graph algorithms (future)
│       ├── container/     # Graph containers
│       │   ├── dynamic_graph.hpp
│       │   ├── compressed_graph.hpp
│       │   ├── undirected_adjacency_list.hpp
│       │   └── traits/    # Container trait specializations
│       ├── detail/        # Implementation details
│       │   ├── cpo_common.hpp     # Shared CPO infrastructure
│       │   ├── edge_cpo.hpp       # Shared edge CPOs
│       │   └── graph_using.hpp    # Common std imports
│       ├── adj_list/      # Adjacency list abstractions
│       │   ├── detail/
│       │   │   └── graph_cpo.hpp  # Adjacency list CPOs
│       │   ├── adjacency_list_concepts.hpp
│       │   ├── adjacency_list_traits.hpp
│       │   └── (descriptors and traits)
│       ├── edge_list/     # Edge list abstractions
│       │   ├── edge_list.hpp
│       │   └── (descriptors and traits)
│       ├── views/         # Graph views
│       ├── descriptor.hpp           # Core descriptor concepts
│       ├── descriptor_traits.hpp    # Descriptor type traits
│       ├── vertex_descriptor.hpp    # Vertex descriptor
│       ├── vertex_descriptor_view.hpp
│       ├── edge_descriptor.hpp      # Edge descriptor
│       ├── edge_descriptor_view.hpp
│       ├── edgelist.hpp            # Edge list container
│       ├── graph.hpp               # Main library header
│       ├── graph_info.hpp          # Graph information structures
│       └── graph_utility.hpp       # Utility CPOs (stub)
├── scripts/                # Build and maintenance scripts
│   └── format.sh          # Code formatting script
├── tests/                  # Unit tests (3931 tests, all passing)
│   ├── adj_list/          # Adjacency list tests
│   │   ├── cpo/          # CPO tests
│   │   ├── concepts/     # Concept tests
│   │   └── traits/       # Trait tests
│   ├── container/         # Container tests
│   │   ├── dynamic_graph/
│   │   ├── compressed_graph/
│   │   └── undirected_adjacency_list/
│   ├── edge_list/         # Edge list tests
│   ├── views/             # View tests
│   │   ├── test_adaptors.cpp
│   │   ├── test_bfs.cpp
│   │   ├── test_dfs.cpp
│   │   ├── test_topological_sort.cpp
│   │   ├── test_vertexlist.cpp
│   │   ├── test_edgelist.cpp
│   │   ├── test_neighbors.cpp
│   │   └── test_incidence.cpp
│   ├── test_adjacency_list_edge_concepts.cpp
│   ├── test_adjacency_list_traits.cpp
│   ├── test_adjacency_list_vertex_concepts.cpp
│   ├── test_compressed_graph.cpp
│   ├── test_compressed_graph_cpo.cpp
│   ├── test_contains_edge_cpo.cpp
│   ├── test_degree_cpo.cpp
│   ├── test_descriptor_traits.cpp
│   ├── test_dynamic_graph_*.cpp      # Multiple dynamic_graph test files
│   ├── test_edge_concepts.cpp
│   ├── test_edge_descriptor.cpp
│   ├── test_edge_value_cpo.cpp
│   ├── test_edges_cpo.cpp
│   ├── test_edges_uid_cpo.cpp
│   ├── test_find_vertex_cpo.cpp
│   ├── test_find_vertex_edge_cpo.cpp
│   ├── test_graph_value_cpo.cpp
│   ├── test_has_edge_cpo.cpp
│   ├── test_num_edges_cpo.cpp
│   ├── test_num_partitions_cpo.cpp
│   ├── test_num_vertices_cpo.cpp
│   ├── test_partition_id_cpo.cpp
│   ├── test_target_cpo.cpp
│   ├── test_target_id_cpo.cpp
│   ├── test_type_aliases.cpp
│   ├── test_undirected_adjacency_list.cpp
│   ├── test_undirected_adjacency_list_cpo.cpp
│   ├── test_vertex_concepts.cpp
│   ├── test_vertex_descriptor.cpp
│   ├── test_vertex_id_cpo.cpp
│   ├── test_vertex_value_cpo.cpp
│   └── test_vertices_cpo.cpp
├── CMakeLists.txt         # Root CMake configuration
├── CMakePresets.json      # CMake build presets
├── MIGRATION_PHASE1.md    # Phase 1 migration details
├── PHASE1_COMPLETE.md     # Phase 1 completion report
└── README.md              # This file
```

## Design Principles

### 1. Descriptor View Architecture
**IMPORTANT**: All range-returning CPOs must return descriptor views:
- `vertices(g)` MUST return `vertex_descriptor_view` 
- `edges(g, u)` MUST return `edge_descriptor_view`

This ensures consistent descriptor semantics across all graph types. The CPOs use a three-tier resolution:
1. Custom override via member function (e.g., `g.vertices()`)
2. Custom override via ADL (e.g., `vertices(g)`)
3. Default implementation using pattern recognition:
   - `vertices(g)` → `vertex_descriptor_view(g)` if g follows inner value patterns
   - `edges(g, u)` → `edge_descriptor_view(u.inner_value(), u)` if u.inner_value() follows edge value patterns

### 2. CPO-Based Interface
All graph operations are customization point objects (CPOs) following MSVC standard library style:
- Member functions (highest priority)
- ADL-findable free functions (medium priority)  
- Default implementations (lowest priority)

This allows adaptation of existing graph data structures without modification.

### 3. Outgoing Edge Focus
The interface focuses on outgoing edges from vertices. If a graph exposes incoming edges, a separate view presents them through the same outgoing-edge interface for consistency.

### 4. Range-of-Ranges Model
Adjacency lists are treated as ranges of vertices, where each vertex is a range of edges. This enables use of standard algorithms and range adaptors.

### 5. Descriptor Abstraction
Descriptors are opaque objects that abstract implementation details:
- Random-access containers: descriptors are integral indices
- Bidirectional containers: descriptors are iterators
- Provides uniform interface regardless of storage strategy

### 6. Zero-Cost Abstraction
Descriptors compile down to simple index or iterator operations with no runtime overhead.

### 7. Type Safety
Different descriptor types cannot be accidentally mixed:
```cpp
using VectorDesc = graph::vertex_descriptor<std::vector<int>::iterator>;
using MapDesc = graph::vertex_descriptor<std::map<int,int>::iterator>;
// These are distinct types - cannot be assigned to each other
```

### 8. Automatic Pattern Recognition
The library automatically recognizes common container patterns:

**Inner Value Patterns** (for `vertices(g)`):
- Random-access containers (vector, deque) - index-based vertex IDs
- Associative containers (map, unordered_map) - key-based vertex IDs
- Bidirectional containers - iterator-based vertex IDs

**Edge Value Patterns** (for `edges(g, u)`):
- Simple edges: `std::vector<int>` - integers as target IDs
- Pair edges: `std::vector<std::pair<int, Weight>>` - .first as target ID
- Tuple edges: `std::vector<std::tuple<int, ...>>` - first element as target ID
- Custom edges: structs/classes with custom target ID extraction

This means simple graph containers like `std::vector<std::vector<int>>` or `std::map<int, std::vector<int>>` work automatically without any customization!

## Documentation

### Main Documentation Files
- **[docs/common_graph_guidelines.md](docs/common_graph_guidelines.md)**: Complete naming conventions, architectural commitments, and project structure requirements
- **[docs/container_interface.md](docs/container_interface.md)**: Comprehensive summary of the Graph Container Interface (adjacency lists and edgelists)
- **[docs/graph_cpo_order.md](docs/graph_cpo_order.md)**: Canonical CPO implementation order with 19 CPOs organized by phases
- **[docs/graph_cpo_implementation.md](docs/graph_cpo_implementation.md)**: Complete CPO implementation guide with full code examples (11 CPOs with MSVC-style _Choice_t pattern)
- **[docs/vertex_inner_value_patterns.md](docs/vertex_inner_value_patterns.md)**: Inner value pattern concepts for automatic `vertices(g)` support
- **[docs/edge_value_concepts.md](docs/edge_value_concepts.md)**: Edge value pattern concepts for automatic `edges(g, u)` support
- **[docs/cpo.md](docs/cpo.md)**: Detailed guide for implementing customization point objects
- **[PHASE1_COMPLETE.md](PHASE1_COMPLETE.md)**: Phase 1 completion report with verification results
- **[MIGRATION_PHASE1.md](MIGRATION_PHASE1.md)**: Detailed record of Phase 1 migration

### Reference
This library follows the P1709 Graph Library proposal specifications:
- P1709/D3130 - Container Interface specification
- P1709 conventions.tex - Naming conventions

## Getting Started

### 1. Clone and Build

```bash
git clone https://github.com/pratzl/desc.git
cd desc

# Configure with preset
cmake --preset linux-gcc-debug

# Build
cmake --build build/linux-gcc-debug

# Run tests (75 tests)
ctest --test-dir build/linux-gcc-debug --output-on-failure
```

### 2. Include in Your Project

```cmake
# In your CMakeLists.txt
add_subdirectory(path/to/desc)
target_link_libraries(your_target PRIVATE graph::graph)
```

```cpp
// In your source files
#include <graph/graph.hpp>  // Main header includes all components
```

## Next Steps

### For Contributors
1. **Review Phase 1 completion**: See [PHASE1_COMPLETE.md](PHASE1_COMPLETE.md) for current status
2. **Implement Phase 2**: Graph utility CPOs following patterns in [docs/cpo.md](docs/cpo.md)
3. **Add containers**: Implement `adjacency_list` in `include/graph/container/`
4. **Follow conventions**: Use naming conventions from [docs/common_graph_guidelines.md](docs/common_graph_guidelines.md)

### For Users
Currently, the library provides:
- ✅ Descriptor abstractions for building custom graph types
- ✅ Complete documentation for the Graph Container Interface
- ✅ CPO framework for customization

Future phases will add:
- 🔜 Ready-to-use graph containers (adjacency_list, compressed_graph)
- 🔜 Graph algorithms (BFS, DFS, shortest paths, etc.)
- 🔜 Graph views (filtered, transformed, etc.)

## Build Configurations

The project includes CMake presets for common configurations:

```bash
# Linux GCC Debug
cmake --preset linux-gcc-debug
cmake --build build/linux-gcc-debug

# Linux GCC Release
cmake --preset linux-gcc-release
cmake --build build/linux-gcc-release

# Custom configuration
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++
cmake --build build
```

## Testing

The project includes 535 unit tests covering descriptor functionality, CPO implementations, partitioning, adjacency list concepts, adjacency list traits, and type aliases:

```bash
# Run all tests
ctest --test-dir build/linux-gcc-debug --output-on-failure

# Run specific test
ctest --test-dir build/linux-gcc-debug -R vertex_descriptor

# Run vertices(g) CPO tests
ctest --test-dir build/linux-gcc-debug -R vertices

# Run vertex_id(g,u) CPO tests
ctest --test-dir build/linux-gcc-debug -R vertex_id

# Run find_vertex(g,uid) CPO tests
ctest --test-dir build/linux-gcc-debug -R find_vertex

# Run edges(g,u) CPO tests
ctest --test-dir build/linux-gcc-debug -R "edges.*cpo"

# Run edges(g,uid) CPO tests
ctest --test-dir build/linux-gcc-debug -R "edges.*uid"

# Run target_id(g,uv) CPO tests
ctest --test-dir build/linux-gcc-debug -R target_id

# Run target(g,uv) CPO tests
ctest --test-dir build/linux-gcc-debug -R "target.*cpo"

# Run num_vertices(g) CPO tests
ctest --test-dir build/linux-gcc-debug -R num_vertices

# Run num_edges(g) CPO tests
ctest --test-dir build/linux-gcc-debug -R num_edges

# Run degree(g,u) and degree(g,uid) CPO tests
ctest --test-dir build/linux-gcc-debug -R degree

# Run partition_id(g,u) CPO tests
ctest --test-dir build/linux-gcc-debug -R partition_id

# Run num_partitions(g) CPO tests
ctest --test-dir build/linux-gcc-debug -R num_partitions

# Run type alias tests
ctest --test-dir build/linux-gcc-debug -R "Type aliases"

# Run adjacency list edge concept tests
ctest --test-dir build/linux-gcc-debug -R "adjacency.*edge.*concepts"

# Run adjacency list vertex concept tests
ctest --test-dir build/linux-gcc-debug -R "adjacency.*vertex.*concepts"

# Run adjacency list traits tests
ctest --test-dir build/linux-gcc-debug -R "has_"

# Verbose output
ctest --test-dir build/linux-gcc-debug -V
```

## Contributing

Contributions are welcome! Please:

1. Follow the naming conventions in [docs/common_graph_guidelines.md](docs/common_graph_guidelines.md)
2. Use CPO patterns from [docs/cpo.md](docs/cpo.md)
3. Add unit tests for new functionality
4. Update documentation as needed
5. Ensure all tests pass before submitting

## License

This library is distributed under the [Boost Software License 1.0](LICENSE). See the [LICENSE](LICENSE) file for details.

## Acknowledgments

This library follows the design principles and specifications from:
- P1709 Graph Library Proposal
- C++ Standards Committee Graph Library discussions
- MSVC Standard Library CPO patterns

---

**Status**: Phase 1-9 Complete ✅ | 3931/3931 Tests Passing ✅ | Core CPOs + Containers + Views Complete ✅

**Implemented CPOs**: vertices • vertex_id • find_vertex • edges • target_id • target • source_id • source • num_vertices • num_edges • degree • find_vertex_edge • contains_edge • has_edge • vertex_value • edge_value • graph_value • partition_id • num_partitions

**Implemented Concepts**: targeted_edge • sourced_edge • sourced_targeted_edge • targeted_edge_range • sourced_targeted_edge_range • vertex_range • index_vertex_range • adjacency_list • index_adjacency_list • sourced_adjacency_list • index_sourced_adjacency_list

**Implemented Containers**: dynamic_graph • compressed_graph • undirected_adjacency_list

**Implemented Views**: vertexlist • edgelist • neighbors • incidence • vertices_bfs • edges_bfs • vertices_dfs • edges_dfs • vertices_topological_sort • edges_topological_sort

**Implemented Traits**: has_degree • has_find_vertex • has_find_vertex_edge • has_contains_edge • define_unordered_edge • has_basic_queries • has_full_queries

````
