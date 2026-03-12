# Uniform Property Access Implementation Strategy

## Goal

Replace the current hardcoded parameter types for per-vertex algorithm outputs (`Distances`,
`Predecessors`) with function-object template parameters constrained by new concepts. This
makes algorithms generic over _where_ a property lives: an external container, a field on a
vertex object, or any other addressable storage.

Reference goal document: [uniform_prop_goal.md](uniform_prop_goal.md)

---

## Current State

### How Algorithms Access Properties Today

In `dijkstra_shortest_paths.hpp` (and `bellman_ford_shortest_paths.hpp`), `Distances` and
`Predecessors` are constrained as `vertex_property_map_for<Distances, G>` — i.e., they must
be subscriptable containers (`vector` for index graphs, `unordered_map` for mapped graphs).
The algorithm body accesses them directly:

```cpp
distances[uid] = combine(d_u, w_e);   // write distance
predecessor[vid] = uid;               // write predecessor
const distance_type d_v = distances[vid]; // read distance
```

Edge weight (`WF`) is _already_ a function object constrained by `basic_edge_weight_function`.
Distances and predecessors are the only algorithm outputs still tied to container types.

### Existing Concept Foundation

| Concept | Location | Purpose |
|---------|----------|---------|
| `vertex_value_function` | `graph_concepts.hpp` | `(const G&, vertex_t<G>) → non-void` |
| `edge_value_function` | `graph_concepts.hpp` | `(const G&, edge_t<G>) → non-void` |
| `basic_edge_weight_function` | `traversal_common.hpp` | Refines `edge_value_function` + arithmetic constraints |
| `edge_weight_function` | `traversal_common.hpp` | Convenience: `basic_edge_weight_function` with `less` + `plus` |

The two new vertex property concepts will mirror `basic_edge_weight_function` /
`edge_weight_function`, applied to vertex descriptors.

### Affected Algorithms

| File | Parameters to convert |
|------|-----------------------|
| `dijkstra_shortest_paths.hpp` | `Distances`, `Predecessors` |
| `bellman_ford_shortest_paths.hpp` | `Distances`, `Predecessors` |
| `mst.hpp` | `Weight` (per-vertex MST weight accumulator) |

---

## New Concepts

Add the following to `include/graph/algorithm/traversal_common.hpp`, after the
`edge_weight_function` concept (maintaining the parallel structure):

### `vertex_property_function`

The vertex analog of `edge_value_function`. Requires the function to accept a graph and a
vertex ID, and return a non-void (typically reference) type.

```cpp
/**
 * @brief Concept for a function that reads/writes a property for a given vertex.
 *
 * Analogous to edge_value_function but for vertex-keyed properties.
 * The function must be invocable with (const Graph&, vertex_id_t<G>) and return
 * a non-void type. For writable properties, the return type should be a reference.
 *
 * @tparam VPF  Vertex property function type
 * @tparam G    Graph type
 */
template <class VPF, class G>
concept vertex_property_function =
    std::invocable<VPF, const std::remove_reference_t<G>&, vertex_id_t<G>> &&
    !std::is_void_v<std::invoke_result_t<VPF, const std::remove_reference_t<G>&, vertex_id_t<G>>>;
```

### `vertex_arithmetic_property_function`

Refines `vertex_property_function` by additionally requiring the return type to be
arithmetic. Useful for documenting intent and as a named constraint in static_assert
checks, though the algorithm `requires` clause derives the arithmetic requirement
indirectly from `basic_edge_weight_function` rather than using this concept directly.

```cpp
/**
 * @brief Concept for a vertex property function whose value type is arithmetic.
 *
 * Refines vertex_property_function. Used to constrain distance functions in
 * shortest-path algorithms.
 *
 * @tparam VPF           Vertex property function type
 * @tparam G             Graph type
 * @tparam DistanceValue Arithmetic type used for distances
 */
template <class VPF, class G, class DistanceValue>
concept vertex_arithmetic_property_function =
    vertex_property_function<VPF, G> &&
    std::is_arithmetic_v<DistanceValue> &&
    std::assignable_from<
        std::add_lvalue_reference_t<DistanceValue>,
        std::invoke_result_t<VPF, const std::remove_reference_t<G>&, vertex_id_t<G>>>;
```

---

## Algorithm Changes

### Template Parameter Replacement

Replace the container-typed `Distances` and `Predecessors` parameters with function-object
parameters `DF` (distance function) and `PF` (predecessor function):

**Before:**
```cpp
template <adjacency_list G, input_range Sources,
          class Distances, class Predecessors, class WF, ...>
requires vertex_property_map_for<Distances, G> &&
         (is_null_range_v<Predecessors> || vertex_property_map_for<Predecessors, G>) &&
         is_arithmetic_v<vertex_property_map_value_t<Distances>> &&
         basic_edge_weight_function<G, WF, vertex_property_map_value_t<Distances>, Compare, Combine>
void dijkstra_shortest_paths(G&& g, const Sources& sources,
                             Distances& distances, Predecessors& predecessor, WF&& weight, ...);
```

**After:**
```cpp
template <adjacency_list G, input_range Sources,
          class DF, class PF, class WF, ...>
requires vertex_property_function<DF, G> &&
         (is_null_vertex_property_function_v<PF> || vertex_property_function<PF, G>) &&
         basic_edge_weight_function<G, WF,
             std::remove_cvref_t<invoke_result_t<DF, const std::remove_reference_t<G>&, vertex_id_t<G>>>,
             Compare, Combine>
void dijkstra_shortest_paths(G&& g, const Sources& sources,
                             DF&& distance_fn, PF&& predecessor_fn, WF&& weight, ...);
```

`DistanceValue` is **not** a user-visible template parameter. It is deduced inside the
algorithm body as:

```cpp
using distance_type = std::remove_cvref_t<
    invoke_result_t<DF, const std::remove_reference_t<G>&, vertex_id_t<G>>>;
```

This is the direct equivalent of the old `vertex_property_map_value_t<Distances>` — the
value type of the distance container is replaced by the return type of the distance function.
`vertex_arithmetic_property_function` is therefore not needed as a constraint in the
`requires` clause; `vertex_property_function` is sufficient, and the arithmetic requirement
on the distance value falls out of `basic_edge_weight_function`'s existing `is_arithmetic_v`
constraint on `DistanceValue`.

### Algorithm Body Changes

All direct container subscript accesses become function calls:

| Before | After |
|--------|-------|
| `distances[uid]` | `distance_fn(g, uid)` |
| `distances[vid] = combine(...)` | `distance_fn(g, vid) = combine(...)` |
| `predecessor[vid] = uid` | `predecessor_fn(g, vid) = uid` |

The `distance_type` alias changes from `vertex_property_map_value_t<Distances>` to the
deduced return type of `DF`.

### Null Predecessor Sentinel

The current `is_null_range_v<Predecessors>` check for optional predecessor tracking needs an
equivalent for the function-object API. Two options:

1. **Tag type** (preferred): Define `null_vertex_property_fn` as a sentinel type. Algorithms
   check `is_null_vertex_property_function_v<PF>` at compile time.
2. **Default no-op lambda**: Default `PF` to a lambda that accepts and ignores writes.

Option 1 maps directly to the existing `_null_predecessors` / `is_null_range_v` pattern and
is preferred for consistency.

---

## Helper Adapters (Optional but Recommended)

To ease migration and reduce boilerplate, provide adapter functions in
`traversal_common.hpp` that wrap containers or vertex-value access into the new function
signatures:

```cpp
// Wrap an external container (vector or unordered_map)
auto make_distance_fn(auto& container) {
    return [&container](const auto&, auto uid) -> auto& { return container[uid]; };
}

auto make_predecessor_fn(auto& container) {
    return [&container](const auto&, auto uid) -> auto& { return container[uid]; };
}

// For vertex-valued properties (reads field from vertex_value)
// (user-defined; documented as a usage example in tests)
```

These adapters let users of the current container-based API migrate with minimal change:
```cpp
// Old
dijkstra_shortest_paths(g, source, distances, predecessor, weight_fn);

// New (with adapter)
dijkstra_shortest_paths(g, source,
                        make_distance_fn(distances),
                        make_predecessor_fn(predecessor),
                        weight_fn);

// New (vertex property)
dijkstra_shortest_paths(g, source,
    [](const auto& g, auto uid) -> auto& { return vertex_value(g, uid).distance; },
    [](const auto& g, auto uid) -> auto& { return vertex_value(g, uid).predecessor; },
    weight_fn);
```

---

## Implementation Steps

1. **Define concepts** in `traversal_common.hpp`:
   - `vertex_property_function<VPF, G>`
   - `vertex_arithmetic_property_function<VPF, G, DistanceValue>`
   - `null_vertex_property_fn` tag type + `is_null_vertex_property_function_v<T>` trait
   - New `init_shortest_paths(g, distance_fn)` and `init_shortest_paths(g, distance_fn, predecessor_fn)` overloads

2. **Update `dijkstra_shortest_paths.hpp`**:
   - Replace `Distances` / `Predecessors` template parameters with `DF` / `PF`
   - Replace `requires` clause constraints accordingly
   - Replace all `distances[uid]` / `predecessor[uid]` accesses with `distance_fn(g, uid)` / `predecessor_fn(g, uid)`
   - Update `distance_type` alias

3. **Update `bellman_ford_shortest_paths.hpp`** — same changes as Dijkstra.

4. **Update `mst.hpp`** — replace `Weight` container parameter where applicable.

5. **Add helper adapters** `make_distance_fn` / `make_predecessor_fn` in `traversal_common.hpp`.

6. **Update tests** in `tests/algorithms/`:
   - Add test cases using external containers via `make_distance_fn` (regression: current behavior)
   - Add test cases using vertex-resident properties (new behavior)
   - Add test cases with null predecessor sentinel

7. **Update documentation** in `docs/reference/` and `docs/user-guide/` to reflect the new
   function-object API with examples for both container and vertex-property use cases.

---

## File Change Summary

| File | Change |
|------|--------|
| `include/graph/algorithm/traversal_common.hpp` | Add `vertex_property_function`, `vertex_arithmetic_property_function`, `null_vertex_property_fn`, adapters, and `init_shortest_paths` function-object overloads |
| `include/graph/algorithm/dijkstra_shortest_paths.hpp` | Replace `Distances`/`Predecessors` params; update body |
| `include/graph/algorithm/bellman_ford_shortest_paths.hpp` | Same as Dijkstra |
| `include/graph/algorithm/mst.hpp` | Replace weight accumulator param |
| `tests/algorithms/test_dijkstra_shortest_paths.cpp` | Add new test cases |
| `tests/algorithms/test_bellman_ford_shortest_paths.cpp` | Add new test cases |

---

## Non-Goals

- Changing how edge weight (`WF`) is accessed — it is already a function object.
- Supporting serialization or persistence of property values.
- Enum-to-accessor association (noted in goal as optional; deferred).
- Layered/tiered distance initialization helpers (noted in goal as optional; deferred).
