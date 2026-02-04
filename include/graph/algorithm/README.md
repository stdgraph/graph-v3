/**
 * @file algorithm/README.md
 * @brief Algorithm directory structure and conventions
 */

# Algorithm Implementation Directory

This directory contains the header-only implementations of graph algorithms.

## File Naming Convention

Each algorithm is in its own header file:
- `dijkstra_shortest_paths.hpp` - Dijkstra's shortest path algorithm
- `bellman_ford.hpp` - Bellman-Ford shortest path algorithm
- `bfs.hpp` - Breadth-first search
- `dfs.hpp` - Depth-first search
- etc.

## Header Structure

Each algorithm header should follow this structure:

```cpp
#ifndef GRAPH_ALGORITHM_<NAME>_HPP
#define GRAPH_ALGORITHM_<NAME>_HPP

#include "../graph.hpp"  // Core graph types and CPOs
#include <concepts>
#include <ranges>
// ... other necessary includes

namespace graph {

// Helper concepts (if needed)
template <class G, class WF>
concept edge_weight_function = /* ... */;

/**
 * @ingroup graph_algorithms
 * @brief Algorithm description
 * 
 * @tparam G Graph type
 * @tparam ... Other template parameters
 * 
 * @param g The graph
 * @param ... Other parameters
 * 
 * @complexity Time: O(...), Space: O(...)
 * 
 * @pre Preconditions
 * @post Postconditions
 */
template <adjacency_list G, /* ... */>
requires /* ... */
void algorithm_name(/* parameters */) {
    // Implementation
}

} // namespace graph

#endif // GRAPH_ALGORITHM_<NAME>_HPP
```

## Implementation Guidelines

### 1. Use Concepts

All algorithms must use concepts to constrain template parameters:

```cpp
template <adjacency_list G,
          random_access_range Distance,
          class WF = /* default */>
requires /* additional constraints */
void algorithm(/* ... */) { /* ... */ }
```

### 2. Use CPOs for Graph Operations

Always use CPOs, never direct member access:

```cpp
// ✅ Correct
auto n = num_vertices(g);
for (auto&& [uid, u] : vertices(g)) { /* ... */ }
for (auto&& [vid, uv, v] : incidence(g, uid)) { /* ... */ }

// ❌ Wrong
auto n = g.vertices().size();
for (auto& u : g.vertices()) { /* ... */ }
```

### 3. Document Thoroughly

Include comprehensive Doxygen comments:
- Brief description
- Template parameter documentation
- Function parameter documentation
- Complexity analysis
- Preconditions and postconditions
- Example usage (in comments or separate example file)

### 4. Handle Edge Cases

Consider and handle:
- Empty graphs
- Single vertex/edge
- Disconnected graphs
- Self-loops
- Multi-edges
- Invalid input (via preconditions/assertions)

### 5. Performance Considerations

- Minimize allocations
- Use views where appropriate
- Consider cache locality
- Profile critical paths
- Document algorithmic complexity

### 6. Exception Safety

- Document exception safety guarantees
- Use `noexcept` where appropriate
- Consider RAII for resource management
- Prefer strong or basic exception safety

## Testing Requirements

Each algorithm must have corresponding tests in `tests/algorithms/`:
- Correctness tests with known results
- Edge case tests
- Parameterized tests across container types
- Property-based tests for invariants

See `tests/algorithms/README.md` for testing guidelines.

## Benchmarking Requirements

Each algorithm should have benchmarks in `benchmark/algorithms/`:
- Multiple graph sizes (small, medium, large)
- Multiple container types
- Complexity verification
- Regression tracking

See `benchmark/algorithms/README.md` for benchmarking guidelines.

## Documentation Requirements

Each algorithm needs documentation in `docs/algorithms/`:
- Algorithm overview in `docs/algorithms/README.md`
- Detailed documentation following `docs/algorithm_template.md`
- Usage examples
- Complexity analysis
- References to academic sources

## Checklist for New Algorithm

- [ ] Implement in `include/graph/algorithm/<name>.hpp`
- [ ] Add Doxygen comments following style guide
- [ ] Create tests in `tests/algorithms/test_<name>.cpp`
- [ ] Add benchmarks in appropriate benchmark file
- [ ] Create documentation in `docs/algorithms/<name>.md`
- [ ] Update `docs/algorithms/README.md` with algorithm info
- [ ] Add include to `include/graph/algorithms.hpp` (commented out initially)
- [ ] Ensure all tests pass
- [ ] Verify benchmarks show expected complexity
- [ ] Submit for code review

## Style Guidelines

### Naming Conventions

- Functions: `snake_case`
- Template parameters: `PascalCase`
- Concepts: `snake_case`
- Variables: `snake_case`
- Constants: `snake_case` or `SCREAMING_SNAKE_CASE`

### Formatting

- Follow project `.clang-format` settings
- Max line length: 120 characters
- Use trailing return types for complex expressions
- Indent with 4 spaces

### Comments

- Use `//` for single-line comments
- Use `/** */` for Doxygen documentation
- Comment non-obvious design decisions
- Reference academic sources where applicable

## Resources

- [Algorithm Template](../../docs/algorithm_template.md)
- [Graph CPO Documentation](../graph_cpo_implementation.md)
- [Testing Guidelines](../../tests/algorithms/README.md)
- [Benchmark Guidelines](../../benchmark/algorithms/README.md)
