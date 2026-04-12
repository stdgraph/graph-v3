<table><tr>
<td><img src="../../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Tarjan's Strongly Connected Components

</td>
</tr></table>

> [← Back to Algorithm Catalog](../algorithms.md)

## Table of Contents
- [Overview](#overview)
- [When to Use](#when-to-use)
- [Include](#include)
- [Algorithm](#algorithm)
- [Parameters](#parameters)
- [Supported Graph Properties](#supported-graph-properties)
- [Examples](#examples)
- [Mandates](#mandates)
- [Preconditions](#preconditions)
- [Effects](#effects)
- [Returns](#returns)
- [Throws](#throws)
- [Complexity](#complexity)
- [See Also](#see-also)

## Overview

Tarjan's algorithm finds all strongly connected components (SCCs) in a directed
graph using a single depth-first search. It uses discovery times and low-link
values to identify SCC roots, then pops completed SCCs from an auxiliary stack.

Unlike Kosaraju's algorithm, Tarjan's requires **no transpose graph** and performs
only **one DFS pass**, making it simpler to use when a transpose is unavailable.

| Property | Value |
|----------|-------|
| Passes | 1 (single DFS) |
| Transpose needed | No |
| SCC order | Reverse topological |
| Return value | Number of SCCs |

## When to Use

- **Use `tarjan_scc`** when you need SCCs and don't have (or don't want to
  construct) a transpose graph. Single-pass, no extra graph required.
- **Use `kosaraju`** when you already have a transpose or bidirectional graph,
  or need topological SCC ordering.
- **Use `connected_components`** for undirected graphs.

## Include

```cpp
#include <graph/algorithm/tarjan_scc.hpp>
```

## Algorithm

```cpp
size_t tarjan_scc(G&& g, ComponentFn&& component);
```

Single-pass iterative DFS using low-link values. Fills `component(g, uid)` with
the SCC ID for each vertex and returns the total number of SCCs.

## Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `adjacency_list` |
| `component` | Callable `(const G&, vertex_id_t<G>) -> ComponentID&` returning a mutable reference. For containers: wrap with `container_value_fn(comp)`. Must satisfy `vertex_property_fn_for<ComponentFn, G>`. |

**Return value:** `size_t` — number of strongly connected components.

## Supported Graph Properties

**Directedness:**
- ✅ Directed graphs (required)
- ❌ Undirected graphs (use `connected_components` instead)

**Edge Properties:**
- ✅ Weighted edges (weights ignored)
- ✅ Self-loops (handled correctly)
- ✅ Multi-edges (treated as single edge)
- ✅ Cycles

**Graph Structure:**
- ✅ Connected graphs
- ✅ Disconnected graphs (processes all components)
- ✅ Empty graphs (returns 0)

**Container Requirements:**
- Required: `adjacency_list<G>`
- `component` must satisfy `vertex_property_fn_for<ComponentFn, G>`

## Examples

### Example 1: Basic SCC Detection

```cpp
#include <graph/algorithm/tarjan_scc.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <vector>

using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
    container::vov_graph_traits<void>>;

// Directed graph: 0→1→2→0 (cycle = 1 SCC), 2→3 (singleton SCC)
Graph g({{0, 1}, {1, 2}, {2, 0}, {2, 3}});

std::vector<uint32_t> comp(num_vertices(g));
size_t num_scc = tarjan_scc(g, container_value_fn(comp));

// num_scc == 2
// comp[0] == comp[1] == comp[2]  (cycle forms one SCC)
// comp[3] != comp[0]             (3 is a singleton SCC)
```

### Example 2: Comparing with Kosaraju

```cpp
// Same graph, both algorithms
std::vector<uint32_t> comp_tarjan(num_vertices(g));
std::vector<uint32_t> comp_kosaraju(num_vertices(g));

size_t n = tarjan_scc(g, container_value_fn(comp_tarjan));
kosaraju(g, g_transpose, container_value_fn(comp_kosaraju));

// Both find the same SCCs (component IDs may differ, but groupings match)
```

## Mandates

- `G` must satisfy `adjacency_list<G>`
- `ComponentFn` must satisfy `vertex_property_fn_for<ComponentFn, G>`

## Preconditions

- `component(g, uid)` must be valid for all vertex IDs in `g`

## Effects

- Writes SCC IDs via `component(g, uid)` for all vertices
- Does not modify the graph `g`

## Returns

`size_t` — the number of strongly connected components.

## Throws

- `std::bad_alloc` if internal allocations fail
- Exception guarantee: Basic. Graph `g` remains unchanged; component output may be partial.

## Complexity

| Metric | Value |
|--------|-------|
| Time | O(V + E) |
| Space | O(V) |

## See Also

- [Connected Components](connected_components.md) — Kosaraju SCC, undirected CC, afforest
- [Articulation Points](articulation_points.md) — cut vertices (also uses Tarjan-style low-link)
- [Biconnected Components](biconnected_components.md) — maximal 2-connected subgraphs
- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_tarjan_scc.cpp](../../../tests/algorithms/test_tarjan_scc.cpp) — test suite
