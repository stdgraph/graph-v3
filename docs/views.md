# Graph Views Documentation

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
#include <graph/views.hpp>

using namespace graph;
using namespace graph::views::adaptors;

// Create a graph (vector-of-vectors for simplicity)
std::vector<std::vector<int>> g(5);
g[0] = {1, 2};
g[1] = {2, 3};
g[2] = {3, 4};
g[3] = {4};
g[4] = {};

// Iterate over all vertices
for (auto [v] : g | vertexlist()) {
    std::cout << "Vertex: " << vertex_id(g, v) << "\n";
}

// Iterate over edges from vertex 0
for (auto [e] : g | incidence(0)) {
    std::cout << "Edge to: " << target_id(g, e) << "\n";
}

// DFS traversal from vertex 0
for (auto [v] : g | vertices_dfs(0)) {
    std::cout << "Visited: " << vertex_id(g, v) << "\n";
}
```

## Basic Views

### vertexlist

Iterates over all vertices in the graph.

**Signature**:
```cpp
auto vertexlist(G&& g);
auto vertexlist(G&& g, VVF&& vvf);  // with value function
```

**Info Struct**:
```cpp
struct vertex_info {
    vertex_descriptor vertex;
    // optional: value_type value (if value function provided)
};
```

**Example**:
```cpp
// Without value function
for (auto [v] : g | vertexlist()) {
    auto id = vertex_id(g, v);
    std::cout << "Vertex " << id << "\n";
}

// With value function (graph passed as parameter, not captured)
auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 2; };
for (auto [v, val] : g | vertexlist(vvf)) {
    std::cout << "Vertex value: " << val << "\n";
}
```

**Use Cases**:
- Iterate all vertices
- Apply operations to every vertex
- Count vertices
- Filter vertices by property

---

### incidence

Iterates over outgoing edges from a specific vertex.

**Signature**:
```cpp
auto incidence(G&& g, UID uid);
auto incidence(G&& g, UID uid, EVF&& evf);  // with value function
```

**Parameters**:
- `uid`: vertex ID or vertex descriptor (seed vertex)
- `evf`: optional edge value function

**Info Struct**:
```cpp
struct edge_info {
    edge_descriptor edge;
    // optional: value_type value (if value function provided)
};
```

**Example**:
```cpp
// Without value function
for (auto [e] : g | incidence(0)) {
    auto target = target_id(g, e);
    std::cout << "Edge to " << target << "\n";
}

// With value function (graph passed as parameter, not captured)
auto evf = [](const auto& g, auto e) { return target_id(g, e); };
for (auto [e, target] : g | incidence(0, evf)) {
    std::cout << "Target: " << target << "\n";
}
```

**Use Cases**:
- Find neighbors of a vertex
- Calculate out-degree
- Traverse outgoing edges
- Check connectivity

---

### neighbors

Iterates over adjacent vertices (targets of outgoing edges).

**Signature**:
```cpp
auto neighbors(G&& g, UID uid);
auto neighbors(G&& g, UID uid, VVF&& vvf);  // with value function
```

**Parameters**:
- `uid`: vertex ID or vertex descriptor (seed vertex)
- `vvf`: optional vertex value function

**Info Struct**:
```cpp
struct neighbor_info {
    vertex_descriptor vertex;  // neighbor vertex descriptor
    // optional: value_type value (if value function provided)
};
```

**Example**:
```cpp
// Without value function
for (auto [n] : g | neighbors(0)) {
    auto id = vertex_id(g, n);
    std::cout << "Neighbor: " << id << "\n";
}

// With value function (graph passed as parameter, not captured)
auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 10; };
for (auto [n, val] : g | neighbors(0, vvf)) {
    std::cout << "Neighbor value: " << val << "\n";
}
```

**Use Cases**:
- Direct neighbor access
- Calculate degree
- Find adjacent vertices
- Build adjacency information

---

### edgelist

Iterates over all edges in the graph (flattened iteration).

**Signature**:
```cpp
auto edgelist(G&& g);
auto edgelist(G&& g, EVF&& evf);  // with value function
```

**Info Struct**:
```cpp
struct edge_info {
    edge_descriptor edge;
    // optional: value_type value (if value function provided)
};
```

**Example**:
```cpp
// Without value function
for (auto [e] : g | edgelist()) {
    auto src = source_id(g, e);
    auto tgt = target_id(g, e);
    std::cout << "Edge: " << src << " -> " << tgt << "\n";
}

// With value function (graph passed as parameter, not captured)
auto evf = [](const auto& g, auto e) { 
    return std::pair{source_id(g, e), target_id(g, e)}; 
};
for (auto [e, endpoints] : g | edgelist(evf)) {
    auto [src, tgt] = endpoints;
    std::cout << src << " -> " << tgt << "\n";
}
```

**Use Cases**:
- Count total edges
- Iterate all edges
- Build edge list
- Graph transformations

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

**Parameters**:
- `seed`: starting vertex (ID or descriptor)
- `vvf`/`evf`: optional value function
- `alloc`: custom allocator for visited tracker

**Info Structs**:
```cpp
struct vertex_info {
    vertex_descriptor vertex;
    // optional: value_type value
};

struct edge_info {
    edge_descriptor edge;
    // optional: value_type value
};
```

**Example**:
```cpp
// DFS from vertex 0
for (auto [v] : g | vertices_dfs(0)) {
    std::cout << "DFS visited: " << vertex_id(g, v) << "\n";
}

// DFS edges (tree edges only)
for (auto [e] : g | edges_dfs(0)) {
    auto src = source_id(g, e);
    auto tgt = target_id(g, e);
    std::cout << "Tree edge: " << src << " -> " << tgt << "\n";
}
```

**Use Cases**:
- DFS traversal
- Reachability testing
- Tree construction
- Path finding

**Search Control**:
```cpp
auto dfs = vertices_dfs(g, 0);
// Cancel search early
dfs.cancel();  // stops iteration
```

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

**Example**:
```cpp
// BFS from vertex 0
for (auto [v] : g | vertices_bfs(0)) {
    std::cout << "BFS level order: " << vertex_id(g, v) << "\n";
}

// BFS edges (tree edges only)
for (auto [e] : g | edges_bfs(0)) {
    std::cout << "BFS tree edge\n";
}
```

**Use Cases**:
- Shortest path (unweighted)
- Level-order traversal
- Distance calculation
- Connected components

**Search Inspection**:
```cpp
auto bfs = vertices_bfs(g, 0);
// Query search state
auto depth = bfs.depth();         // current depth level
auto count = bfs.num_visited();   // vertices visited so far
```

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
```

**Example**:
```cpp
// Topological order
for (auto [v] : g | vertices_topological_sort()) {
    std::cout << "Topo order: " << vertex_id(g, v) << "\n";
}

// Edges in topological order
for (auto [e] : g | edges_topological_sort()) {
    // Process edges in dependency order
}
```

**Use Cases**:
- Task scheduling
- Build systems
- Dependency resolution
- DAG processing

**Cycle Detection**:
```cpp
// Safe version with cycle detection
try {
    for (auto [v] : g | vertices_topological_sort()) {
        // Process vertices
    }
} catch (const graph::views::cycle_detected& e) {
    std::cerr << "Graph has cycles!\n";
}
```

## Range Adaptor Syntax

All views support pipe operator for elegant chaining:

```cpp
// Pipe syntax (recommended)
auto view = g | vertexlist();
auto view = g | incidence(0);
auto view = g | vertices_dfs(0);

// Direct call syntax (also supported)
auto view = vertexlist(g);
auto view = incidence(g, 0);
auto view = vertices_dfs(g, 0);
```

## Value Functions

Value functions receive the **graph and descriptor as parameters**, enabling stateless lambdas
that support full `std::views` chaining:

**Vertex Value Function**:
```cpp
auto vvf = [](const auto& g, auto v) {
    return vertex_id(g, v) * 2;
};
for (auto [v, val] : g | vertexlist(vvf)) {
    // val = vertex_id * 2
}
```

**Edge Value Function**:
```cpp
auto evf = [](const auto& g, auto e) {
    return target_id(g, e);
};
for (auto [e, target] : g | incidence(0, evf)) {
    // target = target vertex ID
}
```

**Complex Value Functions**:
```cpp
// Return struct
struct VertexData {
    size_t id;
    size_t degree;
};

auto vvf = [](const auto& g, auto v) -> VertexData {
    return {vertex_id(g, v), degree(g, v)};
};

for (auto [v, data] : g | vertexlist(vvf)) {
    std::cout << "Vertex " << data.id 
              << " has degree " << data.degree << "\n";
}
```

## Chaining with std::views

Graph views integrate with C++20 standard ranges. Because value functions receive the graph
as a parameter (not via capture), lambdas remain stateless and views satisfy `std::ranges::view`,
enabling full chaining with `std::views` adaptors.

### Without Value Functions

```cpp
#include <ranges>

// Filter vertices
auto even_vertices = g 
    | vertexlist()
    | std::views::filter([&g](auto info) {
        auto [v] = info;
        return vertex_id(g, v) % 2 == 0;
    });

// Take first N vertices
auto first_five = g 
    | vertexlist() 
    | std::views::take(5);
```

### With Value Functions (Full Chaining Support)

```cpp
// Value functions with graph parameter enable chaining!
auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 10; };
auto view = g | vertexlist(vvf) 
              | std::views::take(2);        // ✅ Works!

auto evf = [](const auto& g, auto e) { return target_id(g, e); };
auto edges = g | incidence(0, evf)
               | std::views::take(3);       // ✅ Works!

// Complex chains with value functions
auto result = g
    | vertices_dfs(0, vvf)
    | std::views::drop(1)  // skip first
    | std::views::take(10); // take next 10

for (auto [v, val] : result) {
    std::cout << val << "\n";
}
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

See [View Chaining Limitations](view_chaining_limitations.md) for historical context.

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
| vertices_dfs | O(V) | Visited tracker |
| edges_dfs | O(V) | Visited tracker |
| vertices_bfs | O(V) | Visited + queue |
| edges_bfs | O(V) | Visited + queue |
| vertices_topological_sort | O(V) | In-degree map |
| edges_topological_sort | O(V) | In-degree map |

### Optimization Tips

**1. Reuse Value Functions**:
```cpp
// Good: define once, reuse (graph passed as parameter)
auto vvf = [](const auto& g, auto v) { return vertex_id(g, v); };
auto view1 = g | vertexlist(vvf);
auto view2 = g | vertices_dfs(0, vvf);

// Avoid: repeated lambda definitions
```

**2. Use Structured Bindings**:
```cpp
// Good: structured binding (compiler optimizes)
for (auto [v, val] : g | vertexlist(vvf)) { }

// Avoid: manual extraction
for (auto info : g | vertexlist(vvf)) {
    auto v = info.vertex;
    auto val = info.value;
}
```

**3. Early Termination**:
```cpp
// Stop iteration when done
for (auto [v] : g | vertices_dfs(0)) {
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

Always include both headers:
```cpp
#include <graph/graph.hpp>  // Core types and CPOs
#include <graph/views.hpp>  // Views and adaptors
```

### 2. Use Pipe Syntax

Pipe syntax is more readable:
```cpp
// Preferred
auto view = g | vertexlist();

// Also valid
auto view = vertexlist(g);
```

### 3. Structured Bindings

Use structured bindings for clarity:
```cpp
// Good
for (auto [v, val] : g | vertexlist(vvf)) {
    // Use v and val directly
}

// Avoid
for (auto info : g | vertexlist(vvf)) {
    auto v = info.vertex;
    auto val = info.value;
}
```

### 4. Const Correctness

Views work with const graphs:
```cpp
void process(const Graph& g) {
    for (auto [v] : g | vertexlist()) {
        // Read-only access
    }
}
```

### 5. Value Function Patterns

Value functions receive the graph as first parameter, keeping lambdas stateless:

**Pattern 1: Simple Transformation**
```cpp
auto vvf = [](const auto& g, auto v) { return vertex_id(g, v); };
```

**Pattern 2: Multiple Values**
```cpp
auto vvf = [](const auto& g, auto v) {
    return std::tuple{vertex_id(g, v), degree(g, v)};
};
for (auto [v, id, deg] : g | vertexlist(vvf)) { }
```

**Pattern 3: Computed Properties**
```cpp
auto vvf = [](const auto& g, auto v) {
    size_t id = vertex_id(g, v);
    size_t deg = degree(g, v);
    return id * 100 + deg;  // composite value
};
```

**Pattern 4: Chaining with std::views**
```cpp
auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 10; };
auto view = g | vertexlist(vvf) 
              | std::views::take(5)   // ✅ Full chaining support!
              | std::views::filter([](auto vi) {
                  auto [vid, v, val] = vi;
                  return val > 0;
                });
```

### 6. Error Handling

**Cycle Detection**:
```cpp
try {
    for (auto [v] : g | vertices_topological_sort()) {
        // Process DAG
    }
} catch (const graph::views::cycle_detected&) {
    // Handle cyclic graph
}
```

**Search Cancellation**:
```cpp
auto dfs = vertices_dfs(g, 0);
for (auto [v] : dfs) {
    if (should_stop(v)) {
        dfs.cancel();  // graceful termination
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
    for (auto [n] : g | neighbors(vid)) {
        result.push_back(vertex_id(g, n));
    }
    return result;
}
```

### Pattern 2: Reachability Test
```cpp
// Check if target reachable from source
bool is_reachable(const auto& g, size_t src, size_t tgt) {
    for (auto [v] : g | vertices_dfs(src)) {
        if (vertex_id(g, v) == tgt) {
            return true;
        }
    }
    return false;
}
```

### Pattern 3: Shortest Path (BFS)
```cpp
// Find shortest path length (unweighted)
std::optional<size_t> shortest_path_length(
    const auto& g, size_t src, size_t tgt) {
    
    auto bfs = vertices_bfs(g, src);
    for (auto [v] : bfs) {
        if (vertex_id(g, v) == tgt) {
            return bfs.depth();
        }
    }
    return std::nullopt;
}
```

### Pattern 4: Topological Ordering
```cpp
// Get topological order as vector
std::vector<size_t> topological_order(const auto& g) {
    std::vector<size_t> result;
    for (auto [v] : g | vertices_topological_sort()) {
        result.push_back(vertex_id(g, v));
    }
    return result;
}
```

### Pattern 5: Degree Distribution
```cpp
// Calculate degree distribution
std::unordered_map<size_t, size_t> degree_distribution(const auto& g) {
    std::unordered_map<size_t, size_t> dist;
    
    for (auto [v] : g | vertexlist()) {
        size_t deg = 0;
        for (auto [e] : g | incidence(vertex_id(g, v))) {
            ++deg;
        }
        ++dist[deg];
    }
    return dist;
}
```

### Pattern 6: Filtered Iteration
```cpp
// Process only high-degree vertices
for (auto [v] : g | vertexlist()) {
    size_t deg = 0;
    for (auto [e] : g | incidence(vertex_id(g, v))) {
        ++deg;
    }
    
    if (deg > threshold) {
        // Process high-degree vertex
    }
}
```

## Limitations

### 1. Single-Pass Iteration

Views are input ranges (single-pass):
```cpp
auto view = g | vertexlist();

// First iteration OK
for (auto [v] : view) { }

// Second iteration: behavior undefined
// Create new view instead:
auto view2 = g | vertexlist();
for (auto [v] : view2) { }
```

### 2. Descriptor Lifetime

Descriptors valid only during iteration:
```cpp
// Bad: storing descriptor
std::vector<vertex_descriptor<...>> vertices;
for (auto [v] : g | vertexlist()) {
    vertices.push_back(v);  // descriptor may be invalidated
}

// Good: store vertex IDs
std::vector<size_t> vertex_ids;
for (auto [v] : g | vertexlist()) {
    vertex_ids.push_back(vertex_id(g, v));
}
```

### 3. Graph Mutation

Don't modify graph during iteration:
```cpp
// Bad: modifying during iteration
for (auto [v] : g | vertexlist()) {
    g.add_vertex();  // undefined behavior!
}

// Good: collect then modify
std::vector<size_t> to_process;
for (auto [v] : g | vertexlist()) {
    to_process.push_back(vertex_id(g, v));
}
// Now safe to modify graph
for (auto id : to_process) {
    g.add_edge(id, ...);
}
```

## See Also

- [CPO Documentation](cpo.md) - Customization point objects
- [Graph Concepts](common_graph_guidelines.md) - Graph interface requirements
- [View Chaining Limitations](view_chaining_limitations.md) - Advanced chaining patterns
