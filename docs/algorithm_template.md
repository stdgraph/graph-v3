# Algorithm Documentation Template

This template provides a standardized format for documenting graph algorithms in the graph-v3 library. Follow this structure to ensure consistency and completeness across all algorithm documentation.

## Algorithm Name

### Brief Description

Provide a one-paragraph overview of the algorithm:
- What problem does it solve?
- What is the primary use case?
- When should it be used vs alternatives?

**Example:**
> Dijkstra's algorithm finds the shortest paths from a source vertex to all other vertices in a weighted graph with non-negative edge weights. It is optimal for dense graphs and provides both distance and predecessor information for path reconstruction.

---

## Complexity Analysis

### Time Complexity

| Case | Complexity | Notes |
|------|-----------|-------|
| Best case | O(...) | When does this occur? |
| Average case | O(...) | Typical performance |
| Worst case | O(...) | Under what conditions? |

For graph algorithms, specify complexity in terms of:
- `V` = number of vertices
- `E` = number of edges
- Other relevant parameters (e.g., maximum edge weight)

**Implementation-Specific Notes:**
- Binary heap: O((V + E) log V)
- Fibonacci heap: O(E + V log V)
- Dense graphs (E ≈ V²): O(V²) with adjacency matrix

### Space Complexity

| Component | Space | Purpose |
|-----------|-------|---------|
| Distance array | O(V) | Store shortest distances |
| Predecessor array | O(V) | Store path information |
| Priority queue | O(V) | Pending vertices |
| Visited flags | O(V) | Track processed vertices |
| **Total** | **O(V)** | Excluding graph storage |

**Notes:**
- Specify whether space is auxiliary (temporary) or part of output
- Include recursion stack depth if applicable
- Mention if algorithm can operate in-place

---

## Supported Graph Properties

Clearly state which graph types and configurations are supported:

### Directedness
- ✅ Directed graphs
- ✅ Undirected graphs
- ⚠️ Mixed (with caveats)

### Edge Properties
- ✅ Weighted edges (non-negative)
- ❌ Negative edge weights
- ✅ Uniform weights (can be optimized to BFS)
- ⚠️ Multi-edges: Uses minimum weight edge
- ✅ Self-loops: Allowed (ignored in shortest path)

### Graph Structure
- ✅ Connected graphs
- ✅ Disconnected graphs (finds paths within components)
- ❌ Must be acyclic (DAG)
- ✅ May contain cycles

### Container Requirements
- Requires: `adjacency_list` concept
- Requires: `forward_range<vertex_range_t<G>>`
- Requires: `integral<vertex_id_t<G>>`
- Works with: All `dynamic_graph` container combinations
- Works with: `compressed_graph` (when implemented)
- Limitations: None known

---

## Function Signature

```cpp
template <adjacency_list      G,
          random_access_range Distance,
          random_access_range Predecessor,
          class WF = function<range_value_t<Distance>(edge_t<G>)>>
requires /* ... constraints ... */
void algorithm_name(
      G&&            g,           // graph
      vertex_id_t<G> source,      // starting vertex
      Distance&      distance,    // out: distances
      Predecessor&   predecessor, // out: predecessors
      WF&&           weight)      // edge weight function
```

---

## Parameters

### Template Parameters

#### `G` - Graph Type
- **Requires:** `adjacency_list<G>`
- **Requires:** `forward_range<vertex_range_t<G>>`
- **Requires:** `integral<vertex_id_t<G>>`
- **Description:** The graph type to operate on

#### `Distance` - Distance Range Type
- **Requires:** `random_access_range<Distance>`
- **Requires:** `is_arithmetic_v<range_value_t<Distance>>`
- **Description:** Container for storing vertex distances
- **Typical:** `std::vector<int>` or `std::vector<double>`

#### `Predecessor` - Predecessor Range Type
- **Requires:** `random_access_range<Predecessor>`
- **Requires:** `convertible_to<vertex_id_t<G>, range_value_t<Predecessor>>`
- **Description:** Container for storing predecessor information
- **Typical:** `std::vector<vertex_id_t<G>>`
- **Special:** Can use `null_predecessors` if path reconstruction not needed

#### `WF` - Weight Function Type
- **Requires:** `edge_weight_function<G, WF>`
- **Description:** Callable that returns the weight of an edge
- **Default:** Returns 1 for all edges (unweighted graph)
- **Signature:** `range_value_t<Distance> operator()(const edge_t<G>&)`

### Function Parameters

#### `g` - The Graph
- **Type:** `G&&` (forwarding reference)
- **Precondition:** `num_vertices(g) > 0` if `source` is provided
- **Description:** The graph to process

#### `source` - Source Vertex
- **Type:** `vertex_id_t<G>`
- **Precondition:** `source < num_vertices(g)` (for vector-based containers)
- **Precondition:** `contains_vertex(g, source)` (for map-based containers)
- **Description:** Starting vertex for the algorithm

#### `distance` - Distance Output
- **Type:** `Distance&`
- **Precondition:** `distance.size() >= num_vertices(g)`
- **Postcondition:** `distance[v]` contains shortest distance from source to v
- **Description:** Output container for distances

#### `predecessor` - Predecessor Output
- **Type:** `Predecessor&`
- **Precondition:** `predecessor.size() >= num_vertices(g)` (if not null)
- **Postcondition:** `predecessor[v]` contains predecessor of v in shortest path tree
- **Description:** Output container for path reconstruction
- **Special:** Can pass `null_predecessors` to skip path tracking

#### `weight` - Weight Function
- **Type:** `WF&&` (forwarding reference)
- **Requires:** `weight(uv)` returns arithmetic type for any edge `uv`
- **Description:** Function to extract edge weight
- **Default:** `[](const edge_t<G>& uv) { return 1; }`

---

## Return Value

**Type:** `void` (results stored in output parameters)

**Side Effects:**
- Modifies `distance` range: Sets `distance[v]` for all vertices v
- Modifies `predecessor` range: Sets `predecessor[v]` for all reachable vertices
- Does not modify the graph `g`

---

## Preconditions (Runtime Requirements)

1. `source` must be a valid vertex ID in the graph
2. `distance.size() >= num_vertices(g)`
3. `predecessor.size() >= num_vertices(g)` (if not null_predecessors)
4. For weighted graphs: all edge weights must be non-negative
5. Weight function must not throw exceptions
6. Weight function must not modify graph state

**Validation:**
```cpp
assert(source < num_vertices(g));  // vector-based containers
assert(contains_vertex(g, source)); // map-based containers
assert(distance.size() >= num_vertices(g));
```

---

## Postconditions

1. `distance[source] == 0`
2. For all vertices `v` reachable from `source`:
   - `distance[v]` contains the length of the shortest path from `source` to `v`
   - `predecessor[v]` contains the predecessor of `v` in the shortest path tree
3. For all vertices `v` unreachable from `source`:
   - `distance[v] == std::numeric_limits<weight_type>::max()`
   - `predecessor[v]` is unmodified (implementation-defined)

---

## Mandates (Compile-Time Requirements)

```cpp
requires adjacency_list<G>
requires forward_range<vertex_range_t<G>>
requires integral<vertex_id_t<G>>
requires random_access_range<Distance>
requires is_arithmetic_v<range_value_t<Distance>>
requires convertible_to<vertex_id_t<G>, range_value_t<Predecessor>>
requires edge_weight_function<G, WF>
```

These constraints are enforced via C++20 concepts and will produce a compilation error if not satisfied.

---

## Exception Safety

**Guarantee:** Basic exception safety

**Throws:**
- May throw `std::bad_alloc` if internal containers cannot allocate memory
- May propagate exceptions from container operations (unlikely with standard containers)
- Assumes weight function is `noexcept` (not enforced, but recommended)

**State after exception:**
- Graph `g` remains unchanged
- `distance` and `predecessor` may be partially modified (indeterminate state)
- Client must re-initialize output containers before retry

**Recommendation:** Use `noexcept` weight functions when possible for strong guarantee.

---

## Example Usage

### Basic Example

```cpp
#include <graph/graph.hpp>
#include <graph/algorithm/dijkstra.hpp>
#include <vector>
#include <iostream>

using namespace graph;

int main() {
    // Create a simple graph: 0 -> 1 (weight 4), 0 -> 2 (weight 2), 1 -> 2 (weight 1)
    using Graph = container::dynamic_graph<
        int, void, void, uint32_t, false,
        container::vov_graph_traits<int, void, void, uint32_t, false>>;
    
    Graph g({{0, 1, 4}, {0, 2, 2}, {1, 2, 1}, {1, 3, 5}, {2, 3, 8}});
    
    std::vector<int> distance(num_vertices(g));
    std::vector<uint32_t> predecessor(num_vertices(g));
    
    dijkstra(g, 0, distance, predecessor,
             [](const auto& uv) { return edge_value(uv); });
    
    // Print results
    for (uint32_t v = 0; v < num_vertices(g); ++v) {
        std::cout << "Distance to " << v << ": " << distance[v] << "\n";
    }
    
    return 0;
}
```

**Output:**
```
Distance to 0: 0
Distance to 1: 4
Distance to 2: 2
Distance to 3: 9
```

### Advanced Example: Path Reconstruction

```cpp
std::vector<uint32_t> reconstruct_path(
    uint32_t source, uint32_t target,
    const std::vector<uint32_t>& predecessor) {
    
    std::vector<uint32_t> path;
    uint32_t current = target;
    
    while (current != source) {
        path.push_back(current);
        current = predecessor[current];
    }
    path.push_back(source);
    
    std::ranges::reverse(path);
    return path;
}
```

### Example: Unweighted Graph (BFS-like behavior)

```cpp
// For unweighted graphs, Dijkstra becomes equivalent to BFS
Graph g({{0, 1}, {0, 2}, {1, 2}, {1, 3}});

std::vector<int> distance(num_vertices(g));
std::vector<uint32_t> predecessor(num_vertices(g));

// Default weight function returns 1 for all edges
dijkstra(g, 0, distance, predecessor);
```

---

## Implementation Notes

### Algorithm Overview

1. **Initialization:**
   - Set `distance[source] = 0`
   - Set `distance[v] = ∞` for all other vertices
   - Add source to priority queue

2. **Main Loop:**
   - Extract minimum distance vertex `u` from queue
   - For each edge `(u, v)` with weight `w`:
     - If `distance[u] + w < distance[v]`:
       - Update `distance[v] = distance[u] + w`
       - Update `predecessor[v] = u`
       - Add `v` to priority queue

3. **Termination:**
   - Queue becomes empty (all reachable vertices processed)

### Data Structures

- **Priority Queue:** Uses `std::priority_queue` with max-heap (inverted comparison)
- **Visited Tracking:** Implicit via distance comparison (no explicit visited set)
- **Re-insertion:** Allows vertices to be inserted multiple times (lazy deletion)

### Design Decisions

1. **Why allow re-insertion instead of decrease-key?**
   - `std::priority_queue` doesn't support decrease-key
   - Re-insertion with lazy deletion is simpler and nearly as efficient
   - Alternative: Use external heap library (Boost, etc.) for decrease-key

2. **Why require random_access_range for distance/predecessor?**
   - Ensures O(1) lookup by vertex ID
   - Essential for algorithm efficiency
   - Could be relaxed for map-based containers (future enhancement)

3. **Why separate weight function instead of requiring edge_value?**
   - Flexibility: Can compute weights dynamically
   - Supports graphs without stored edge values
   - Allows weight transformations (e.g., inverted, scaled)

### Optimization Opportunities

- For dense graphs: Consider adjacency matrix + array-based heap
- For sparse graphs: Current implementation is near-optimal
- For uniform weights: Use BFS instead (O(V + E) without heap)
- For DAG: Use topological sort + relaxation (O(V + E))

---

## References

### Academic Papers
- Dijkstra, E. W. (1959). "A note on two problems in connexion with graphs". *Numerische Mathematik*, 1(1), 269-271.

### Textbooks
- Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2009). *Introduction to Algorithms* (3rd ed.). MIT Press. Section 24.3.

### Online Resources
- [Wikipedia: Dijkstra's Algorithm](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm)
- [CP-Algorithms: Dijkstra](https://cp-algorithms.com/graph/dijkstra.html)

### Related Algorithms
- **Bellman-Ford:** Handles negative weights, O(VE) time
- **A* Search:** Heuristic-guided variant for single target
- **BFS:** Unweighted shortest path, O(V + E) time
- **Floyd-Warshall:** All-pairs shortest paths, O(V³) time

---

## Testing

### Test Coverage Requirements

1. **Correctness Tests:**
   - Known result validation (e.g., CLRS example graph)
   - Path reconstruction verification
   - Multiple source vertices
   - Disconnected graph components

2. **Edge Cases:**
   - Empty graph
   - Single vertex
   - Single edge
   - Self-loops
   - No path to target
   - All vertices reachable
   - All vertices unreachable

3. **Container Tests:**
   - Test with all major container types (vov, dov, vol, dol, etc.)
   - Sparse vertex IDs (map-based containers)
   - Different edge value types (int, double, float)

4. **Performance Tests:**
   - Verify O((V + E) log V) complexity
   - Compare with naive O(V²) implementation
   - Benchmark on various graph sizes
   - Memory usage validation

### Test File Location
- `tests/algorithms/test_dijkstra.cpp`

### Benchmark File Location
- `benchmark/algorithms/benchmark_shortest_path.cpp`

---

## Future Enhancements

- [ ] Add bidirectional Dijkstra for single source-target queries
- [ ] Support custom comparator for priority queue
- [ ] Add early termination when target is reached
- [ ] Optimize for graphs with small integer weights (dial's algorithm)
- [ ] Add parallel/concurrent version for large graphs
- [ ] Support for external memory graphs

---

## Version History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-02-03 | - | Initial implementation |

---

## See Also

- [Graph Algorithms Overview](README.md)
- [Graph CPO Documentation](../graph_cpo_implementation.md)
- [Container Interface](../container_interface.md)
- [Testing Guidelines](../../tests/README.md)
