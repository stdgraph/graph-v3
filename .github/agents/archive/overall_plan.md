# Overall plan for implementation

## Phase 1 Descriptors and Descriptor Views

1. Descriptors
- descriptor base
- vertex_descriptor
- edge_descriptor

2. Descriptor Views
- vertex_descriptor_view
- edge_descriptor_view

## Phase 2 CPO and concept definitions

## Phase 3 compressed_graph

- Add comprehensive tests for existing functionality
- Add required CPO implementations and tests

## PHase 4 Cross-platform improvements

- Test on Windows
- CMake improvements (use Jason Turner's example)
- Use in Visual Studio

## Phase 5 dynamic_graph

- Add comprehensive tests for existing functionality
- Add required CPO implementations and tests
- Add support for map and unordered_map for vertices
- Add support for map, set, unordered_map and unordered_set for edges
- Add support for non-integral vertex id types

## Phase 6 undirected_graph

- Add from history
- Add comprehensive tests for existing functionality
- Add required CPO implementations and tests
- Add support for map and unordered_map for vertices
- Add support for non-integral vertex id types

## Phase 7 Add Edgelist to complement Adjacency List

## Phase 8 Add Views

- vertexlist (compare to vertex_descriptor_view)
- incidence
- neighbors
- edgelist
- bfs
- dfs
- topo sort

## Phase 9 Add Algorithms

