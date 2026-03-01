<table><tr>
<td><img src="../../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Biconnected Components

</td>
</tr></table>

> [← Back to Algorithm Catalog](../algorithms.md)

## Table of Contents
- [Overview](#overview)
- [When to Use](#when-to-use)
- [Include](#include)
- [Signature](#signature)
- [Parameters](#parameters)
- [Examples](#examples)
  - [Finding Biconnected Components](#example-1-finding-biconnected-components)
  - [Bridge Detection](#example-2-bridge-detection)
  - [Identifying Articulation Points](#example-3-identifying-articulation-points)
  - [Complete Graph — Single Component](#example-4-complete-graph--single-component)
  - [Disconnected Graph with Isolated Vertices](#example-5-disconnected-graph-with-isolated-vertices)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [See Also](#see-also)

## Overview

A **biconnected component** is a maximal subgraph that remains connected when any
single vertex is removed. The algorithm uses the iterative **Hopcroft-Tarjan**
approach with discovery times and low-link values to find all biconnected
components.

The graph must satisfy `index_adjacency_list<G>` — vertices are stored in a
contiguous, integer-indexed random-access range.

Key properties of the output:

- Each biconnected component is a set of vertex IDs.
- **Articulation points** appear in multiple biconnected components.
- A **bridge** (cut edge) forms its own 2-vertex biconnected component.
- A single edge between two non-articulation vertices forms a component.
- **Isolated vertices** are emitted as single-vertex components.

## When to Use

- **Network reliability analysis** — identify parts of a network that survive
  any single-node failure. A biconnected component has at least two
  vertex-disjoint paths between every pair of vertices.
- **Bridge detection** — bridges are biconnected components with exactly 2
  vertices (representing a single edge whose removal disconnects the graph).
- **Finding articulation points indirectly** — vertices appearing in multiple
  biconnected components are articulation points (though the dedicated
  [articulation_points](articulation_points.md) algorithm is simpler if that's
  all you need).

**Not suitable when:**

- You only need connected components → use
  [connected_components](connected_components.md).
- You only need articulation points → use
  [articulation_points](articulation_points.md).

## Include

```cpp
#include <graph/algorithm/biconnected_components.hpp>
```

## Signature

```cpp
void biconnected_components(G&& g, OuterContainer& components);
```

Where `OuterContainer` is typically `std::vector<std::vector<vertex_id_t<G>>>`.

## Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `index_adjacency_list` |
| `components` | Output container of containers. Each inner container holds vertex IDs in one biconnected component. Cleared and refilled by the algorithm. |

## Examples

### Example 1: Finding Biconnected Components

Find all biconnected components in a graph with a bridge.

```cpp
#include <graph/algorithm/biconnected_components.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <vector>

using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
    container::vov_graph_traits<void>>;

// Bridge graph: two triangles connected by a bridge
//   0 - 1     3 - 4
//    \ |  --- |  /
//     2  ---- 3
// Edges (bidirectional for undirected):
Graph g({{0,1},{1,0}, {1,2},{2,1}, {0,2},{2,0},   // triangle 0-1-2
         {2,3},{3,2},                                // bridge 2-3
         {3,4},{4,3}, {3,5},{5,3}, {4,5},{5,4}});   // triangle 3-4-5

std::vector<std::vector<uint32_t>> components;
biconnected_components(g, components);

// components contains 3 biconnected components:
//   {0, 1, 2}  — left triangle
//   {2, 3}     — the bridge (appears as its own 2-vertex component)
//   {3, 4, 5}  — right triangle
// Note: vertices 2 and 3 are articulation points (appear in multiple components)
```

### Example 2: Bridge Detection

Bridges are biconnected components with exactly 2 vertices — they represent
a single edge whose removal disconnects the graph.

```cpp
std::vector<std::vector<uint32_t>> components;
biconnected_components(g, components);

std::cout << "Bridges:\n";
for (const auto& comp : components) {
    if (comp.size() == 2) {
        std::cout << "  " << comp[0] << " — " << comp[1] << "\n";
    }
}
// Output: Bridge: 2 — 3
```

### Example 3: Identifying Articulation Points

Articulation points appear in more than one biconnected component.

```cpp
std::vector<std::vector<uint32_t>> components;
biconnected_components(g, components);

// Count how many components each vertex appears in
std::vector<int> count(num_vertices(g), 0);
for (const auto& comp : components) {
    for (auto v : comp)
        ++count[v];
}

for (uint32_t v = 0; v < count.size(); ++v) {
    if (count[v] >= 2)
        std::cout << "Articulation point: " << v << "\n";
}
// Output: Articulation point: 2, Articulation point: 3

// For a dedicated algorithm, see articulation_points.md
```

### Example 4: Complete Graph — Single Component

A complete graph (K_n) is itself biconnected — removing any single vertex
still leaves a connected graph.

```cpp
// K4: complete graph on 4 vertices
Graph k4({{0,1},{1,0}, {0,2},{2,0}, {0,3},{3,0},
          {1,2},{2,1}, {1,3},{3,1}, {2,3},{3,2}});

std::vector<std::vector<uint32_t>> components;
biconnected_components(k4, components);

// components.size() == 1  — the entire graph is one biconnected component
// components[0] = {0, 1, 2, 3}
// No articulation points, no bridges
```

### Example 5: Disconnected Graph with Isolated Vertices

Isolated vertices form their own single-vertex components. Disconnected
subgraphs are processed independently.

```cpp
// Disconnected: triangle {0,1,2}, isolated vertex 3, edge {4,5}
Graph g({{0,1},{1,0}, {1,2},{2,1}, {0,2},{2,0},
         {4,5},{5,4}});
// Vertex 3 has no edges

std::vector<std::vector<uint32_t>> components;
biconnected_components(g, components);

// components contains:
//   {0, 1, 2}  — the triangle
//   {3}        — isolated vertex
//   {4, 5}     — the single edge (a "bridge" with no further structure)
```

## Complexity

| Metric | Value |
|--------|-------|
| Time | O(V + E) |
| Space | O(V + E) for the component output and internal stack |

## Preconditions

- Graph must satisfy `index_adjacency_list<G>`.
- For undirected graphs, **both directions** of each edge must be stored (or use
  `undirected_adjacency_list`).
- Self-loops are ignored and do not affect the result.

## See Also

- [Articulation Points](articulation_points.md) — dedicated cut-vertex algorithm
- [Connected Components](connected_components.md) — connected/strongly connected components
- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_biconnected_components.cpp](../../../tests/algorithms/test_biconnected_components.cpp) — test suite
