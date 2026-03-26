<table><tr>
<td><img src="../../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Connected Components

</td>
</tr></table>

> [← Back to Algorithm Catalog](../algorithms.md)

## Table of Contents
- [Overview](#overview)
- [When to Use](#when-to-use)
- [Include](#include)
- [Algorithms](#algorithms)
  - [`connected_components` — undirected](#connected_components--undirected)
  - [`kosaraju` — strongly connected components](#kosaraju--strongly-connected-components)
  - [`afforest` — union-find with neighbor sampling](#afforest--union-find-with-neighbor-sampling)
- [Parameters](#parameters)
- [Supported Graph Properties](#supported-graph-properties)
- [Examples](#examples)
  - [Undirected Connected Components](#example-1-undirected-connected-components)
  - [Strongly Connected Components (Kosaraju)](#example-2-strongly-connected-components-kosaraju)
  - [Union-Find Components (Afforest)](#example-3-union-find-components-afforest)
  - [Undirected Adjacency List](#example-4-undirected-adjacency-list)
  - [Counting Components and Sizes](#example-5-counting-components-and-sizes)
  - [Compressing Afforest Component IDs](#example-6-compressing-afforest-component-ids)
- [Mandates](#mandates)
- [Preconditions](#preconditions)
- [Effects](#effects)
- [Returns](#returns)
- [Throws](#throws)
- [Complexity](#complexity)
- [See Also](#see-also)

## Overview

The connected components header provides three algorithms for partitioning a
graph into connected components. All three require a graph satisfying
`adjacency_list<G>` — both index-based (contiguous integer-indexed) and
map-based (sparse vertex ID) graphs are supported.

| Algorithm | Use case | Approach |
|-----------|----------|----------|
| `connected_components` | Undirected graphs | DFS-based |
| `kosaraju` | Directed graphs (SCC) | Two DFS passes (requires transpose) |
| `afforest` | Large graphs, parallel-friendly | Union-find with neighbor sampling |

All three fill a `component` array where `component[v]` is the component ID for
vertex v.

## When to Use

- **`connected_components`** — simple and efficient for undirected graphs. Returns
  the number of components directly.
- **`kosaraju`** — when you need strongly connected components of a directed
  graph. Requires constructing the transpose graph (all edges reversed).
- **`afforest`** — when working with large graphs or when you intend to
  parallelize later. Uses union-find with neighbor sampling, which has good
  cache behavior on large inputs.

**Not suitable when:**

- You need biconnected components or articulation points → use
  [biconnected_components](biconnected_components.md) or
  [articulation_points](articulation_points.md).

## Include

```cpp
#include <graph/algorithm/connected_components.hpp>
```

## Algorithms

### `connected_components` — undirected

```cpp
size_t connected_components(G&& g, ComponentFn&& component);
```

DFS-based algorithm for undirected graphs. Returns the number of connected
components. `component(g, uid)` is called with each vertex ID to assign its
component ID (0-based).

### `kosaraju` — strongly connected components

```cpp
void kosaraju(G&& g, GT&& g_transpose, ComponentFn&& component);

// Bidirectional overload — no transpose graph needed
void kosaraju(G&& g, ComponentFn&& component);
```

Kosaraju's two-pass DFS algorithm for directed graphs. Fills
`component(g, uid)` with the SCC ID for each vertex.

### `afforest` — union-find with neighbor sampling

```cpp
void afforest(G&& g, Component& component, size_t neighbor_rounds = 2);

void afforest(G&& g, GT&& g_transpose, Component& component,
    size_t neighbor_rounds = 2);
```

Union-find-based algorithm with neighbor sampling for large or parallel-friendly
workloads. The `neighbor_rounds` parameter controls how many initial sampling
rounds are performed before falling back to full edge iteration. Call
`compress(component)` afterwards for canonical (root) component IDs.

> **Note:** `afforest` retains a container-based `Component&` interface (not the
> function-based API) because its internal union-find helpers (`link`, `compress`,
> `sample_frequent_element`) require direct subscript access to the component array.

## Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `adjacency_list` |
| `g_transpose` | Transpose graph (for `kosaraju` and `afforest` with transpose). Must satisfy `adjacency_list`. |
| `component` | For `connected_components` and `kosaraju`: callable `(const G&, vertex_id_t<G>) -> ComponentID&` returning a mutable reference. For containers: wrap with `container_value_fn(comp)`. Must satisfy `vertex_property_fn_for<ComponentFn, G>`. For `afforest`: a subscriptable container (`vector` or `unordered_map`), still using the container API. |
| `neighbor_rounds` | Number of neighbor-sampling rounds for `afforest` (default: 2) |

**Return value (`connected_components` only):** `size_t` — number of connected
components. `kosaraju` and `afforest` return `void`.

## Supported Graph Properties

**Directedness:**
- ✅ Undirected graphs (`connected_components`, `afforest` without transpose)
- ✅ Directed graphs (`kosaraju`, `afforest` with transpose)

**Edge Properties:**
- ✅ Unweighted edges
- ✅ Weighted edges (weights ignored)
- ✅ Multi-edges (do not affect component assignment)
- ✅ Self-loops (do not affect component assignment)

**Graph Structure:**
- ✅ Connected graphs (single component)
- ✅ Disconnected graphs (each component labeled independently)
- ✅ Empty graphs (zero components)
- ✅ Isolated vertices (each is its own component)

**Container Requirements:**
- Required: `adjacency_list<G>`
- `component` (`connected_components` and `kosaraju`) must satisfy `vertex_property_fn_for<ComponentFn, G>`
- `component` (`afforest`) must satisfy `vertex_property_map_for<Component, G>`
- `kosaraju`: transpose graph must also satisfy `adjacency_list`

## Examples

### Example 1: Undirected Connected Components

Find connected components in an undirected graph simulated with bidirectional
edges.

```cpp
#include <graph/algorithm/connected_components.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <vector>

using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
    container::vov_graph_traits<void>>;

// Undirected graph (both directions stored):
// Component 0: {0, 1, 2}
// Component 1: {3, 4}
Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {3, 4}, {4, 3}});

std::vector<uint32_t> comp(num_vertices(g));
size_t num_cc = connected_components(g, container_value_fn(comp));

// num_cc == 2
// comp[0] == comp[1] == comp[2]  (same component)
// comp[3] == comp[4]             (same component)
```

### Example 2: Strongly Connected Components (Kosaraju)

Find SCCs in a directed graph using Kosaraju's algorithm.

```cpp
// Directed graph with 2 SCCs:
//   SCC 0: {0, 1, 2} (cycle: 0→1→2→0)
//   SCC 1: {3}
Graph g({{0, 1}, {1, 2}, {2, 0}, {2, 3}});

// Transpose: reverse all edges
Graph g_t({{1, 0}, {2, 1}, {0, 2}, {3, 2}});

std::vector<uint32_t> comp(num_vertices(g));
kosaraju(g, g_t, container_value_fn(comp));

// comp[0] == comp[1] == comp[2]  (cycle forms an SCC)
// comp[3] != comp[0]             (3 is its own SCC — not on any cycle)
```

### Example 3: Union-Find Components (Afforest)

Afforest is suitable for large graphs and parallel-friendly workloads.

```cpp
std::vector<uint32_t> comp(num_vertices(g));
afforest(g, comp, 2);  // 2 neighbor-sampling rounds (default)

// comp[v] gives a component representative — vertices in the same
// component have the same value. Call compress() for canonical root IDs.
```

### Example 4: Undirected Adjacency List

With `undirected_adjacency_list`, each logical edge is stored once — no need for
bidirectional edge pairs.

```cpp
#include <graph/container/undirected_adjacency_list.hpp>

undirected_adjacency_list<int, int> g({{0, 1, 1}, {1, 2, 1}, {3, 4, 1}});

std::vector<uint32_t> comp(num_vertices(g));
size_t num_cc = connected_components(g, container_value_fn(comp));

// num_cc == 2
// Same results as vov with bidirectional edges, but simpler construction
```

### Example 5: Counting Components and Sizes

After computing components, gather per-component statistics.

```cpp
// After running connected_components(g, comp) ...

size_t n = num_vertices(g);
std::map<uint32_t, size_t> component_sizes;
for (size_t v = 0; v < n; ++v) {
    ++component_sizes[comp[v]];
}

// Find the largest component
auto largest = std::max_element(component_sizes.begin(), component_sizes.end(),
    [](const auto& a, const auto& b) { return a.second < b.second; });

std::cout << "Largest component has " << largest->second << " vertices\n";

// List vertices in each component
for (const auto& [id, size] : component_sizes) {
    std::cout << "Component " << id << ": " << size << " vertices\n";
}
```

### Example 6: Compressing Afforest Component IDs

After `afforest`, component IDs may use intermediate representatives rather than
root IDs. Call `compress()` to canonicalize them.

```cpp
std::vector<uint32_t> comp(num_vertices(g));
afforest(g, comp);

// Before compress: comp values are valid for equality testing
// (comp[u] == comp[v] iff same component) but may not be root IDs

compress(comp);

// After compress: comp[v] is the canonical root representative
// for vertex v's component. Still valid for equality testing
// and now consistent across all vertices.
```

## Mandates

- `G` must satisfy `adjacency_list<G>`
- `connected_components` and `kosaraju`: `ComponentFn` must satisfy `vertex_property_fn_for<ComponentFn, G>`
- `afforest`: `Component` must satisfy `vertex_property_map_for<Component, G>`
- For `kosaraju`: `GT` must satisfy `adjacency_list<GT>`

## Preconditions

- `component(g, uid)` must be valid for all vertex IDs in `g` (`connected_components`, `kosaraju`)
- `component` must be pre-sized for all vertex IDs in `g` (`afforest`)
- For `connected_components`: undirected graphs must store both directions of
  each edge (or use `undirected_adjacency_list`)
- For `kosaraju`: the transpose graph must contain all edges reversed

## Effects

- Writes component IDs via `component(g, uid)` for all vertices (`connected_components`, `kosaraju`)
- Writes component IDs to `component[uid]` for all vertices (`afforest`)
- Does not modify the graph `g` (or `g_transpose`)
- For `afforest`: call `compress(component)` afterwards for canonical root IDs

## Returns

- `connected_components` returns `size_t` — the number of connected components
- `kosaraju` and `afforest` return `void`

## Throws

- `std::bad_alloc` if internal allocations fail
- Exception guarantee: Basic. Graph `g` remains unchanged; component output may be partial.

## Complexity

| Algorithm | Time | Space |
|-----------|------|-------|
| `connected_components` | O(V + E) | O(V) |
| `kosaraju` | O(V + E) | O(V) |
| `afforest` | O(V + E) | O(V) |

## See Also

- [Biconnected Components](biconnected_components.md) — maximal 2-connected subgraphs
- [Articulation Points](articulation_points.md) — cut vertices
- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_connected_components.cpp](../../../tests/algorithms/test_connected_components.cpp) — test suite
