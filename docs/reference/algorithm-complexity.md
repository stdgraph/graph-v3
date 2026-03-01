<table><tr>
<td><img src="../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Algorithm Complexity Cheat Sheet


> Quick-reference for time complexity, space complexity, required concepts, and
headers for every algorithm in graph-v3.

</td>
</tr></table>

> [← Back to Documentation Index](../index.md) · [User Guide: Algorithms](../user-guide/algorithms.md)

---

## Shortest Paths

| Algorithm | Functions | Time | Space | Required Concepts | Header |
|-----------|----------|------|-------|-------------------|--------|
| **Dijkstra** | `dijkstra_shortest_paths` | O((V+E) log V) | O(V) | `index_adjacency_list`, `edge_weight_function` | `dijkstra_shortest_paths.hpp` |
| | `dijkstra_shortest_distances` | O((V+E) log V) | O(V) | same | same |
| **Bellman-Ford** | `bellman_ford_shortest_paths` | O(V·E) | O(1) aux | `index_adjacency_list`, `edge_weight_function` | `bellman_ford_shortest_paths.hpp` |
| | `bellman_ford_shortest_distances` | O(V·E) | O(1) aux | same | same |
| | `find_negative_cycle` | O(V) | O(1) aux | `index_adjacency_list` | same |

## Traversal

| Algorithm | Functions | Time | Space | Required Concepts | Header |
|-----------|----------|------|-------|-------------------|--------|
| **BFS** | `breadth_first_search` | O(V+E) | O(V) | `index_adjacency_list` | `breadth_first_search.hpp` |
| **DFS** | `depth_first_search` | O(V+E) | O(V) | `index_adjacency_list` | `depth_first_search.hpp` |
| **Topological Sort** | `topological_sort` | O(V+E) | O(V) | `index_adjacency_list` | `topological_sort.hpp` |

## Minimum Spanning Tree

| Algorithm | Functions | Time | Space | Required Concepts | Header |
|-----------|----------|------|-------|-------------------|--------|
| **Kruskal** | `kruskal` | O(E log E) | O(E+V) | `basic_sourced_index_edgelist` | `mst.hpp` |
| | `inplace_kruskal` | O(E log E) | O(V) | same + sortable | same |
| **Prim** | `prim` | O(E log V) | O(V) | `index_adjacency_list`, `edge_weight_function` | `mst.hpp` |

## Connected Components

| Algorithm | Functions | Time | Space | Required Concepts | Header |
|-----------|----------|------|-------|-------------------|--------|
| **Connected Components** | `connected_components` | O(V+E) | O(V) | `index_adjacency_list` | `connected_components.hpp` |
| **Kosaraju (SCC)** | `kosaraju` | O(V+E) | O(V) | `index_adjacency_list` (graph + transpose) | `connected_components.hpp` |
| **Afforest** | `afforest` | O(V + E·α(V)) | O(V) | `index_adjacency_list` | `connected_components.hpp` |
| **Biconnected Components** | `biconnected_components` | O(V+E) | O(V+E) | `index_adjacency_list` | `biconnected_components.hpp` |

## Structural

| Algorithm | Functions | Time | Space | Required Concepts | Header |
|-----------|----------|------|-------|-------------------|--------|
| **Articulation Points** | `articulation_points` | O(V+E) | O(V) | `index_adjacency_list` | `articulation_points.hpp` |

## Similarity & Community

| Algorithm | Functions | Time | Space | Required Concepts | Header |
|-----------|----------|------|-------|-------------------|--------|
| **Jaccard Coefficient** | `jaccard_coefficient` | O(V + E·d_min) typical | O(V+E) | `index_adjacency_list` | `jaccard.hpp` |
| **Label Propagation** | `label_propagation` | O(E) per iteration | O(V) | `index_adjacency_list` | `label_propagation.hpp` |

---

## Notes

- **V** = number of vertices, **E** = number of edges, **d_min** = minimum
  degree of two endpoints
- **α(V)** is the inverse Ackermann function (effectively constant for all
  practical inputs)
- "O(1) aux" means the algorithm needs no auxiliary storage beyond the
  caller-provided output arrays (distances, predecessors, etc.)
- All algorithms accept an optional **visitor** for event callbacks. Visitors
  do not change complexity.
- Dijkstra uses a binary min-heap; for Fibonacci heaps, time would be
  O(V log V + E) amortized.

---

## All Headers

All algorithm headers live under `include/graph/algorithm/`.

```
algorithm/
├── articulation_points.hpp
├── bellman_ford_shortest_paths.hpp
├── biconnected_components.hpp
├── breadth_first_search.hpp
├── connected_components.hpp
├── depth_first_search.hpp
├── dijkstra_shortest_paths.hpp
├── jaccard.hpp
├── label_propagation.hpp
├── mst.hpp
├── topological_sort.hpp
└── traversal_common.hpp        (shared types: visitors, init helpers)
```

---

## See Also

- [User Guide: Algorithms](../user-guide/algorithms.md) — overview with examples
- [Concepts Reference](concepts.md) — concept definitions
- [CPO Reference](cpo-reference.md) — CPO signatures
