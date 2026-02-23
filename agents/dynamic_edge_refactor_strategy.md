# Strategy: Rename dynamic_edge → dynamic_out_edge, Create dynamic_in_edge, Remove Sourced

## Motivation

1. **Symmetric edge types for distinct roles.** Outgoing and incoming edges are symmetric: an out-edge stores a `target_id` (where it goes); an in-edge stores a `source_id` (where it comes from). Each edge object stores exactly one vertex id — the "other" id is always available from the edge descriptor (the owning vertex). Today, a single `dynamic_edge` class conflates both roles using the `Sourced` flag. Introducing `dynamic_out_edge` (stores `target_id`) and `dynamic_in_edge` (stores `source_id`) makes the design explicit, keeps both types lightweight, and allows the library to support both **uniform** and **non-uniform** bidirectional edge configurations.

2. **Uniform and non-uniform bidirectional modes.** For bidirectional graphs, the library must support two edge configurations:
   - **Uniform (default for bidirectional):** The same edge type (`dynamic_out_edge`) is used for both outgoing and incoming edge containers. Out-edges are always available; in-edges are optional. Using `dynamic_out_edge` as the common type keeps out-edges as the primary concept. In the in-edge container, the `target_id_` member stores the source vertex; the edge descriptor with `in_edge_tag` handles the semantic reinterpretation. This enables moving/copying edges between lists and simplifies generic code.
   - **Non-uniform:** Outgoing edges use `dynamic_out_edge` (stores `target_id`) and incoming edges use `dynamic_in_edge` (stores `source_id`). Each type names its stored id for its role, making code more self-documenting. The CPO dispatch uses `_native_edge_member` (Tier 1) for `dynamic_in_edge::source_id()` and the descriptor (Tier 4) for out-edge `source_id`. Both modes produce identical CPO results.
   The traits layer controls which mode is active. For non-bidirectional or uniform bidirectional graphs, traits only need to define `edge_type` — the `dynamic_graph` implementation derives `out_edge_type = edge_type` and `in_edge_type = edge_type`. Defining both `out_edge_type` and `in_edge_type` is only required for non-uniform bidirectional graphs.

3. **The `Sourced` parameter is unnecessary.** Whether an edge stores source_id is now determined by the *type* chosen (`dynamic_out_edge` vs `dynamic_in_edge`), not by a `bool Sourced` template parameter. The edge descriptor for out-edges already provides `source_id()` from context (Tier 4 of the CPO dispatch), so storing it on out-edges is optional. For in-edges, source_id is always required. The `Sourced` parameter can be removed entirely.

4. **Reduced specializations.** Removing `Sourced` eliminates the 2×2 specialization matrix (`EV × Sourced`) from the old `dynamic_edge`. The new design has `dynamic_out_edge` (2 specializations: `EV!=void`, `EV==void`) and `dynamic_in_edge` (2 specializations: `EV!=void`, `EV==void`). Total: 4 specializations, down from 4, but with clearer semantics and no conditional emptiness.

## Current State

### Classes affected in `dynamic_graph.hpp`

| Class | Role | Specializations |
|-------|------|-----------------|
| `dynamic_edge_target<..., Sourced, ...>` | Stores `target_id` | 1 (always present) |
| `dynamic_edge_source<..., Sourced, ...>` | Stores `source_id` when `Sourced=true`; empty when `false` | 2 (primary + `Sourced=false` empty) |
| `dynamic_edge_value<..., Sourced, ...>` | Stores edge value when `EV!=void`; empty when `EV==void` | 2 (primary + `EV=void` empty) |
| `dynamic_edge<EV, VV, GV, VId, Sourced, Bidir, Traits>` | Composes target + source + value | 4 (`EV×Sourced` = {EV,true}, {EV,false}, {void,true}, {void,false}) |
| `dynamic_vertex_bidir_base<..., Sourced, Bidir, ...>` | Bidirectional: stores `edges_type` for in-edges | 2 (`Bidir=true`, `Bidir=false` empty) |
| `dynamic_vertex_base<..., Sourced, ...>` | Stores outgoing edges, ADL friends for `edges()`, `in_edges()` | 1 |
| `dynamic_vertex<..., Sourced, ...>` | Vertex with optional value | 2 (`VV!=void`, `VV==void`) |
| `dynamic_graph_base<..., Sourced, ...>` | Core graph: `load_edges`, `vertices_`, CPO friends | 1 |
| `dynamic_graph<..., Sourced, ...>` | Graph with/without graph value | 2 (`GV!=void`, `GV==void`) |

### Template parameter lists (current)

Every class above carries `bool Sourced` as a template parameter pierced through the entire hierarchy.

### Traits (27 files in `include/graph/container/traits/`)

Each traits struct has the signature:
```cpp
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional>
struct xxx_graph_traits {
  static constexpr bool sourced = Sourced;
  using edge_type   = dynamic_edge<EV, VV, GV, VId, Sourced, Bidirectional, xxx_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Sourced, Bidirectional, xxx_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Sourced, Bidirectional, xxx_graph_traits>;
  ...
};
```

### Tests (48 files in `tests/container/dynamic_graph/`)

- `graph_test_types.hpp` defines tag types where each tag has `template<..., bool Sourced> using traits = ...;` and `graph_test_types<Tag>` generates `sourced_void`, `sourced_int`, `sourced_all` type aliases with `Sourced=true`.
- Individual test files (e.g., `test_dynamic_graph_uol.cpp`, `test_dynamic_graph_dous.cpp`) define `xxx_sourced` type aliases and test sourced-specific behavior.
- Bidirectional tests (`test_dynamic_graph_bidirectional.cpp`) use `Sourced=true, Bidirectional=true`.

### `load_edges` in `dynamic_graph_base`

The `load_edges` method has `if constexpr (Sourced)` branches:
- **Sourced=true:** Constructs `edge_type(source_id, target_id [, value])` (2 or 3 args)
- **Sourced=false:** Constructs `edge_type(target_id [, value])` (1 or 2 args)
- **Bidirectional (only when Sourced=true):** Also emplaces a reverse copy into `in_edges()`

### `std::hash` specializations (bottom of `dynamic_graph.hpp`)

Two specializations keyed on `Sourced`:
- `Sourced=true`: hashes `source_id() ^ target_id()`
- `Sourced=false`: hashes `target_id()` only

### `dynamic_adjacency_graph` alias

```cpp
template <typename Traits>
using dynamic_adjacency_graph = dynamic_graph<..., Traits::sourced, ...>;
```

### CPO dispatch for `source_id`

The `source_id(g, uv)` CPO in `edge_cpo.hpp` has 7 tiers. The `_native_edge_member` tier (priority 1) checks if the underlying edge has a `.source_id()` method. The `_adj_list_descriptor` tier (priority 4) uses the descriptor's `source_id()` which comes from the stored source vertex — no edge member needed. After removing `Sourced`, out-edges won't have `.source_id()`, so the CPO will naturally fall through to Tier 4 (descriptor-based), which is the correct behavior.

For in-edges, the `dynamic_in_edge` will have `.source_id()`, so the `_native_edge_member` tier will fire and return the stored source id from the edge object.

## Target State

### New edge class hierarchy

```
// Base classes (composable property mixins)
dynamic_edge_target<EV, VV, GV, VId, Bidir, Traits>   ← stores target_id_ (VId)
dynamic_edge_source<EV, VV, GV, VId, Bidir, Traits>   ← stores source_id_ (VId)  [repurposed; Sourced removed]
dynamic_edge_value<EV, VV, GV, VId, Bidir, Traits>    ← stores value when EV!=void
dynamic_edge_value<void, ...>                          ← empty

// Out-edge: stores target_id + optional value
dynamic_out_edge<EV, VV, GV, VId, Bidir, Traits>
  inherits: dynamic_edge_target + dynamic_edge_value
  specializations: EV!=void, EV==void
  constructors: (target_id), (target_id, value), (target_id, value&&)
  Members: target_id() — the vertex this edge points to

// In-edge: stores source_id + optional value (symmetric with out-edge)
dynamic_in_edge<EV, VV, GV, VId, Bidir, Traits>
  inherits: dynamic_edge_source + dynamic_edge_value
  specializations: EV!=void, EV==void
  constructors: (source_id), (source_id, value), (source_id, value&&)
  Members: source_id() — the vertex this edge comes from
```

**Bidirectional mode selection** is expressed through the traits' type aliases, not through a template parameter. Traits only need to define `edge_type`; the `dynamic_graph` implementation derives `out_edge_type` and `in_edge_type` from it (defaulting both to `edge_type`). Traits that define `in_edge_type` explicitly opt into non-uniform mode:

| Mode | Traits defines | `out_edge_type` (derived) | `in_edge_type` (derived) | Notes |
|------|---------------|--------------------------|--------------------------|-------|
| Non-bidirectional | `edge_type` only | `= edge_type` (`dynamic_out_edge`) | `= edge_type` (unused) | Lightweight; source_id from descriptor |
| Uniform bidirectional | `edge_type` only | `= edge_type` (`dynamic_out_edge`) | `= edge_type` (`dynamic_out_edge`) | Same type in both containers; in-edge `target_id_` stores source vertex; descriptor flips semantics |
| Non-uniform bidirectional | `edge_type` + `in_edge_type` | `= edge_type` (`dynamic_out_edge`) | `= Traits::in_edge_type` (`dynamic_in_edge`) | Each type names its stored VId for its role; most self-documenting |

Both `dynamic_out_edge` and `dynamic_in_edge` store exactly **one VId** plus an optional value. They are structurally identical but name their stored id according to their role. The "other" id (source for out-edges, target for in-edges) is always provided by the edge descriptor from the owning vertex.

In **uniform** mode, the in-edge container stores `dynamic_out_edge` objects where `target_id_` holds the source vertex. The `in_edge_tag` descriptor reinterprets: `source_id(g, uv)` extracts `target_id()` from the raw edge (the actual source), and `target_id(g, uv)` returns the owning vertex (the actual target).

### Removed/repurposed classes

- `dynamic_edge_source`: **Repurposed** — the `Sourced=false` empty specialization is removed; the primary class is kept (with `Sourced` removed from template params) as the base for `dynamic_in_edge`. It stores `source_id_` unconditionally.

- All 4 specializations of the old `dynamic_edge` — replaced by `dynamic_out_edge` (2 specializations) and `dynamic_in_edge` (2 specializations).

### Template parameter removal: `Sourced`

`Sourced` is removed from **every** template parameter list:
- `dynamic_out_edge<EV, VV, GV, VId, Bidirectional, Traits>` — 6 params (was 7)
- `dynamic_in_edge<EV, VV, GV, VId, Bidirectional, Traits>` — 6 params (was N/A, new class)
- `dynamic_vertex_bidir_base`, `dynamic_vertex_base`, `dynamic_vertex` — 6 params
- `dynamic_graph_base`, `dynamic_graph` — 6 params (default: `<EV=void, VV=void, GV=void, VId=uint32_t, Bidirectional=false, Traits=vofl_graph_traits<...>>`)
- All traits structs — 5 params: `<EV, VV, GV, VId, Bidirectional>`

### Traits changes

The key usability principle: **traits only need to define `edge_type`** for non-bidirectional or uniform bidirectional graphs. The `dynamic_graph` implementation derives `out_edge_type` and `in_edge_type` from it. Explicitly defining `out_edge_type` and `in_edge_type` is only required for non-uniform bidirectional graphs.

**How `dynamic_graph` derives the edge types from traits:**

The graph implementation uses detection idioms to determine what the traits provide:

```cpp
// In dynamic_graph_base (or a traits-helper):
using edge_type     = typename Traits::edge_type;                         // required by all traits
using out_edge_type = edge_type;                                          // alias: edge_type IS the out-edge type
using in_edge_type  = detected_or_t<edge_type, detect_in_edge_type, Traits>; // Traits::in_edge_type if present, else edge_type
using edges_type    = typename Traits::edges_type;                        // required by all traits
using in_edges_type = detected_or_t<edges_type, detect_in_edges_type, Traits>; // Traits::in_edges_type if present, else edges_type
```

When `Traits::in_edge_type` exists and differs from `edge_type`, the graph is in non-uniform mode. Otherwise it is uniform (both containers use `edge_type`).
Similarly, when `Traits::in_edges_type` is not provided, the graph defaults `in_edges_type = edges_type`.

Note: `detected_or_t`/`detect_*` names above are illustrative pseudocode for a detection-idiom helper.

**Standard traits (non-bidirectional or uniform bidirectional — the common case):**
```cpp
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Bidirectional = false>
struct xxx_graph_traits {
  static constexpr bool bidirectional = Bidirectional;

  using edge_type     = dynamic_out_edge<EV, VV, GV, VId, Bidirectional, xxx_graph_traits>;
  // No out_edge_type or in_edge_type needed — dynamic_graph derives them from edge_type.
  // out_edge_type = edge_type, in_edge_type = edge_type (uniform).

  using vertex_type   = dynamic_vertex<EV, VV, GV, VId, Bidirectional, xxx_graph_traits>;
  using graph_type    = dynamic_graph<EV, VV, GV, VId, Bidirectional, xxx_graph_traits>;

  using edges_type    = Container<edge_type>;       // e.g., vector, list, forward_list, set, ...
  // Optional in_edges_type omitted in standard traits; dynamic_graph derives in_edges_type = edges_type.
  using vertices_type = Container<vertex_type>;
};
```

This is the only form needed for the existing 27 standard traits files. The `edge_type` alias doubles as `out_edge_type`. For bidirectional graphs, both containers use the same `edge_type` (uniform mode), matching the current behavior where both containers store `dynamic_edge`.

**Non-uniform bidirectional traits (user-defined or advanced):**
```cpp
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t>
struct xxx_nonuniform_graph_traits {
  static constexpr bool bidirectional = true;

  using edge_type     = dynamic_out_edge<EV, VV, GV, VId, true, xxx_nonuniform_graph_traits>;
  using in_edge_type  = dynamic_in_edge<EV, VV, GV, VId, true, xxx_nonuniform_graph_traits>;
  // Defining in_edge_type signals non-uniform mode to dynamic_graph.
  // out_edge_type is still derived as edge_type by dynamic_graph.

  using vertex_type   = dynamic_vertex<EV, VV, GV, VId, true, xxx_nonuniform_graph_traits>;
  using graph_type    = dynamic_graph<EV, VV, GV, VId, true, xxx_nonuniform_graph_traits>;

  using edges_type    = Container<edge_type>;         // Container<dynamic_out_edge<...>>
  using in_edges_type = Container<in_edge_type>;      // Required for non-uniform: container value type differs
  using vertices_type = Container<vertex_type>;
};
```

**No detection needed in `load_edges`**: Because `edge_type` (= `out_edge_type`) is always `dynamic_out_edge`, out-edge construction is always `edge_type(target_id [, value])`. No `if constexpr` to detect `source_id()` on `edge_type` is needed.

### `load_edges` changes

The `if constexpr (Sourced)` branches are removed entirely. Construction is now straightforward:

```cpp
// Out-edge construction (always) — edge_type = dynamic_out_edge in all modes
emplace_edge(vertex_edges, e.target_id, edge_type(e.target_id [, value]));

// In-edge construction (bidirectional only)
if constexpr (Bidirectional) {
  auto& rev = vertices_[e.target_id].in_edges();
  // in_edge_type is either dynamic_out_edge (uniform) or dynamic_in_edge (non-uniform)
  // Both take a single VId: source_id for the edge being reversed
  if constexpr (std::is_same_v<in_edge_type, edge_type>) {
    // Uniform: in_edge_type = dynamic_out_edge, construct with source_id
    // The target_id_ member stores what is semantically the source vertex
    emplace_edge(rev, e.source_id, in_edge_type(e.source_id [, value]));
  } else {
    // Non-uniform: in_edge_type = dynamic_in_edge, construct with source_id
    emplace_edge(rev, e.source_id, in_edge_type(e.source_id [, value]));
  }
}
```

Note: Both branches of the `if constexpr` produce the same call (`in_edge_type(source_id [, value])`), so the detection can be omitted entirely — it is shown above only for documentation clarity. The simplified form is:

```cpp
if constexpr (Bidirectional) {
  auto& rev = vertices_[e.target_id].in_edges();
  emplace_edge(rev, e.source_id, in_edge_type(e.source_id [, value]));
}
```

Key points:
- The `Sourced` guard is gone; `edge_type` is always `dynamic_out_edge`, so no constructor-arity detection is needed.
- Out-edge construction is always `edge_type(target_id [, value])`.
- In-edge construction is always `in_edge_type(source_id [, value])` — this works for both uniform and non-uniform because both `dynamic_out_edge` and `dynamic_in_edge` take a single VId.
- The initializer list constructor follows the same pattern.

### Vertex bidir base changes

`dynamic_vertex_bidir_base<..., Bidirectional, Traits>`:
- When `Bidirectional=true`, stores graph-derived `in_edges_type` (from `Traits::in_edges_type` if present, else `Traits::edges_type`)
- The `static_assert(Sourced, ...)` is removed since `Sourced` no longer exists. In non-uniform mode, `dynamic_in_edge` always has `source_id()`. In uniform mode, `source_id` is provided by the descriptor.
- In **uniform** mode, `in_edges_type == edges_type` (same container and value type).
- In **non-uniform** mode, `in_edges_type` is a different container type (same container *kind* but parameterized on `in_edge_type` instead of `out_edge_type`).
- The `in_edges()` accessor return type is always graph-derived `in_edges_type&`, regardless of mode.

### `dynamic_adjacency_graph` alias

```cpp
template <typename Traits>
using dynamic_adjacency_graph = dynamic_graph<typename Traits::edge_value_type,
                                              typename Traits::vertex_value_type,
                                              typename Traits::graph_value_type,
                                              typename Traits::vertex_id_type,
                                              Traits::bidirectional,
                                              Traits>;
```

### `std::hash` specializations

Collapse from 2 (keyed on `Sourced`) to 2 (keyed on edge class):
- `hash<dynamic_out_edge<...>>`: hashes `target_id()` only
- `hash<dynamic_in_edge<...>>`: hashes `source_id()` only

In uniform mode, only the `dynamic_out_edge` hash is used (for both containers). Both hash functions hash a single VId.

### Test infrastructure changes

- `graph_test_types.hpp`: Remove `bool Sourced` from tag `traits` aliases. Replace `sourced_void`, `sourced_int`, `sourced_all` with bidirectional configurations:
  - `bidir_uniform_void`: `Bidirectional=true`, uniform edges (traits define only `edge_type`; graph derives `in_edge_type = edge_type`)
  - `bidir_uniform_int`: same with `EV=int`
  - `bidir_nonuniform_void`: `Bidirectional=true`, non-uniform edges (traits define `edge_type` + `in_edge_type = dynamic_in_edge`)
  - `bidir_nonuniform_int`: same with `EV=int`
- Individual test files: Remove `xxx_sourced` type aliases. Tests that verified `Sourced=true` behavior should be converted to test bidirectional in-edge behavior (both uniform and non-uniform modes).
- Tests for `source_id(g, uv)` on out-edges: These should still pass — the CPO resolves via the descriptor (Tier 4) in both modes. Behavior is identical.
- New tests needed:
  - Verify that uniform and non-uniform bidirectional graphs produce identical traversal results.
  - Verify that `source_id(g, uv)` works for out-edges in both modes.
  - Verify that `G::in_edge_type` and `G::edge_type` are the same type in uniform mode (traits without `in_edge_type`) and different in non-uniform mode (traits with explicit `in_edge_type`).

### CPO dispatch interactions

| Mode | Out-edge `source_id(g,uv)` | Out-edge `target_id(g,uv)` | In-edge `source_id(g,uv)` | In-edge `target_id(g,uv)` |
|------|---------------------------|---------------------------|--------------------------|---------------------------|
| Non-uniform | Tier 4 (descriptor) | Tier 1 (native `.target_id()`) | Tier 1 (native `.source_id()`) | Tier 4 (descriptor → owning vertex) |
| Uniform | Tier 4 (descriptor) | Tier 1 (native `.target_id()`) | Tier 4 (descriptor extracts `.target_id()` from raw edge = actual source) | Tier 4 (descriptor → owning vertex) |

In **uniform** mode, the in-edge container stores `dynamic_out_edge` which has no `.source_id()` method. So in-edge `source_id(g, uv)` cannot use Tier 1; it falls to Tier 4 where the descriptor extracts `.target_id()` from the raw edge (which stores the source vertex in the in-edge container). The result is correct.

Both modes produce the same CPO results; only the dispatch tier differs for in-edge `source_id`. No changes to `edge_cpo.hpp` are required.

### Potential library-level implications

- **Graph concepts** (`graph_concepts.hpp`): If any concept assumes a single `edge_type`, it may need updating to account for `in_edge_type` as a potentially different type. Review `has_in_edges`, `adjacency_list`, etc.
- **Views** (`incidence.hpp`, `bfs.hpp`, `dfs.hpp`): These work with `edges(g, u)` (out-edges) and are parameterized on edge iterator types. They should work unchanged with either edge type.
- **Algorithms**: Most algorithms operate on out-edges. Bidirectional algorithms (e.g., SCC) that use `in_edges(g, u)` will see `in_edge_type` objects. The CPOs (`source_id`, `target_id`, `edge_value`) abstract away the type difference, so no algorithm changes should be needed.
- **`edge_descriptor_view`**: Already parameterized on `EdgeIter` and `EdgeDirection` (`out_edge_tag`/`in_edge_tag`). Different edge types for in/out containers work naturally because the view is templated on the iterator type of each container.

## Phased Execution Plan

### Phase 1: Create `dynamic_in_edge` alongside existing code (additive, no breakage)

**Goal:** Introduce `dynamic_in_edge` without changing any existing code.

1. Add `dynamic_in_edge` class with base classes:
   - Inherits `dynamic_edge_source` (stores source_id — repurposed from current class, with `Sourced` param removed)
   - Inherits `dynamic_edge_value` (optional edge value)
   - Provide 2 specializations: `EV!=void`, `EV==void`
   - Include `operator<=>` (compares by `source_id`), `operator==`, and `std::hash` specialization (hashes `source_id()` only)
   - Constructors: `(source_id)`, `(source_id, value)`, `(source_id, value&&)`

2. Add graph-side derived aliases in `dynamic_graph_base` for compatibility with existing traits:
  - `out_edge_type = edge_type`
  - `in_edge_type = Traits::in_edge_type` if present, else `edge_type`
  - `in_edges_type = Traits::in_edges_type` if present, else `edges_type`
  - This allows existing standard traits to remain minimal (`edge_type` + `edges_type`) while enabling non-uniform traits to opt in by defining `in_edge_type` + `in_edges_type`.

3. Add forward declaration for `dynamic_in_edge` in `dynamic_graph.hpp`.

4. **Verify:** Full test suite passes with no changes to existing tests.

### Phase 2: Wire `dynamic_in_edge` into bidirectional support

**Goal:** The bidirectional vertex stores graph-derived `in_edges_type` (fallback to `edges_type`), so standard traits need no extra aliases while non-uniform traits can opt in.

1. Change `dynamic_vertex_bidir_base<..., true, Traits>` to use graph-derived `in_edges_type` for the in-edge container.

2. Update `load_edges` bidirectional branches to construct `in_edge_type(source_id [, value])` instead of `edge_type(source_id, target_id [, value])` for in-edge emplacement. Both uniform (`dynamic_out_edge`) and non-uniform (`dynamic_in_edge`) take a single VId.
   - **Phase ordering note:** In Phase 2, `Sourced` still exists, so some legacy edge types may still require `(source_id, target_id [, value])`.
   - Use a temporary constructibility-based dispatch in Phase 2:
     - If `in_edge_type` is constructible from `(source_id, target_id [, value])`, use that form.
     - Else construct from `(source_id [, value])`.
   - This temporary dispatch is removed in Phase 4d after `Sourced` is eliminated and constructors are standardized to one VId.

3. Update the `in_edges()` ADL friend functions in `dynamic_vertex_base` to use graph-derived `in_edges_type`.

4. At this point, uniform mode still works because `in_edges_type` defaults to `edges_type` via fallback aliases. Non-uniform mode is not yet active (requires Phase 3–4 to introduce `dynamic_out_edge` and `dynamic_in_edge` as separate types).

5. **Verify:** Bidirectional tests pass (both uniform behavior preserved).

### Phase 3: Rename `dynamic_edge` → `dynamic_out_edge` (mechanical rename)

**Goal:** The out-edge class has the correct name, but `Sourced` still exists as a template parameter (to be removed next).

1. Rename `dynamic_edge` → `dynamic_out_edge` in:
   - Class declarations & definitions (primary + 3 specializations) in `dynamic_graph.hpp`
  - All `using edge_type = dynamic_edge<...>` → `using edge_type = dynamic_out_edge<...>`
   - Forward declarations in `dynamic_graph.hpp` and all 27 traits headers
   - `std::hash` specializations
   - Test files that reference `dynamic_edge` directly (if any)
   - The `edge_type` alias in traits: `using edge_type = dynamic_out_edge<...>;` (keep as main alias)

2. Add a deprecated alias: `template<...> using dynamic_edge = dynamic_out_edge<...>;` for transition.

3. **Verify:** Full test suite passes.

### Phase 4: Remove `Sourced` template parameter

**Goal:** Eliminate the `Sourced` bool from every template parameter list.

This is the highest-risk phase. Work file by file, smallest scope first.

#### 4a: Remove `Sourced` from edge classes

1. Repurpose `dynamic_edge_source`: remove `Sourced` template parameter and the `Sourced=false` empty specialization. Keep the primary class (stores `source_id_` unconditionally) as the base for `dynamic_in_edge`.
2. Remove `Sourced` from `dynamic_edge_target`, `dynamic_edge_value` template params.
3. Remove `Sourced` from all 4 `dynamic_out_edge` specializations. Collapse from 4 to 2 (only `EV` matters now):
   - `dynamic_out_edge<EV, VV, GV, VId, Bidir, Traits>` — stores target_id + value
   - `dynamic_out_edge<void, VV, GV, VId, Bidir, Traits>` — stores target_id only
   - Remove the `Sourced=true` specializations (the ones that took `(uid, vid)` constructors)
4. Update constructors: all out-edges take `(target_id [, value])` — never `(source_id, target_id [, value])`.

#### 4b: Remove `Sourced` from vertex and graph classes

1. Remove `Sourced` from `dynamic_vertex_bidir_base`, `dynamic_vertex_base`, `dynamic_vertex`, `dynamic_graph_base`, `dynamic_graph`.
2. Remove `static constexpr bool sourced = Sourced;` from `dynamic_graph`.
3. Remove `static_assert(Sourced, ...)` from `dynamic_vertex_bidir_base` (replaced by design — `dynamic_in_edge` always has source_id).

#### 4c: Remove `Sourced` from traits

1. Update all 27 traits files: remove `bool Sourced` template parameter, remove `static constexpr bool sourced`, update `using edge_type` to use `dynamic_out_edge`. No `out_edge_type` or `in_edge_type` definitions needed — `dynamic_graph` derives them from `edge_type`.

  Clarification: for standard traits, `in_edges_type` may be omitted because `dynamic_graph` fallback derives it from `edges_type`. For non-uniform custom traits, `in_edges_type` must be explicitly provided and must match `in_edge_type`.

#### 4d: Update `load_edges`

1. Remove `if constexpr (Sourced)` branches.
2. Out-edge construction is straightforward — `edge_type` is always `dynamic_out_edge`:
   ```cpp
   emplace_edge(vertex_edges, e.target_id, edge_type(e.target_id [, value]));
   ```
3. In-edge construction (when `Bidirectional`): always `in_edge_type(source_id [, value])`. This works for both uniform (`dynamic_out_edge`) and non-uniform (`dynamic_in_edge`) because both take a single VId.
4. Initializer list constructor: same pattern.

#### 4e: Update `dynamic_adjacency_graph` alias

Remove `Traits::sourced` from the template arguments.

#### 4f: Update `std::hash` specializations

Remove `Sourced` from hash template params. Two specializations:
- `hash<dynamic_out_edge<EV, VV, GV, VId, Bidir, Traits>>` — hash `target_id()`
- `hash<dynamic_in_edge<EV, VV, GV, VId, Bidir, Traits>>` — hash `source_id()`

Both hash a single VId. In uniform mode, only the `dynamic_out_edge` hash is used.

#### 4g: **Verify:** Compile all non-test code.

### Phase 5: Update tests

1. **`graph_test_types.hpp`:**
   - Remove `bool Sourced` from all tag `traits` template aliases.
   - Replace `sourced_void`, `sourced_int`, `sourced_all` with bidirectional test configurations:
     - `bidir_uniform_void`, `bidir_uniform_int`, `bidir_uniform_all` — bidirectional, uniform edges
     - `bidir_nonuniform_void`, `bidir_nonuniform_int` — bidirectional, non-uniform edges
   - Each tag needs an additional traits alias for the non-uniform variant (or a second tag template parameter).

2. **Individual container test files** (48 files):
   - Remove `xxx_sourced` type aliases and tests that assert `G::sourced == true/false`.
   - Convert tests for "sourced edge" behavior to bidirectional in-edge tests.
   - Add parallel tests for both uniform and non-uniform bidirectional modes where applicable.
   - Keep `source_id(g, uv)` tests for out-edges — verify they work via descriptor (Tier 4) in both modes.

3. **Container-native bidirectional tests** (`tests/container/dynamic_graph/test_dynamic_graph_bidirectional.cpp`):
   This file tests the `dynamic_graph` implementation directly — construction, `load_edges`, copy/move, `in_edges()` ADL friends, in-edge container behavior. Following the existing separation, it should NOT test CPO dispatch semantics.
   - Expand to test both uniform and non-uniform configurations.
   - Test `load_edges` populates in-edge containers correctly.
   - Test copy/move preserves in-edges.
   - Test `in_edges()` accessor returns correct container in both modes.
   - Test initializer-list construction with bidirectional edges.
   - Test that `in_edges_type == edges_type` in uniform mode and differs in non-uniform mode.
   - Non-bidirectional baselines remain as regression checks.

4. **CPO-only bidirectional tests** (`tests/adj_list/cpo/`):
   Following the existing pattern (e.g., `test_in_edges_cpo.cpp`, `test_source_id_cpo.cpp`), these test CPO dispatch against `dynamic_graph` instances without testing construction or mutable actions. The graph is built once and then only read through CPOs.
   - **Existing files to update:**
     - `test_in_edges_cpo.cpp`: Add `dynamic_graph` bidirectional sections (uniform and non-uniform) alongside the existing stub-graph tests.
     - `test_source_id_cpo.cpp`: Add sections verifying `source_id(g, uv)` on out-edges (Tier 4) and in-edges (Tier 1 non-uniform, Tier 4 uniform) for `dynamic_graph`.
     - `test_find_in_edge_cpo.cpp`, `test_contains_in_edge_cpo.cpp`: Add `dynamic_graph` bidirectional sections.
   - **New CPO test file** (`test_bidir_cpo_consistency.cpp` in `tests/adj_list/cpo/`):
     - Build the same graph with both uniform and non-uniform traits.
     - Verify that `source_id`, `target_id`, `edge_value`, `in_edges`, `in_degree` produce identical results through CPOs regardless of mode.
   - These tests should use pre-built `const` graphs and only exercise read-only CPO access.

5. **Concept tests** (`tests/adj_list/concepts/test_bidirectional_concepts.cpp`):
   - Add `STATIC_REQUIRE` checks that uniform and non-uniform `dynamic_graph` bidirectional types satisfy `bidirectional_adjacency_list`.
   - Verify non-bidirectional `dynamic_graph` does NOT satisfy the concept.

6. **New test: uniform/non-uniform consistency** (housed in the CPO test above, item 4).

7. **Algorithm test types** (`algorithm_test_types.hpp`) and any algorithm tests:
   - Remove `Sourced` from any graph type definitions used in algorithm tests.

8. **Examples and benchmarks:**
   - Update any explicit `Sourced` usage (likely minimal — most use default `Sourced=false`).

9. **Verify:** Full test suite passes.

### Phase 6: Cleanup

1. Remove the deprecated `dynamic_edge` alias (if added in Phase 3).
2. Update all Doxygen comments that reference `Sourced`.
3. Update user-facing docs (`docs/`, `README.md`).
4. Remove old `agents/incoming_edges_design.md` if superseded.

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| `source_id` CPO breaks for out-edges after removing Sourced | Tests fail for source_id on out-edges | In both uniform and non-uniform modes, out-edge `source_id` resolves via descriptor Tier 4. Verify in Phase 4. |
| Traits file mechanical changes (27 files) introduce typos | Compile failures | Script the changes; compile after each trait file. |
| Test files reference `Sourced` template parameter | Compile failures in tests | Phase 5 is dedicated to this. Search for `Sourced` across all test files. |
| In-edge hash collision with out-edge hash | Runtime issues in unordered containers | Both hash functions hash a single VId (`target_id` for out-edges, `source_id` for in-edges). In uniform mode, only the `dynamic_out_edge` hash is used. |
| Uniform mode: `target_id_` stores source in in-edge container | Confusing semantics when inspecting raw edge objects | The descriptor layer handles the semantic reinterpretation. Document clearly: in uniform mode, do NOT interpret `target_id_` directly from in-edge container objects — always use CPOs through descriptors. |
| `dynamic_in_edge` and `dynamic_out_edge` are same size | No memory savings in non-uniform mode | Both store one VId + optional value. The choice between modes is about naming clarity and CPO dispatch path, not memory. The non-uniform mode is more self-documenting; the uniform mode enables type-homogeneous containers. |
| Non-uniform mode: out-edges lack self-contained source_id | Confusing for users who extract raw edge objects | Document clearly: in non-uniform mode, use descriptors to access source_id for out-edges. Uniform mode preserves self-contained edges. |
| Library concepts assume single `edge_type` | Type mismatches for in-edge operations | Review `graph_concepts.hpp` for `has_in_edges` and related concepts. Ensure they work with `in_edge_type` distinct from `edge_type`. |
| Derived aliases (`in_edge_type` / `in_edges_type`) fallback incorrect | Compile/runtime mismatch in bidirectional paths | Add explicit compile-time checks in `dynamic_graph_base`: uniform mode requires `std::same_as<in_edge_type, edge_type>` and `std::same_as<in_edges_type, edges_type>` when aliases are absent; non-uniform traits must provide both aliases consistently. |
| Existing user code uses `Sourced=true` for standalone edge objects | API breakage | Document migration: use `dynamic_in_edge` for self-contained edges, or use descriptors for out-edges. |
| Two bidirectional modes (uniform/non-uniform) increase test matrix | Longer test times, more test code | Focus non-uniform tests on a representative subset of container types (e.g., vov, vol). Full matrix for uniform only. |

## Files to Modify (Summary)

| Category | Files | Count |
|----------|-------|-------|
| Core header | `include/graph/container/dynamic_graph.hpp` | 1 |
| Traits headers | `include/graph/container/traits/*.hpp` | 27 |
| Graph concepts | `include/graph/graph_concepts.hpp` (review for `in_edge_type` compatibility) | 1 |
| Test infrastructure | `tests/common/graph_test_types.hpp` | 1 |
| Test infrastructure | `tests/common/graph_fixtures.hpp` (if it references Sourced) | 1 |
| Test infrastructure | `tests/common/algorithm_test_types.hpp` (if it references Sourced) | 1 |
| Container tests | `tests/container/dynamic_graph/*.cpp` | ~48 |
| Container bidir tests | `tests/container/dynamic_graph/test_dynamic_graph_bidirectional.cpp` (native impl) | 1 |
| CPO tests | `tests/adj_list/cpo/test_source_id_cpo.cpp`, `test_source_cpo.cpp` | 2 |
| CPO bidir tests | `tests/adj_list/cpo/test_in_edges_cpo.cpp`, `test_find_in_edge_cpo.cpp`, `test_contains_in_edge_cpo.cpp` (add dynamic_graph sections) | 3 |
| CPO bidir consistency | `tests/adj_list/cpo/test_bidir_cpo_consistency.cpp` (new) | 1 |
| Bidirectional concepts | `tests/adj_list/concepts/test_bidirectional_concepts.cpp` | 1 |
| Examples | `examples/*.cpp` | ~3 |
| Benchmarks | `benchmark/*.cpp` | ~3 |
| **Total** | | **~90** |

## Verification Strategy

- After each phase, run the full test suite (`ctest` or equivalent).
- Use `grep -r "Sourced" include/ tests/` after Phase 4 to confirm complete removal.
- Use `grep -r "dynamic_edge[^_]" include/ tests/` after Phase 3 to confirm complete rename (excluding the deprecated alias).
- Static analysis: ensure no `Sourced` appears in any template parameter list.
- **Uniform/non-uniform parity check:** After Phase 5, run a dedicated test that builds the same graph with both uniform and non-uniform traits and validates that all CPO results match.
- **Memory validation:** Verify `sizeof(dynamic_out_edge<void,...>)` == `sizeof(dynamic_in_edge<void,...>)` — both store a single VId. Confirm no accidental padding or extra members.

## Definition of Done

- `dynamic_graph` compiles when traits define only `edge_type` + `edges_type` (non-bidirectional and uniform bidirectional).
- Non-uniform traits compile only when `in_edge_type` and `in_edges_type` are provided consistently.
- No references to `Sourced` remain under `include/` and `tests/`.
- All container-native bidirectional tests pass, all CPO-only bidirectional tests pass, and concept tests pass.
- Uniform and non-uniform bidirectional CPO results are identical for `source_id`, `target_id`, `edge_value`, `in_edges`, and `in_degree`.
