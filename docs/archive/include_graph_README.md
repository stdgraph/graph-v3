# Graph Library Directory Structure

This project follows the standard graph library structure as defined in `docs/common_graph_guidelines.md`.

## Directory Layout

```
include/graph/
├── algorithm/                  # Graph algorithms (BFS, DFS, shortest paths, etc.)
├── container/                  # Graph container implementations (adjacency_list, CSR, etc.)
├── detail/                     # Private implementation details
├── views/                      # Graph views and adaptors
├── descriptor.hpp              # Core descriptor concepts
├── descriptor_traits.hpp       # Type traits for descriptors
├── vertex_descriptor.hpp       # Vertex descriptor implementation
├── vertex_descriptor_view.hpp  # Vertex descriptor view
├── edge_descriptor.hpp         # Edge descriptor implementation
├── edge_descriptor_view.hpp    # Edge descriptor view
├── graph.hpp                   # Main graph library header (include this)
└── graph_utility.hpp           # Graph utility CPOs (future)
```

## Current Status

### ✅ Completed
- Core descriptor types and concepts
- Vertex and edge descriptors for keyed and direct storage
- Descriptor views for range-based access
- C++20 concepts for graph types
- Comprehensive CPO implementation guide (`docs/cpo.md`)
- Directory structure aligned with common_graph_guidelines.md

### 🚧 In Progress
- Migration from `desc` namespace to `graph` namespace
- Directory reorganization to match standard structure

### 📋 Planned
- **Graph Container Interface** (`include/graph/graph_cpo.hpp`)
  - CPO functions that define a common interface for working with graphs, with defaults for common use cases
  - Use vertex and edge descriptors for all functions except find functions

- **Containers** (`include/graph/container/`)
  - `dynamic_graph.hpp` - Dynamic adjacency list
  - `compressed_graph.hpp` - CSR graph representation
  
- **Views** (`include/graph/views/`)
  - `vertexlist.hpp` - Vertex range views
  - `incidence.hpp` - Range of outgoing edges of a vertex
  - `neighbors.hpp` - Range of outgoing vertices of a vertex
  - `edgelist.hpp` - All edges in a graph
  
- **Algorithms** (`include/graph/algorithm/`)
  - `breadth_first_search.hpp`
  - `depth_first_search.hpp`
  - `dijkstra_shortest_paths.hpp`
  - `topological_sort.hpp`
  
- **Utility CPOs** (`include/graph/graph_utility.hpp`)
  - `vertex_id(g, v)` - Get vertex identifier
  - `num_vertices(g)` - Count vertices
  - `num_edges(g)` - Count edges
  - `vertices(g)` - Get vertex range
  - `edges(g)` - Get edge range
  - `out_edges(g, v)` - Get outgoing edges

## Building

```bash
cmake --preset linux-gcc-debug
cmake --build build/linux-gcc-debug
ctest --test-dir build/linux-gcc-debug
```

## Usage

```cpp
#include <graph/graph.hpp>

// Currently available: descriptor types and concepts
// Future: full graph containers and algorithms
```

## Design Principles

1. **CPO-based interface** - All graph operations are customization point objects following MSVC style (see `docs/cpo.md`)
2. **Adapt existing structures** - Can wrap existing graph data structures
3. **C++20 concepts** - Type-safe, clear constraints
4. **Zero-overhead abstractions** - No runtime cost for abstraction layers
5. **Cross-platform** - Windows, Linux, macOS support

See `docs/common_graph_guidelines.md` for complete architectural requirements.
