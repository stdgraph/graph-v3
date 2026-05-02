# Multi-Partite Graph Design Analysis

**Goal.** Today every graph in the library carries a *single* `vertex_value_type` (VV)
and *single* `edge_value_type` (EV). Multi-partite scenarios — bipartite movie/actor,
heterogeneous knowledge graphs, multi-layer networks — must shoehorn all vertex
kinds into one type, typically a `std::variant` (see [`bacon3()` in
examples/BGLWorkshop2026/bacon.cpp](../examples/BGLWorkshop2026/bacon.cpp)).
This document analyses the library to identify what would need to change to
support a graph whose **partition is the discriminator on a vertex's C++ type**,
i.e. each partition `p ∈ [0..P)` has its own `VV_p` and (independently) its own
edge-value types `EV_{p→q}`.

---

## 1. Where the single-type assumption lives today

### 1.1 Container template signature

[`include/graph/container/dynamic_graph.hpp` line 893](../include/graph/container/dynamic_graph.hpp#L893):

```cpp
template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
class dynamic_graph_base {
public:
  using partition_id_type = VId;
  using partition_vector  = std::vector<VId>;
  using vertex_id_type    = VId;
  using vertex_type       = dynamic_vertex<EV, VV, GV, VId, Bidirectional, Traits>;
  using edge_type         = dynamic_out_edge<EV, VV, GV, VId, Bidirectional, Traits>;
  using vertices_type     = typename Traits::vertices_type;   // e.g. std::vector<vertex_type>
  using edges_type        = typename Traits::edges_type;      // e.g. std::list<edge_type>
  ...
};
```

`vertices_type` is one container of one `vertex_type`. The same holds for
`compressed_graph_base` (CSR row values are a single `vector<VV>`).

### 1.2 Descriptor payload

[`vertex_value` ADL hook in dynamic_graph.hpp lines 1700–1715](../include/graph/container/dynamic_graph.hpp#L1700-L1715):

```cpp
template <typename G, typename U>
requires std::derived_from<std::remove_cvref_t<G>, dynamic_graph_base> && vertex_descriptor_type<U> &&
         (!std::is_void_v<VV>)
[[nodiscard]] friend constexpr decltype(auto) vertex_value(G&& g, U&& u) noexcept {
  return std::forward<U>(u).inner_value(std::forward<G>(g).vertices_).value();
}
```

`vertex_value(g,u)` always returns a reference to the *single* `VV`. Same story
for `edge_value(g,uv) → EV&` lines 1730–1755.

### 1.3 Partitions exist but are vertex-id ranges, not types

[compressed_graph.hpp lines 1370–1430](../include/graph/container/compressed_graph.hpp#L1370-L1430):

```cpp
friend constexpr auto partition_id(G&& g, const VertexDesc& u) noexcept -> partition_id_type {
  const auto vid = u.vertex_id();
  ...
  auto it = std::upper_bound(g.partition_.begin(), g.partition_.end() - 1, vid);
  --it;
  return static_cast<partition_id_type>(std::distance(g.partition_.begin(), it));
}
```

Today's "partition" is a contiguous **vertex-id range** stored in a
`std::vector<VId> partition_`, queried by binary search. There is no link
between partition id and C++ type — every vertex in every partition still has
the same `VV`. The CPOs `partition_id(g,u)`, `num_partitions(g)`,
`vertices(g, pid)`, `num_vertices(g, pid)` are all integer-driven.

### 1.4 CPOs are uniform across all vertices/edges

The CPO list (from
[adj_list/detail/graph_cpo.hpp](../include/graph/adj_list/detail/graph_cpo.hpp)
and the friend functions on `dynamic_graph_base`):

| CPO | Returns | Currently single-type? |
|---|---|---|
| `vertices(g)` | range of `vertex_t<G>` | yes |
| `vertices(g, pid)` | range of `vertex_t<G>` | yes (subrange of same type) |
| `vertex_id(g, u)` | `vertex_id_t<G>` | yes |
| `vertex_value(g, u)` | `vertex_value_t<G>&` | **yes — single VV** |
| `find_vertex(g, uid)` | iterator to `vertex_t<G>` | yes |
| `partition_id(g, u)` | `partition_id_type` (integer) | n/a |
| `num_partitions(g)` | integer | n/a |
| `edges(g, u)` / `edges(g, uid)` | range of `edge_t<G>` | yes |
| `edge_value(g, uv)` | `edge_value_t<G>&` | **yes — single EV** |
| `target_id`, `source_id`, `target`, `source` | `vertex_id_t<G>` / iter | yes |

Every signature is parameterised over a single `G` whose `vertex_t<G>` and
`edge_t<G>` are concrete types. There is no language for *"the value type of
the vertex `u` you happen to have"*.

### 1.5 Views and algorithms — actually mostly type-agnostic

This is the **good news** that shapes the design space.

[`include/graph/views/bfs.hpp` lines 144–148](../include/graph/views/bfs.hpp#L144-L148):

```cpp
template <adj_list::adjacency_list G, class VVF = void, class Alloc = std::allocator<bool>, ...>
class vertices_bfs_view;

template <adj_list::adjacency_list G, class EVF = void, class Alloc = std::allocator<bool>, ...>
class edges_bfs_view;
```

Views never read `vertex_value_t<G>` directly — they accept a **user value
function** `vvf : (G&, vertex_t<G>) -> auto` and propagate
`std::invoke_result_t`. So `vertices_bfs(g, seed, vvf)` already returns
`[v, val]` tuples whose `val` type can vary with `v`. The same is true of
`edges_bfs`, `vertices_dfs`, `vertexlist`, `edgelist`, `neighbors`, etc.

Algorithms are similar.
[dijkstra_shortest_paths.hpp lines 90–120](../include/graph/algorithm/dijkstra_shortest_paths.hpp#L90-L120):

```cpp
* @param weight  Edge weight function: (const G&, const edge_t<G>&) -> Distance.
```

Dijkstra never calls `edge_value(g,uv)` itself — the weight comes from a
user-supplied `WF`. Same with `bellman_ford`, `mst`, `articulation_points`,
`tarjan_scc`. The library deliberately decoupled "value" from "weight/key"
years ago.

**Implication.** Whatever we do at the *container* layer to support multiple
vertex/edge types, **most views and algorithms work unchanged** as long as the
graph still satisfies the basic adjacency-list concepts (you can iterate
`vertices(g)`, `edges(g,u)`, get `vertex_id`/`target_id`, etc.) and as long as
"the descriptor" remains a single concrete type. The pinch points are exactly:

1. `vertex_value(g,u)` and `edge_value(g,uv)` — what type do they return when
   the vertex/edge type is partition-dependent?
2. `vertex_t<G>` and `edge_t<G>` — does the library still have *one*
   descriptor type per graph?
3. `find_vertex(g, uid)`, `vertices(g)` — does iteration cross partitions?

The three design options below differ exactly on how they answer these
questions.

---

## 2. Design Options

| # | Name | Vertex C++ types | Single descriptor? | Library impact |
|---|---|---|---|---|
| **A** | **Variant value (status quo)** | one VV = `variant<...>` | yes | none — works today |
| **B** | **Type-erased payload** | one VV = `any` / poly handle | yes | small — value access stays `VV&` |
| **C** | **Partition-typed values, single graph** | `tuple<VV_0, VV_1, ...>` indexed by partition | yes | medium — `vertex_value` becomes a *visitor* CPO |
| **D** | **Multi-graph composite** | `tuple<G_0, G_1, ...>`; vertex desc = `(pid, sub_desc)` | **no** — descriptor carries pid | large — descriptors, CPOs, views, algorithms |
| **E** | **Heterogeneous descriptors via expression-template wrapper** | each partition keeps its real type; a wrapper graph projects them | yes per-partition, multiple per composite | large — dual-layer concept hierarchy |

The three serious candidates are **B**, **C**, and **D**. Option A is what
`bacon3()` does already. The body of this document develops B, C, and D in full,
and §2.5 gives a complete treatment of E for comparison.

---

### 2.1 Option B — Type-erased payload (`std::any` / polymorphic handle)

**Idea.** Keep `dynamic_graph<EV, VV, GV, VId, ...>` exactly as it is, but
choose `VV = std::any` (or a pointer to a polymorphic base, or a small-buffer
handle). Each vertex stores any payload; the user retrieves it with
`std::any_cast<actor>(vertex_value(g,u))` after checking
`partition_id(g,u)`.

**`bacon3` rewrite:**

```cpp
using G = dynamic_adjacency_graph<
    uol_graph_traits<void, std::any, void, std::string, false>>;
G g;
g.load_vertices(movies, [](const auto& m) { return VD{m, std::any{movie{m}}}; });
g.load_vertices(actors, [](const auto& a) { return VD{a, std::any{actor{a}}}; });
// ...
const actor& a = std::any_cast<const actor&>(vertex_value(g, u));
```

**CPOs.** No change. `vertex_value` still returns `std::any&`.

**Views & algorithms.** No change. They never inspect the payload.

**Pros.**
- Zero library changes.
- Cleaner than `variant` for *open* type sets (new vertex kinds at runtime).
- Default-constructibility issue from `bacon3()` disappears (`std::any{}` is fine).

**Cons.**
- Every payload access is a runtime type check + heap allocation
  (`std::any` for non-SBO types). For a `bipartite movie ↔ actor` graph this
  is strictly worse than the variant.
- No compile-time type safety — `bad_any_cast` instead of a compile error.
- Doesn't address the *partition* dimension at all. The user has to invent a
  `partition → type` map themselves.

**Verdict.** A wash. It removes the variant boilerplate but loses static
type safety and pessimises performance. Useful only for plug-in
architectures where vertex kinds are not known at compile time.

---

### 2.2 Option C — Partition-typed values in a single graph

This is the design I think best fits the library's existing shape.

**Idea.** Replace the single `VV` template parameter with a **`std::tuple` of
per-partition value types** `VVs = tuple<VV_0, VV_1, ..., VV_{P-1}>`. The
graph stores **P parallel vertex containers** — one per partition — and
the descriptor encodes `(partition_id, local_index)`.

```cpp
template <class EVs, class VVs, class GV, class VId, bool Bidirectional, class Traits>
class multipart_graph_base;
//      EVs = tuple<EV_0, EV_1, ...>     — edge value per *target* partition (or per (src,tgt) pair, see §2.2.4)
//      VVs = tuple<VV_0, VV_1, ...>     — vertex value per partition
```

#### 2.2.1 New trait

```cpp
template <class EVs, class VVs, class GV, class VId, bool Bidirectional>
struct multipart_uol_traits {
  using edge_value_types   = EVs;   // tuple
  using vertex_value_types = VVs;   // tuple
  using vertex_id_type     = VId;
  static constexpr std::size_t partition_count = std::tuple_size_v<VVs>;

  // One vertex container per partition.
  // For a 2-partition movie/actor graph this is
  //   tuple<unordered_map<string, dynamic_vertex<..., movie, ...>>,
  //         unordered_map<string, dynamic_vertex<..., actor, ...>>>
  template <std::size_t P>
  using vertex_container = std::unordered_map<VId, dynamic_vertex<EV_for<P>, std::tuple_element_t<P, VVs>, ...>>;

  using vertices_type = std::tuple< /* expand partitions */ >;
  using edges_type    = /* either uniform std::list<edge_t> or one per (src,tgt) pair */;
};
```

#### 2.2.2 New descriptor

```cpp
namespace graph::adj_list {
template <std::size_t PartitionCount, class... Iters>   // one Iter type per partition
class multipart_vertex_descriptor {
  std::size_t pid_;                       // which partition
  std::variant<Iters...> stored_iter_;    // exactly one alternative active
public:
  constexpr std::size_t partition() const noexcept { return pid_; }

  template <std::size_t P>
  constexpr auto& iter() const noexcept { return std::get<P>(stored_iter_); }

  // vertex_id(g,u) is uniform — VId is the same across partitions
  constexpr decltype(auto) vertex_id() const noexcept;
};
}
```

The descriptor is *one C++ type* for the whole graph (so views and the
adjacency_list concept still see "one vertex_t<G>"), but it remembers which
partition it came from.

#### 2.2.3 CPO impact

| CPO | Change | Notes |
|---|---|---|
| `vertices(g)` | unchanged interface | returns a *flattened view* (`std::ranges::join` over the per-partition containers); element type is `multipart_vertex_descriptor` |
| `vertices(g, pid)` | now returns the **typed** subrange of partition `pid` | element type is still the multipart descriptor — only iteration scope differs |
| `vertex_id(g, u)` | unchanged | `VId` is uniform |
| **`vertex_value(g, u)`** | **now a visitor** — see below | |
| `partition_id(g, u)` | reads `u.partition()` directly | O(1) instead of O(log P) |
| `num_partitions(g)` | `std::tuple_size_v<VVs>` | constexpr |
| `edges(g, u)` | unchanged | element type still `edge_t<G>` |
| `target_id` / `source_id` | unchanged | |
| `edge_value(g, uv)` | visitor (same shape as vertex_value) | |
| `find_vertex(g, uid)` | **ambiguous** — see §2.2.5 | needs disambiguation |

**`vertex_value` becomes a `visit` CPO.** Two flavours:

```cpp
// 1. Compile-time dispatch — caller knows the partition
template <std::size_t P, class G, class U>
constexpr auto& vertex_value(G&& g, const U& u);   // mandates u.partition() == P at runtime

// 2. Visitor-style — caller provides an overload set
template <class G, class U, class Visitor>
constexpr decltype(auto) visit_vertex_value(G&& g, const U& u, Visitor&& vis);
//   calls vis(actor&), vis(movie&), etc., dispatched on u.partition()
```

Existing call sites that today write `vertex_value(g,u)` with a single VV
**continue to compile** when `partition_count == 1` via a partial
specialisation — the multi-partite path is opt-in.

#### 2.2.4 Edge value typing

Three sub-options for `EVs`:

1. **Uniform across partitions** — `EVs = E` (a single type). Trivial.
2. **One EV per source partition** — `EVs = tuple<EV_src_0, EV_src_1, ...>`.
   All edges leaving partition `p` carry `EV_p`. Fits "movie-edges have a year,
   actor-edges have a credit-order".
3. **One EV per (src, tgt) pair** — `EVs = matrix<P, P, EV>`. Most expressive,
   but `O(P²)` instantiations. Fits knowledge graphs where each relation type
   is its own table.

I recommend **(2) per source-partition** as the default — it lines up with
how out-edges are stored, keeps the type matrix linear, and matches the
common case (every relation is owned by one of its endpoints).

#### 2.2.5 `find_vertex(g, uid)` — the hardest CPO

If two partitions share the same `VId` type (say both keyed by `std::string`),
`find_vertex(g, "Top Gun")` is ambiguous: is it the movie or the actor?

Three answers, in increasing strictness:

a. **Search all partitions in declaration order, return the first match.**
   Simple; matches the variant-key approach in `bacon3()` Option 3 (key IS
   the discriminator). Cost: O(P) lookups per find.

b. **Require the user to pass the partition id**: `find_vertex<1>(g, uid)`
   or `find_vertex(g, pid, uid)`. New overload; existing single-partition
   `find_vertex(g, uid)` still works.

c. **Mandate distinct `VId` types per partition** at the trait level.
   `find_vertex` then dispatches on the argument type. Most type-safe; rules
   out string-keyed bipartite graphs where both sides are strings.

I recommend (b) as the primary CPO, with (a) as a free-standing helper for
discovery use-cases.

#### 2.2.6 Example — `bacon4()` under Option C

```cpp
using VVs   = std::tuple<movie, actor>;           // partition 0 = movie, 1 = actor
using EVs   = void;                                // edges carry no value
using G     = multipart_adjacency_graph<
                multipart_uol_traits<EVs, VVs, void, std::string, false>>;

G g;
g.load_vertices<0>(movies, [](const auto& m) { return movie{m}; });
g.load_vertices<1>(actors, [](const auto& a) { return actor{a}; });
g.load_edges<0,1>(movies_actors, [](const auto& e) {
  return std::pair{std::get<0>(e), std::get<1>(e)};   // movie → actor
});
g.load_edges<1,0>(movies_actors, [](const auto& e) {
  return std::pair{std::get<1>(e), std::get<0>(e)};   // actor → movie
});

auto seed = find_vertex<1>(g, "Kevin Bacon"s);   // partition 1 = actor

// Views work unchanged:
for (auto&& [uv] : graph::views::edges_bfs(g, *seed)) { /* ... */ }

// Visitor for printing:
visit_vertex_value(g, u, overloaded{
  [](const movie& m) { cout << "[movie] " << m.title() << '\n'; },
  [](const actor& a) { cout << "[actor] " << a.name()  << '\n'; },
});
```

#### 2.2.7 Pros / cons

**Pros.**
- Static type safety — `vertex_value<P>(g,u)` returns the *real* type.
- Each partition can use a different storage strategy (one `vector`-backed
  partition for dense ids, one `unordered_map`-backed for sparse) by
  parametrising `Traits` per-partition. Powerful.
- BFS / DFS / dijkstra / etc. all work unchanged because they treat
  `vertex_t<G>` opaquely.
- `partition_id(g,u)` becomes O(1) (descriptor carries it directly).
- The descriptor is still *one* type → `adjacency_list<G>` concept holds.

**Cons.**
- The descriptor grows from `iterator` to `(size_t, variant<iterator...>)`
  — typically 16→24 or 24→40 bytes. Affects BFS queue / DFS stack memory.
- `vertex_value` callers need the visitor pattern (or a compile-time pid)
  to use the typed payload. More verbose at the value-access call site.
- `find_vertex` requires partition disambiguation in the general case.
- `vertices(g)` has to be a `join_view` over heterogeneous containers — the
  iterator type is non-trivial; needs `std::common_iterator` machinery.
- New trait family `multipart_*_traits` doubles the trait surface.
- `compressed_graph` cannot be retrofitted easily — its CSR layout assumes
  one row-values vector. Would be `multipart_compressed_graph` from scratch.

---

### 2.3 Option D — Multi-graph composite

**Idea.** Each partition is a *separate* `dynamic_graph` with its own VV and
EV. A *composite graph* holds `tuple<G_0, G_1, ...>` plus an inter-partition
edge map. Vertex descriptors carry `(pid, vertex_t<G_pid>)`.

```cpp
template <class... Gs>
class composite_graph {
  std::tuple<Gs...> partitions_;
  std::array<inter_edge_table, sizeof...(Gs) * sizeof...(Gs)> cross_edges_;
};
```

#### CPO impact

Every CPO must be re-implemented at the composite level because
`vertex_t<composite>` is now a tagged variant, not a single iterator type.
`adjacency_list<composite_graph<...>>` requires a fresh proof — most concept
checks fail until the composite forwards them.

`edges(g, u)` becomes a *concatenation* of:

- `edges(get<u.pid>(g.partitions_), u.local_desc())` — intra-partition
- `cross_edges_[(u.pid, *)].find(u.local_id)` — inter-partition

Algorithm impact:

- BFS/DFS view construction needs a way to *push* a heterogeneous descriptor
  into the queue. Today the queue is `std::queue<vertex_id_t<G>>`; in the
  composite that becomes `std::queue<composite_descriptor>` with a tagged
  union. Doable but invasive — every visited tracker, every heap, every
  predecessor map needs to key on the composite descriptor type.
- Dijkstra's `vector_position_map` and `assoc_position_map` (see
  [`heap_position_map.hpp`](../include/graph/detail/heap_position_map.hpp))
  rely on `vertex_id_t<G>` being hashable / index-able. With composite
  descriptors the user must provide a hash for `(pid, sub_id)`.

#### Pros / cons

**Pros.**
- Each partition keeps its existing graph type unchanged — *zero* changes to
  per-partition CPOs, views, algorithms.
- Maximum heterogeneity — partitions can be `dynamic_graph`, `compressed_graph`,
  even user-supplied adapters, all in the same composite.
- Conceptually clean: you compose what you have rather than parameterise a
  new container.

**Cons.**
- The composite layer reimplements *every* CPO. The amount of glue rivals the
  size of `dynamic_graph_base` itself.
- `vertex_t<composite>` is necessarily heavier than today's vertex descriptor
  (must encode pid). More expensive in queues / maps.
- Iteration order across partitions is not contiguous → poor cache behaviour
  for BFS-heavy workloads.
- Existing single-partition algorithms must learn to "see" the composite
  shape. They do this via concepts today, but the concept proofs become
  conditional.

**Verdict.** Theoretically the cleanest separation of concerns, but the
composite layer is a second framework on top of the first. Best reserved for
truly heterogeneous storage (e.g., one partition is a remote graph, another
is in-memory).

---

### 2.5 Option E — Heterogeneous descriptors via a concept-projection wrapper

**Idea.** Every partition is a normal, self-contained, single-type graph.
A thin *wrapper* graph owns no vertex or edge storage itself; it holds
references to the underlying partition graphs and **re-exposes a projected
view** of them through the standard `adjacency_list` concept. Within each
partition the user works with the partition's own types; across partition
boundaries the wrapper provides a *minimal common projection* for algorithms
that need to walk the whole graph (e.g., BFS over all partitions).

This is the "expression-template" parallel: just as `std::views::join` wraps
a range-of-ranges without owning storage, the wrapper graph wraps a
graph-of-graphs without owning vertices or edges.

#### 2.5.1 Layered concept hierarchy

Option E is only coherent if the library supports **two levels** of graph
concept:

```
┌─────────────────────────────────────────────────────────────────┐
│  partitioned_graph<G>     (multi-partition; heterogeneous)      │
│    requires:                                                     │
│      – partition_count<G>  (constexpr integral)                 │
│      – partition_graph_t<G, P>  (the graph type for partition P)│
│      – the partition graph satisfies adjacency_list             │
└──────────────┬──────────────────────────────────────────────────┘
               │ projected/erased cross-partition descriptor
               ▼
┌─────────────────────────────────────────────────────────────────┐
│  adjacency_list<G>        (today's concept; single-type)        │
│    requires:                                                     │
│      – vertices(g), edges(g,u), vertex_id(g,u), ...            │
└─────────────────────────────────────────────────────────────────┘
```

The wrapper satisfies `adjacency_list` via a projected view, allowing all
existing single-partition algorithms to consume it unchanged. The richer
`partitioned_graph` concept lets multi-partition-aware code access typed
partition graphs directly.

#### 2.5.2 Wrapper structure

```cpp
template <class... Gs>   // one graph type per partition
class partitioned_graph_wrapper {

  std::tuple<Gs*...> partitions_;   // non-owning; each Gs is stored elsewhere

public:
  // ── Cross-partition (projected) interface ──────────────────────────────
  // Satisfies adjacency_list by erasing the per-partition type into a
  // common cross_vertex_descriptor.

  struct cross_vertex_descriptor {
    std::size_t pid;    // which partition
    std::size_t lid;    // local vertex index within that partition
  };

  // vertices(wrapper) = join over all partition vertices, tagged with pid.
  // Returns a range whose value_type is cross_vertex_descriptor.
  friend auto vertices(const partitioned_graph_wrapper& g) {
    return cross_vertex_range{g.partitions_};   // see §2.5.3
  }

  // vertex_id(wrapper, cvd) = some globally unique id, e.g. (pid, lid) pair
  friend cross_vertex_descriptor vertex_id(const partitioned_graph_wrapper& g,
                                           const cross_vertex_descriptor& cvd) {
    return cvd;   // the descriptor IS the id in this design
  }

  // edges(wrapper, cvd) = partition-local out-edges, tagged with pid
  friend auto edges(const partitioned_graph_wrapper& g,
                    const cross_vertex_descriptor& cvd) {
    return std::visit([&](auto* part) {
      return cross_edge_range{cvd.pid, graph::edges(*part, cvd.lid)};
    }, partition_ptr_variant(g.partitions_, cvd.pid));
  }

  // ── Per-partition typed interface ──────────────────────────────────────
  // Satisfies partitioned_graph; gives back the real graph.

  template <std::size_t P>
  auto& partition() noexcept { return *std::get<P>(partitions_); }

  template <std::size_t P>
  const auto& partition() const noexcept { return *std::get<P>(partitions_); }

  static constexpr std::size_t partition_count = sizeof...(Gs);
};
```

#### 2.5.3 The cross-vertex range and descriptor

`vertices(wrapper)` must expose a lazy, joined range whose iterator yields
`cross_vertex_descriptor`. The simplest strategy:

```cpp
class cross_vertex_range {
  // Iterates partition 0 first (pid=0, lid=0..N0-1), then partition 1, etc.
  // The iterator wraps a (pid, local_iterator) pair, advances through
  // partitions when the local iterator hits end().
};
```

This range satisfies `std::ranges::forward_range`. `vertex_t<wrapper>` = 
`cross_vertex_descriptor` — a plain struct with `pid` and `lid`, no
iterator storage.

The cross-edge descriptor mirrors it:

```cpp
struct cross_edge_descriptor {
  cross_vertex_descriptor source;
  cross_vertex_descriptor target;
  // No EV stored — for typed edge access, go through partition<P>().
};
```

#### 2.5.4 Value access — the fundamental split

This is where Option E differs most sharply from the others.

**Within a partition** (typed, static, direct):

```cpp
// User holds a compile-time partition index P and a local descriptor.
auto& m = graph::vertex_value(wrapper.partition<0>(), local_u);  // → movie&
auto& a = graph::vertex_value(wrapper.partition<1>(), local_v);  // → actor&
```

Nothing new here — these are calls on the ordinary single-type sub-graphs.

**Across partitions** (via the projected wrapper):

```cpp
// visitor dispatch on pid at runtime:
auto val = visit_vertex(wrapper, cvd, overloaded{
  [](const movie& m, std::integral_constant<std::size_t,0>) { ... },
  [](const actor& a, std::integral_constant<std::size_t,1>) { ... },
});
```

`visit_vertex` is a new free function, not a CPO:

```cpp
template <class PG, class CVD, class Visitor>
decltype(auto) visit_vertex(PG&& pg, const CVD& cvd, Visitor&& vis) {
  // runtime switch on cvd.pid; calls vis(vertex_value(partition<P>(), local), ic)
  return [&]<std::size_t... Ps>(std::index_sequence<Ps...>) {
    using result_t = std::common_type_t<
      std::invoke_result_t<Visitor,
        vertex_value_t<std::tuple_element_t<Ps, typename PG::partition_tuple>>,
        std::integral_constant<std::size_t,Ps>
      >...>;
    result_t ret;
    ([&]{ if (cvd.pid == Ps) {
      ret = vis(vertex_value(pg.partition<Ps>(), to_local(cvd)), ic<Ps>);
    } }(), ...);
    return ret;
  }(std::make_index_sequence<PG::partition_count>{});
}
```

`vertex_value(wrapper, cvd)` (the CPO) is intentionally **not defined** for
the wrapper — it would have to return a `std::variant` anyway, collapsing
back to Option A. Instead, value access always goes either:
- directly through `partition<P>()` for typed single-partition work, or
- through `visit_vertex` for cross-partition visitor dispatch.

#### 2.5.5 Views and algorithms over the wrapper

Because the wrapper satisfies `adjacency_list`, existing views work:

```cpp
// BFS over all partitions, no changes to bfs_view:
for (auto&& [uv] : graph::views::edges_bfs(wrapper, seed_cvd)) {
  // uv is cross_edge_descriptor; no value access here
  auto uid = source_id(wrapper, uv);   // → cross_vertex_descriptor
  auto vid = target_id(wrapper, uv);   // → cross_vertex_descriptor
}
```

As long as the algorithm doesn't call `vertex_value` or `edge_value` on the
wrapper itself — and today's algorithms don't — this works without a single
line of algorithm code changing.

Algorithms that *do* need values (e.g., user code inside a BFS visitor
callback) retrieve them via `visit_vertex`:

```cpp
for (auto&& [uv] : graph::views::edges_bfs(wrapper, seed)) {
  auto v = target_id(wrapper, uv);
  visit_vertex(wrapper, v, overloaded{
    [](const movie& m, auto) { cout << "[movie] " << m.title() << '\n'; },
    [](const actor& a, auto) { cout << "[actor] " << a.name()  << '\n'; },
  });
}
```

#### 2.5.6 `find_vertex` in the wrapper

`find_vertex(wrapper, uid)` where `uid` is a `cross_vertex_descriptor`
is trivial — the descriptor *is* the id. The harder case is
"find by application key":

```cpp
// "Find the movie vertex called Top Gun"
auto local_it = graph::find_vertex(wrapper.partition<0>(), "Top Gun"s);
cross_vertex_descriptor cvd{0, local_it->vertex_id()};
```

There is no single `find_vertex(wrapper, "Top Gun"s)` because the
partition is not deducible from the key type alone (same ambiguity as
Option C §2.2.5). The same three solutions apply; the recommended
answer is the same: require an explicit partition index.

#### 2.5.7 `bacon4()` under Option E

```cpp
void bacon4() {
  using namespace graph::container;

  // Each partition is an ordinary single-type dynamic graph.
  using MovieGraph = dynamic_adjacency_graph<uol_graph_traits<void, movie, void, std::string, false>>;
  using ActorGraph = dynamic_adjacency_graph<uol_graph_traits<void, actor, void, std::string, false>>;

  MovieGraph mg;
  ActorGraph ag;

  mg.load_vertices(movies, [](const auto& m) { return VD{m, movie{m}}; });
  ag.load_vertices(actors, [](const auto& a) { return VD{a, actor{a}}; });

  // The wrapper holds cross-partition edges; it does NOT store intra-partition edges.
  // Cross edges are stored in a separate bipartite edge table.
  partitioned_graph_wrapper<MovieGraph, ActorGraph> wrapper{mg, ag};
  wrapper.load_cross_edges<0,1>(movies_actors, [](const auto& e) {
    return cross_edge_t{"Top Gun", "Tom Cruise"};   // movie key → actor key
  });
  wrapper.load_cross_edges<1,0>(movies_actors, [](const auto& e) {
    return cross_edge_t{"Tom Cruise", "Top Gun"};
  });

  // seed is a cross_vertex_descriptor encoding (partition=1, local="Kevin Bacon")
  auto seed_local = graph::find_vertex(wrapper.partition<1>(), "Kevin Bacon"s);
  cross_vertex_descriptor seed{1, seed_local->vertex_id()};

  std::unordered_map<cross_vertex_descriptor, std::size_t> depth{{seed, 0}};
  for (auto&& [uv] : graph::views::edges_bfs(wrapper, seed)) {
    depth[target_id(wrapper, uv)] = depth[source_id(wrapper, uv)] + 1;
  }

  for (const auto& a : actors) {
    auto local = graph::find_vertex(wrapper.partition<1>(), a);
    cross_vertex_descriptor cvd{1, local->vertex_id()};
    auto it = depth.find(cvd);
    cout << a << " has Bacon number "
         << (it == depth.end() ? "infinity" : std::to_string(it->second / 2))
         << '\n';
  }
}
```

#### 2.5.8 Pros / cons

**Pros.**
- **Zero changes to existing partition graph code.** `MovieGraph` and
  `ActorGraph` are completely ordinary graphs. `vertex_value` on each works
  exactly as today.
- **No new descriptor variant overhead for single-partition work.** When
  code only touches partition 0, it works with `vertex_t<MovieGraph>`
  directly — a plain iterator, not a fat tagged struct.
- **Cleanest separation of concerns.** The wrapper enforces: "I can
  traverse all vertices/edges for algorithmic purposes; for value access go
  to the typed partition."
- **Open to non-`dynamic_graph` backends.** `partition<2>()` can be a
  `compressed_graph`, an external database adapter, or any type that
  satisfies `adjacency_list`.
- **Incremental adoption.** Existing code continues to use `MovieGraph`
  directly; only code that needs cross-partition traversal touches the wrapper.

**Cons.**
- **Two-tier concept hierarchy is new library infrastructure.** The
  `partitioned_graph` concept, `visit_vertex`, `cross_vertex_descriptor`,
  and the `cross_vertex_range` iterator are all genuinely new.
- **Cross-partition edges need a separate store.** In a bipartite graph
  *all* edges are cross-partition, so the "wrapper holds no edges" stance
  forces a new `cross_edge_table` container. That container needs its own
  `load_edges` interface, iterator, CPO hooks.
- **`find_vertex` across the whole wrapper is ambiguous** (same issue as
  C and D) and there is no single-natural solution.
- **BFS visited tracker keys on `cross_vertex_descriptor`**, which is a
  struct — needs a `std::hash` specialisation (or uses the ordered-map
  path). Straightforward but non-zero friction.
- **Depth map / predecessor map complexity doubles.** Today
  `std::unordered_map<vertex_t<G>, size_t>` where `vertex_t` is a plain
  iterator. Under E it becomes `unordered_map<cross_vertex_descriptor, size_t>`,
  requiring a hash. Under C the descriptor is the same shape (pid, lid) so
  the cost is identical — but C hides it inside a single graph type.
- **`vertex_value(wrapper, cvd)` is intentionally absent.** This is
  philosophically correct, but means any generic algorithm that expects
  `vertex_value` to be callable on *any* `adjacency_list` will silently
  fail (compile error or `static_assert`) when given the wrapper. The
  library must document this clearly.

#### 2.5.9 Relationship to the other options

| Dimension | C | D | E |
|---|---|---|---|
| Storage ownership | single composite container | tuple of full graphs | tuple of external graphs (non-owning) |
| Descriptor type | one fat multipart descriptor | tagged sub-descriptor | `cross_vertex_descriptor` |
| Typed value access | `vertex_value<P>(g,u)` CPO | `vertex_value(get<P>(g),u)` | `vertex_value(wrapper.partition<P>(),u)` |
| Cross-partition value | `visit_vertex_value` CPO | `visit_vertex_value` CPO | `visit_vertex` free function |
| `vertex_value` on composite | returns `variant<VVs>` (optional) | returns `variant<VVs>` (optional) | **intentionally absent** |
| Algorithms on composite | `adjacency_list` holds automatically | `adjacency_list` re-proved | `adjacency_list` holds via projection |
| Single-partition existing code | must use `partition<P>()` API | must use `get<P>(g)` API | **unchanged — uses original graph** |

The key philosophical difference: C and D bring all vertex types *into* a
single graph object (they own the storage). E keeps each partition *outside*
the wrapper (the wrapper borrows them). This makes E the most composition-
friendly option for codebases where partition graphs already exist
independently, and the most invasive option for new graph construction
(cross edges need a separate home).

---

## 3. Comparison Summary

| Concern | A — variant VV | B — `std::any` | C — typed-tuple | D — composite | E — wrapper |
|---|---|---|---|---|---|
| Static type safety on `vertex_value` | weak (`std::get`) | none (`any_cast`) | **strong** (per-partition CPO) | **strong** (per sub-graph) | **strong** (per partition) |
| `vertex_value` on composite | returns `variant` | returns `any` | `variant` or absent | `variant` or absent | **absent by design** |
| Performance vs. status quo | baseline | worse (heap+RTTI) | ≈baseline | worse (het. queues) | ≈baseline |
| Library code changes | none | none | **medium** | **large** | **large** |
| Concept hierarchy changes | none | none | none | `adjacency_list` re-proved | new `partitioned_graph` concept |
| `find_vertex` story | trivial | trivial | `<P>` overload | composite-defined | `partition<P>().find_vertex` |
| Storage choice per partition | no | no | **yes** | **yes** | **yes** |
| Existing single-partition code | unchanged | unchanged | use `partition<P>()` | use `get<P>(g)` | **unchanged** |
| Cross-edge storage | in-graph | in sub-graphs | in composite | in composite | **separate store needed** |
| Best for | small ad-hoc bipartite | open/plug-in vertex kinds | static finite partitions | mixed storage backends | *composing pre-existing graphs* |

---

## 4. Recommendation

**Option C** is the right next step.

1. It preserves the library's invariants: one `vertex_t<G>`, one
   `vertex_id_t<G>`, the same `adjacency_list` concept, the same view
   pipeline.
2. It makes `partition_id` *meaningful* — today it's an integer tied to a
   contiguous id range with no semantic content; in Option C it indexes into
   `tuple<VV_0, VV_1, ...>`.
3. The work is bounded and incremental:
   - Add `multipart_*_traits` (sibling of `vol_graph_traits` etc.).
   - Add `multipart_vertex_descriptor` (sibling of `vertex_descriptor`).
   - Implement `multipart_dynamic_graph` re-using `dynamic_vertex` /
     `dynamic_out_edge` per partition.
   - Add `vertex_value<P>(g,u)` and `visit_vertex_value(g,u,vis)` CPOs.
   - Add `find_vertex<P>(g,uid)` overload.
   - Provide the partial specialisation that collapses to today's behaviour
     when `partition_count == 1`.
4. Views and algorithms need *no changes* — they already accept user value
   functions (`VVF`, `EVF`, `WF`) and never call `vertex_value` /
   `edge_value` themselves.

Option B is a useful escape hatch for the "open type set" case and could be
shipped as a documented convention (`VV = std::any`) without library
changes.

Option D is interesting future work for federated graphs but pays for
flexibility with a doubled CPO surface — premature for now.

---

## 5. Migration story for `bacon3()`

Under Option C the example collapses to (no `std::variant`, no `std::monostate`,
no `std::hash` specialisations):

```cpp
void bacon4() {
  using namespace graph::container;

  using VVs = std::tuple<movie, actor>;
  using G   = multipart_adjacency_graph<
                multipart_uol_traits<void, VVs, void, std::string, false>>;

  G g;
  g.load_vertices<0>(movies, [](const auto& m) { return movie{m}; });
  g.load_vertices<1>(actors, [](const auto& a) { return actor{a}; });
  g.load_edges<0,1>(movies_actors, [](const auto& e) {
    return copyable_edge_t<std::string,void>{std::get<0>(e), std::get<1>(e)};
  });
  g.load_edges<1,0>(movies_actors, [](const auto& e) {
    return copyable_edge_t<std::string,void>{std::get<1>(e), std::get<0>(e)};
  });

  auto seed = *find_vertex<1>(g, "Kevin Bacon"s);

  std::unordered_map<vertex_t<G>, std::size_t> depth{{seed, 0}};
  for (auto&& [uv] : graph::views::edges_bfs(g, seed)) {
    auto u = *find_vertex(g, source_id(g, uv));   // partition disambiguated by descriptor
    auto v = *find_vertex(g, target_id(g, uv));
    depth[v] = depth[u] + 1;
  }

  for (const auto& a : actors) {
    auto it = depth.find(*find_vertex<1>(g, a));
    cout << a << " has Bacon number "
         << (it == depth.end() ? std::string{"infinity"} : std::to_string(it->second / 2))
         << '\n';
  }
}
```

The domain classes `movie` and `actor` need **no** comparison/hash/default
constructors — exactly the constraint that drove the `bacon3()` complexity.
