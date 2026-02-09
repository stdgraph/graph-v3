# Graph Library

A modern C++20 graph library providing a comprehensive, customizable interface for graph data structures, algorithms, and views.

**graph-v3** is the next iteration from graph-v2, focused on descriptors for improved abstractions.

_Near-term goals are to add more algorithms and to reorganize and improve the documentation._

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

### Enhancements and Differences From graph-v2

A significant shift occurred with the use of descriptors to the underlying containers for vertices
and edges. This changed the implementation from a reference-based to a value-based implementation and had
subtle changes that rippled through the library.

Here's a summary of the major changes
- Benefits from descriptors
  - "sourced" functions no longer needed because source vertex is always available with the edge descriptor
  - Function overloads for separate vertex_id-only and vertex references are no longer needed
  - Fewer concepts needed
  - No need to tag an adjacency list as "undirected"
- Graph Container changes
  - The `undirected_adjacency_list` graph data structure was added to test important use cases.
  - Vertices can be stored in `map` and `unordered_map` containers in `dynamic_graph` for sparse vertex_ids.
  - Edges can be stored in `map`, `set`, `unordered_map` and `unordered_set` in `dynamic_graph`.
  - Non-integral vertex_ids Support can be used in `dynamic_graph`.
- Graph Container Interface changes
  - Vertices can be stored in bidirectional (e.g. `map`) and forward (e.g. )`unordered_map`) ranges.
  - Edges can be stored in `map`, `set`, `unordered_map` and `unordered_set`.
  - Add support for non-integral vertex_ids.
- View changes
  - The `topological_sort_view` was implemented. It was in the graph proposal documents but never implemented.
    A "_safe" version of the view was added to detect cycles. (The implementation works on the whole graph.)
  - Cancellation and depth() was added to the `vertices_bfs` and `edges_bfs` views.
- Improved organization and use of namespaces
  - The `graph::adj_list` namespace was added
  - Edge lists are now a peer of adjacency lists in `graph::edge_list`
  - Directories have been created that roughtly resemble the namespace organization
- Extensive documentation was added
- Extensive unit tests have been added

We're trying to stay with C++20. However, `std::expected` from C++23 has been introduced and is being used
for `topological_sort_view`. An open-source library is being used for it until C++23 is enabled. There is
no target date for doing that.

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

**Implemented Concepts**: edge • vertex • vertex_edge_range • vertex_range • index_vertex_range • adjacency_list • index_adjacency_list • ordered_vertex_edges

**Implemented Containers**: dynamic_graph • compressed_graph • undirected_adjacency_list

**Implemented Views**: vertexlist • edgelist • neighbors • incidence • vertices_bfs • edges_bfs • vertices_dfs • edges_dfs • vertices_topological_sort • edges_topological_sort

**Implemented Traits**: has_degree • has_find_vertex • has_find_vertex_edge • has_contains_edge • has_basic_queries • has_full_queries

````
