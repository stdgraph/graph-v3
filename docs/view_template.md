# View Documentation Template

This template provides a standardized format for documenting graph views in the graph-v3 library. Follow this structure to ensure consistency and completeness across all view documentation.

## View Name

### Brief Description

Provide a one-paragraph overview of the view:
- What graph elements does it iterate over?
- What information does each iteration yield?
- When should it be used vs alternatives?

**Example:**
> `vertexlist` iterates over all vertices in a graph, yielding vertex IDs and descriptors via structured bindings. Use it for whole-graph vertex traversal; for neighbor iteration from a specific vertex, use `incidence` or `neighbors` instead.

---

## View Variants

List all variants of this view:

| Variant | Structured Binding | Description |
|---------|--------------------|-------------|
| `view_name(g)` | `[id, descriptor]` | Standard view |
| `view_name(g, vf)` | `[id, descriptor, value]` | With value function |
| `basic_view_name(g)` | `[id]` | Simplified (id only) |
| `basic_view_name(g, vf)` | `[id, value]` | Simplified with value function |

---

## Structured Bindings

Clearly document what each structured binding element represents:

### Standard View
```cpp
for (auto [id, descriptor] : view_name(g)) {
    // id:         vertex_id_t<G> - vertex/target identifier
    // descriptor: vertex_t<G> or edge_t<G> - element descriptor
}
```

### With Value Function
```cpp
for (auto [id, descriptor, value] : view_name(g, vf)) {
    // id:         vertex_id_t<G> - vertex/target identifier
    // descriptor: vertex_t<G> or edge_t<G> - element descriptor
    // value:      invoke_result_t<VF, G&, descriptor_t> - computed value
}
```

### basic\_ Variant
```cpp
for (auto [id] : basic_view_name(g)) {
    // id: vertex_id_t<G> - identifier only (no descriptor)
}
```

### basic\_ Variant with Value Function
```cpp
for (auto [id, value] : basic_view_name(g, vf)) {
    // id:    vertex_id_t<G> - identifier only
    // value: invoke_result_t<VF, G&, descriptor_t> - computed value
}
```

---

## Factory Function Signatures

```cpp
// Standard view
template <adjacency_list G>
auto view_name(G&& g);

// Standard view with value function
template <adjacency_list G, class VF>
auto view_name(G&& g, VF&& vf);

// Standard view over subrange (if applicable)
template <adjacency_list G>
auto view_name(G&& g, vertex_iterator_t<G> first, vertex_iterator_t<G> last);

// Standard view over subrange with value function (if applicable)
template <adjacency_list G, class VF>
auto view_name(G&& g, vertex_iterator_t<G> first, vertex_iterator_t<G> last, VF&& vf);

// basic_ variant
template <adjacency_list G>
auto basic_view_name(G&& g);

// basic_ variant with value function
template <adjacency_list G, class VF>
auto basic_view_name(G&& g, VF&& vf);
```

---

## Parameters

### Template Parameters

#### `G` - Graph Type
- **Requires:** `adjacency_list<G>`
- **Requires:** `forward_range<vertex_range_t<G>>`
- **Description:** The graph type to create a view over

#### `VF` - Value Function Type (optional)
- **Requires:** `invocable<VF, const G&, vertex_t<G>>` (for vertex value functions)
- **Requires:** `invocable<VF, const G&, edge_t<G>>` (for edge value functions)
- **Description:** Callable that computes a value for each element
- **Note:** Should be a stateless lambda (empty capture) for full `std::views` chaining support

### Function Parameters

#### `g` - The Graph
- **Type:** `G&&` (forwarding reference)
- **Description:** The graph to create a view over. The view holds a reference; the graph must outlive the view.

#### `uid` - Seed Vertex (for incidence/neighbors views)
- **Type:** `vertex_id_t<G>`
- **Precondition:** `uid` must be a valid vertex ID in the graph
- **Description:** The source vertex whose incident edges or neighbors to iterate

#### `vf` - Value Function (optional)
- **Type:** `VF&&` (forwarding reference)
- **Signature:** `auto operator()(const G& g, vertex_t<G> v)` or `auto operator()(const G& g, edge_t<G> e)`
- **Description:** Function invoked per element to compute a user-defined value
- **Default:** None (view yields id + descriptor only)

#### `first`, `last` - Subrange (if applicable)
- **Type:** `vertex_iterator_t<G>`
- **Precondition:** `[first, last)` is a valid subrange of `vertices(g)`
- **Description:** Restricts iteration to vertices in the subrange

---

## View Class

### Class Hierarchy

```cpp
class view_name_view<G, VF = void>
    : public std::ranges::view_interface<view_name_view<G, VF>> {
    // ...
};

class basic_view_name_view<G, VF = void>
    : public std::ranges::view_interface<basic_view_name_view<G, VF>> {
    // ...
};
```

### Iterator Category

| Property | Value |
|----------|-------|
| Iterator concept | `std::forward_iterator` (or `std::input_iterator` for search views) |
| Range concept | `std::ranges::forward_range` (or `std::ranges::input_range`) |
| Sized | ✅ Yes (for structural views) / ❌ No (for search views) |
| Borrowed | ❌ No (view holds reference to graph) |
| Common | ✅ Yes (begin/end same type) / ❌ No (sentinel) |

### Info Struct

Document the `value_type` returned by the iterator's `operator*()`:

```cpp
// Internal value type (typically a struct or tuple)
struct view_element {
    vertex_id_t<G>  id;         // vertex/target id
    vertex_t<G>     descriptor; // vertex/edge descriptor (standard view only)
    value_type      value;      // computed value (value function overloads only)
};
```

---

## Pipe Adaptor Syntax

### Using Pipe Syntax

```cpp
using namespace graph::views::adaptors;

// Without value function
for (auto [id, descriptor] : g | view_name()) {
    // ...
}

// With value function
auto vf = [](const auto& g, auto x) { return /* ... */; };
for (auto [id, descriptor, val] : g | view_name(vf)) {
    // ...
}

// basic_ variant
for (auto [id] : g | basic_view_name()) {
    // ...
}
```

### Adaptor Implementation

The pipe adaptor follows the standard closure pattern:

```cpp
namespace graph::views::adaptors {

struct _view_name_fn {
    template <adjacency_list G>
    auto operator()(G&& g) const { return view_name(std::forward<G>(g)); }

    template <adjacency_list G, class VF>
    auto operator()(G&& g, VF&& vf) const { return view_name(std::forward<G>(g), std::forward<VF>(vf)); }
};

// Closure for pipe syntax
struct _view_name_closure {
    auto operator()() const { /* returns partial application object */ }
    auto operator()(auto&&... args) const { /* returns partial application object */ }
};

inline constexpr _view_name_closure view_name;

} // namespace graph::views::adaptors
```

---

## Supported Graph Properties

### Graph Requirements
- **Requires:** `adjacency_list` concept (for all views)
- **Requires:** `index_adjacency_list` concept (if vertex ID indexing needed)
- **Works with:** All `dynamic_graph` container combinations
- **Works with:** `compressed_graph` (when implemented)

### Directedness
- ✅ Directed graphs
- ✅ Undirected graphs

### Container Compatibility
- ✅ Vector-of-vectors (`vov`)
- ✅ Deque-of-vectors (`dov`)
- ✅ Vector-of-lists (`vol`)
- ✅ Deque-of-lists (`dol`)
- ⚠️ Map-based containers (if applicable, note caveats)

---

## Chaining with std::views

### Basic Chaining
```cpp
// Take first N elements
auto first_five = g | view_name() | std::views::take(5);

// Filter elements
auto filtered = g | view_name()
    | std::views::filter([&g](auto info) {
        auto [id, descriptor] = info;
        return /* predicate */;
    });
```

### With Value Functions (Full Chaining Support)
```cpp
// Stateless lambda → default constructible → satisfies view concept
auto vf = [](const auto& g, auto x) { return /* ... */; };
auto view = g | view_name(vf)
              | std::views::take(5)      // ✅ works
              | std::views::drop(1);     // ✅ works
```

**Why this works:** Stateless lambdas (empty capture `[]`) are `std::semiregular`, so the resulting view satisfies `std::ranges::view`. See [View Chaining Limitations](view_chaining_limitations.md) for details.

---

## Performance Characteristics

### Complexity

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| Construction | O(1) | Lazy — no computation at construction |
| `begin()` | O(1) | Returns iterator to first element |
| `operator++` | O(1) amortized | Advances to next element |
| `operator*` | O(1) | Dereferences current element |
| Full iteration | O(N) | N = number of yielded elements |
| `size()` | O(1) | If view is `sized_range` |

### Memory Usage

| Component | Space | Notes |
|-----------|-------|-------|
| View object | O(1) | Holds reference to graph + iterator state |
| Value function | O(1) | Stateless — no captured state |
| Visited tracker | O(V) | Search views only |
| Queue/stack | O(V) | Search views only |

### Zero-Copy Design

Structural views (vertexlist, incidence, neighbors, edgelist) hold only a reference to the graph and produce elements on-demand with no allocation. `basic_` variants are even lighter — they skip descriptor lookup and yield only IDs.

Search views (DFS, BFS, topological sort) allocate internal state (visited tracker, queue/stack) proportional to `num_vertices(g)`.

---

## Mandates (Compile-Time Requirements)

```cpp
requires adjacency_list<G>
requires forward_range<vertex_range_t<G>>
// If value function is provided:
requires invocable<VF, const G&, vertex_t<G>>   // for VVF
requires invocable<VF, const G&, edge_t<G>>     // for EVF
```

These constraints are enforced via C++20 concepts and will produce a compilation error if not satisfied.

---

## Preconditions (Runtime Requirements)

1. The graph `g` must outlive the view (view holds a reference)
2. The graph must not be mutated during iteration
3. For seeded views: `uid` must be a valid vertex ID
4. For subrange views: `[first, last)` must be a valid subrange of `vertices(g)`

---

## Postconditions

1. The graph `g` is not modified
2. All elements in the range are visited exactly once (for structural views)
3. For search views: elements are yielded in traversal order (DFS/BFS/topological)
4. For search views: only reachable vertices/edges from the seed are yielded

---

## Exception Safety

**Guarantee:** Basic exception safety (strong for construction)

**Construction:**
- View construction is `noexcept` if the graph reference and value function are `noexcept` move-constructible
- Search view construction may throw `std::bad_alloc` (allocates visited tracker)

**Iteration:**
- May propagate exceptions from value function invocation
- May propagate exceptions from graph CPO calls
- Assumes graph operations on well-formed containers do not throw

**Special:**
- `vertices_topological_sort` throws `cycle_detected` if the graph contains a cycle

---

## Remarks *(Optional)*

Include any additional information:
- Interaction with other views or algorithms
- Design rationale for binding layout
- Known limitations or caveats
- Descriptor lifetime considerations (valid only during current iteration step)
- Single-pass vs multi-pass guarantees

---

## Example Usage

### Basic Example

```cpp
#include <graph/graph.hpp>
#include <graph/views.hpp>

using namespace graph;
using namespace graph::views::adaptors;

// Create a graph
auto g = /* ... */;

// Standard view
for (auto [id, descriptor] : g | view_name()) {
    std::cout << "Element: " << id << "\n";
}
```

### With Value Function

```cpp
// Stateless lambda — graph passed as parameter
auto vf = [](const auto& g, auto x) { return /* computed value */; };

for (auto [id, descriptor, val] : g | view_name(vf)) {
    std::cout << "Value: " << val << "\n";
}
```

### basic\_ Variant

```cpp
// Lightweight — ids only, no descriptors
for (auto [id] : g | basic_view_name()) {
    std::cout << "ID: " << id << "\n";
}

// basic_ with value function
auto vf = [](const auto& g, auto x) { return /* ... */; };
for (auto [id, val] : g | basic_view_name(vf)) {
    std::cout << "ID: " << id << ", Value: " << val << "\n";
}
```

### Chaining with std::views

```cpp
auto vf = [](const auto& g, auto x) { return /* ... */; };

// Chain: take first 5 elements
auto view = g | view_name(vf) | std::views::take(5);

for (auto [id, descriptor, val] : view) {
    std::cout << val << "\n";
}
```

### Subrange (if applicable)

```cpp
// Iterate over a subset of vertices
auto first = begin(vertices(g));
auto last  = first + 5;

for (auto [id, descriptor] : view_name(g, first, last)) {
    std::cout << "Vertex: " << id << "\n";
}
```

---

## Implementation Notes

### View Architecture

1. **View class:** Derives from `std::ranges::view_interface<view_class>`, provides `begin()` and `end()`
2. **Iterator:** Forward iterator (structural) or input iterator (search), stores graph reference and current position
3. **Sentinel:** `std::default_sentinel_t` or matching iterator type
4. **Info struct:** Internal `value_type` yielded by `operator*()`, supports structured bindings via `get<>()` specializations or public members

### Data Flow

```
Factory function → view_class(g, ...) → iterator(g, begin, end) → operator*() → info struct
                                                                                      ↓
                                                                              structured binding
```

### Design Decisions

1. **Why descriptors + IDs in standard views?**
   - Descriptors enable CPO access (`vertex_value(g, v)`, `target_id(g, e)`)
   - IDs provide direct indexing into external containers (`distance[id]`)
   - `basic_` variants drop descriptors when only IDs are needed

2. **Why value functions take `(G&, descriptor)` instead of capturing the graph?**
   - Stateless lambdas are `std::semiregular` → view satisfies `std::ranges::view`
   - Enables full chaining with `std::views::take`, `filter`, etc.
   - See [View Chaining Limitations](view_chaining_limitations.md) for rationale

3. **Why `view_interface` base?**
   - Provides `empty()`, `operator bool()`, `front()`, `back()`, `operator[]` automatically
   - Reduces boilerplate; only `begin()` and `end()` needed

4. **Why separate standard and basic\_ views?**
   - `basic_` views avoid `find_vertex(g, uid)` lookups — O(1) for indexed graphs but the descriptor is often unnecessary
   - Algorithms that only need IDs (e.g., `topological_sort`, `mst`) should use `basic_` variants
   - Enables zero-overhead abstraction: pay only for what you use

### Optimization Opportunities

- For indexed graphs: `basic_` variants eliminate descriptor lookup overhead
- Stateless value functions enable compiler inlining and devirtualization
- `view_interface` provides `operator bool()` and `empty()` for free
- Iterator caching can amortize repeated `operator*()` calls

---

## References

### C++ Standards and Proposals
- [P2387R3: Pipe support for user-defined range adaptors](https://wg21.link/p2387r3)
- [std::ranges::view_interface](https://en.cppreference.com/w/cpp/ranges/view_interface)
- [std::ranges::view concept](https://en.cppreference.com/w/cpp/ranges/view)

### Related Library Documentation
- [Graph CPO Documentation](graph_cpo_implementation.md) - Customization point objects
- [Container Interface](container_interface.md) - Graph container requirements
- [View Chaining Limitations](view_chaining_limitations.md) - Chaining design rationale
- [Views Overview](user-guide/views.md) - All views summary and usage patterns

### Related Views

List related views that serve similar or complementary purposes:

- **`view_name`:** (this view)
- **`basic_view_name`:** Simplified variant (ids only)
- **`related_view`:** Description of relationship

---

## Testing

### Test Coverage Requirements

1. **Correctness Tests:**
   - Known graph → verify yielded elements match expected
   - Verify structured binding types
   - Verify iteration order (if specified)
   - Compare standard and `basic_` variant results

2. **Edge Cases:**
   - Empty graph (0 vertices)
   - Single vertex, no edges
   - Self-loops
   - Disconnected components (for search views)
   - Large fan-out (vertex with many edges)

3. **Value Function Tests:**
   - Stateless lambda
   - Function object
   - Return type deduction
   - Complex return types (struct, tuple)

4. **Pipe Adaptor Tests:**
   - `g | view_name()` syntax
   - `g | view_name(vf)` syntax
   - `g | basic_view_name()` syntax
   - Chaining with `std::views::take`, `std::views::filter`

5. **Container Tests:**
   - Test with all major container types (vov, dov, vol, dol, etc.)
   - Sparse vertex IDs (map-based containers, if supported)

6. **Range Concept Satisfaction:**
   - `static_assert(std::ranges::forward_range<decltype(view)>)`
   - `static_assert(std::ranges::view<decltype(view)>)`
   - `static_assert(std::ranges::sized_range<decltype(view)>)` (if applicable)

### Test File Location
- `tests/views/test_view_name.cpp`

### Benchmark File Location
- `benchmark/benchmark_views.cpp`

---

## Future Enhancements

- [ ] Enhancement 1
- [ ] Enhancement 2
- [ ] Enhancement 3

---

## Version History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | YYYY-MM-DD | - | Initial implementation |

---

## See Also

- [Views Overview](user-guide/views.md)
- [Graph CPO Documentation](graph_cpo_implementation.md)
- [Container Interface](container_interface.md)
- [View Chaining Limitations](view_chaining_limitations.md)
- [Testing Guidelines](../tests/README.md)
