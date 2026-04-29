# Enable the use of BGL graph data structures by graph-v3 views and algorithms

I want algorithms and views in this library to work with graphs created in BGL.
This will help BGL users explore this library before committing to it, and ease transition
from BGL.

## Definitions
- `BGL`: Boost Graph Library
- `CPO`: Customization Point Object

## Requirements
- Create ADL-based CPO customizations to adapt BGL graph types for use by views and algorithms
  in this library.
- Identify any additional features needed to enable the adaptation of BGL graphs. 
  - Simple changes can be done immediately. 
  - Tasks that take longer may require separate efforts to enable the adaptation.
- Identify how readable and writable BGL property maps can be represented using this library's
  `vertex_value_function` and `edge_value_function` concepts (function objects that access
  properties on a vertex or edge).
  - Function objects that expose writable properties should return a reference to the underlying
    value.
- Test the supported phase-1 BGL graph types and the graph-v3 views and algorithms intended to
  work with them.
- Create documentation to help BGL users use their graphs.

## Scope & Priority
Start with the most common BGL graph types and expand:
1. `adjacency_list<vecS, vecS, directedS>` — most common BGL graph; `vecS` vertex lists
   map naturally to `index_adjacency_list` because vertex descriptors are integral indices.
2. Undirected and bidirectional variants (`undirectedS`, `bidirectionalS`).
3. `compressed_sparse_row_graph` — valid phase-1 target because it matches `compressed_graph`
  in the current library.
4. Alternative vertex/edge selectors (`listS`, `setS`) — follow-on work after the phase-1
  targets are working.
5. `adjacency_matrix` — candidate for later expansion.

## Phase-1 Adaptation Surface
Phase 1 should establish the minimum graph interface needed for the initial BGL targets to work
with graph-v3 views and algorithms. This includes support for:
- Vertex and edge access used by graph-v3 concepts and algorithms, such as `vertices`,
  `out_edges`, `vertex_id`, `source_id`, `target_id`, and `num_vertices`.
- `in_edges` where bidirectional BGL graphs are part of the supported target set.
- Property access for readable and writable vertex and edge properties.

Detailed concept mapping, descriptor-model differences, and ID-mapping approaches for non-index
descriptor types should be captured in `bgl_migration_strategy.md` rather than expanded here.

## Phase-1 Views and Algorithms
Phase 1 should prove that the adaptor works with a focused set of graph-v3 functionality:
- Views: `vertexlist`, `incidence`, `neighbors`, and `edgelist`.
- Algorithms: `dijkstra_shortest_paths` first, then the additional algorithms that naturally fit
  the supported phase-1 graph types.

## Acceptance Criteria
- A BGL user can run graph-v3's `dijkstra_shortest_paths` on a
  BGL `adjacency_list<vecS, vecS, directedS>` graph with minimal adaptor boilerplate.
- A BGL user can run the phase-1 target views and algorithms on supported BGL graph types.
- BGL property maps are accessible through graph-v3's function-object property interface,
  including writable properties needed by algorithms such as shortest paths.
- At least one end-to-end example and corresponding tests exist.
- After all previous acceptance criteria are achieved, update `bgl_migration_strategy.md` with any
  additional information that may be useful.

## Other Information
- A measure of success is a minimal set of work required by the BGL user to use their graphs.
- `bgl2_comparison_query.md` and `bgl2_comparison_result.md` contain information for an experimental,
  agent-assisted conversion of BGL to use C++20 and its idioms. They should be avoided because we want 
  to focus on the active BGL library.
- `bgl_migration_strategy.md` contains a comprehensive BGL → graph-v3 migration analysis;
  Section 12 specifically covers adapting an existing BGL graph for graph-v3.
- `uniform_prop_goal.md` defines the uniform property-function design that BGL property maps
  must map into; progress on that effort may affect what is possible here.
- `benchmark/algorithms/bgl_dijkstra_fixtures.hpp` already builds BGL and graph-v3 graphs
  from identical edge sources and is a useful starting point.

## References
- BGL headers are at `/home/phil/dev_graph/boost/boost/graph/`
- BGL source is at `/home/phil/dev_graph/boost/libs/graph/`
