# Label Propagation

> [← Back to Algorithm Catalog](../algorithms.md)

- [Overview](#overview)
- [Include](#include)
- [Signatures](#signatures)
- [Parameters](#parameters)
- [Examples](#examples)
  - [Basic Community Detection](#example-1-basic-community-detection)
  - [Majority-Vote Convergence](#example-2-majority-vote-convergence)
  - [Partial Labels with Empty Sentinel](#example-3-partial-labels-with-empty-sentinel)
  - [Controlling Iterations](#example-4-controlling-iterations)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [See Also](#see-also)

## Overview

Label propagation is a **community detection** algorithm. Each vertex starts
with a label (typically its own vertex ID), and in each iteration, every vertex
adopts the most frequent label among its neighbors (majority vote). The process
repeats until convergence or a maximum iteration count is reached.

The graph must satisfy `index_adjacency_list<G>` — vertices are stored in a
contiguous, integer-indexed random-access range.

Key behaviors:

- **Tie-breaking** is random, controlled by the caller-supplied RNG.
- **Vertex processing order** is shuffled each iteration to avoid bias.
- An optional **empty label sentinel** allows starting with partially-labeled
  graphs — unlabeled vertices don't vote and propagate nothing until they
  receive a label.

## Include

```cpp
#include <graph/algorithm/label_propagation.hpp>
```

## Signatures

```cpp
// Basic: all vertices start with labels
void label_propagation(G&& g, Label& label,
    Gen&& rng = /* default */,
    T max_iters = std::numeric_limits<T>::max());

// With empty-label sentinel: unlabeled vertices don't participate
void label_propagation(G&& g, Label& label,
    range_value_t<Label> empty_label,
    Gen&& rng = /* default */,
    T max_iters = std::numeric_limits<T>::max());
```

## Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `index_adjacency_list` |
| `label` | Random-access range sized to `num_vertices(g)`. Initial labels in, community labels out. Modified in-place. |
| `empty_label` | Sentinel value for unlabeled vertices (they don't vote until labeled) |
| `rng` | Random number generator for tie-breaking and shuffle (e.g., `std::mt19937`) |
| `max_iters` | Maximum number of iterations. Default: unlimited (run until convergence). |

## Examples

### Example 1: Basic Community Detection

Each vertex starts as its own community. After propagation, densely connected
groups converge to shared labels.

```cpp
#include <graph/algorithm/label_propagation.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <numeric>
#include <random>
#include <vector>

using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
    container::vov_graph_traits<void>>;

// Two cliques connected by a bridge: {0,1,2} and {3,4,5}
Graph g({{0,1},{1,0}, {0,2},{2,0}, {1,2},{2,1},   // clique A
         {2,3},{3,2},                                // bridge
         {3,4},{4,3}, {3,5},{5,3}, {4,5},{5,4}});   // clique B

std::vector<uint32_t> label(num_vertices(g));
std::iota(label.begin(), label.end(), 0u);  // label[v] = v initially

std::mt19937 rng{42};
label_propagation(g, label, rng);

// After convergence:
//   label[0] == label[1] == label[2]  (community A)
//   label[3] == label[4] == label[5]  (community B)
// Exact label values depend on RNG and processing order
```

### Example 2: Majority-Vote Convergence

When most vertices in a clique share a label, the minority converges to the
majority.

```cpp
// K4: vertices 0, 1, 2, 3 — all connected to all
Graph k4({{0,1},{1,0}, {0,2},{2,0}, {0,3},{3,0},
          {1,2},{2,1}, {1,3},{3,1}, {2,3},{3,2}});

// 3 vertices labeled 99, 1 labeled 42
std::vector<int> label = {99, 99, 99, 42};

std::mt19937 rng{42};
label_propagation(k4, label, rng);

// All converge to 99 — majority wins
// label = {99, 99, 99, 99}
```

### Example 3: Partial Labels with Empty Sentinel

Start with only some vertices labeled. Unlabeled vertices (sentinel value −1)
don't vote until they receive a label from a neighbor.

```cpp
std::vector<int> label = {42, -1, -1, -1, -1, 99};

std::mt19937 rng{42};
label_propagation(g, label, -1, rng);

// Labels propagate from initially-labeled vertices outward
// Vertices near 0 adopt label 42, vertices near 5 adopt label 99
```

### Example 4: Controlling Iterations

Limit the number of iterations for large graphs or real-time constraints.

```cpp
std::vector<uint32_t> label(num_vertices(g));
std::iota(label.begin(), label.end(), 0u);

std::mt19937 rng{42};
label_propagation(g, label, rng, 1);  // single iteration

// After just 1 iteration, some vertices may not have converged yet
// Run more iterations for finer community structure

label_propagation(g, label, rng, 0);  // 0 iterations = no change
// label unchanged
```

## Complexity

| Metric | Value |
|--------|-------|
| Time | O(E) per iteration |
| Space | O(V) auxiliary (shuffle buffer, frequency counts) |

Typically converges in a small number of iterations (often < 10) for
real-world graphs.

## Preconditions

- Graph must satisfy `index_adjacency_list<G>`.
- `label` must be sized to `num_vertices(g)` and pre-initialized (e.g., with
  `std::iota` for one-label-per-vertex).
- The RNG must satisfy `UniformRandomBitGenerator`.
- With `empty_label` sentinel: vertices with the sentinel label don't vote.
  If all vertices start with the sentinel, no propagation occurs.

## See Also

- [Jaccard Coefficient](jaccard.md) — neighborhood similarity metric
- [Connected Components](connected_components.md) — structural component detection
- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_label_propagation.cpp](../../../tests/algorithms/test_label_propagation.cpp) — test suite
