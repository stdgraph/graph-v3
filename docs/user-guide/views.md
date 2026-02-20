# Graph Views Documentation

> [← Back to Documentation Index](../index.md)

- [Overview](#overview)
- [Quick Start](#quick-start)
- [Basic Views](#basic-views) — vertexlist, incidence, neighbors, edgelist
- [Simplified Views (basic\_)](#simplified-views-basic_) — id-only variants
- [Search Views](#search-views) — DFS, BFS, topological sort
- [Range Adaptor Syntax](#range-adaptor-syntax)
- [Value Functions](#value-functions)
- [Chaining with std::views](#chaining-with-stdviews)
- [Performance Considerations](#performance-considerations)
- [Best Practices](#best-practices)
- [Common Patterns](#common-patterns)
- [Limitations](#limitations)

## Overview

Graph views provide lazy, range-based access to graph elements using C++20 ranges and structured bindings. Views are composable, support pipe syntax, and integrate seamlessly with standard library range adaptors.

**Key Features**:
- Lazy evaluation - elements computed on-demand
- Zero-copy where possible - views reference graph data
- Structured bindings - elegant `auto [v, val]` syntax
- Range adaptor closures - pipe syntax `g | view()`
- Standard library integration - chain with `std::views`
- Descriptor-based - value functions receive descriptors

## Quick Start

```cpp
#include <graph/graph.hpp>

using namespace graph;

// Create a graph (vector-of-vectors for simplicity)
std::vector<std::vector<int>> g(5);
g[0] = {1, 2};
g[1] = {2, 3};
g[2] = {3, 4};
g[3] = {4};
g[4] = {};

// Iterate over all vertices
for (auto [uid, u] : views::vertexlist(g)) {
    std::cout << "Vertex: " << uid << "\n";
}

// Iterate over edges from vertex 0
for (auto [vid, uv] : views::incidence(g, 0)) {
    std::cout << "Edge to: " << vid << "\n";
}

// DFS traversal from vertex 0
for (auto [v] : views::vertices_dfs(g, 0)) {
    std::cout << "Visited: " << vertex_id(g, v) << "\n";
}
```

## Basic Views

### vertexlist

Iterates over all vertices in the graph.

**Signature**:
```cpp
auto vertexlist(G&& g);
auto vertexlist(G&& g, VVF&& vvf);            // with value function
auto vertexlist(G&& g, first, last);          // subrange by descriptor
auto vertexlist(G&& g, first, last, VVF&&);   // subrange + value function
auto vertexlist(G&& g, vr);                   // vertex range
auto vertexlist(G&& g, vr, VVF&&);            // vertex range + value function
```

**Structured Bindings**:

| Variant | Binding |
|---------|---------|
| `vertexlist(g)` | `[uid, u]` |
| `vertexlist(g, vvf)` | `[uid, u, val]` |

**Example**:
```cpp
// Without value function
for (auto [uid, u] : views::vertexlist(g)) {
    std::cout << "Vertex " << uid << "\n";
}

// With value function (graph passed as parameter, not captured)
auto vvf = [](const auto& g, auto& u) { return degree(g, u); };
for (auto [uid, u, deg] : views::vertexlist(g, vvf)) {
    std::cout << "Vertex " << uid << " has degree " << deg << "\n";
}
```

---

### incidence

Iterates over outgoing edges from a specific vertex.

**Signature**:
```cpp
auto incidence(G&& g, UID uid);
auto incidence(G&& g, UID uid, EVF&& evf);  // with value function
```

**Structured Bindings**:

| Variant | Binding |
|---------|---------|
| `incidence(g, uid)` | `[vid, uv]` |
| `incidence(g, uid, evf)` | `[vid, uv, val]` |

**Example**:
```cpp
// Without value function
for (auto [vid, uv] : views::incidence(g, 0)) {
    std::cout << "Edge to " << vid << "\n";
}

// With value function (graph passed as parameter, not captured)
auto evf = [](const auto& g, auto& uv) { return edge_value(g, uv); };
for (auto [vid, uv, w] : views::incidence(g, 0, evf)) {
    std::cout << "Edge to " << vid << " weight " << w << "\n";
}
```

---

### neighbors

Iterates over adjacent vertices (targets of outgoing edges).

**Signature**:
```cpp
auto neighbors(G&& g, UID uid);
auto neighbors(G&& g, UID uid, VVF&& vvf);  // with value function
```

**Structured Bindings**:

| Variant | Binding |
|---------|---------|
| `neighbors(g, uid)` | `[vid, n]` |
| `neighbors(g, uid, vvf)` | `[vid, n, val]` |

**Example**:
```cpp
// Without value function
for (auto [vid, n] : views::neighbors(g, 0)) {
    std::cout << "Neighbor: " << vid << "\n";
}

// With value function (graph passed as parameter, not captured)
auto vvf = [](const auto& g, auto& v) { return vertex_value(g, v); };
for (auto [vid, n, val] : views::neighbors(g, 0, vvf)) {
    std::cout << "Neighbor " << vid << " value: " << val << "\n";
}
```

---

### edgelist

Iterates over all edges in the graph (flattened iteration).

**Signature**:
```cpp
auto edgelist(G&& g);
auto edgelist(G&& g, EVF&& evf);  // with value function
```

**Structured Bindings**:

| Variant | Binding |
|---------|---------|
| `edgelist(g)` | `[uid, vid, uv]` |
| `edgelist(g, evf)` | `[uid, vid, uv, val]` |

**Example**:
```cpp
// Without value function
for (auto [uid, vid, uv] : views::edgelist(g)) {
    std::cout << "Edge: " << uid << " -> " << vid << "\n";
}

// With value function (graph passed as parameter, not captured)
auto evf = [](const auto& g, auto& uv) { return edge_value(g, uv); };
for (auto [uid, vid, uv, w] : views::edgelist(g, evf)) {
    std::cout << uid << " -> " << vid << " weight " << w << "\n";
}
```

## Simplified Views (basic\_)

Each basic view has a `basic_` variant that returns **ids only** (no vertex/edge descriptors).
These are lighter-weight when you only need identifiers and don't need to access the
vertex or edge objects themselves.

| Standard | Simplified | Binding |
|----------|-----------|---------|
| `vertexlist` → `[uid, u]` | `basic_vertexlist` → `[uid]` | vertex id only |
| `incidence` → `[vid, uv]` | `basic_incidence` → `[vid]` | target id only |
| `neighbors` → `[vid, n]` | `basic_neighbors` → `[vid]` | target id only |
| `edgelist` → `[uid, vid, uv]` | `basic_edgelist` → `[uid, vid]` | source + target id |

All `basic_` views support value functions, adding one extra binding element:
- `basic_vertexlist(g, vvf)` → `[uid, val]`
- `basic_incidence(g, uid, evf)` → `[vid, val]`
- `basic_neighbors(g, uid, vvf)` → `[vid, val]`
- `basic_edgelist(g, evf)` → `[uid, vid, val]`

```cpp
using namespace graph::views;

for (auto [uid] : basic_vertexlist(g)) { ... }
for (auto [vid] : basic_incidence(g, 0)) { ... }
for (auto [vid] : basic_neighbors(g, 0)) { ... }
for (auto [uid, vid] : basic_edgelist(g)) { ... }
```

## Search Views

Search views perform graph traversal and yield vertices/edges in traversal order.

### vertices_dfs / edges_dfs

Depth-first search traversal yielding vertices or edges.

**Signature**:
```cpp
auto vertices_dfs(G&& g, Seed seed);
auto vertices_dfs(G&& g, Seed seed, VVF&& vvf);
auto vertices_dfs(G&& g, Seed seed, VVF&& vvf, Alloc alloc);

auto edges_dfs(G&& g, Seed seed);
auto edges_dfs(G&& g, Seed seed, EVF&& evf);
auto edges_dfs(G&& g, Seed seed, EVF&& evf, Alloc alloc);
```

**Structured Bindings**:

| Variant | Binding |
|---------|---------|
| `vertices_dfs(g, seed)` | `[v]` |
| `vertices_dfs(g, seed, vvf)` | `[v, val]` |
| `edges_dfs(g, seed)` | `[uv]` |
| `edges_dfs(g, seed, evf)` | `[uv, val]` |

**Iterator category**: input (single-pass).

**Example**:
```cpp
// DFS from vertex 0
for (auto [v] : views::vertices_dfs(g, 0)) {
    std::cout << "DFS visited: " << vertex_id(g, v) << "\n";
}

// DFS edges
for (auto [uv] : views::edges_dfs(g, 0)) {
    auto src = source_id(g, uv);
    auto tgt = target_id(g, uv);
    std::cout << "Tree edge: " << src << " -> " << tgt << "\n";
}
```

**View accessors**:
- `depth()` — current stack depth
- `num_visited()` — vertices visited so far
- `cancel(cancel_search::cancel_all)` — stop iteration immediately
- `cancel(cancel_search::cancel_branch)` — skip current subtree

---

### vertices_bfs / edges_bfs

Breadth-first search traversal yielding vertices or edges.

**Signature**:
```cpp
auto vertices_bfs(G&& g, Seed seed);
auto vertices_bfs(G&& g, Seed seed, VVF&& vvf);
auto vertices_bfs(G&& g, Seed seed, VVF&& vvf, Alloc alloc);

auto edges_bfs(G&& g, Seed seed);
auto edges_bfs(G&& g, Seed seed, EVF&& evf);
auto edges_bfs(G&& g, Seed seed, EVF&& evf, Alloc alloc);
```

**Structured Bindings**:

| Variant | Binding |
|---------|---------|
| `vertices_bfs(g, seed)` | `[v]` |
| `vertices_bfs(g, seed, vvf)` | `[v, val]` |
| `edges_bfs(g, seed)` | `[uv]` |
| `edges_bfs(g, seed, evf)` | `[uv, val]` |

**Iterator category**: input (single-pass).

**Example**:
```cpp
// BFS from vertex 0
for (auto [v] : views::vertices_bfs(g, 0)) {
    std::cout << "BFS level order: " << vertex_id(g, v) << "\n";
}

// BFS edges
for (auto [uv] : views::edges_bfs(g, 0)) {
    std::cout << "BFS edge: " << source_id(g, uv) << " -> " << target_id(g, uv) << "\n";
}
```

**View accessors**:
- `depth()` — maximum depth reached so far
- `num_visited()` — vertices visited so far
- `cancel(cancel_search::cancel_all)` — stop iteration immediately
- `cancel(cancel_search::cancel_branch)` — skip current branch

---

### vertices_topological_sort / edges_topological_sort

Topological sort traversal for directed acyclic graphs (DAGs).

**Signature**:
```cpp
auto vertices_topological_sort(G&& g);
auto vertices_topological_sort(G&& g, VVF&& vvf);
auto vertices_topological_sort(G&& g, VVF&& vvf, Alloc alloc);

auto edges_topological_sort(G&& g);
auto edges_topological_sort(G&& g, EVF&& evf);
auto edges_topological_sort(G&& g, EVF&& evf, Alloc alloc);

// Safe variants with cycle detection
auto vertices_topological_sort_safe(G&& g);    // returns tl::expected<view, vertex_t>
auto edges_topological_sort_safe(G&& g);       // returns tl::expected<view, vertex_t>
```

**Structured Bindings**:

| Variant | Binding |
|---------|---------|
| `vertices_topological_sort(g)` | `[v]` |
| `vertices_topological_sort(g, vvf)` | `[v, val]` |
| `edges_topological_sort(g)` | `[uv]` |
| `edges_topological_sort(g, evf)` | `[uv, val]` |

**Iterator category**: forward (multi-pass).

**Example**:
```cpp
// Topological order
for (auto [v] : views::vertices_topological_sort(g)) {
    std::cout << "Topo order: " << vertex_id(g, v) << "\n";
}
```

**View accessors**:
- `num_visited()` — vertices visited so far
- `size()` — total vertices (vertex views only)
- `cancel(cancel_search::cancel_all)` — stop iteration immediately

**Cycle detection** — use the `_safe` variants, which return `tl::expected`:
```cpp
auto result = views::vertices_topological_sort_safe(g);
if (result) {
    for (auto [v] : *result) {
        // Process vertices in topological order
    }
} else {
    auto cycle_vertex = result.error();  // vertex that closes the back edge
    std::cerr << "Graph has a cycle involving vertex " << cycle_vertex << "\n";
}
```

## Range Adaptor Syntax

All views support both direct call and pipe syntax:

```cpp
using namespace graph::views;

// Direct call syntax
auto view = vertexlist(g);
auto view = incidence(g, 0);
auto view = vertices_dfs(g, 0);

// Pipe syntax via adaptors
using namespace graph::views::adaptors;
auto view = g | vertexlist();
auto view = g | incidence(0);
auto view = g | vertices_dfs(0);
```

## Value Functions

Value functions receive the **graph and descriptor as parameters**, enabling stateless lambdas
that support full `std::views` chaining:

**Vertex Value Function**:
```cpp
auto vvf = [](const auto& g, auto& v) {
    return degree(g, v);
};
for (auto [uid, u, deg] : views::vertexlist(g, vvf)) {
    // deg = degree of vertex
}
```

**Edge Value Function**:
```cpp
auto evf = [](const auto& g, auto& uv) {
    return edge_value(g, uv);
};
for (auto [vid, uv, w] : views::incidence(g, 0, evf)) {
    // w = edge weight
}
```

**Complex Value Functions**:
```cpp
// Return struct
struct VertexData {
    size_t id;
    size_t deg;
};

auto vvf = [](const auto& g, auto& v) -> VertexData {
    return {vertex_id(g, v), degree(g, v)};
};

for (auto [uid, u, data] : views::vertexlist(g, vvf)) {
    std::cout << "Vertex " << data.id 
              << " has degree " << data.deg << "\n";
}
```

## Chaining with std::views

Graph views integrate with C++20 standard ranges. Because value functions receive the graph
as a parameter (not via capture), lambdas remain stateless and views satisfy `std::ranges::view`,
enabling full chaining with `std::views` adaptors.

### Without Value Functions

```cpp
#include <ranges>

// Take first N vertices
auto first_five = views::vertexlist(g) | std::views::take(5);

// Filter edges
for (auto [vid, uv] : views::incidence(g, 0)
                       | std::views::filter([](auto info) {
                           auto [vid, uv] = info;
                           return vid > 3;
                         })) {
    // Only edges to vertices > 3
}
```

### With Value Functions (Full Chaining Support)

```cpp
// Value functions with graph parameter enable chaining!
auto vvf = [](const auto& g, auto& v) { return degree(g, v); };
auto view = views::vertexlist(g, vvf) 
              | std::views::take(2);        // ✅ Works!

auto evf = [](const auto& g, auto& uv) { return edge_value(g, uv); };
auto edges = views::incidence(g, 0, evf)
               | std::views::take(3);       // ✅ Works!
```

### Why This Works

Stateless lambdas (empty capture list `[]`) are default constructible and semiregular,
which satisfies `std::ranges::view`. Since value functions receive the graph as a parameter
rather than capturing it, they remain stateless:

```cpp
auto vvf = [](const auto& g, auto v) { return vertex_id(g, v); };
static_assert(std::default_initializable<decltype(vvf)>);  // ✅
static_assert(std::semiregular<decltype(vvf)>);            // ✅
```

See [View Chaining](../contributing/view-chaining.md) for historical context.

## Performance Considerations

### Lazy Evaluation

Views compute elements on-demand:

```cpp
// No computation until iteration
auto view = g | vertexlist();  // O(1)

// Computation happens during iteration
for (auto [v] : view) {  // O(n) total
    // Process vertex
}
```

### Zero-Copy Design

Basic views reference graph data without copying:

```cpp
// vertexlist, incidence, neighbors are zero-copy
auto view = g | vertexlist();  // No allocation

// Search views maintain internal state
auto dfs = g | vertices_dfs(0);  // Allocates visited tracker
```

### Memory Usage

| View | Memory | Notes |
|------|--------|-------|
| vertexlist | O(1) | References graph |
| incidence | O(1) | References graph |
| neighbors | O(1) | References graph |
| edgelist | O(1) | References graph |
| vertices_dfs | O(V) | Visited tracker + stack |
| edges_dfs | O(V) | Visited tracker + stack |
| vertices_bfs | O(V) | Visited tracker + queue |
| edges_bfs | O(V) | Visited tracker + queue |
| vertices_topological_sort | O(V) | Visited tracker + result |
| edges_topological_sort | O(V) | Visited tracker + result |

### Optimization Tips

**1. Reuse Value Functions**:
```cpp
// Good: define once, reuse (graph passed as parameter)
auto vvf = [](const auto& g, auto& v) { return vertex_id(g, v); };
auto view1 = views::vertexlist(g, vvf);
auto view2 = views::vertices_dfs(g, 0, vvf);

// Avoid: repeated lambda definitions
```

**2. Use Structured Bindings**:
```cpp
// Good: structured binding (compiler optimizes)
for (auto [uid, u, val] : views::vertexlist(g, vvf)) { }
```

**3. Early Termination**:
```cpp
// Stop iteration when done
for (auto [v] : views::vertices_dfs(g, 0)) {
    if (vertex_id(g, v) == target) {
        break;  // efficient early exit
    }
}
```

**4. Custom Allocators**:
```cpp
// Use custom allocator for search views
std::pmr::monotonic_buffer_resource pool;
std::pmr::polymorphic_allocator<bool> alloc(&pool);

auto dfs = vertices_dfs(g, 0, void{}, alloc);
// Faster allocation for visited tracker
```

## Best Practices

### 1. Include Headers

```cpp
#include <graph/graph.hpp>  // Core types, CPOs, and views
```

Or include individual view headers:
```cpp
#include <graph/views/vertexlist.hpp>
#include <graph/views/dfs.hpp>
```

### 2. Use Pipe Syntax

Pipe syntax is available via adaptor objects:
```cpp
using namespace graph::views::adaptors;

// Pipe syntax
auto view = g | vertexlist();

// Direct call (also valid)
auto view = graph::views::vertexlist(g);
```

### 3. Structured Bindings

Use structured bindings for clarity:
```cpp
// Good
for (auto [uid, u, val] : views::vertexlist(g, vvf)) {
    // Use uid, u, and val directly
}
```

### 4. Const Correctness

Views work with const graphs:
```cpp
void process(const Graph& g) {
    for (auto [uid, u] : views::vertexlist(g)) {
        // Read-only access
    }
}
```

### 5. Value Function Patterns

Value functions receive the graph as first parameter, keeping lambdas stateless:

**Pattern 1: Simple Transformation**
```cpp
auto vvf = [](const auto& g, auto& v) { return vertex_id(g, v); };
```

**Pattern 2: Multiple Values**
```cpp
auto vvf = [](const auto& g, auto& v) {
    return std::tuple{vertex_id(g, v), degree(g, v)};
};
for (auto [uid, u, id_and_deg] : views::vertexlist(g, vvf)) { }
```

**Pattern 3: Computed Properties**
```cpp
auto vvf = [](const auto& g, auto& v) {
    size_t id = vertex_id(g, v);
    size_t deg = degree(g, v);
    return id * 100 + deg;  // composite value
};
```

**Pattern 4: Chaining with std::views**
```cpp
auto vvf = [](const auto& g, auto& v) { return degree(g, v); };
auto view = views::vertexlist(g, vvf) 
              | std::views::take(5);   // ✅ Full chaining support!
```

### 6. Error Handling

**Cycle Detection** (topological sort):
```cpp
auto result = views::vertices_topological_sort_safe(g);
if (result) {
    for (auto [v] : *result) {
        // Process DAG
    }
} else {
    // result.error() is the vertex closing the back edge
}
```

**Search Cancellation**:
```cpp
auto dfs = views::vertices_dfs(g, 0);
for (auto [v] : dfs) {
    if (should_stop(v)) {
        dfs.cancel(cancel_search::cancel_all);  // stop immediately
        break;
    }
}
```

## Common Patterns

### Pattern 1: Neighbor Iteration
```cpp
// Find all neighbors of a vertex
std::vector<size_t> get_neighbors(const auto& g, size_t vid) {
    std::vector<size_t> result;
    for (auto [vid, n] : views::neighbors(g, vid)) {
        result.push_back(vid);
    }
    return result;
}
```

### Pattern 2: Reachability Test
```cpp
// Check if target reachable from source
bool is_reachable(const auto& g, size_t src, size_t tgt) {
    for (auto [v] : views::vertices_dfs(g, src)) {
        if (vertex_id(g, v) == tgt) {
            return true;
        }
    }
    return false;
}
```

### Pattern 3: Topological Ordering
```cpp
// Get topological order as vector
std::vector<size_t> topological_order(const auto& g) {
    std::vector<size_t> result;
    for (auto [v] : views::vertices_topological_sort(g)) {
        result.push_back(vertex_id(g, v));
    }
    return result;
}
```

### Pattern 4: Degree Distribution
```cpp
// Calculate degree distribution
std::unordered_map<size_t, size_t> degree_distribution(const auto& g) {
    std::unordered_map<size_t, size_t> dist;
    for (auto [uid, u] : views::vertexlist(g)) {
        ++dist[degree(g, u)];
    }
    return dist;
}
```

## Limitations

### 1. Iterator Categories

Basic views (`vertexlist`, `incidence`, `neighbors`, `edgelist`) and topological sort views
are **forward ranges** (multi-pass). DFS and BFS views are **input ranges** (single-pass):

```cpp
// Forward range — can iterate multiple times
auto vl = views::vertexlist(g);
for (auto [uid, u] : vl) { }  // first pass
for (auto [uid, u] : vl) { }  // second pass — OK

// Input range — single pass only
auto dfs = views::vertices_dfs(g, 0);
for (auto [v] : dfs) { }  // first pass
// Second iteration: create a new view
auto dfs2 = views::vertices_dfs(g, 0);
for (auto [v] : dfs2) { }
```

### 2. Descriptor Lifetime

Descriptors are valid only during iteration:
```cpp
// Good: store vertex IDs
std::vector<size_t> vertex_ids;
for (auto [uid, u] : views::vertexlist(g)) {
    vertex_ids.push_back(uid);
}
```

### 3. Graph Mutation

Don't modify graph structure during iteration:
```cpp
// Bad: modifying during iteration
for (auto [uid, u] : views::vertexlist(g)) {
    g.add_vertex();  // undefined behavior!
}

// Good: collect then modify
std::vector<size_t> to_process;
for (auto [uid, u] : views::vertexlist(g)) {
    to_process.push_back(uid);
}
// Now safe to modify graph
```

> **Note:** Modifying property values (`edge_value`, `vertex_value`, `graph_value`) during
> iteration is safe — only structural changes (adding/removing vertices or edges) are prohibited.

- [Adjacency Lists User Guide](adjacency-lists.md) — concepts, CPOs, descriptors
- [Containers User Guide](containers.md) — graph container options
- [Edge Lists User Guide](edge-lists.md) — edge list input for graph construction
- [CPO Reference](../reference/cpo-reference.md) — customization point objects
- [View Chaining](../contributing/view-chaining.md) — advanced chaining patterns
