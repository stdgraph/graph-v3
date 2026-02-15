# Common Graph Guidelines

These requirements apply to every graph-related instruction set, including implementations, views, algorithms, and the data model.

## Core Architectural Commitments
1. The interface MUST be able to adapt to existing graph data structures.
2. The interface focuses on outgoing edges. If the underlying graph exposes incoming edges, a separate view will present them through this outgoing-edge interface for consistency.
3. Graph, vertex, and edge functions are customization point objects (CPOs) so they can be overridden for a graph data structure.

## Implementation Requirements
- MUST use C++20.
- MUST support Windows, Linux, and macOS.
- MUST support the primary compiler for each OS; MAY support additional compilers per OS.

## Naming Conventions

### Graph Types

| Template Parameter | Type Alias | Variable Name | Description |
|-------------------|------------|---------------|-------------|
| `G` | | `g` | Graph type |
| | `graph_reference_t<G>` | `g` | Graph reference |
| `GV` | | `val` | Graph Value, value or reference |

### Edgelist Types

| Template Parameter | Type Alias | Variable Name | Description |
|-------------------|------------|---------------|-------------|
| `EL` | | `el` | Edge list |

### Vertex Types

| Template Parameter | Type Alias | Variable Name | Description |
|-------------------|------------|---------------|-------------|
| `V` | `vertex_t<G>` | `u`, `v` | Vertex descriptor. `u` is the source (or only) vertex. `v` is the target vertex. |
| `VId` | `vertex_id_t<G>` | `uid`, `vid`, `source` | Vertex id. `uid` is the source (or only) vertex id. `vid` is the target vertex id. |
| `VV` | `vertex_value_t<G>` | `val` | Vertex Value, value or reference. This can be either the user-defined value on a vertex, or a value returned by a function object (e.g. `VVF`) that is related to the vertex. |
| `VR` | `vertex_range_t<G>` | `ur`, `vr` | Vertex Range |
| `VI` | `vertex_iterator_t<G>` | `ui`, `vi` | Vertex Iterator. `ui` is the source (or only) vertex iterator. `vi` is the target vertex iterator. |
| | | `first`, `last` | `first` and `last` are the begin and end iterators of a vertex range. |
| `VVF` | | `vvf` | Vertex Value Function: `vvf(g, u)` → vertex value. Graph passed as first parameter for stateless lambdas enabling `std::views` chaining. |
| `VProj` | | `vproj` | Vertex info projection function: `vproj(u)` → `vertex_info<VId,VV>`. |

### Partition Types

| Template Parameter | Type Alias | Variable Name | Description |
|-------------------|------------|---------------|-------------|
| | `partition_id_t<G>` | `pid` | Partition id |
| | | `P` | Number of partitions |
| `PVR` | `partition_vertex_range_t<G>` | `pur`, `pvr` | Partition vertex range |

### Edge Types

| Template Parameter | Type Alias | Variable Name | Description |
|-------------------|------------|---------------|-------------|
| `E` | `edge_t<G>` | `uv`, `vw` | Edge descriptor. `uv` is an edge from vertices `u` to `v`. `vw` is an edge from vertices `v` to `w`. |
| `EV` | `edge_value_t<G>` | `val` | Edge Value, value or reference. This can be either the user-defined value on an edge, or a value returned by a function object (e.g. `EVF`) that is related to the edge. |
| `ER` | `vertex_edge_range_t<G>` | | Edge Range for edges of a vertex |
| `EI` | `vertex_edge_iterator_t<G>` | `uvi`, `vwi` | Edge Iterator for an edge of a vertex. `uvi` is an iterator for an edge from vertices `u` to `v`. `vwi` is an iterator for an edge from vertices `v` to `w`. |
| `EVF` | | `evf` | Edge Value Function: `evf(g, uv)` → edge value. Graph passed as first parameter for stateless lambdas enabling `std::views` chaining. |
| `EProj` | | `eproj` | Edge info projection function: `eproj(uv)` → `edge_info<VId,Sourced,EV>`. |

## Parameterized Type Aliases
Types use the `_t<G>` suffix to indicate they are parameterized by graph type `G`:
- `vertex_t<G>` - Vertex descriptor type (corresponds to `V`)
- `edge_t<G>` - Edge descriptor type (corresponds to `E`)
- `vertex_id_t<G>`, `vertex_value_t<G>`, `edge_value_t<G>`, etc. - Other derived types
- `vertex_iterator_t<G>` - Iterator type for traversing vertices
- `vertex_range_t<G>` - Range type that enumerates vertices
- `vertex_edge_range_t<G>` - Range type for the outgoing edges of a vertex
- `edge_iterator_t<G>` - Iterator type for traversing edges

## Public Interface Requirements
All public functions for the graph data model have the following requirements:
1. All implementations MUST be customization point objects (CPO) and MUST follow the style used by the MSVC standard library implementation.
2. The return types of the functions MUST be used to define parameterized types.
3. Default implementations MUST be provided whenever possible in the CPO.
4. Each CPO function MAY be overridden for each graph type, vertex type and edge type.

When creating a new instruction document, reference this file near the top and only document the requirements unique to that artifact.

## Project Structure

The implementation MUST follow this directory structure (based on graph-v2):

```
graph-v3/
├── benchmark/              # Performance benchmarks
│   └── CMakeLists.txt
├── cmake/                  # CMake modules and scripts
├── data/                   # Test data files
├── doc/                    # Documentation (Doxygen output)
├── docs/                   # Documentation source
│   └── sphinx/
├── example/                # Example programs
│   ├── CMakeLists.txt
│   ├── AdaptingThirdPartyGraph/
│   ├── CppCon2021/
│   ├── CppCon2022/
│   └── PageRank/
├── include/                # Public header files
│   └── graph/
│       ├── algorithm/        # Graph algorithms
│       ├── container/        # Graph container implementations
│       ├── detail/           # Implementation details (private)
│       │   ├── graph_cpo.hpp # Graph CPOs
│       │   └── graph_using.hpp # Using statements for common types and functions in the standard library
│       ├── views/            # Graph views
│       ├── edgelist.hpp      # Edge list functionality
│       ├── graph.hpp         # Main graph header with graph concepts and graph traits
│       ├── graph_info.hpp    # Graph information structures
│       └── graph_utility.hpp # Utility functions
├── scripts/                # Build and maintenance scripts
│   └── format.sh
├── tests/                  # Unit tests
│   ├── CMakeLists.txt
│   ├── catch_main.cpp      # Catch2 main
│   ├── *_tests.cpp         # Test files for each component
│   ├── csv.hpp             # Test utilities
│   ├── csv_routes.cpp
│   ├── csv_routes.hpp
│   ├── examples.cpp
│   └── tests.cpp
├── .clang-format           # Code formatting rules
├── .clang-tidy             # Static analysis rules
├── .cmake-format.yaml      # CMake formatting rules
├── .gitignore              # Git ignore patterns
├── CMakeLists.txt          # Root CMake configuration
├── CMakePresets.json       # CMake presets for build configurations
├── CODE_OF_CONDUCT.md      # Code of conduct
├── CONTRIBUTING.md         # Contribution guidelines
├── LICENSE                 # License file
├── README.md               # Project readme
├── Notes.md                # Development notes
└── ToDo.md                 # TODO list
```

**Key Directories**:
- `include/graph/`: All public headers implementing the graph data model interface
- `tests/`: Comprehensive unit tests for all functionality
- `example/`: Example programs demonstrating usage patterns
- `benchmark/`: Performance benchmarks and comparison tests
- `cmake/`: Modular CMake configuration for dependencies and build settings
- `data/`: Test data in various formats (Matrix Market, CSV)

**Note**: The `out/` directory (generated build artifacts) is excluded from version control and documentation.
