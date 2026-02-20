# Algorithms

> [← Back to Documentation Index](../index.md)

- [Overview](#overview)
- [Algorithm Catalog](#algorithm-catalog)
  - [By Category](#by-category)
  - [Alphabetical](#alphabetical)
- [Shortest Paths](#shortest-paths)
- [Traversal](#traversal)
- [Components](#components)
- [Minimum Spanning Trees](#minimum-spanning-trees)
- [Graph Analytics](#graph-analytics)
- [Common Infrastructure](#common-infrastructure)
- [Roadmap](#roadmap)

## Overview

graph-v3 algorithms operate on any graph type satisfying the `index_adjacency_list`
concept — this includes all three library containers (`dynamic_graph`,
`compressed_graph`, `undirected_adjacency_list`) and range-of-ranges graphs with
random-access vertex containers.

Algorithms follow a consistent pattern:

- **Input**: graph `g`, source vertex (or vertices), and optional parameters
- **Output**: filled via caller-provided output ranges (distances, predecessors,
  component labels) — not returned by value
- **Weight functions**: passed as callable `WF(g, uv)` returning edge weight
- **Visitors**: optional structs with callback methods for fine-grained event hooks
- **Initialization**: use `init_shortest_paths()` to properly set up distance and
  predecessor vectors before calling shortest-path algorithms

```cpp
#include <graph/algorithm/dijkstra_shortest_paths.hpp>

// Typical usage pattern
std::vector<int>    distance(num_vertices(g));
std::vector<size_t> predecessor(num_vertices(g));

init_shortest_paths(distance, predecessor);

dijkstra_shortest_paths(g, 0, distance, predecessor,
    [](const auto& g, const auto& uv) { return edge_value(g, uv); });
```

## Algorithm Catalog

All headers are under `include/graph/algorithm/`.

### By Category

**Shortest Paths**

| Algorithm | Header | Brief description | Time | Space |
|-----------|--------|-------------------|------|-------|
| [Bellman-Ford](algorithms/bellman_ford.md) | `bellman_ford_shortest_paths.hpp` | Shortest paths with negative weights; cycle detection | O(V·E) | O(1) |
| [Dijkstra](algorithms/dijkstra.md) | `dijkstra_shortest_paths.hpp` | Single/multi-source shortest paths (non-negative weights) | O((V+E) log V) | O(V) |

**Traversal**

| Algorithm | Header | Brief description | Time | Space |
|-----------|--------|-------------------|------|-------|
| [BFS](algorithms/bfs.md) | `breadth_first_search.hpp` | Level-order traversal from source(s) | O(V+E) | O(V) |
| [DFS](algorithms/dfs.md) | `depth_first_search.hpp` | Depth-first traversal with edge classification | O(V+E) | O(V) |
| [Topological Sort](algorithms/topological_sort.md) | `topological_sort.hpp` | Linear ordering of DAG vertices | O(V+E) | O(V) |

**Components**

| Algorithm | Header | Brief description | Time | Space |
|-----------|--------|-------------------|------|-------|
| [Articulation Points](algorithms/articulation_points.md) | `articulation_points.hpp` | Cut vertices whose removal disconnects the graph | O(V+E) | O(V) |
| [Biconnected Components](algorithms/biconnected_components.md) | `biconnected_components.hpp` | Maximal 2-connected subgraphs (Hopcroft-Tarjan) | O(V+E) | O(V+E) |
| [Connected Components](algorithms/connected_components.md) | `connected_components.hpp` | Undirected CC, directed SCC (Kosaraju), union-find (afforest) | O(V+E) | O(V) |

**Minimum Spanning Trees**

| Algorithm | Header | Brief description | Time | Space |
|-----------|--------|-------------------|------|-------|
| [Kruskal MST](algorithms/mst.md#kruskals-algorithm) | `mst.hpp` | Edge-list-based MST via union-find | O(E log E) | O(E+V) |
| [Prim MST](algorithms/mst.md#prims-algorithm) | `mst.hpp` | Adjacency-list-based MST via priority queue | O(E log V) | O(V) |

**Analytics**

| Algorithm | Header | Brief description | Time | Space |
|-----------|--------|-------------------|------|-------|
| [Jaccard Coefficient](algorithms/jaccard.md) | `jaccard.hpp` | Pairwise neighbor-set similarity per edge | O(V + E·d) | O(V+E) |
| [Label Propagation](algorithms/label_propagation.md) | `label_propagation.hpp` | Community detection via majority-vote labels | O(E) per iter | O(V) |
| [Maximal Independent Set](algorithms/mis.md) | `mis.hpp` | Greedy MIS (non-adjacent vertex set) | O(V+E) | O(V) |
| [Triangle Count](algorithms/triangle_count.md) | `tc.hpp` | Count 3-cliques via sorted-list intersection | O(m^{3/2}) | O(1) |

### Alphabetical

| Algorithm | Category | Header | Time | Space |
|-----------|----------|--------|------|-------|
| [Articulation Points](algorithms/articulation_points.md) | Components | `articulation_points.hpp` | O(V+E) | O(V) |
| [Bellman-Ford](algorithms/bellman_ford.md) | Shortest Paths | `bellman_ford_shortest_paths.hpp` | O(V·E) | O(1) |
| [BFS](algorithms/bfs.md) | Traversal | `breadth_first_search.hpp` | O(V+E) | O(V) |
| [Biconnected Components](algorithms/biconnected_components.md) | Components | `biconnected_components.hpp` | O(V+E) | O(V+E) |
| [Connected Components](algorithms/connected_components.md) | Components | `connected_components.hpp` | O(V+E) | O(V) |
| [DFS](algorithms/dfs.md) | Traversal | `depth_first_search.hpp` | O(V+E) | O(V) |
| [Dijkstra](algorithms/dijkstra.md) | Shortest Paths | `dijkstra_shortest_paths.hpp` | O((V+E) log V) | O(V) |
| [Jaccard Coefficient](algorithms/jaccard.md) | Analytics | `jaccard.hpp` | O(V + E·d) | O(V+E) |
| [Kruskal MST](algorithms/mst.md#kruskals-algorithm) | MST | `mst.hpp` | O(E log E) | O(E+V) |
| [Label Propagation](algorithms/label_propagation.md) | Analytics | `label_propagation.hpp` | O(E) per iter | O(V) |
| [Maximal Independent Set](algorithms/mis.md) | Analytics | `mis.hpp` | O(V+E) | O(V) |
| [Prim MST](algorithms/mst.md#prims-algorithm) | MST | `mst.hpp` | O(E log V) | O(V) |
| [Topological Sort](algorithms/topological_sort.md) | Traversal | `topological_sort.hpp` | O(V+E) | O(V) |
| [Triangle Count](algorithms/triangle_count.md) | Analytics | `tc.hpp` | O(m^{3/2}) | O(1) |

---

## Shortest Paths

### [Dijkstra's Shortest Paths](algorithms/dijkstra.md)

Finds single-source or multi-source shortest paths in a graph with **non-negative**
edge weights using a binary-heap priority queue. Provides both `dijkstra_shortest_paths`
(distances + predecessors) and `dijkstra_shortest_distances` (distances only) variants.

**Time:** O((V+E) log V) — **Space:** O(V) — **Header:** `dijkstra_shortest_paths.hpp`

### [Bellman-Ford Shortest Paths](algorithms/bellman_ford.md)

Finds shortest paths supporting **negative edge weights** and detects negative-weight
cycles. Returns `std::optional<vertex_id_t<G>>` — empty if no negative cycle, or a
vertex on the cycle. Use `find_negative_cycle` to extract the full cycle path.

**Time:** O(V·E) — **Space:** O(1) — **Header:** `bellman_ford_shortest_paths.hpp`

---

## Traversal

### [Breadth-First Search](algorithms/bfs.md)

Explores vertices in level-order (FIFO) from one or more sources. Entirely
**visitor-driven** — you provide callback methods for vertex/edge events. Supports
single-source and multi-source variants.

**Time:** O(V+E) — **Space:** O(V) — **Header:** `breadth_first_search.hpp`

### [Depth-First Search](algorithms/dfs.md)

Performs iterative DFS with three-color marking (White/Gray/Black), enabling precise
**edge classification** into tree, back, and forward/cross edges. Foundation for cycle
detection, topological sorting, and SCC discovery.

**Time:** O(V+E) — **Space:** O(V) — **Header:** `depth_first_search.hpp`

### [Topological Sort](algorithms/topological_sort.md)

Produces a linear ordering of vertices in a DAG such that for every edge (u,v),
u appears before v. Returns `bool` — `false` if a cycle is detected. Supports
full-graph, single-source, and multi-source variants.

**Time:** O(V+E) — **Space:** O(V) — **Header:** `topological_sort.hpp`

---

## Components

### [Connected Components](algorithms/connected_components.md)

Three algorithms in one header: `connected_components` (DFS-based, undirected),
`kosaraju` (two-pass DFS for directed SCC, requires transpose graph), and
`afforest` (union-find with neighbor sampling, parallel-friendly).

**Time:** O(V+E) — **Space:** O(V) — **Header:** `connected_components.hpp`

### [Biconnected Components](algorithms/biconnected_components.md)

Finds all maximal 2-connected subgraphs using the iterative Hopcroft-Tarjan
algorithm. Articulation points appear in multiple components; bridges form their
own 2-vertex components.

**Time:** O(V+E) — **Space:** O(V+E) — **Header:** `biconnected_components.hpp`

### [Articulation Points](algorithms/articulation_points.md)

Finds cut vertices whose removal disconnects the graph, using the iterative
Hopcroft-Tarjan algorithm with discovery times and low-link values. Each
articulation point is emitted exactly once.

**Time:** O(V+E) — **Space:** O(V) — **Header:** `articulation_points.hpp`

---

## Minimum Spanning Trees

### [Kruskal's Algorithm](algorithms/mst.md#kruskals-algorithm)

Edge-list-based MST using sort + union-find. Returns `{total_weight, num_components}`.
Includes `inplace_kruskal` variant that sorts input in-place. Pass `std::greater<>{}`
for maximum spanning tree.

**Time:** O(E log E) — **Space:** O(E+V) — **Header:** `mst.hpp`

### [Prim's Algorithm](algorithms/mst.md#prims-algorithm)

Adjacency-list-based MST using a priority queue. Grows the MST from a seed vertex,
filling predecessor and weight arrays. Returns total MST weight.

**Time:** O(E log V) — **Space:** O(V) — **Header:** `mst.hpp`

---

## Graph Analytics

### [Triangle Count](algorithms/triangle_count.md)

Counts 3-cliques via merge-based sorted-list intersection. Requires adjacency lists
sorted by target ID (`ordered_vertex_edges` concept). Works with sorted-edge
containers (`vos`, `dos`) and `undirected_adjacency_list`.

**Time:** O(m^{3/2}) — **Space:** O(1) — **Header:** `tc.hpp`

### [Maximal Independent Set](algorithms/mis.md)

Greedy MIS — finds a maximal set of non-adjacent vertices starting from a seed.
Result is seed-dependent (different seeds may yield different-sized sets). Self-loops
exclude a vertex from the MIS.

**Time:** O(V+E) — **Space:** O(V) — **Header:** `mis.hpp`

### [Jaccard Coefficient](algorithms/jaccard.md)

Computes pairwise neighbor-set similarity $J(u,v) = |N(u) \cap N(v)| / |N(u) \cup N(v)|$
for every directed edge. Callback-driven — invokes a user-provided callable for each
edge with its coefficient in [0.0, 1.0].

**Time:** O(V + E·d) — **Space:** O(V+E) — **Header:** `jaccard.hpp`

### [Label Propagation](algorithms/label_propagation.md)

Community detection via iterative majority-vote label propagation. Each vertex adopts
the most frequent label among its neighbors until convergence. Supports partial
labeling via an empty-label sentinel. Tie-breaking is random.

**Time:** O(E) per iteration — **Space:** O(V) — **Header:** `label_propagation.hpp`

---

## Common Infrastructure

All shortest-path algorithms share utilities from `traversal_common.hpp`:

| Utility | Purpose |
|---------|---------|
| `init_shortest_paths(distances)` | Set all distances to infinity |
| `init_shortest_paths(distances, predecessors)` | Set distances to infinity, predecessors to self |
| `shortest_path_infinite_distance<T>()` | Returns the "infinity" sentinel for type `T` |
| `shortest_path_zero<T>()` | Returns the additive identity for type `T` |

### Visitors

Algorithms accept an optional visitor struct with callback methods. Only the
callbacks you define are called — unimplemented callbacks are silently skipped.

**Shortest-path visitor events:**

| Event | Called when |
|-------|------------|
| `on_initialize_vertex(g, u)` | Before traversal starts, for each vertex |
| `on_discover_vertex(g, u)` | Vertex first reached |
| `on_examine_vertex(g, u)` | Vertex popped from queue |
| `on_examine_edge(g, uv)` | Edge examined |
| `on_edge_relaxed(g, uv)` | Edge improved a shorter path |
| `on_edge_not_relaxed(g, uv)` | Edge did not improve path |
| `on_edge_minimized(g, uv)` | Edge achieved final minimum (Bellman-Ford) |
| `on_edge_not_minimized(g, uv)` | Edge not at final minimum (Bellman-Ford) |

**DFS-specific visitor events:**

| Event | Called when |
|-------|------------|
| `on_start_vertex(g, u)` | DFS root vertex |
| `on_tree_edge(g, uv)` | Edge to unvisited vertex (White → Gray) |
| `on_back_edge(g, uv)` | Edge to ancestor (Gray vertex) |
| `on_forward_or_cross_edge(g, uv)` | Edge to already-finished vertex (Black) |
| `on_finish_edge(g, uv)` | All descendants of edge target explored |
| `on_finish_vertex(g, u)` | All adjacent edges explored |

Each callback also has an `_id` variant that receives vertex/edge IDs instead of
descriptors.

---

## Roadmap

The following algorithms are **planned but not yet implemented**:

- A* search
- Bidirectional Dijkstra
- Johnson's all-pairs shortest paths
- Floyd-Warshall all-pairs shortest paths
- Maximum flow (push-relabel, Dinic's)
- Minimum cut
- Graph coloring
- Betweenness centrality
- PageRank

---

## See Also

- [Adjacency Lists User Guide](adjacency-lists.md) — concepts and CPOs used by algorithms
- [Views User Guide](views.md) — lazy search views (DFS, BFS, topological sort)
- [Containers User Guide](containers.md) — graph container options
- [Getting Started](../getting-started.md) — quick-start examples
