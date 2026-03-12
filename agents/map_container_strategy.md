# Map-Based Vertex Container Strategy

## Goal

Expand the graph library's concept hierarchy, algorithm infrastructure, and data structure
support so that algorithms work with both **index-based** (vector, deque) and **key-based**
(std::map, std::unordered_map) vertex containers. Currently every algorithm requires
`index_adjacency_list`, which gates on `std::integral<vertex_id_t<G>>` and
`std::integral<storage_type>`. Map-based graphs already compile at the container layer but
cannot be used with any algorithm.

---

## Current State

### Concept Hierarchy

```
vertex_range          ← forward + sized range of vertex descriptors
  └─ index_vertex_range  ← adds: integral vertex_id_t, integral storage_type

adjacency_list        ← vertex_range + out_edge_range
  └─ index_adjacency_list  ← adds: index_vertex_range

bidirectional_adjacency_list  ← adjacency_list + in_edge_range
  └─ index_bidirectional_adjacency_list  ← adds: index_vertex_range
```

Every algorithm constrains `G` with `index_adjacency_list` or
`index_bidirectional_adjacency_list`. No algorithm uses the un-prefixed
`adjacency_list` concept.

### Vertex ID Types

| Container          | `storage_type`          | `vertex_id_t<G>` | `raw_vertex_id_t<G>` |
|--------------------|-------------------------|-------------------|-----------------------|
| `std::vector<T>`   | `size_t`                | `size_t`          | `size_t`              |
| `std::deque<T>`    | `size_t`                | `size_t`          | `size_t`              |
| `std::map<K, V>`   | `map::iterator`         | `K`               | `const K&`            |
| `std::unordered_map<K, V>` | `unordered_map::iterator` | `K`       | `const K&`            |

For index-based graphs, `vertex_id_t<G>` is `size_t` and IDs form a dense range `[0, N)`.
For map-based graphs, `vertex_id_t<G>` is the key type (`VId` template parameter, default
`uint32_t`), and IDs are **sparse** — not necessarily contiguous.

### Algorithm Index Assumptions

All 14 algorithm files exhibit three categories of index dependency:

| Category | Pattern | Files |
|----------|---------|-------|
| **Internal state arrays** | `vector<bool> visited(N)`, `vector<Color> color(N)`, `vector<size_t> disc(N)`, etc. sized by `num_vertices(g)` and indexed by `arr[uid]` | 13 of 14 |
| **Parameter arrays** | `random_access_range Distances`, `random_access_range Predecessors`, `random_access_range Component` — accessed as `distances[uid]` | 7 of 14 |
| **Sequential iteration** | `for (id_type uid = 0; uid < N; ++uid)` | 6 of 14 |
| **Sequential fill** | `std::iota(pred.begin(), pred.end(), 0)` | 3 of 14 |
| **`static_cast<size_t>(uid)`** | Explicit cast to index into arrays | 3 of 14 |

### Existing Map-Based Infrastructure

The container layer already supports map-based graphs:

- **7 map-based traits**: `mofl`, `mol`, `mov`, `mod`, `mos`, `mous`, `mom` — all use
  `std::map<VId, vertex_type>` for vertices
- **`find_vertex` CPO**: Has an `_associative` path that calls `g.find(uid)` — O(log n)
  for `std::map`
- **`vertex_descriptor`**: Already branches on iterator category — stores iterator for
  bidirectional, stores index for random-access
- **`vertex_id_store_t<G>`**: In `traversal_common.hpp`, conditionally uses
  `reference_wrapper` for non-trivial IDs — shows awareness of the duality

---

## Design Strategy

### 1. New Concept: `mapped_vertex_range`

Add a concept that captures key-based vertex access — the map analog of
`index_vertex_range`:

```cpp
/// Satisfied by graphs whose vertex IDs are hashable keys (map/unordered_map).
/// Vertex IDs are sparse; lookup is via find_vertex(g, uid).
template <class G>
concept mapped_vertex_range =
    !index_vertex_range<G> &&               // mutually exclusive with index
    requires(G& g) {
      { vertices(g) } -> vertex_range<G>;
    } &&
    requires(G& g, const vertex_id_t<G>& uid) {
      find_vertex(g, uid);                  // key-based lookup must exist
    };
```

And the composed concepts:

```cpp
template <class G>
concept mapped_adjacency_list = adjacency_list<G> && mapped_vertex_range<G>;

template <class G>
concept mapped_bidirectional_adjacency_list =
    bidirectional_adjacency_list<G> && mapped_vertex_range<G>;
```

The hierarchy becomes:

```
vertex_range
  ├─ index_vertex_range      (integral IDs, dense [0,N))
  └─ mapped_vertex_range     (key-based IDs, sparse)

adjacency_list
  ├─ index_adjacency_list
  └─ mapped_adjacency_list

bidirectional_adjacency_list
  ├─ index_bidirectional_adjacency_list
  └─ mapped_bidirectional_adjacency_list
```

### 2. Vertex Property Map: `vertex_map<G, T>`

The core abstraction that allows algorithms to store per-vertex data regardless of
container type. This replaces the hard-coded `vector<T>` + `arr[uid]` pattern.

```cpp
/// A per-vertex associative container: vector<T> for index graphs, 
/// unordered_map<vertex_id_t<G>, T> for mapped graphs.
template <class G, class T>
using vertex_map = std::conditional_t<
    index_vertex_range<G>,
    std::vector<T>,
    std::unordered_map<vertex_id_t<G>, T>
>;
```

#### Access Pattern

Algorithms currently write `distances[static_cast<size_t>(uid)]`. With `vertex_map`, we
need a uniform access function:

```cpp
/// O(1) for vector, O(1) amortized for unordered_map.
template <class Map, class Key>
constexpr auto& vertex_map_get(Map& m, const Key& uid) {
  if constexpr (std::ranges::random_access_range<Map>) {
    return m[static_cast<size_t>(uid)];
  } else {
    return m[uid];  // unordered_map::operator[]
  }
}
```

Or, preferably, a lighter approach using `operator[]` directly — since `size_t` already
works as an index for `vector` and the key type works for `unordered_map`, we can simply
write `distances[uid]` if the map's key type matches `vertex_id_t<G>`. This may require
a thin wrapper type rather than a bare type alias, to ensure `operator[]` works uniformly.

#### Read vs Write: Guarding Against Spurious Insertion

For `unordered_map`, `operator[]` **inserts** a default-constructed entry when the key is
absent. This is correct for write-first patterns (`distances[uid] = 0`) but dangerous for
read/test patterns (`if (visited[uid])`) — it would silently populate the map with every
queried vertex, wasting memory and masking logic errors.

The correct access pattern depends on the algorithm's intent:

| Intent | vector (index) | unordered_map (mapped) | Helper |
|--------|---------------|----------------------|--------|
| **Write (set value)** | `m[uid] = val` | `m[uid] = val` | `operator[]` — same for both |
| **Read (known present)** | `m[uid]` | `m[uid]` | `operator[]` — safe only after eager init |
| **Test existence** | `m[uid]` (always valid) | `m.contains(uid)` | `vertex_map_contains(m, uid)` |
| **Read-or-default** | `m[uid]` | `m.contains(uid) ? m[uid] : default_val` | `vertex_map_get(m, uid, default_val)` |

This is **situational** — each algorithm must choose the right pattern:

- **Eager-initialized maps** (e.g. `make_vertex_map(g, false)` pre-populates all keys):
  `operator[]` is safe for both reads and writes since every vertex has an entry.
  Appropriate for distances, colors, component labels — where every vertex needs an
  initial value anyway.

- **Lazy / sparse maps** (e.g. `visited` where only discovered vertices are inserted):
  Must use `contains()` to test and `insert()`/`emplace()` to write. This is more
  memory-efficient when the algorithm only touches a fraction of vertices (e.g.
  single-source BFS on a large graph).

Recommended access helpers:

```cpp
/// Test whether a vertex ID has an entry. Always true for vector (index graphs).
template <class Map, class Key>
constexpr bool vertex_map_contains(const Map& m, const Key& uid) {
  if constexpr (std::ranges::random_access_range<Map>) {
    return true;  // vector: all indices in [0, size) are valid
  } else {
    return m.contains(uid);
  }
}

/// Read with a default fallback. No insertion for unordered_map.
template <class Map, class Key, class T>
constexpr auto vertex_map_get(const Map& m, const Key& uid, const T& default_val) {
  if constexpr (std::ranges::random_access_range<Map>) {
    return m[static_cast<size_t>(uid)];
  } else {
    auto it = m.find(uid);
    return it != m.end() ? it->second : default_val;
  }
}
```

**Algorithm-specific guidance:**

| Algorithm | Map Pattern | Rationale |
|-----------|------------|-----------|
| BFS `visited` | **Lazy + `contains()`** | Only discovered vertices need tracking; single-source may touch a small fraction |
| DFS `color` | **Lazy + default=White** | Undiscovered vertices are implicitly White; only touched vertices get Gray/Black entries |
| Dijkstra `distances` | **Lazy + default=∞** | Absent key means infinite distance; entries created only as vertices are discovered/relaxed |
| Dijkstra `predecessor` | **Lazy** | Entries created during relaxation; absent means "no predecessor" (unreached) |
| Dijkstra priority queue | N/A | Stores IDs, not a property map |
| Connected components `component` | **Eager** | User-provided, all vertices need a label |
| Articulation points `disc`/`low` | **Eager** | Iterates all vertices in outer loop |
| Label propagation `label` | **Eager** | User-provided, all vertices need a label |
| MIS `removed` | **Lazy + `contains()`** | Only removed vertices need tracking |

**Key principle for mapped graphs:** If an algorithm's semantics assign a meaningful
default to absent vertices (infinity for distances, White for DFS color, false for
visited), the map can start **empty** and treat absence as that default. This avoids
O(V) initialization cost and O(V) memory — critical when a single-source algorithm on
a large mapped graph may only reach a small fraction of vertices.

#### Factory Functions

Three overloads cover the eager/lazy spectrum:

```cpp
/// Eager: create a vertex_map with every vertex pre-populated to init_value.
/// For index graphs: vector<T>(N, init_value).  O(V) always.
/// For mapped graphs: unordered_map with all keys inserted.  O(V).
/// Use when the algorithm reads all vertices before writing (e.g. component labels).
template <class G, class T>
constexpr auto make_vertex_map(const G& g, const T& init_value) {
  if constexpr (index_vertex_range<G>) {
    return std::vector<T>(num_vertices(g), init_value);
  } else {
    std::unordered_map<vertex_id_t<G>, T> m;
    m.reserve(num_vertices(g));
    for (auto&& [uid, u] : graph::adj_list::views::vertexlist(g)) {
      m[uid] = init_value;
    }
    return m;
  }
}

/// Lazy: create an empty vertex_map with capacity hint.
/// For index graphs: vector<T>(N) with default-constructed values (dense, O(V)).
/// For mapped graphs: empty unordered_map with reserved buckets (O(1) until use).
/// Use with vertex_map_get(m, uid, default_val) when absence has a semantic meaning
/// (e.g. infinity for distances, White for DFS color, false for visited).
template <class G, class T>
constexpr auto make_vertex_map(const G& g) {
  if constexpr (index_vertex_range<G>) {
    return std::vector<T>(num_vertices(g));
  } else {
    std::unordered_map<vertex_id_t<G>, T> m;
    m.reserve(num_vertices(g));
    return m;
  }
}
```

**Note:** For index graphs, both overloads are O(V) — vectors must be sized upfront.
The lazy advantage is exclusively for mapped graphs, where `make_vertex_map<G, T>(g)`
returns an empty map and entries are created only as the algorithm discovers vertices.

### 3. Algorithm Generalization Pattern

#### Step 1: Dual-Concept Constraint

Each algorithm needs to accept both `index_adjacency_list` and `mapped_adjacency_list`.

**Decision: Separate overloads (Option B)**

Keep the existing `index_adjacency_list` overloads **unchanged** — 100% backward
compatible, no risk to existing callers. Add parallel overloads for `mapped_adjacency_list`
where the property map parameters use a `vertex_id_map` concept instead of
`random_access_range`:

```cpp
// Existing — unchanged, Distances is random_access_range
template <index_adjacency_list G, random_access_range Distances, ...>
void dijkstra_shortest_paths(G&&, Sources&, Distances&, ...);

// New — Distances satisfies vertex_id_map
template <mapped_adjacency_list G, class Distances, class Predecessors, ...>
requires vertex_id_map<Distances, G> && ...
void dijkstra_shortest_paths(G&&, Sources&, Distances&, ...);
```

Both overloads can share a common implementation function (private `_impl`) that uses
`if constexpr (index_vertex_range<G>)` for the few places that genuinely differ
(e.g. `vertex_map_get` vs direct `operator[]`). The public overloads are thin wrappers
that dispatch to the shared impl.

For algorithms with **no user property map parameters** (BFS, DFS, topological sort,
triangle count), the two overloads are identical except for the concept constraint on `G`,
so they can be merged into a single template constrained with `adjacency_list<G>`
if preferred — the separate-overload rule applies specifically to algorithms that
accept `Distances`, `Predecessors`, `Component`, `Label`, etc.

**Future enhancement:** Once the separate overloads are stable, consider unifying them
into a single template per algorithm using a `vertex_property_map<M, G>` concept that
both `vector<T>` (index) and `unordered_map<K,T>` (mapped) satisfy. This would eliminate
the overload duplication while preserving type safety. Deferred to avoid complexity
during the initial migration.

#### Step 2: Replace Internal State Arrays

Transform every internal `vector<T>(N, init)` + `arr[uid]` into
`make_vertex_map<G, T>(g, init)` + `vmap[uid]`:

**Before (BFS):**
```cpp
std::vector<bool> visited(num_vertices(g), false);
// ...
visited[uid] = true;
```

**After (BFS):**
```cpp
auto visited = make_vertex_map<G, bool>(g, false);
// ... 
visited[uid] = true;   // works for both vector and unordered_map
```

Note: `vector<bool>` is a special case — it uses proxy references and bit-packing. For
unordered_map we'd use `unordered_map<VId, bool>`. The `make_vertex_map` approach
handles this naturally since `conditional_t` picks the right type.

#### Step 3: Replace Sequential Loops

**Before:**
```cpp
for (id_type uid = 0; uid < N; ++uid) { ... }
```

**After:**
```cpp
for (auto&& [uid, u] : views::vertexlist(g)) { ... }
```

Most algorithms already use `views::vertexlist` or `views::basic_vertexlist` for outer
loops. The `for (uid = 0; uid < N)` pattern appears in 6 files and must be converted.

#### Step 4: Replace `std::iota` for Predecessors

**Before:**
```cpp
std::iota(predecessors.begin(), predecessors.end(), 0);
```

**After:**
```cpp
for (auto&& [uid, u] : views::vertexlist(g)) {
  predecessors[uid] = uid;  // each vertex is its own predecessor
}
```

#### Step 5: Replace `static_cast<size_t>(uid)`

For index graphs, `uid` is already `size_t`, so the cast is a no-op. For map graphs,
`uid` is the key type and should be used directly. The `vertex_map` abstraction makes
casts unnecessary:

**Before:** `distances[static_cast<size_t>(uid)]`  
**After:** `distances[uid]`

### 4. Distances & Predecessors Parameter Types

Currently `Distances` and `Predecessors` are constrained as `random_access_range`. For
map-based graphs, they'll be `unordered_map<vertex_id_t<G>, Distance>` or similar.

**Decision: `vertex_id_map` concept for mapped overloads**

The index overloads keep `random_access_range` unchanged. The new mapped overloads
use this concept:

```cpp
/// Satisfied by any associative container that maps vertex IDs to values.
template <class M, class G>
concept vertex_id_map =
    requires(M& m, const vertex_id_t<G>& uid) {
      { m[uid] } -> std::convertible_to<typename M::mapped_type&>;
    };
```

`unordered_map<VId, T>` satisfies this. Users can also pass custom property map types
as long as they support `operator[vertex_id_t<G>]`.

### 5. Vertex ID Storage: `vertex_id_store_t<G>`

Algorithms store vertex IDs in queues, stacks, vectors, and other transient containers.
For index-based graphs `vertex_id_t<G>` is `size_t` — trivially copyable. But for
map-based graphs it can be an expensive type (e.g. `std::string`). The existing
`vertex_id_store_t<G>` in `traversal_common.hpp` already solves this:

```cpp
template <typename G>
using vertex_id_store_t = std::conditional_t<
    std::is_reference_v<adj_list::raw_vertex_id_t<G>>,
    std::reference_wrapper<std::remove_reference_t<adj_list::raw_vertex_id_t<G>>>,
    adj_list::vertex_id_t<G>>;
```

- **Index graphs:** `vertex_id_store_t<G>` = `size_t` (same as `vertex_id_t<G>`, zero cost)
- **Map graphs:** `vertex_id_store_t<G>` = `reference_wrapper<const K>` (8 bytes, trivially
  copyable, references the stable key in the map node — no copy of the key itself)

Every algorithm that stores vertex IDs in transient containers must use
`vertex_id_store_t<G>` instead of `vertex_id_t<G>`:

| Algorithm | Current ID Storage | Change To |
|-----------|-------------------|-----------|
| BFS | `std::queue<id_type>` | `std::queue<vertex_id_store_t<G>>` |
| DFS | stack frame with `vertex_id` field | field type → `vertex_id_store_t<G>` |
| Dijkstra | `std::priority_queue<id_type>` | `std::priority_queue<vertex_id_store_t<G>>` |
| Bellman-Ford | iterates `id_type` loop vars | loop vars → `vertex_id_store_t<G>` |
| Topological sort | `std::vector<id_type> finish_order` | `std::vector<vertex_id_store_t<G>>` |
| Connected components | `std::vector<vertex_id_t<G>> order` | `std::vector<vertex_id_store_t<G>>` |
| Label propagation | `std::vector<vertex_id_t<G>> order` | `std::vector<vertex_id_store_t<G>>` |

**Note:** `reference_wrapper<const K>` implicitly converts to `const K&`, so comparisons
and use as map keys work transparently. The priority queue comparator, `unordered_map`
lookups, and `operator[]` on vertex maps all accept it without casts.

### 6. Priority Queue Adaptation

Dijkstra uses `std::priority_queue` with `id_type` elements compared via
`distances[static_cast<size_t>(lhs)] > distances[static_cast<size_t>(rhs)]`.

For map-based graphs, the queue stores `vertex_id_store_t<G>` values (cheap reference
wrappers). The comparator changes to:

```cpp
auto cmp = [&distances](const auto& lhs, const auto& rhs) {
  return distances[lhs] > distances[rhs];
};
```

No structural change needed — drop `static_cast<size_t>()` and use `vertex_id_store_t<G>`
as the queue element type.

### 7. `_null_range_type` Adaptation

The `_null_predecessors` type currently derives from `vector<size_t>`. For mapped graphs,
it should be detected at compile time so that predecessor tracking is skipped entirely.
The algorithm already checks `if constexpr (is_same_v<Predecessors, _null_range_type>)`
(or similar).

**Strategy:** Make `_null_range_type` satisfy both the `random_access_range` concept (as
now) and any new `vertex_id_map` concept, or detect it with a simple type trait:

```cpp
template <class T>
inline constexpr bool is_null_range_v = std::is_same_v<std::remove_cvref_t<T>, _null_range_type>;
```

Algorithms use `if constexpr (is_null_range_v<Predecessors>)` to skip predecessor writes.

### 8. Hashability Requirement for Map-Based Graphs

`unordered_map<vertex_id_t<G>, T>` requires that `vertex_id_t<G>` is hashable. For
`std::map`-based graphs where `VId` might be a custom type, we need:

```cpp
template <class G>
concept hashable_vertex_id = requires(const vertex_id_t<G>& uid) {
  { std::hash<vertex_id_t<G>>{}(uid) } -> std::convertible_to<size_t>;
};
```

This should be part of the `mapped_vertex_range` concept or required by `make_vertex_map`.

### 9. `init_shortest_paths` Generalization

The utility function in `traversal_common.hpp` needs dual-path support.

For **index graphs**, the current behavior is preserved: fill the pre-sized vector with
infinity values and iota-fill predecessors.

For **mapped graphs**, initialization can be a **no-op** if the algorithm treats absent
keys as "infinite distance" / "no predecessor":

```cpp
template <class G, class Distances>
constexpr void init_shortest_paths(const G& g, Distances& distances) {
  if constexpr (std::ranges::random_access_range<Distances>) {
    // Index graph: fill the pre-sized vector
    std::ranges::fill(distances, shortest_path_infinite_distance<range_value_t<Distances>>());
  } else {
    // Mapped graph: if distances map is empty (lazy pattern), nothing to fill —
    // the algorithm uses vertex_map_get(distances, uid, infinity) for reads.
    // If the caller pre-populated keys, set them all to infinity.
    for (auto& [key, val] : distances) {
      val = shortest_path_infinite_distance<typename Distances::mapped_type>();
    }
  }
}

template <class G, class Distances, class Predecessors>
constexpr void init_shortest_paths(const G& g, Distances& distances, Predecessors& predecessors) {
  init_shortest_paths<G>(g, distances);
  if constexpr (is_null_range_v<Predecessors>) {
    // No-op: predecessor tracking disabled
  } else if constexpr (std::ranges::random_access_range<Predecessors>) {
    // Index graph: iota-fill [0, 1, 2, ...]
    std::iota(predecessors.begin(), predecessors.end(), 0);
  } else {
    // Mapped graph: leave empty — absent key means "self-predecessor" (unreached).
    // The algorithm inserts entries only for vertices whose predecessor changes.
    // Callers can check: if (!vertex_map_contains(pred, uid)) => uid is a root/unreached.
  }
}
```

This means for a mapped graph, the typical call sequence is:
```cpp
auto distances   = make_vertex_map<G, Distance>(g);  // empty map, O(1)
auto predecessor = make_vertex_map<G, vertex_id_store_t<G>>(g);  // empty map, O(1)
init_shortest_paths<G>(g, distances, predecessor);  // no-op for empty maps
// Algorithm populates entries only for reached vertices.
```

---

## Transition Strategy: Preserving Existing Implementations

Before generalizing any algorithm, copy the current index-only implementation to a
reference location so it can be reviewed side-by-side with the new generalized code.

**Approach:** For each algorithm file being generalized, copy the original to
`include/graph/algorithm/index/`:

```
include/graph/algorithm/
  breadth_first_search.hpp          ← generalized (new)
  depth_first_search.hpp            ← generalized (new)
  dijkstra_shortest_paths.hpp       ← generalized (new)
  ...
  index/                            ← originals preserved for reference
    breadth_first_search.hpp
    depth_first_search.hpp
    dijkstra_shortest_paths.hpp
    ...
```

**Workflow per algorithm:**
1. Copy `algorithm/foo.hpp` → `algorithm/index/foo.hpp` (unchanged original)
2. Modify `algorithm/foo.hpp` in place to support both index and mapped graphs
3. Verify all existing index-based tests still pass (backward compatibility)
4. Add new tests with map-based graphs
5. Review diff between `algorithm/foo.hpp` and `algorithm/index/foo.hpp` to verify
   the generalization is correct and no index-graph performance was lost

The `index/` directory is a **temporary review aid** — once all algorithms are
generalized and verified, it can be removed. It is not included in the public API
or installed headers.

---

## Implementation Plan

Each phase below is a self-contained unit of work that compiles, passes all existing tests,
and can be committed independently. Phases are ordered so that each builds on the previous
one. Algorithm conversions within a phase are **one-at-a-time** — each algorithm is a
separate sub-task for review before proceeding to the next.

### Notation

- **Existing tests** = the 4343+ tests in `tests/algorithms/` that use index-based graphs.
  These must continue to pass after every sub-task. Run `ctest` after each change.
- **Preserve** = copy the original file to `include/graph/algorithm/index/` before editing.
- **Review gate** = stop and review the diff before starting the next sub-task.

---

### Phase 0: Preparation (no code changes)

**Goal:** Set up the reference directory and verify the starting baseline.

| Step | Action | Verification |
|------|--------|-------------|
| 0.1 | Run full test suite, confirm all tests pass | `ctest` — all green |
| 0.2 | Create `include/graph/algorithm/index/` directory | Directory exists |
| 0.3 | Copy all 14 algorithm `.hpp` files to `index/` | Files are byte-identical copies |
| 0.4 | Commit: `"chore: snapshot index-only algorithm implementations for reference"` | Clean commit on `mapped` branch |

**Files created:**
```
include/graph/algorithm/index/
  traversal_common.hpp
  breadth_first_search.hpp
  depth_first_search.hpp
  topological_sort.hpp
  dijkstra_shortest_paths.hpp
  bellman_ford_shortest_paths.hpp
  connected_components.hpp
  articulation_points.hpp
  biconnected_components.hpp
  label_propagation.hpp
  mis.hpp
  jaccard.hpp
  triangle_count.hpp
  mst.hpp
```

---

### Phase 1: Concepts & Vertex Map Infrastructure

**Goal:** Add new concepts and the `vertex_map` utility. No algorithm files touched.
All existing tests must still pass (the new code is additive only).

| Step | Action | Verification |
|------|--------|-------------|
| 1.1 | Add `mapped_vertex_range`, `mapped_adjacency_list`, `mapped_bidirectional_adjacency_list` concepts to `adjacency_list_concepts.hpp` | Compiles, existing tests pass |
| 1.2 | Create `include/graph/adj_list/vertex_map.hpp` with: `vertex_map<G,T>` alias, `make_vertex_map` (eager + lazy overloads), `vertex_map_contains`, `vertex_map_get` | Compiles |
| 1.3 | Add `is_null_range_v<T>` trait to `traversal_common.hpp` | Compiles, existing tests pass |
| 1.4 | Create `tests/common/map_graph_fixtures.hpp` with helper functions that build small map-based graphs (`mov_graph_traits`, `mol_graph_traits`) matching the same topologies as existing fixtures (e.g. the CLRS Dijkstra graph, a simple BFS tree) | Compiles |
| 1.5 | Create `tests/adj_list/test_vertex_map.cpp` — unit tests for `vertex_map`, `make_vertex_map`, `vertex_map_contains`, `vertex_map_get` on both index and mapped graph types | All new + existing tests pass |
| 1.6 | Create `tests/adj_list/test_mapped_concepts.cpp` — static_assert tests verifying: map-based graphs satisfy `mapped_adjacency_list` and `adjacency_list` but NOT `index_adjacency_list`; index-based graphs satisfy `index_adjacency_list` but NOT `mapped_vertex_range` | All tests pass |
| 1.7 | Commit: `"feat: add mapped_adjacency_list concepts and vertex_map infrastructure"` | Clean commit |

**Review gate:** Confirm concept mutual exclusivity and vertex_map behavior before
proceeding to algorithm changes.

---

### Phase 2: Traversal Algorithms (one at a time)

**Goal:** Generalize BFS, DFS, and topological sort. These are the simplest algorithms
(only internal state arrays, no user-supplied property map parameters).

Each sub-task follows this workflow:
1. Edit `algorithm/foo.hpp` — relax concept from `index_adjacency_list` to `adjacency_list`,
   replace internal `vector<T>` with `make_vertex_map`, replace `vertex_id_t<G>` in
   containers with `vertex_id_store_t<G>`, replace any `for(uid=0;uid<N)` loops.
2. Run existing tests — must all pass (backward compatibility).
3. Add new test section in existing test file using `SPARSE_VERTEX_TYPES`.
4. Commit, then review diff against `algorithm/index/foo.hpp`.

| Step | Algorithm | Key Changes | Test File |
|------|-----------|------------|-----------|
| 2.1 | `breadth_first_search` | `vector<bool> visited` → lazy `vertex_map<G,bool>` + `contains()`; queue element → `vertex_id_store_t<G>` | `test_breadth_first_search.cpp` |
| 2.2 | **Review gate** | Review diff of BFS against `index/breadth_first_search.hpp` | — |
| 2.3 | `depth_first_search` | `vector<Color> color` → lazy `vertex_map<G,uint8_t>` + default=White; stack frame ID → `vertex_id_store_t<G>` | `test_depth_first_search.cpp` |
| 2.4 | **Review gate** | Review diff of DFS | — |
| 2.5 | `topological_sort` | Same color pattern as DFS; `vector<id_type> finish_order` → `vector<vertex_id_store_t<G>>` | `test_topological_sort.cpp` |
| 2.6 | **Review gate** | Review diff of topological sort | — |
| 2.7 | Commit: `"feat: generalize BFS, DFS, topological_sort for mapped graphs"` | All tests pass |

---

### Phase 3: Shortest Path Infrastructure

**Goal:** Generalize `init_shortest_paths`, `_null_range_type`, and the weight function
concepts to work with both container families. No algorithm logic changes yet.

| Step | Action | Verification |
|------|--------|-------------|
| 3.1 | Add graph-parameterized overloads of `init_shortest_paths` to `traversal_common.hpp` (keep old overloads for backward compat; new ones take `const G& g` first param) | Existing tests pass |
| 3.2 | Verify `_null_range_type` detection works with `is_null_range_v` in both index and mapped contexts | Unit test |
| 3.3 | Commit: `"feat: generalize init_shortest_paths for mapped graphs"` | All tests pass |

**Review gate:** Confirm init functions work before touching Dijkstra/Bellman-Ford.

---

### Phase 4: Shortest Path Algorithms (one at a time)

**Goal:** Generalize Dijkstra and Bellman-Ford. These have the deepest index coupling
(user-provided `Distances`/`Predecessors` parameters, priority queue, `static_cast<size_t>`).

| Step | Algorithm | Key Changes | Test File |
|------|-----------|------------|-----------|
| 4.1 | `dijkstra_shortest_paths` | Relax concept to `adjacency_list`; relax `Distances`/`Predecessors` from `random_access_range` to accept vertex maps; drop `static_cast<size_t>()` in favor of direct `distances[uid]`; queue element → `vertex_id_store_t<G>`; use `vertex_map_get` with infinity default for distance reads; use graph-param `init_shortest_paths` | `test_dijkstra_shortest_paths.cpp` |
| 4.2 | **Review gate** | Review diff against `index/dijkstra_shortest_paths.hpp` | — |
| 4.3 | `bellman_ford_shortest_paths` | Same treatment as Dijkstra; additionally convert `for(k=0; k<N)` relaxation loop to `views::vertexlist` iteration | `test_bellman_ford_shortest_paths.cpp` |
| 4.4 | **Review gate** | Review diff against `index/bellman_ford_shortest_paths.hpp` | — |
| 4.5 | Commit: `"feat: generalize dijkstra, bellman_ford for mapped graphs"` | All tests pass |

---

### Phase 5: Component Algorithms (one at a time)

**Goal:** Generalize connected components, which involves both internal arrays and
user-supplied `Component` parameter.

| Step | Algorithm | Key Changes | Test File |
|------|-----------|------------|-----------|
| 5.1 | `connected_components` — `connected_components()` function | Relax `Component` from `random_access_range` to vertex map; `for(uid=0;uid<N)` → `views::vertexlist`; `iota` → vertex iteration | `test_connected_components.cpp` |
| 5.2 | **Review gate** | Diff review | — |
| 5.3 | `connected_components` — `kosaraju()` (both overloads) | `vector<bool> visited` → vertex_map; `vector<id_type> order` → `vector<vertex_id_store_t<G>>` | `test_scc_bidirectional.cpp` |
| 5.4 | **Review gate** | Diff review | — |
| 5.5 | `connected_components` — `afforest()` (both overloads) | `iota(component)` → vertex iteration; `static_cast<vertex_id_t<G>>` adjustments | `test_connected_components.cpp` |
| 5.6 | **Review gate** | Diff review | — |
| 5.7 | Commit: `"feat: generalize connected_components for mapped graphs"` | All tests pass |

---

### Phase 6: Simple Algorithms (batch, low risk)

**Goal:** Generalize algorithms with minimal index coupling — each has at most one
internal array and no user property map parameters.

| Step | Algorithm | Key Changes | Test File |
|------|-----------|------------|-----------|
| 6.1 | `jaccard_coefficient` | `vector<unordered_set<vid_t>> nbrs` → `vertex_map<G, unordered_set<id_store_t>>` | `test_jaccard.cpp` |
| 6.2 | `mis` | `vector<uint8_t> removed` → lazy vertex_map + `contains()` | `test_mis.cpp` |
| 6.3 | `triangle_count` | `for(uid=0; uid<N)` → `views::vertexlist` | `test_triangle_count.cpp` |
| 6.4 | `label_propagation` | `vector<id_type> order` → `vector<vertex_id_store_t<G>>`; `iota` → vertex iteration; relax `Label` param | `test_label_propagation.cpp` |
| 6.5 | **Review gate** | Diff review of all four | — |
| 6.6 | Commit: `"feat: generalize jaccard, mis, triangle_count, label_propagation for mapped graphs"` | All tests pass |

---

### Phase 7: Structural Algorithms (highest effort)

**Goal:** Generalize articulation points and biconnected components, which have the most
internal arrays (5 and 3 respectively).

| Step | Algorithm | Key Changes | Test File |
|------|-----------|------------|-----------|
| 7.1 | `articulation_points` | 5 internal arrays (`disc`, `low`, `parent`, `child_count`, `emitted`) → vertex_maps; `static_cast<vid_t>(N)` sentinel → use `vertex_map_contains` | `test_articulation_points.cpp` |
| 7.2 | **Review gate** | Diff review — most complex transformation | — |
| 7.3 | `biconnected_components` | 3 internal arrays (`disc`, `low`, `parent`) → vertex_maps; same sentinel change | `test_biconnected_components.cpp` |
| 7.4 | **Review gate** | Diff review | — |
| 7.5 | Commit: `"feat: generalize articulation_points, biconnected_components for mapped graphs"` | All tests pass |

---

### Phase 8: MST Algorithms

**Goal:** Evaluate and generalize Prim and Kruskal.

| Step | Algorithm | Key Changes | Test File |
|------|-----------|------------|-----------|
| 8.1 | `prim` | Internal `vector<EV> distance` → vertex_map; relax `Predecessor`/`Weight` from `random_access_range`; `for(v=0;v<N)` → vertex iteration; drop `static_cast<size_t>` | `test_mst.cpp` |
| 8.2 | **Review gate** | Diff review | — |
| 8.3 | `kruskal` / `inplace_kruskal` | Uses `x_index_edgelist_range` and `disjoint_vector`. Evaluate whether map support makes sense for edge-list-centric algorithms. May defer. | `test_mst.cpp` |
| 8.4 | **Review gate** | Decide continue or defer | — |
| 8.5 | Commit if applicable | All tests pass |

---

### Phase 9: Cleanup

**Goal:** Remove temporary reference copies and finalize.

| Step | Action | Verification |
|------|--------|-------------|
| 9.1 | Final full test run across all presets (clang-debug, gcc-release, gcc-asan) | All pass |
| 9.2 | Remove `include/graph/algorithm/index/` directory | Clean tree |
| 9.3 | Update documentation to reflect that algorithms now support mapped graphs | Docs review |
| 9.4 | Commit: `"chore: remove index-only reference copies, update docs"` | Clean commit |

---

### Phase Summary

| Phase | Scope | Risk | Gate |
|-------|-------|------|------|
| 0 | Snapshot originals | None | Baseline tests pass |
| 1 | Concepts + vertex_map | Low — additive only | Concept tests pass |
| 2 | BFS, DFS, topo sort | Low — internal arrays only | Per-algorithm diff review |
| 3 | init_shortest_paths | Low — additive overloads | Init tests pass |
| 4 | Dijkstra, Bellman-Ford | Medium — parameter types change | Per-algorithm diff review |
| 5 | Connected components | Medium — 4 function variants | Per-function diff review |
| 6 | Jaccard, MIS, TC, LP | Low — simple patterns | Batch diff review |
| 7 | Art. points, biconn. | High — many internal arrays | Per-algorithm diff review |
| 8 | Prim, Kruskal | Medium — may defer Kruskal | Per-algorithm diff review |
| 9 | Cleanup | None | Final full test suite |

---

## Algorithm Impact Summary

| Algorithm | Internal Arrays | Parameter Arrays | Sequential Loops | Effort |
|-----------|----------------|-----------------|------------------|--------|
| `breadth_first_search` | `visited` (1) | none | none | Low |
| `depth_first_search` | `color` (1) | none | none | Low |
| `topological_sort` | `color`, `finish_order` (2) | output iter | none | Low |
| `dijkstra_shortest_paths` | none | `Distances`, `Predecessors` (2) | `uid < N` (1) | Medium |
| `bellman_ford_shortest_paths` | none | `Distances`, `Predecessors` (2) | `k < N` (1) | Medium |
| `connected_components` | `visited` (1) | `Component` (1) | `uid < N` (1) | Medium |
| `articulation_points` | 5 arrays | none | none | High |
| `biconnected_components` | 3 arrays | none | none | High |
| `label_propagation` | `order` (1) | `Label` (1) | none | Medium |
| `jaccard_coefficient` | `nbrs` (1) | none | none | Low |
| `mis` | `removed` (1) | none | none | Low |
| `triangle_count` | none | none | `uid < N` (1) | Low |
| `prim` (MST) | `distance` (1) | `Predecessor`, `Weight` (2) | `v < N` (1) | Medium |
| `kruskal` (MST) | `disjoint_set` (1) | none | `uid <= N` (1) | Separate |

---

## Design Decisions

1. **`vertex_map` is a type alias.** Zero-cost, no wrapper overhead. If uniform
   `operator[]` semantics or debug bounds-checking prove necessary later, it can be
   promoted to a class — but start simple.

2. **Prefer lazy over eager population.** For mapped graphs, start with an empty map
   and treat absence as the default value (infinity, White, false, etc.) wherever the
   algorithm's semantics allow it. Only use eager pre-population when the algorithm
   truly requires every vertex to have an entry before the first read (e.g. user-supplied
   component labels). See the per-algorithm guidance table in §2.

3. **`vertex_map` uses `unordered_map` internally, not `std::map`.** Algorithm
   bookkeeping (visited, colors, distances) needs O(1) access — `unordered_map` is
   the right choice. `std::map` would be O(log n) per access, too slow for inner loops.
   The graph's vertex container may be `std::map` (ordered), but the algorithm's scratch
   data need not be.

4. **`mapped_vertex_range` requires `hashable_vertex_id`.** Algorithms universally need
   hashable IDs for their internal `unordered_map` usage, so bake this into the concept
   rather than requiring each algorithm to add it separately.

5. **Add new `init_shortest_paths` overloads with graph parameter.** The current
   signatures `(Distances&)` and `(Distances&, Predecessors&)` are preserved for backward
   compatibility. New overloads taking `const G& g` as the first parameter are added
   alongside them. The old overloads can be deprecated once migration is complete.
