# Coding Guidelines

These requirements apply to every graph-related component, including implementations, views, algorithms, and the data model.

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
| `VProj` | | `vproj` | Vertex info projection function: `vproj(u)` → `vertex_data<VId,VV>`. |

### Partition Types

| Template Parameter | Type Alias | Variable Name | Description |
|-------------------|------------|---------------|-------------|
| | `partition_id_t<G>` | `pid` | Partition id |
| | | `P` | Number of partitions |
| `PVR` | `partition_vertex_range_t<G>` | `pur`, `pvr` | Partition vertex range |

### Edge Types

| Template Parameter | Type Alias | Variable Name | Description |
|-------------------|------------|---------------|-------------|
| `E` | `out_edge_t<G>` | `uv`, `vw` | Edge descriptor. `uv` is an edge from vertices `u` to `v`. `vw` is an edge from vertices `v` to `w`. Alias: `edge_t<G>`. |
| `EV` | `edge_value_t<G>` | `val` | Edge Value, value or reference. This can be either the user-defined value on an edge, or a value returned by a function object (e.g. `EVF`) that is related to the edge. |
| `ER` | `out_edge_range_t<G>` | | Edge Range for outgoing edges of a vertex. Alias: `vertex_edge_range_t<G>`. |
| `EI` | `out_edge_iterator_t<G>` | `uvi`, `vwi` | Edge Iterator for an outgoing edge of a vertex. `uvi` is an iterator for an edge from vertices `u` to `v`. `vwi` is an iterator for an edge from vertices `v` to `w`. Alias: `vertex_edge_iterator_t<G>`. |
| `EVF` | | `evf` | Edge Value Function: `evf(g, uv)` → edge value. Graph passed as first parameter for stateless lambdas enabling `std::views` chaining. |
| `EProj` | | `eproj` | Edge info projection function: `eproj(uv)` → `edge_data<VId,Sourced,EV>`. |

## Parameterized Type Aliases
Types use the `_t<G>` suffix to indicate they are parameterized by graph type `G`:
- `vertex_t<G>` - Vertex descriptor type (corresponds to `V`)
- `out_edge_t<G>` - Edge descriptor type (corresponds to `E`). Alias: `edge_t<G>`.
- `vertex_id_t<G>`, `vertex_value_t<G>`, `edge_value_t<G>`, etc. - Other derived types
- `vertex_iterator_t<G>` - Iterator type for traversing vertices
- `vertex_range_t<G>` - Range type that enumerates vertices
- `out_edge_range_t<G>` - Range type for the outgoing edges of a vertex. Alias: `vertex_edge_range_t<G>`.
- `out_edge_iterator_t<G>` - Iterator type for traversing outgoing edges. Alias: `vertex_edge_iterator_t<G>`.

## Public Interface Requirements
All public functions for the graph data model have the following requirements:
1. All implementations MUST be customization point objects (CPO) and MUST follow the style used by the MSVC standard library implementation.
2. The return types of the functions MUST be used to define parameterized types.
3. Default implementations MUST be provided whenever possible in the CPO.
4. Each CPO function MAY be overridden for each graph type, vertex type and edge type.

When creating a new instruction document, reference this file near the top and only document the requirements unique to that artifact.

## Project Structure

## Project Structure

See [Architecture](architecture.md) for the full directory tree and design rationale.

**Key directories:**
- `include/graph/` — all public headers (header-only library)
- `include/graph/algorithm/` — one header per algorithm
- `include/graph/views/` — lazy range views
- `include/graph/container/` — library-provided containers (`dynamic_graph`, `compressed_graph`)
- `include/graph/adj_list/` — descriptor system (vertex/edge descriptors, traits)
- `include/graph/edge_list/` — edge list model
- `include/graph/detail/` — CPO machinery and internal helpers
- `tests/` — Catch2 test suite, mirroring the source structure
- `examples/` — compilable example programs
- `benchmark/` — Google Benchmark micro-benchmarks
