# Algorithms

> [← Back to Documentation Index](../index.md)

- [Overview](#overview)
- [Algorithm Catalog](#algorithm-catalog)
- [Shortest Paths](#shortest-paths) — Dijkstra, Bellman-Ford
- [Traversal](#traversal) — BFS, DFS, topological sort
- [Components](#components) — connected, biconnected, articulation points
- [Minimum Spanning Tree](#minimum-spanning-tree) — Kruskal, Prim
- [Graph Analytics](#graph-analytics) — triangle count, MIS, Jaccard, label propagation
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

| Algorithm | Header | Brief description | Complexity |
|-----------|--------|-------------------|------------|
| [Dijkstra](#dijkstra-shortest-paths) | `dijkstra_shortest_paths.hpp` | Single/multi-source shortest paths (non-negative weights) | O((V+E) log V) |
| [Bellman-Ford](#bellman-ford-shortest-paths) | `bellman_ford_shortest_paths.hpp` | Shortest paths with negative weights; cycle detection | O(V·E) |
| [BFS](#breadth-first-search) | `breadth_first_search.hpp` | Level-order traversal from source(s) | O(V+E) |
| [DFS](#depth-first-search) | `depth_first_search.hpp` | Depth-first traversal with edge classification | O(V+E) |
| [Topological Sort](#topological-sort) | `topological_sort.hpp` | Linear ordering of DAG vertices | O(V+E) |
| [Connected Components](#connected-components) | `connected_components.hpp` | Undirected CC, directed SCC (Kosaraju), union-find (afforest) | O(V+E) |
| [Biconnected Components](#biconnected-components) | `biconnected_components.hpp` | Maximal 2-connected subgraphs (Hopcroft-Tarjan) | O(V+E) |
| [Articulation Points](#articulation-points) | `articulation_points.hpp` | Cut vertices whose removal disconnects the graph | O(V+E) |
| [Kruskal MST](#kruskals-algorithm) | `mst.hpp` | Edge-list-based MST via union-find | O(E log E) |
| [Prim MST](#prims-algorithm) | `mst.hpp` | Adjacency-list-based MST via priority queue | O(E log V) |
| [Triangle Count](#triangle-count) | `tc.hpp` | Count 3-cliques via sorted-list intersection | O(m^{3/2}) |
| [Maximal Independent Set](#maximal-independent-set) | `mis.hpp` | Greedy MIS (non-adjacent vertex set) | O(V+E) |
| [Jaccard Coefficient](#jaccard-coefficient) | `jaccard.hpp` | Pairwise neighbor-set similarity per edge | O(V + E·d) |
| [Label Propagation](#label-propagation) | `label_propagation.hpp` | Community detection via majority-vote labels | O(E) per iter |

All headers are under `include/graph/algorithm/`.

---

## Shortest Paths

### Dijkstra Shortest Paths

Finds single-source or multi-source shortest paths in a graph with non-negative
edge weights using a binary-heap priority queue.

```cpp
#include <graph/algorithm/dijkstra_shortest_paths.hpp>
```

**Signatures:**

```cpp
// Shortest paths (distances + predecessors)
void dijkstra_shortest_paths(G&& g, Sources sources, Distances& distances,
    Predecessors& predecessors, WF weight = /*default 1*/, Visitor vis = {},
    Compare compare = less<>{}, Combine combine = plus<>{});

void dijkstra_shortest_paths(G&& g, vertex_id_t<G> source, Distances& distances,
    Predecessors& predecessors, WF weight = /*default 1*/, Visitor vis = {},
    Compare compare = less<>{}, Combine combine = plus<>{});

// Shortest distances only (no predecessors)
void dijkstra_shortest_distances(G&& g, Sources sources, Distances& distances,
    WF weight = /*default 1*/, Visitor vis = {},
    Compare compare = less<>{}, Combine combine = plus<>{});

void dijkstra_shortest_distances(G&& g, vertex_id_t<G> source,
    Distances& distances, WF weight = /*default 1*/, Visitor vis = {},
    Compare compare = less<>{}, Combine combine = plus<>{});
```

**Usage:**

```cpp
auto g = /* weighted graph */;
std::vector<double>              dist(num_vertices(g));
std::vector<vertex_id_t<decltype(g)>> pred(num_vertices(g));

init_shortest_paths(dist, pred);

dijkstra_shortest_paths(g, 0, dist, pred,
    [](const auto& g, const auto& uv) { return edge_value(g, uv); });

// dist[v] = shortest distance from source 0 to v
// pred[v] = predecessor of v on the shortest path
```

**Complexity:** O((V+E) log V) time, O(V) space.

See [test_dijkstra_shortest_paths.cpp](../../tests/algorithms/test_dijkstra_shortest_paths.cpp) for more examples.

---

### Bellman-Ford Shortest Paths

Finds single/multi-source shortest paths supporting negative edge weights and
detecting negative weight cycles.

```cpp
#include <graph/algorithm/bellman_ford_shortest_paths.hpp>
```

**Signatures:**

```cpp
// Returns empty optional if no negative cycle; vertex ID in cycle otherwise
[[nodiscard]] optional<vertex_id_t<G>>
bellman_ford_shortest_paths(G&& g, Sources sources, Distances& distances,
    Predecessors& predecessors, WF weight = /*default 1*/, Visitor vis = {},
    Compare compare = less<>{}, Combine combine = plus<>{});

[[nodiscard]] optional<vertex_id_t<G>>
bellman_ford_shortest_paths(G&& g, vertex_id_t<G> source, Distances& distances,
    Predecessors& predecessors, WF weight, Visitor vis = {},
    Compare compare = less<>{}, Combine combine = plus<>{});

// Distances only
[[nodiscard]] optional<vertex_id_t<G>>
bellman_ford_shortest_distances(G&& g, Sources sources, Distances& distances,
    WF weight, Visitor vis = {}, Compare compare = less<>{}, Combine combine = plus<>{});

[[nodiscard]] optional<vertex_id_t<G>>
bellman_ford_shortest_distances(G&& g, vertex_id_t<G> source, Distances& distances,
    WF weight, Visitor vis = {}, Compare compare = less<>{}, Combine combine = plus<>{});

// Helper: extract the negative cycle
void find_negative_cycle(G&& g, Predecessors& pred, vertex_id_t<G> cycle_vid,
    OutputIterator out_cycle);
```

**Usage:**

```cpp
auto g = /* graph with potentially negative weights */;
std::vector<int>    dist(num_vertices(g));
std::vector<size_t> pred(num_vertices(g));

init_shortest_paths(dist, pred);

auto cycle = bellman_ford_shortest_paths(g, 0, dist, pred,
    [](const auto& g, const auto& uv) { return edge_value(g, uv); });

if (cycle) {
    // Negative cycle detected — extract it
    std::vector<size_t> cycle_verts;
    find_negative_cycle(g, pred, *cycle, std::back_inserter(cycle_verts));
} else {
    // dist[v] = shortest distance from source 0 to v
}
```

**Complexity:** O(V·E) time, O(1) auxiliary space.

See [test_bellman_ford_shortest_paths.cpp](../../tests/algorithms/test_bellman_ford_shortest_paths.cpp) for more examples.

---

## Traversal

### Breadth-First Search

Explores vertices in level-order (FIFO) from one or more sources. Output is
entirely visitor-driven.

```cpp
#include <graph/algorithm/breadth_first_search.hpp>
```

**Signatures:**

```cpp
void breadth_first_search(G&& g, Sources sources, Visitor vis = {});
void breadth_first_search(G&& g, vertex_id_t<G> source, Visitor vis = {});
```

**Visitor events:** `on_initialize_vertex`, `on_discover_vertex`, `on_examine_vertex`,
`on_examine_edge`, `on_finish_vertex`.

**Usage:**

```cpp
struct DistanceRecorder {
    std::vector<int>& dist;
    template <class G, class VDesc>
    void on_discover_vertex(G& g, VDesc u) {
        // record distance via predecessors or other state
    }
};

std::vector<int> dist(num_vertices(g), -1);
dist[0] = 0;
breadth_first_search(g, 0, DistanceRecorder{dist});
```

**Complexity:** O(V+E) time, O(V) space.

See [test_breadth_first_search.cpp](../../tests/algorithms/test_breadth_first_search.cpp) for more examples.

---

### Depth-First Search

Performs iterative DFS with three-color marking (White/Gray/Black), enabling
precise edge classification into tree, back, and forward/cross edges.

```cpp
#include <graph/algorithm/depth_first_search.hpp>
```

**Signature:**

```cpp
void depth_first_search(G&& g, vertex_id_t<G> source, Visitor vis = {});
```

**Visitor events:** `on_initialize_vertex`, `on_start_vertex`, `on_discover_vertex`,
`on_examine_edge`, `on_tree_edge`, `on_back_edge`, `on_forward_or_cross_edge`,
`on_finish_edge`, `on_finish_vertex`.

**Usage:**

```cpp
struct TreeEdgeCollector {
    std::vector<std::pair<size_t, size_t>>& tree_edges;
    template <class G, class EDesc>
    void on_tree_edge(G& g, EDesc uv) {
        tree_edges.push_back({source_id(g, uv), target_id(g, uv)});
    }
};

std::vector<std::pair<size_t, size_t>> tree;
depth_first_search(g, 0, TreeEdgeCollector{tree});
```

**Complexity:** O(V+E) time, O(V) space.

See [test_depth_first_search.cpp](../../tests/algorithms/test_depth_first_search.cpp) for more examples.

---

### Topological Sort

Produces a linear ordering of vertices in a DAG such that for every directed
edge (u,v), u appears before v.

```cpp
#include <graph/algorithm/topological_sort.hpp>
```

**Signatures:**

```cpp
bool topological_sort(G&& g, OutputIterator result);                       // full graph
bool topological_sort(G&& g, vertex_id_t<G> source, OutputIterator result); // single-source
bool topological_sort(G&& g, Sources sources, OutputIterator result);       // multi-source
```

**Returns** `true` if the graph is a DAG (valid ordering produced), `false` if a
cycle is detected.

**Usage:**

```cpp
std::vector<size_t> order;
bool is_dag = topological_sort(g, std::back_inserter(order));

if (is_dag) {
    // order contains vertex IDs in topological order
} else {
    // graph has a cycle
}
```

**Complexity:** O(V+E) time, O(V) space.

See [test_topological_sort.cpp](../../tests/algorithms/test_topological_sort.cpp) for more examples.

---

## Components

### Connected Components

Three algorithms for finding connected components.

```cpp
#include <graph/algorithm/connected_components.hpp>
```

#### `connected_components` — undirected

```cpp
size_t connected_components(G&& g, Component& component);
```

Fills `component[v]` with the component ID for each vertex. Returns the number
of connected components.

#### `kosaraju` — strongly connected components (directed)

```cpp
void kosaraju(G&& g, GT&& g_transpose, Component& component);
```

Requires both the original graph and its transpose. Fills `component[v]`
with the SCC ID. Uses two DFS passes.

#### `afforest` — union-find with neighbor sampling

```cpp
void afforest(G&& g, Component& component, size_t neighbor_rounds = 2);
void afforest(G&& g, GT&& g_transpose, Component& component, size_t neighbor_rounds = 2);
```

Parallel-friendly design. Call `compress(component)` for canonical component IDs.

**Usage:**

```cpp
std::vector<size_t> comp(num_vertices(g));
size_t num_cc = connected_components(g, comp);
// comp[v] = component ID for vertex v
```

**Complexity:** O(V+E) time, O(V) space.

See [test_connected_components.cpp](../../tests/algorithms/test_connected_components.cpp) for more examples.

---

### Biconnected Components

Finds all biconnected components (maximal 2-connected subgraphs) using the
iterative Hopcroft-Tarjan algorithm.

```cpp
#include <graph/algorithm/biconnected_components.hpp>
```

**Signature:**

```cpp
void biconnected_components(G&& g, OuterContainer& components);
```

**Usage:**

```cpp
std::vector<std::vector<size_t>> components;
biconnected_components(g, components);
// Each inner vector is a biconnected component (set of vertex IDs)
// Articulation points appear in multiple components
```

**Complexity:** O(V+E) time, O(V+E) space.

**Precondition:** For undirected graphs, both directions of each edge must be stored.

See [test_biconnected_components.cpp](../../tests/algorithms/test_biconnected_components.cpp) for more examples.

---

### Articulation Points

Finds all cut vertices whose removal disconnects the graph, using the iterative
Hopcroft-Tarjan algorithm with discovery times and low-link values.

```cpp
#include <graph/algorithm/articulation_points.hpp>
```

**Signature:**

```cpp
void articulation_points(G&& g, OutputIterator cut_vertices);
```

**Usage:**

```cpp
std::vector<size_t> cuts;
articulation_points(g, std::back_inserter(cuts));
// cuts contains vertex IDs of articulation points (each emitted once)
```

**Complexity:** O(V+E) time, O(V) space.

**Precondition:** For undirected graphs, both directions of each edge must be stored.

See [test_articulation_points.cpp](../../tests/algorithms/test_articulation_points.cpp) for more examples.

---

## Minimum Spanning Tree

```cpp
#include <graph/algorithm/mst.hpp>
```

### Kruskal's Algorithm

Edge-list-based MST using union-find. Four overloads:

```cpp
auto kruskal(EdgeList& edges, OutputIterator tree);
auto kruskal(EdgeList& edges, OutputIterator tree, Compare compare);
auto inplace_kruskal(EdgeList& edges, OutputIterator tree);              // sorts input in-place
auto inplace_kruskal(EdgeList& edges, OutputIterator tree, Compare compare);
```

**Returns** `pair<EV, size_t>` — total MST weight and number of components.

**Usage:**

```cpp
std::vector<simple_edge<size_t, double>> edges = {
    {0, 1, 4.0}, {0, 2, 2.0}, {1, 2, 1.0}, {1, 3, 5.0}, {2, 3, 8.0}
};
std::vector<simple_edge<size_t, double>> tree;

auto [total_weight, num_components] = kruskal(edges, std::back_inserter(tree));
```

**Complexity:** O(E log E) time, O(E+V) space (O(V) for `inplace_kruskal`).

### Prim's Algorithm

Adjacency-list-based MST using a priority queue.

```cpp
auto prim(G&& g, Predecessors& pred, Weights& weight, vertex_id_t<G> seed = 0);
auto prim(G&& g, Predecessors& pred, Weights& weight, Compare compare,
          range_value_t<Weights> init_dist, WF weight_fn, vertex_id_t<G> seed = 0);
```

**Returns** total MST weight. Fills `pred[v]` with the parent vertex in the MST
and `weight[v]` with the edge weight from the parent.

**Usage:**

```cpp
std::vector<size_t> pred(num_vertices(g));
std::vector<double> weight(num_vertices(g));

auto total = prim(g, pred, weight);
```

**Complexity:** O(E log V) time, O(V) space.

See [test_mst.cpp](../../tests/algorithms/test_mst.cpp) for more examples.

---

## Graph Analytics

### Triangle Count

Counts the number of triangles (3-cliques) in an undirected graph using
merge-based sorted-list intersection.

```cpp
#include <graph/algorithm/tc.hpp>
```

**Signature:**

```cpp
size_t triangle_count(G&& g);
```

**Returns** the total number of triangles.

**Requirement:** adjacency lists must be **sorted by target ID** (e.g., use `vos`,
`dos`, or pre-sorted `vov` traits).

**Complexity:** O(m^{3/2}) average for sparse graphs, O(1) auxiliary space.

See [test_triangle_count.cpp](../../tests/algorithms/test_triangle_count.cpp) for more examples.

---

### Maximal Independent Set

Finds a maximal independent set (greedy) — a set of non-adjacent vertices that
cannot be extended further.

```cpp
#include <graph/algorithm/mis.hpp>
```

**Signature:**

```cpp
size_t maximal_independent_set(G&& g, OutputIterator mis, vertex_id_t<G> seed = 0);
```

**Returns** the number of vertices in the MIS. Writes selected vertex IDs to the
output iterator.

> **Note:** This is a *maximal* (not maximum) independent set. The result is
> greedy and order-dependent — different seeds may yield different results.

**Complexity:** O(V+E) time, O(V) space.

See [test_mis.cpp](../../tests/algorithms/test_mis.cpp) for more examples.

---

### Jaccard Coefficient

Computes the Jaccard similarity coefficient
$J(u,v) = |N(u) \cap N(v)| / |N(u) \cup N(v)|$ for every directed edge in the
graph.

```cpp
#include <graph/algorithm/jaccard.hpp>
```

**Signature:**

```cpp
void jaccard_coefficient(G&& g, OutOp out);
```

Invokes `out(uid, vid, uv, coefficient)` for every directed edge with the
coefficient in [0.0, 1.0]. Self-loops are skipped.

**Usage:**

```cpp
std::vector<std::tuple<size_t, size_t, double>> results;
jaccard_coefficient(g, [&](auto uid, auto vid, auto& uv, double j) {
    results.emplace_back(uid, vid, j);
});
```

**Complexity:** O(V + E·d_min) time, O(V+E) space (precomputed neighbor sets).

See [test_jaccard.cpp](../../tests/algorithms/test_jaccard.cpp) for more examples.

---

### Label Propagation

Community detection via iterative majority-vote label propagation: each vertex
adopts the most frequent label among its neighbors until convergence.

```cpp
#include <graph/algorithm/label_propagation.hpp>
```

**Signatures:**

```cpp
void label_propagation(G&& g, Label& label, Gen&& rng = /*default*/, T max_iters = max);
void label_propagation(G&& g, Label& label, range_value_t<Label> empty_label,
    Gen&& rng = /*default*/, T max_iters = max);
```

The second overload accepts an `empty_label` sentinel — unlabelled vertices don't
vote and aren't counted. Ties are broken randomly via the supplied RNG. Vertex
processing order is shuffled each iteration.

**Usage:**

```cpp
std::vector<size_t> label(num_vertices(g));
std::iota(label.begin(), label.end(), 0);  // each vertex starts as its own community

label_propagation(g, label);
// label[v] = community ID for vertex v
```

**Complexity:** O(E) per iteration, O(V) auxiliary space. Typically converges in a
small number of iterations.

See [test_label_propagation.cpp](../../tests/algorithms/test_label_propagation.cpp) for more examples.

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
