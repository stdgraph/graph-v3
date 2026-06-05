# Strategy: Pass `vertex_id_t<G>` by `const&` Throughout the Library

## Problem Statement

`vertex_id_t<G>` is defined as the return type of the `vertex_id(g, u)` CPO. While it is
typically an integral value (e.g. `size_t`) for vector-based graphs, it can be **any type** — for
example, `std::string` is already used as a vertex ID in map-based graph containers
(`mos_graph_traits`, `mol_graph_traits`, etc.) and exercised in integration tests.

Currently, `vertex_id_t<G>` is passed **by value** in the vast majority of the codebase:
algorithms, view factory functions, concept `requires`-expressions, and internal lambda captures.
This is correct for integral types but introduces **unnecessary copies** for non-trivial types like
`std::string`. The library should consistently pass vertex IDs by `const vertex_id_t<G>&` where
the value is only read, and use forwarding references (`auto&&` / `VId&&`) where ownership transfer
or move semantics are beneficial.

## Current State Audit

### Where `vertex_id_t<G>` Is **Already** Passed by `const&`

| Location | Convention | Notes |
|----------|-----------|-------|
| **CPOs** (`find_vertex`, `edges`, `degree`, `in_edges`, `in_degree`) | `const VId& uid` | All CPOs in `graph_cpo.hpp` already use `const VId&` for vertex ID parameters |
| **Visitor concepts** (`traversal_common.hpp`) | `const vertex_id_t<G>& uid` | `has_on_discover_vertex_id`, `has_on_examine_vertex_id`, etc. |
| **`depth_first_search` algorithm** | `const vertex_id_t<G>& source` | The only algorithm that does this |
| **Container member functions** (`dynamic_graph`) | `const vertex_id_type& id` | `find_vertex()`, `try_find_vertex()` |

### Where `vertex_id_t<G>` Is Passed **by Value** (Needs Change)

| Category | Count | Example |
|----------|-------|---------|
| **View factory functions** (incidence, neighbors, bfs, dfs, etc.) | ~40+ overloads | `incidence(G& g, vertex_id_t<G> uid)` |
| **Algorithms** (all except DFS) | ~15 signatures | `dijkstra_shortest_paths(G&&, vertex_id_t<G> source, ...)` |
| **Concept requires-expressions** | ~10 concepts | `vertex` concept uses `vertex_id_t<G> uid` |
| **Trait detection concepts** | ~8 concepts | `has_degree_uid_impl`, `has_find_vertex_impl` |
| **View class constructors** | ~10 | `vertices_bfs_view(G& g, vertex_id_type seed, ...)` |
| **Internal local variables** | many | `vertex_id_t<G> uid = vertex_id(g, u);` — these are fine by-value |
| **Transpose view friend functions** | 2 | `find_vertex(transpose_view&, vertex_id_t<G> uid)` |

### Where `vertex_id_t<G>` Is **Returned** by Value

| Location | Returns | Notes |
|----------|---------|-------|
| `vertex_descriptor::vertex_id()` | `auto` (by value) | Has existing TODO comment to change to `decltype(auto)` |
| `edge_descriptor::source_id()` | `auto` (by value) | Delegates to `vertex_descriptor::vertex_id()` |
| `edge_descriptor::target_id()` | `auto` (by value) | All extraction branches return copies |
| CPO `vertex_id(g, u)` | `decltype(auto)` | Currently resolves to value because underlying methods return `auto` |
| CPO `target_id(g, uv)` | `decltype(auto)` | Currently resolves to value for same reason |
| CPO `source_id(g, uv)` | value | Same issue |

## Requirements

### R1: Parameters — Use `const vertex_id_t<G>&` for Read-Only Vertex IDs

Any function parameter of type `vertex_id_t<G>` that is only read (not modified, not necessarily
stored) should become `const vertex_id_t<G>&`. This avoids copies for non-trivial ID types while
being identical in performance for integral types (compilers elide the indirection for small types
passed by const-ref).

**Scope:** Algorithm signatures, view factory functions, view class constructors, concept
requires-expressions, trait detection concepts, transpose view friend functions.

### R2: Storage — Use `vertex_id_store_t<G>` for Efficient Internal Storage

Local variables declared for temporary use and container elements that must outlive the graph
(e.g., returned results) should continue to store `vertex_id_t<G>` **by value**.

However, algorithm-internal containers (queues, stacks, priority queues, visited-set keys) that
exist only during a traversal can exploit a key invariant: **the graph must be stable while the
algorithm runs.** For map-based graphs, vertex ID keys are embedded in stable map nodes whose
addresses do not change during iteration. A `std::reference_wrapper` pointing at such a key is
valid for the entire algorithm lifetime and is trivially copyable (pointer-sized, 8 bytes),
eliminating per-enqueue copies of non-trivial types like `std::string`.

The pattern uses a compile-time conditional:

```cpp
// Defined in a common algorithm header (see R6).
template <typename G>
using vertex_id_store_t = std::conditional_t<
    std::is_reference_v<raw_vertex_id_t<G>>,
    std::reference_wrapper<std::remove_reference_t<raw_vertex_id_t<G>>>,
    vertex_id_t<G>>;
```

For integral IDs (`size_t`), `raw_vertex_id_t<G>` is a prvalue so `is_reference_v` is false,
and `vertex_id_store_t<G>` is just `size_t` — zero overhead, identical codegen.

For map-based IDs (`const std::string&`), `is_reference_v` is true, and
`vertex_id_store_t<G>` is `std::reference_wrapper<const std::string>` — 8 bytes, trivially
copyable, implicitly convertible back to `const std::string&`.

Algorithm-internal data structures use this type:
```cpp
using id_type = vertex_id_store_t<G>;
std::priority_queue<id_type, std::vector<id_type>, Cmp> queue;
std::queue<id_type> frontier;
```

Assignments from `const vertex_id_t<G>&` parameters to `vertex_id_store_t<G>` local variables
work naturally: for integral types it copies the value; for reference types `reference_wrapper`
captures the reference.

### R3: Return Types — Use `decltype(auto)` with Parenthesized Returns

`vertex_descriptor::vertex_id()`, `edge_descriptor::source_id()`, and
`edge_descriptor::target_id()` should return `decltype(auto)` instead of `auto`. For map-based
storage where the key is embedded in the iterator's value, this enables **returning a const
reference** to the key rather than copying it.

The implementation uses parenthesized return expressions:
```cpp
// Before:
[[nodiscard]] constexpr auto vertex_id() const noexcept {
    return std::get<0>(*storage_);       // copies the key
}

// After:
[[nodiscard]] constexpr decltype(auto) vertex_id() const noexcept {
    return (std::get<0>(*storage_));     // returns const& to the key
}
```

For random-access (integral) cases, `return storage_;` already returns an integral prvalue — no
parentheses needed there, and adding them would only bind to a `const size_t&` to a local, which
is fine with `decltype(auto)` since the caller typically copies into a local `vertex_id_t<G>`
anyway. **However**, returning a reference to a local/member is dangerous if the descriptor is a
temporary. This needs careful analysis per code path. A safer approach: only parenthesize the
keyed-storage branch using `if constexpr`.

### R4: Forwarding References in Algorithms (Selective)

In algorithms that accept sources as a range, or where a vertex ID may be constructed in-place,
a forwarding reference `auto&& uid` (or `VId&& uid` with `std::forward`) may be appropriate. This
is a **secondary optimization** — the primary change should be `const&`.

Candidate sites:
- Multi-source algorithm overloads where a `std::initializer_list` or range of IDs is passed
- Adaptor pipe-syntax functions that forward to view constructors

### R5: `vertex_id_t<G>` Type Alias Must Remain a Value Type

The type alias `vertex_id_t<G>` itself (in `graph_cpo.hpp` and `edge_list.hpp`) should remain a
**value type** (i.e., the `remove_cvref_t` of the return). It represents the canonical value type
of vertex IDs, not a reference type. Code that needs `const&` semantics applies `const&` at the
use site.

If R3 changes cause `vertex_id(g, u)` to return a reference, the `vertex_id_t<G>` alias
definition should apply `std::remove_cvref_t`:

```cpp
template <typename G>
using vertex_id_t = std::remove_cvref_t<
    decltype(vertex_id(std::declval<G&>(), std::declval<vertex_t<G>>()))>;
```

This may already be the case for the edge_list definition but must be made consistent.

### R6: Introduce `raw_vertex_id_t<G>` and `vertex_id_store_t<G>` Aliases

Two new type aliases are needed alongside `vertex_id_t<G>`:

```cpp
// Preserves reference-ness from the CPO return. For vector-based graphs this is
// size_t (prvalue). For map-based graphs this is const std::string& (lvalue ref).
template <typename G>
using raw_vertex_id_t = decltype(vertex_id(std::declval<G&>(), std::declval<vertex_t<G>>()));

// Efficient storage type for algorithm internals. For integral IDs this is the
// value type itself. For reference-returning IDs this is a reference_wrapper,
// which is trivially copyable (pointer-sized) and implicitly converts back to
// const ref.
template <typename G>
using vertex_id_store_t = std::conditional_t<
    std::is_reference_v<raw_vertex_id_t<G>>,
    std::reference_wrapper<std::remove_reference_t<raw_vertex_id_t<G>>>,
    vertex_id_t<G>>;
```

The three aliases form a consistent family:

| Alias | Type for vector graph | Type for map graph | Use case |
|-------|----------------------|-------------------|----------|
| `vertex_id_t<G>` | `size_t` | `std::string` | Value type — parameters (`const&`), result containers, type constraints |
| `raw_vertex_id_t<G>` | `size_t` | `const std::string&` | Raw CPO return — used only in `conditional_t` and SFINAE |
| `vertex_id_store_t<G>` | `size_t` | `reference_wrapper<const string>` | Algorithm-internal queues, stacks, priority queues |

## Implementation Plan

### Phase 0: Type Aliases — Protect `vertex_id_t<G>`, Add `raw_vertex_id_t<G>` and `vertex_id_store_t<G>`

**Files:** `include/graph/adj_list/detail/graph_cpo.hpp`, `include/graph/edge_list/edge_list.hpp`,
and a common algorithm header (e.g., `include/graph/algorithm/common.hpp` or `traversal_common.hpp`).

1. Wrap both `vertex_id_t<G>` definitions in `std::remove_cvref_t<>` to ensure they remain value
   types regardless of whether underlying returns change to `decltype(auto)`.

2. Add `raw_vertex_id_t<G>` alongside `vertex_id_t<G>` in the same headers. This alias preserves
   the reference-ness of the CPO return:
   ```cpp
   template <typename G>
   using raw_vertex_id_t = decltype(vertex_id(std::declval<G&>(), std::declval<vertex_t<G>>()));
   ```

3. Add `vertex_id_store_t<G>` in a header visible to all algorithms:
   ```cpp
   template <typename G>
   using vertex_id_store_t = std::conditional_t<
       std::is_reference_v<raw_vertex_id_t<G>>,
       std::reference_wrapper<std::remove_reference_t<raw_vertex_id_t<G>>>,
       vertex_id_t<G>>;
   ```

**Risk:** Low. `remove_cvref_t` on a value type is a no-op. The new aliases are additive.

**Validation:** Build all presets. Run full test suite.

---

### Phase 1: Descriptor Return Types (`decltype(auto)`)

**Files:**
- `include/graph/adj_list/vertex_descriptor.hpp` — `vertex_id()`
- `include/graph/adj_list/edge_descriptor.hpp` — `source_id()`, `target_id()`

**Changes:**
1. Change `auto` → `decltype(auto)` on the three methods.
2. In the keyed-storage branch (map iterators), parenthesize
   return expressions to enable reference binding:
   ```cpp
   if constexpr (std::random_access_iterator<VertexIter>) {
       return storage_;                       // prvalue (integral) — no change
   } else {
       return (std::get<0>(*storage_));       // lvalue ref to key
   }
   ```
3. Verify that all callers handle both prvalue and lvalue returns correctly. The
   `vertex_id_t<G>` alias (Phase 0) strips references, so code using the alias as a local
   variable type will still copy — which is correct for storage.

**Risk:** Medium. `decltype(auto)` propagating a reference to a temporary is dangerous. Must audit
all sites where `vertex_id()` is called on a temporary descriptor. If any exist, the descriptor
must be named (bound to a variable) first.

**Validation:** Build all presets (especially with `-fsanitize=address`). Run full test suite
including `test_dynamic_graph_integration.cpp` which tests string vertex IDs.

---

### Phase 2: Concept and Trait Definitions

**Files:**
- `include/graph/adj_list/adjacency_list_concepts.hpp`
- `include/graph/adj_list/adjacency_list_traits.hpp`

**Changes in `adjacency_list_concepts.hpp`:**

Change requires-expression variables from `vertex_id_t<G> uid` to
`const vertex_id_t<G>& uid` in all concepts that only read the ID:

```cpp
// Before (vertex concept, ~L103):
concept vertex = requires(G&& g, vertex_t<G> u, vertex_id_t<G> uid) { ... };

// After:
concept vertex = requires(G&& g, vertex_t<G> u, const vertex_id_t<G>& uid) { ... };
```

Similarly for: `index_vertex_range`, `adjacency_list`, `bidirectional_adjacency_list`, etc.

**Changes in `adjacency_list_traits.hpp`:**

Change all trait detection concepts:
```cpp
// Before (~L32):
concept has_degree_uid_impl = requires(G&& g, vertex_id_t<G> uid) { ... };

// After:
concept has_degree_uid_impl = requires(G&& g, const vertex_id_t<G>& uid) { ... };
```

Similarly for: `has_find_vertex_impl`, `has_find_vertex_edge_uidvid_impl`,
`has_contains_edge`, etc.

**Risk:** Low-Medium. Changing concept constraints may cause overload resolution changes if
container member functions expect `vertex_id_type` by value rather than `const&`. Must verify
that all concrete container member functions that participate in concept satisfaction accept
`const vertex_id_type&` (they already do for `dynamic_graph`).

**Validation:** Build all presets. Run full test suite. Specifically verify concept satisfaction for
all graph container types (vector-based and map-based).

---

### Phase 3: View Factory Functions and View Constructors

**Files:**
- `include/graph/views/incidence.hpp` — ~14 factory overloads + 2 constructors
- `include/graph/views/neighbors.hpp` — ~14 factory overloads + 2 constructors
- `include/graph/views/bfs.hpp` — ~12 factory overloads + 4 constructors
- `include/graph/views/dfs.hpp` — ~12 factory overloads + 4 constructors
- `include/graph/views/transpose.hpp` — 2 friend functions

**Pattern for factory functions:**
```cpp
// Before:
template <adj_list::index_adjacency_list G>
[[nodiscard]] constexpr auto incidence(G& g, adj_list::vertex_id_t<G> uid) { ... }

// After:
template <adj_list::index_adjacency_list G>
[[nodiscard]] constexpr auto incidence(G& g, const adj_list::vertex_id_t<G>& uid) { ... }
```

**Pattern for view constructors:**
```cpp
// Before:
vertices_bfs_view(G& g, vertex_id_type seed, Alloc alloc = {})

// After:
vertices_bfs_view(G& g, const vertex_id_type& seed, Alloc alloc = {})
```

Note: The constructor body stores `seed` into a member or pushes it into a queue — that copy is
intentional and correct (R2). Only the parameter binding changes.

**Risk:** Low. These are direct signature changes. Internal usage copies the ID into queues/stacks
which remains by-value.

**Validation:** Build all presets. Run full test suite, view tests, and benchmark suite.

---

### Phase 4: Algorithm Signatures and Internal Storage

**Files:**
- `include/graph/algorithm/dijkstra_shortest_paths.hpp`
- `include/graph/algorithm/bellman_ford_shortest_paths.hpp`
- `include/graph/algorithm/breadth_first_search.hpp`
- `include/graph/algorithm/topological_sort.hpp`
- `include/graph/algorithm/mis.hpp`
- `include/graph/algorithm/connected_components.hpp`
- `include/graph/algorithm/label_propagation.hpp`

This phase has two sub-parts: (a) parameter signatures, and (b) internal storage via
`vertex_id_store_t<G>`.

#### Phase 4a: Parameter Signatures — `const vertex_id_t<G>&`

**Pattern for single-source algorithms:**
```cpp
// Before:
template <index_adjacency_list G, ...>
void dijkstra_shortest_paths(G&& g, vertex_id_t<G> source, ...) { ... }

// After:
template <index_adjacency_list G, ...>
void dijkstra_shortest_paths(G&& g, const vertex_id_t<G>& source, ...) { ... }
```

**Internal lambdas** (e.g. `relax_target` in Dijkstra/Bellman-Ford):
```cpp
// Before:
auto relax_target = [&](const edge_t<G>& e, vertex_id_t<G> uid, const weight_type& w_e) -> bool {

// After:
auto relax_target = [&](const edge_t<G>& e, const vertex_id_t<G>& uid, const weight_type& w_e) -> bool {
```

**Note on `breadth_first_search` single-source wrapper:**
```cpp
// Before:
void breadth_first_search(G&& g, vertex_id_t<G> source, Visitor&& visitor) {
    std::array<vertex_id_t<G>, 1> sources{source};  // copies into array — correct
    ...
}

// After:
void breadth_first_search(G&& g, const vertex_id_t<G>& source, Visitor&& visitor) {
    std::array<vertex_id_t<G>, 1> sources{source};  // still copies — correct
    ...
}
```

**`depth_first_search`** already uses `const vertex_id_t<G>&` — no change needed.

**`mis.hpp`** has `vertex_id_t<G> seed = 0` with a default argument. Change to:
```cpp
const vertex_id_t<G>& seed = vertex_id_t<G>{0}
```
Or, since a default of `0` implies integrality, this function may be exclusively for
`index_adjacency_list` and can be left as-is. Evaluate whether a const-ref default makes sense.

**Algorithms with loop variables** (`tc.hpp`, `articulation_points.hpp`,
`biconnected_components.hpp`): Loop variables like `for (vertex_id_t<G> uid = 0; ...)`
remain **by value** — they are local storage, not parameters.

#### Phase 4b: Internal Storage — `vertex_id_store_t<G>` with `reference_wrapper`

Change algorithm-local `using id_type = vertex_id_t<G>` to
`using id_type = vertex_id_store_t<G>`.

The graph **must be stable** while an algorithm runs (this is already an implicit precondition
for all traversal algorithms — iterators and edge references are invalidated by mutation). For
map-based graphs, vertex ID keys reside in stable map nodes. A `reference_wrapper` pointing at
such a key is valid for the entire algorithm lifetime. This avoids per-enqueue copies of
non-trivial types while remaining pointer-sized and trivially copyable.

**Concrete example — `dijkstra_shortest_paths` (multi-source):**

```cpp
// Before:
constexpr void dijkstra_shortest_paths(G&& g, const Sources& sources, ...) {
  using id_type       = vertex_id_t<G>;                    // size_t or string (by value)
  using distance_type = range_value_t<Distances>;
  using weight_type   = invoke_result_t<WF, ...>;

  auto relax_target = [&](const edge_t<G>& e, vertex_id_t<G> uid, ...) -> bool {
    const id_type vid = target_id(g, e);                    // copies string
    ...
  };

  const id_type N = static_cast<id_type>(num_vertices(g));

  auto qcompare = [&distances](id_type a, id_type b) {     // queue elements are strings
    return distances[...a] > distances[...b];
  };
  using Queue = std::priority_queue<id_type, std::vector<id_type>, decltype(qcompare)>;
  Queue queue(qcompare);
  ...
  queue.push(vid);                                         // copies string into queue
}

// After:
constexpr void dijkstra_shortest_paths(G&& g, const Sources& sources, ...) {
  using id_type       = vertex_id_store_t<G>;              // size_t or reference_wrapper<const string>
  using distance_type = range_value_t<Distances>;
  using weight_type   = invoke_result_t<WF, ...>;

  auto relax_target = [&](const edge_t<G>& e, const vertex_id_t<G>& uid, ...) -> bool {
    const id_type vid = target_id(g, e);                    // wraps ref to stable key (8 bytes)
    ...
  };

  const id_type N = static_cast<id_type>(num_vertices(g));

  auto qcompare = [&distances](id_type a, id_type b) {     // queue elements are 8-byte wrappers
    return distances[...a] > distances[...b];               // implicit conversion to const string&
  };
  using Queue = std::priority_queue<id_type, std::vector<id_type>, decltype(qcompare)>;
  Queue queue(qcompare);
  ...
  queue.push(vid);                                         // copies 8-byte wrapper, not string
}
```

**What changes at each use site inside an algorithm:**

| Internal use | Before (`vertex_id_t<G>`) | After (`vertex_id_store_t<G>`) |
|---|---|---|
| `id_type vid = target_id(g, e)` | Copies string | Wraps ref (8 bytes) — `target_id` returns `decltype(auto)` ref after Phase 1 |
| `queue.push(vid)` | Copies string into queue | Copies 8-byte wrapper |
| `const id_type uid = queue.top()` | Copies string out | Copies 8-byte wrapper |
| `qcompare(a, b)` | Compares strings by value | `reference_wrapper` implicitly converts to `const string&` |
| `predecessor[vid] = uid` | Copies string | Copies 8-byte wrapper (predecessor stores `id_type` too) |
| `visitor.on_discover_vertex(g, uid)` | Passes string by value | `reference_wrapper` converts to `const vertex_id_t<G>&` matching visitor concepts |
| `distances[static_cast<size_t>(uid)]` | Direct indexing | Same for integral; N/A for map graphs (separate concern) |

**Why this is safe:** `reference_wrapper` references the vertex ID key stored inside the graph's
map node. The graph is not mutated during algorithm execution (precondition already documented),
so the map nodes — and their keys — remain at stable addresses. The `reference_wrapper` is valid
from construction until the algorithm returns.

**Note for `index_adjacency_list`-constrained algorithms (Dijkstra, Bellman-Ford, etc.):**
Since `vertex_id_t<G>` is always integral for these, `is_reference_v<raw_vertex_id_t<G>>` is
`false`, and `vertex_id_store_t<G>` collapses to `vertex_id_t<G>` — zero overhead, identical
codegen. The `reference_wrapper` path is a zero-cost abstraction that only activates for future
algorithms relaxed to non-integral IDs.

**Risk:** Low for integral-constrained algorithms (conditional selects the value path — no
behavioral change). Medium for future non-integral algorithms that would be the first to exercise
the `reference_wrapper` path — requires careful testing with map-based graph containers.

**Validation:** Build all presets. Run full algorithm test suite. For Phase 4b specifically,
add a `static_assert` in each algorithm to verify the expected `id_type` resolution:
```cpp
static_assert(std::is_same_v<id_type, vertex_id_t<G>>);  // for index_adjacency_list algorithms
```
These guards can be relaxed when an algorithm is generalized to non-integral IDs.

---

### Phase 5: Adaptor / Pipe Syntax Functions

**Files:**
- `include/graph/views/adaptors.hpp`

These typically use template forwarding:
```cpp
adj_list::vertex_id_t<std::remove_cvref_t<G>>(std::forward<UID>(uid))
```

Review whether `UID` is already a forwarding reference and ensure the forwarding works correctly
with both lvalue (const ref) and rvalue vertex IDs. These may already be correct.

**Risk:** Low.

**Validation:** Build and test adaptor/pipe-syntax tests.

---

### Phase 6: Documentation and Examples

**Files:**
- `docs/user-guide/`, `docs/reference/`
- `examples/dijkstra_clrs_example.cpp`, `examples/basic_usage.cpp`

Update any API documentation or examples that show `vertex_id_t<G>` by value in function
signatures.

**Risk:** None (documentation only).

---

## Items Explicitly Out of Scope

1. **Internal local variables used as loop counters** — `for (vertex_id_t<G> uid = 0; ...)`
   remains by value. These are incremented/compared as values.

2. **Result containers returned to the caller** — `std::vector<vertex_id_t<G>>` for results
   like predecessor arrays or topological orderings must store owned values, since they outlive
   the algorithm call and possibly the graph itself.

3. **Algorithm concept constraints requiring `std::integral`** — Algorithms constrained to
   `index_adjacency_list` still require integral vertex IDs. The `const&` and
   `vertex_id_store_t<G>` changes are still beneficial for API consistency and future-proofing,
   but do not directly unlock non-integral algorithms. Relaxing algorithms to work with
   non-integral IDs is a separate effort that builds on this foundation.

4. **Move semantics for vertex IDs** — Using `vertex_id_t<G>&&` (rvalue reference) for
   sink parameters where the caller is done with the ID. The `reference_wrapper` pattern for
   internal storage and `const&` for parameters covers the important cases without the API
   complexity of rvalue reference overloads.

## Risk Summary

| Phase | Risk | Mitigation |
|-------|------|-----------|
| 0 — Type aliases | Low | `remove_cvref_t` is a no-op on value types; new aliases are additive |
| 1 — Descriptor returns | Medium | Audit temporary descriptor usage; sanitizer testing |
| 2 — Concepts/traits | Low-Medium | Verify concept satisfaction for all container types |
| 3 — Views | Low | Straightforward signature changes |
| 4a — Algorithm signatures | Low | Parameter-only changes; bodies unaffected |
| 4b — Algorithm `id_type` | Low (integral) / Medium (future non-integral) | `conditional_t` selects value path for current algorithms; `static_assert` guards |
| 5 — Adaptors | Low | Likely already correct |
| 6 — Documentation | None | — |

## Build/Test Strategy

Each phase should:
1. Make the changes
2. Build with **all presets** (`linux-gcc-debug`, `linux-gcc-release`, `linux-clang-debug`,
   `linux-clang-release`)
3. Run the **full test suite** (not just affected tests) to catch concept satisfaction regressions
4. Run with **AddressSanitizer** (especially Phase 1) to detect dangling references
5. Run **benchmarks** (especially Phase 3) to verify no performance regression

## Open Questions

1. **~~Should `vertex_id_t<G>` parameters use forwarding references instead of `const&`?~~**
   **Resolved.** The `reference_wrapper` pattern for internal storage eliminates the main
   motivation for forwarding references (avoiding copies into queues/stacks). Parameters use
   `const vertex_id_t<G>&`; internal storage uses `vertex_id_store_t<G>`. No forwarding
   references needed.

2. **CPO return type propagation.** If `vertex_descriptor::vertex_id()` returns `decltype(auto)`,
   and the CPO `vertex_id(g, u)` uses `-> decltype(auto)`, the CPO will return a reference
   for map-based graphs. **This is the desired semantics** — it is what enables
   `raw_vertex_id_t<G>` to detect reference-ness and select the `reference_wrapper` path.
   The reference is valid as long as the graph is alive, which is guaranteed during algorithm
   execution. The `vertex_id_t<G>` alias strips the reference (Phase 0) so code declaring
   local value variables still gets copy semantics.

3. **Default arguments.** Functions like `mis(g, ..., vertex_id_t<G> seed = 0)` use a default
   value. With `const vertex_id_t<G>& seed`, the default must be a materializable value:
   `const vertex_id_t<G>& seed = vertex_id_t<G>{}` or similar. For algorithms constrained to
   `index_adjacency_list`, keeping by-value for defaulted parameters may be pragmatic.

4. **Map-based property containers for non-integral vertex IDs.** When vertices are keyed by
   non-integral types (e.g., `std::string`), every algorithm property container — distances,
   predecessors, color maps, etc. — must also become map-based. Dijkstra currently requires
   `random_access_range` for `Distances` and `Predecessors` with `distances[static_cast<size_t>(uid)]`
   indexing. For map-based graphs this would become
   `std::unordered_map<vertex_id_t<G>, distance_type>` and
   `std::unordered_map<vertex_id_t<G>, vertex_id_t<G>>` respectively, with `distances[uid]` or
   `distances.at(uid)` access.

   This has cascading design implications beyond simple container substitution:

   - **Concept constraints change.** `random_access_range<Distances>` would need to be relaxed
     to something like an associative-container concept or a generic property-map concept (similar
     to BGL1's `ReadWritePropertyMap`). A property-map abstraction that unifies `vector`-indexing
     and `map`-lookup behind a single `get(map, key)` / `put(map, key, value)` interface would
     be the cleanest approach. This is the pattern BGL1 used and it scaled well.

   - **Initialization differs.** `std::vector<distance_type>` is pre-sized and filled with
     `infinite` in O(V). An `unordered_map` starts empty; vertices are inserted on first
     discovery. The algorithm must handle "key not present" as equivalent to "distance = infinite"
     (defaulting behavior), or pre-populate from the vertex range.

   - **Predecessor storage.** For `predecessor[vid] = uid`, if both `predecessor` and the queue
     use `vertex_id_store_t<G>` (`reference_wrapper`), predecessor entries are 8-byte wrappers
     referencing stable graph keys — efficient. But if predecessors are a result returned to the
     caller (outliving the algorithm), they must store owned `vertex_id_t<G>` values, not
     `reference_wrapper`s. This means the predecessor map's value type should be `vertex_id_t<G>`
     (the value type), even though internal queue elements are `vertex_id_store_t<G>`. The
     assignment `predecessor[vid] = uid` where `uid` is a `reference_wrapper` works because
     `reference_wrapper<const string>` implicitly converts to `const string&`, which can
     initialize a `string` value in the map.

   - **Sort-order for priority queue.** `std::priority_queue` with `reference_wrapper` elements
     works if the comparator accepts `const vertex_id_t<G>&` (which `reference_wrapper` converts
     to). But if the comparator needs to index into distances, it must use the same map-based
     lookup: `distances[a.get()]` rather than `distances[static_cast<size_t>(a)]`.

   **This is out of scope for the current effort** but is the natural next step after the
   `const&` / `reference_wrapper` foundation is in place. A suggested approach for that future
   work is discussed below, along with a comparison to BGL1's property maps.

   **Property maps vs. function objects for algorithm parameters:**

   BGL1 used a `PropertyMap` concept (`get(pm, key)` / `put(pm, key, value)`) as the universal
   abstraction for all algorithm data — distances, predecessors, color maps, edge weights, vertex
   indices. This was powerful but widely criticized for its complexity: users had to understand
   property map categories (`ReadablePropertyMap`, `WritablePropertyMap`,
   `ReadWritePropertyMap`, `LvaluePropertyMap`), tag-dispatch for bundled vs. external properties,
   and the `property_traits` machinery. The learning curve was steep even for common cases.

   The modern C++ standard library has established a different pattern: **function objects**
   (callables) for customization. `std::sort` takes a comparator. `std::transform` takes a
   unary operation. `std::accumulate` takes a binary operation. Graph-v3 already follows this
   idiom — Dijkstra takes `WF` (weight function), `Compare`, and `Combine` as callable template
   parameters. This is familiar to any C++ developer.

   The question is whether distances, predecessors, and color maps should follow the property-map
   pattern or the function-object pattern. Arguments for each:

   **Arguments for a property-map concept (BGL1-style `get`/`put`):**
   - Unifies `vector[i]`, `map[key]`, and computed-on-the-fly access behind one interface.
   - Natural for read-write properties: `get(dist, uid)` and `put(dist, uid, value)` mirror
     the dual-direction access that algorithms actually need.
   - Enables static dispatch on property-map category (readable vs. writable vs. lvalue) for
     algorithm optimizations.
   - Predecessors, distances, and color maps are conceptually "associated data indexed by vertex"
     — a property map is the natural abstraction for that.
   - If a `_null_predecessors` sentinel is used to skip predecessor tracking, a property-map
     concept makes this natural: `null_property_map` satisfies the concept but `put()` is a no-op.

   **Arguments against property maps / for function objects or direct container concepts:**
   - **Complexity.** BGL1's property maps are the single most-cited barrier to adoption. Adding a
     property-map layer to graph-v3 risks repeating that mistake.
   - **Standard practice.** The standard library does not use property maps. Algorithms take
     ranges and callables. Developers expect `vector<int>& distances`, not
     `WritablePropertyMap<VId, int> distances`.
   - **`operator[]` already works.** Both `vector` and `unordered_map` support `operator[]` with
     the appropriate key type. A concept requiring `operator[]` (or a subscriptable/indexable
     concept) achieves the same unification as `get`/`put` without introducing new vocabulary.
   - **Function objects compose better.** A weight function `WF(g, edge) -> weight` can wrap
     an edge property, compute on-the-fly, or look up in an external array. No property-map
     adapter needed. The same pattern can work for distance access if needed:
     `dist_get(uid) -> distance`, `dist_put(uid, value)`. But this is arguably just reinventing
     property maps with lambda syntax.
   - **Ranges + projections.** C++20 ranges use projections (`std::ranges::sort(r, comp, proj)`)
     as the standard way to customize element access. This could be extended to graph properties.

   **Recommendation for graph-v3:**

   A minimal approach that avoids BGL1's complexity while solving the indexing problem:

   1. **Keep the current approach for the integral path.** `random_access_range<Distances>` with
      `distances[uid]` is simple, efficient, and familiar. No change needed for vector-based
      graphs.
   2. **For the non-integral path, require `operator[]` on the container.** Both `vector` and
      `unordered_map` support `distances[uid]`. A concept like:
      ```cpp
      template <typename M, typename Key, typename Value>
      concept vertex_property_map = requires(M& m, const Key& k) {
          { m[k] } -> std::convertible_to<Value&>;
      };
      ```
      This is not a BGL1-style property map — it is a simple subscriptable concept that both
      `vector<T>` and `unordered_map<Key, T>` satisfy naturally. No `get`/`put` free functions,
      no property-map categories, no traits machinery. Just `m[k]`.
   3. **`_null_predecessors` remains a special type** that satisfies the concept with no-op
      `operator[]` (as it effectively does today via `_null_range_type`).
   4. **Edge weight stays as a function object** (`WF`), unchanged. This is the right abstraction
      for "compute a value from an edge" and is consistent with standard library practice.

   This avoids the BGL1 property-map abstraction entirely while still supporting map-based
   containers. The key insight is that `operator[]` is already the universal access syntax
   that both `vector` and `unordered_map` share — no additional abstraction layer is needed.

5. **`reference_wrapper` and `std::hash`.** If an algorithm uses `std::unordered_set<id_type>` for
   visited tracking, `std::hash<reference_wrapper<const string>>` is not provided by default.
   A custom hasher delegating to `std::hash<vertex_id_t<G>>` via `.get()` would be needed.
   Alternatively, the visited set could use `vertex_id_t<G>` (value type) directly, accepting
   the copy cost for set insertion in exchange for simpler hashing.
