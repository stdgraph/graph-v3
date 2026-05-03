# Strategy for Adapting BGL Graphs to graph-v3

This strategy implements the goals defined in `bgl_graph_adapt_goal.md`.

---

## Table of Contents

1. [Approach](#1-approach)
2. [CPO Adaptation Surface](#2-cpo-adaptation-surface)
3. [Phase 1A: adjacency_list vecS/vecS directed](#3-phase-1a-adjacency_list-vecsvecs-directed)
4. [Phase 1B: Undirected and Bidirectional Variants](#4-phase-1b-undirected-and-bidirectional-variants)
5. [Phase 1C: compressed_sparse_row_graph](#5-phase-1c-compressed_sparse_row_graph)
6. [Property Map Bridge](#6-property-map-bridge)
7. [Views and Algorithm Validation](#7-views-and-algorithm-validation)
8. [Testing Strategy](#8-testing-strategy)
9. [Performance](#9-performance)
10. [Risks](#10-risks)
11. [Missing Elements in the Current Library](#11-missing-elements-in-the-current-library)

---

## 1. Approach

### Thin Wrapper + ADL Override Functions

The adaptation uses a thin wrapper type with ADL free functions. This avoids modifying BGL code
or injecting functions into `namespace boost`, and it avoids ADL collisions with BGL's own
free functions.

```cpp
namespace graph::bgl {

template <typename BGL_Graph>
class graph_adaptor {
  BGL_Graph* g_;       // Non-owning pointer to the BGL graph
public:
  explicit graph_adaptor(BGL_Graph& g) : g_(&g) {}
  BGL_Graph&       bgl_graph()       { return *g_; }
  const BGL_Graph& bgl_graph() const { return *g_; }
};

// ADL free functions found through graph_adaptor's namespace:
template <typename G>
auto vertices(const graph_adaptor<G>& ga) { /* ... */ }

template <typename G>
auto out_edges(const graph_adaptor<G>& ga, /* vertex */ ) { /* ... */ }

// ... additional CPO customizations

} // namespace graph::bgl
```

### Why a Wrapper (Not Direct CPO Overloads on BGL Types)

1. **ADL collision avoidance.** BGL defines `vertices(g)`, `source(e, g)`, `target(e, g)`, etc.
   in `namespace boost`. Because BGL types live in `boost`, unqualified lookup from graph-v3's
   CPO ADL tier would find BGL's own functions. BGL's functions have different signatures
   (e.g. `out_edges(u, g)` vs graph-v3's `out_edges(g, u)`) and return `std::pair<It, It>`
   instead of C++20 ranges, so they would either cause ambiguity or silently fail concept checks.
   A wrapper in a separate namespace avoids this entirely.

2. **C++20 iterator boundary.** BGL iterators are built on `boost::iterator_facade` /
   `boost::iterator_adaptor`, which predate C++20 and do not define `iterator_concept`.
   graph-v3's `vertex_descriptor_view` and `edge_descriptor_view` require C++20
   `forward_iterator` or `random_access_iterator` on the underlying iterators. The wrapper
   is the natural place to convert BGL iterator pairs into C++20-compatible ranges.

3. **Descriptor wrapping.** graph-v3's `vertex_id` CPO requires `vertex_descriptor_type`,
   satisfied only by `vertex_descriptor<VertexIter>`. BGL's bare `size_t` vertices do not
   match. The wrapper's ADL functions return graph-v3-compatible types, and the CPO's
   auto-wrapping (`_wrap_if_needed`) handles the rest transparently.

### User-Facing API

The user wraps their BGL graph once and then uses it with graph-v3 views and algorithms:

```cpp
boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, ...> bgl_g;
// ... populate bgl_g ...

auto g = graph::bgl::graph_adaptor(bgl_g);

// Now usable with graph-v3:
for (auto [uid, u] : graph::vertexlist(g)) { /* ... */ }
graph::dijkstra_shortest_paths(g, sources, distance_fn, predecessor_fn, weight_fn);
```

---

## 2. CPO Adaptation Surface

The following CPOs must be customized (via ADL free functions in the wrapper's namespace)
for the phase-1 targets. The CPO resolution tier that will match is noted.

| CPO | ADL Function Signature | Resolution Tier | Notes |
|-----|------------------------|-----------------|-------|
| `vertices(g)` | `vertices(const graph_adaptor<G>&)` | Tier 2 (ADL) | Returns C++20 range; CPO auto-wraps in `vertex_descriptor_view` |
| `num_vertices(g)` | `num_vertices(const graph_adaptor<G>&)` | Tier 2 (ADL) | Delegates to `boost::num_vertices` |
| `out_edges(g, u)` | `out_edges(graph_adaptor<G>&, vertex_desc)` | Tier 2 (ADL) | Returns C++20 range; CPO auto-wraps in `edge_descriptor_view` |
| `target_id(g, uv)` | `target_id(const graph_adaptor<G>&, edge_desc)` | Tier 2 (ADL) | Unwraps edge descriptor, calls `boost::target(e, g)` |
| `source_id(g, uv)` | `source_id(const graph_adaptor<G>&, edge_desc)` | Tier 2 (ADL) | Unwraps edge descriptor, calls `boost::source(e, g)` |
| `find_vertex(g, uid)` | No override needed for vecS | Tier 4 (random-access default) | `vertices(g)` returns a `vertex_descriptor_view` over `iota_view`, which is a `sized_range`; Tier 4 fires: `std::ranges::next(begin(vertices(g)), uid)`. An ADL override would only be needed for non-index selector types (`listS`, `setS`) where `vertices(g)` is not a sized range. |
| `vertex_id(g, u)` | Not needed if `vertex_descriptor<...>::vertex_id()` works | Tier 2 (descriptor method) | Auto-wrapping makes this work for vecS (index-based) |
| `in_edges(g, u)` | `in_edges(graph_adaptor<G>&, vertex_desc)` | Tier 2 (ADL) | Only for bidirectional BGL graphs |
| `degree(g, u)` | Optional | Default tier | Defaults to `size(out_edges(g, u))` |
| `edge_value(g, uv)` | `edge_value(const graph_adaptor<G>&, edge_desc)` | Tier 2 (ADL) | Access bundled edge properties |
| `vertex_value(g, u)` | `vertex_value(const graph_adaptor<G>&, vertex_desc)` | Tier 2 (ADL) | Access bundled vertex properties |

### CPOs That Should Work via Defaults (No Override Needed)

| CPO | How It Resolves | Depends On |
|-----|-----------------|------------|
| `target(g, uv)` | Default tier: `*find_vertex(g, target_id(g, uv))` | `target_id` + `find_vertex` |
| `source(g, uv)` | Default tier: `*find_vertex(g, source_id(g, uv))` | `source_id` + `find_vertex` |
| `degree(g, u)` | Default: `ranges::size(out_edges(g, u))` | `out_edges` |
| `num_edges(g)` | Default or ADL override | May need explicit override |

---

## 3. Phase 1A: `adjacency_list<vecS, vecS, directedS>`

This is the highest-priority target because `vecS` vertex descriptors are `size_t`,
matching graph-v3's integral `vertex_id` expectation directly.

### Iterator Conversion

BGL's `vertices(g)` returns `std::pair<vertex_iterator, vertex_iterator>` where
`vertex_iterator` is `boost::counting_iterator<size_t>`. This does NOT satisfy C++20
`std::forward_iterator` (no `iterator_concept` defined by `boost::iterator_facade`).

**Solution:** The ADL `vertices(graph_adaptor<G>&)` function returns a
`std::views::iota(size_t{0}, boost::num_vertices(g))`. An `iota_view<size_t>` is a
C++20 `random_access_range` with integral elements. The CPO auto-wraps it in
`vertex_descriptor_view`, yielding `vertex_descriptor<iota_iterator>` objects where
`.vertex_id()` returns the `size_t` index directly.

```cpp
template <typename G>
auto vertices(const graph_adaptor<G>& ga) {
  using vid = typename boost::graph_traits<G>::vertex_descriptor; // size_t for vecS
  return std::views::iota(vid{0}, boost::num_vertices(ga.bgl_graph()));
}
```

BGL's `out_edges(u, g)` returns `std::pair<out_edge_iterator, out_edge_iterator>`.
The out_edge_iterator is based on `boost::iterator_facade` and does NOT satisfy C++20
iterator concepts.

**Solution:** Create a thin C++20 iterator adaptor that wraps a BGL out_edge_iterator.
This adaptor defines `iterator_concept = std::random_access_iterator_tag` (or
`forward_iterator_tag`, depending on the BGL iterator's actual capabilities) and forwards
all operations to the wrapped BGL iterator.

```cpp
template <typename BGL_Iter, typename BGL_Graph>
class bgl_edge_iterator {
  BGL_Iter it_;
public:
  using iterator_concept = std::forward_iterator_tag;
  using value_type = typename std::iterator_traits<BGL_Iter>::value_type;
  using difference_type = typename std::iterator_traits<BGL_Iter>::difference_type;

  bgl_edge_iterator& operator++() { ++it_; return *this; }
  decltype(auto) operator*() const { return *it_; }
  bool operator==(const bgl_edge_iterator& o) const { return it_ == o.it_; }
  // ... remaining iterator requirements
};
```

The ADL `out_edges` function then returns a `std::ranges::subrange` of these wrapped
iterators. The CPO auto-wraps the result in `edge_descriptor_view`.

### Edge ID Extraction

After CPO auto-wrapping, `edge_t<G>` is `edge_descriptor<bgl_edge_iterator, iota_iterator>`.
The `target_id` and `source_id` ADL overrides unwrap this:

```cpp
template <typename G, typename EdgeDesc>
auto target_id(const graph_adaptor<G>& ga, const EdgeDesc& uv) {
  const auto& bgl_edge = *uv.value();                       // Dereference to BGL edge
  return boost::target(bgl_edge, ga.bgl_graph());            // BGL extraction
}

template <typename G, typename EdgeDesc>
auto source_id(const graph_adaptor<G>& ga, const EdgeDesc& uv) {
  const auto& bgl_edge = *uv.value();
  return boost::source(bgl_edge, ga.bgl_graph());
}
```

### Concept Satisfaction Chain

With the above overrides in place, the graph_adaptor satisfies:

```
vertices(g)      → iota_view → auto-wrapped → vertex_descriptor_view  ✓ forward_range
vertex_id(g, u)  → vertex_descriptor::vertex_id() → size_t            ✓ integral
out_edges(g, u)  → subrange<bgl_edge_iterator> → auto-wrapped         ✓ forward_range
source_id(g, uv) → ADL override → size_t                              ✓
target_id(g, uv) → ADL override → size_t                              ✓
find_vertex(g, uid) → Tier 4 default: next(begin(vertices(g)), uid)   ✓ no override needed
num_vertices(g)  → ADL override                                        ✓

→ adjacency_list<graph_adaptor<G>>         ✓
→ index_adjacency_list<graph_adaptor<G>>   ✓ (vecS is random-access + integral IDs)
```

---

## 4. Phase 1B: Undirected and Bidirectional Variants

### Bidirectional (`bidirectionalS`)

Add an ADL `in_edges(graph_adaptor<G>&, vertex_desc)` override using the same
iterator-wrapping pattern as `out_edges`. This enables:

```
→ bidirectional_adjacency_list<graph_adaptor<G>>         ✓
→ index_bidirectional_adjacency_list<graph_adaptor<G>>   ✓
```

BGL bidirectional graphs store in-edges explicitly, so no extra mapping is needed.

### Undirected (`undirectedS`)

BGL undirected graphs expose both `out_edges(u, g)` and `in_edges(u, g)` that return
the same edge set (each edge visible from both endpoints). The same adaptor pattern works,
but `source_id` and `target_id` must correctly reflect the stored direction and the
traversal direction. BGL's `source(e, g)` and `target(e, g)` already handle this, so the
ADL overrides delegate to them unchanged.

One consideration: graph-v3 does not have a concept for undirected graphs.
The adaptor should model `bidirectional_adjacency_list` where `in_edges` and `out_edges`
return the same set. This needs to be verified as correct behavior for graph-v3's algorithms.

---

## 5. Phase 1C: `compressed_sparse_row_graph`

BGL's `compressed_sparse_row_graph` uses `size_t` vertex descriptors and
`csr_edge_descriptor<Vertex, EdgeIndex>` edge descriptors. The same wrapper pattern
applies with minor differences:

- `vertex_iterator` is `boost::counting_iterator<size_t>` — same `iota_view` approach.
- `out_edge_iterator` is `csr_out_edge_iterator` — needs the same C++20 iterator wrapper.
- Edge descriptor has `.src` and `.idx` members; `boost::source(e, g)` returns `.src`
  and `boost::target(e, g)` looks up the target from the CSR column array.
- CSR graphs are directed only; no in_edges needed.

The `graph_adaptor` template works for both `adjacency_list` and
`compressed_sparse_row_graph` because the ADL overrides call through `boost::graph_traits`
and BGL's own free functions, which are polymorphic across BGL graph types.

---

## 6. Property Map Bridge

### Read-Only Properties (e.g., Edge Weights)

Create a function object that wraps BGL's property access:

```cpp
template <typename BGL_Graph, typename Tag>
auto make_bgl_property_fn(const BGL_Graph& g, Tag tag) {
  auto pmap = boost::get(tag, g);
  return [pmap](const auto& adapted_graph, const auto& descriptor) {
    // Extract the BGL key from the descriptor and call get()
    return get(pmap, /* extracted BGL key */);
  };
}
```

For edge weights (the most common case with Dijkstra):

```cpp
template <typename BGL_Graph>
auto make_bgl_edge_weight_fn(const BGL_Graph& g) {
  return [&g](const auto& ga, const auto& uv) -> double {
    const auto& bgl_edge = *uv.value();  // Unwrap edge_descriptor to BGL edge
    return g[bgl_edge].weight;           // Access bundled property
  };
}
```

This satisfies `edge_value_function<EVF, G, edge_t<G>>` because it is invocable with
`(const G&, edge_descriptor)` and returns a non-void type.

### Writable Properties (e.g., Distances, Predecessors)

Algorithms like `dijkstra_shortest_paths` need writable distance and predecessor
accessors. These use `distance_fn_for<DistanceFn, G>` and
`predecessor_fn_for<PredecessorFn, G>`, which require the function to return a
reference.

For vertex-indexed properties (the common case with vecS):

```cpp
template <typename T>
auto make_vertex_property_fn(std::vector<T>& storage) {
  return [&storage](const auto& g, auto uid) -> T& {
    return storage[uid];  // uid is size_t for vecS
  };
}
```

This returns a reference, satisfying the writable requirement.

### Bundled Properties

BGL's `operator[]` returns a reference to the bundled property struct:
`g[v]` for vertex bundles, `g[e]` for edge bundles. The wrapper function objects
use this to provide both read and write access:

```cpp
// Vertex bundle access
auto vvf = [](const auto& ga, const auto& u) -> auto& {
  auto uid = u.vertex_id();
  return ga.bgl_graph()[uid];           // Returns VertexProps&
};

// Edge bundle member access
auto evf = [](const auto& ga, const auto& uv) -> const double& {
  const auto& bgl_edge = *uv.value();
  return ga.bgl_graph()[bgl_edge].weight;  // Returns member reference
};
```

---

## 7. Views and Algorithm Validation

### Phase-1 Views

| View | Key CPO Dependencies | Expected to Work |
|------|---------------------|------------------|
| `vertexlist(g)` | `vertices`, `vertex_id` | Yes — auto-wrapped iota_view |
| `incidence(g, u)` | `out_edges`, `target_id`, `source_id` | Yes — via ADL overrides |
| `neighbors(g, u)` | `out_edges`, `target_id`, `target` (→ `find_vertex`) | Yes — target falls through to default tier |
| `edgelist(g)` | `vertices`, `out_edges`, `source_id`, `target_id` | Yes — all dependencies covered |

### Phase-1 Algorithms

| Algorithm | Graph Concept Required | Additional Requirements | Priority |
|-----------|----------------------|------------------------|----------|
| `dijkstra_shortest_paths` | `adjacency_list` | `distance_fn_for`, `predecessor_fn_for`, `edge_weight_function` | First |
| `breadth_first_search` | `adjacency_list` | Visitor | Second |
| `depth_first_search` | `adjacency_list` | Visitor | Second |
| `bellman_ford_shortest_paths` | `adjacency_list` | Same as Dijkstra | Third |
| `connected_components` | `adjacency_list` | — | Third |
| `topological_sort` | `adjacency_list` | — | Third |

---

## 8. Testing Strategy

### Incremental Concept Verification

Build tests bottom-up, verifying concept satisfaction at each layer:

1. **Iterator wrapper tests.** Verify that `bgl_edge_iterator` satisfies
   `std::forward_iterator` (or `std::random_access_iterator`).
2. **CPO unit tests.** For each ADL override, verify it compiles and returns the correct
   type and value against a known BGL graph.
3. **Concept satisfaction tests.** Static assertions:
   ```cpp
   static_assert(graph::adjacency_list<graph::bgl::graph_adaptor<bgl_adj_graph_t>>);
   static_assert(graph::index_adjacency_list<graph::bgl::graph_adaptor<bgl_adj_graph_t>>);
   ```
4. **View integration tests.** Iterate `vertexlist`, `incidence`, `neighbors`, `edgelist`
   on adapted BGL graphs and compare results against direct BGL traversal.
5. **Algorithm end-to-end tests.** Run `dijkstra_shortest_paths` on an adapted BGL graph
   and compare distances/predecessors against BGL's own Dijkstra on the same graph.
   The existing `benchmark/algorithms/bgl_dijkstra_fixtures.hpp` provides graph-building
   infrastructure for this.

### Graph Configurations to Test

| BGL Graph Type | Selector | directedness |
|----------------|----------|-------------|
| `adjacency_list<vecS, vecS, directedS>` | vecS/vecS | directed |
| `adjacency_list<vecS, vecS, undirectedS>` | vecS/vecS | undirected |
| `adjacency_list<vecS, vecS, bidirectionalS>` | vecS/vecS | bidirectional |
| `compressed_sparse_row_graph<directedS>` | — | directed |

---

## 9. Performance

The adaptor adds several layers between a graph-v3 algorithm call and the underlying BGL
data. Most are zero-cost in optimized builds; one deserves attention.

### Zero-Cost Layers (Inline Away at -O2)

| Layer | What Happens | Why It's Free |
|-------|-------------|---------------|
| `graph_adaptor` | Single pointer dereference to reach the BGL graph | Trivial inline |
| `vertices(g)` → `iota_view` | Replaces BGL's `counting_iterator` with `iota_view` | Native graph-v3 graphs go through the same `vertex_descriptor_view` wrapping |
| `vertex_descriptor` synthesis | Created by value on each dereference | Same as native graph-v3 |
| `target_id` / `source_id` ADL | Unwrap `edge_descriptor`, dereference to BGL edge, call `boost::target(e, g)` | Reads `.m_target` inline |

### Layer With Real Inlining Pressure

`bgl_edge_iterator` wraps each BGL out-edge iterator, forwarding `++`, `*`, `==`. The
compiler must inline through the wrapper AND through BGL's `iterator_facade` underneath.
With `-O2` this should collapse, but it is the deepest call chain:

```
graph-v3 CPO → edge_descriptor_view → bgl_edge_iterator → BGL iterator_facade → storage
```

### Compared to Native graph-v3 Usage

Essentially zero overhead. Both paths go through the same CPO dispatch, descriptor view
wrapping, and value synthesis. The only difference is that the final data access calls
BGL's inline free functions instead of reading graph-v3 container members directly.

### Compared to Using BGL Directly

The adaptor adds the descriptor-wrapping layer that BGL does not have. In tight inner
loops (e.g., relaxing edges in Dijkstra), this means an extra `edge_descriptor` value
synthesized per edge visit. Whether that optimizes away depends on inlining depth. The
risk is small but measurable for very large graphs.

### Validation

`benchmark/algorithms/bgl_dijkstra_fixtures.hpp` already runs BGL Dijkstra and graph-v3
Dijkstra on identical graphs. Add an adapted-BGL benchmark alongside to get empirical
confirmation. If `bgl_edge_iterator` shows up in profiles, it can be specialized (e.g.,
store a raw pointer + offset instead of wrapping the full BGL iterator).

---

## 10. Risks

### Risk 1 (High): BGL Iterators Do Not Satisfy C++20 Iterator Concepts

**Impact:** This is the single biggest technical risk. BGL's iterators are built on
`boost::iterator_facade` / `boost::iterator_adaptor`, which define `iterator_category`
but NOT `iterator_concept`. C++20 `std::ranges` and graph-v3's descriptor views require
C++20 iterator concepts (`std::forward_iterator`, `std::random_access_iterator`).

Without mitigation, `std::ranges::subrange` over BGL iterators may not model
`std::ranges::forward_range`, and graph-v3's `edge_descriptor_view` template
constraints will reject BGL edge iterators.

**Mitigation:** Create a thin C++20 iterator adaptor (`bgl_edge_iterator`) that wraps
a BGL iterator and adds the required `iterator_concept` typedef. For vertex iteration,
bypass BGL iterators entirely and use `std::views::iota` (since vecS vertex descriptors
are plain integers).

**Validation:** Test `static_assert(std::forward_iterator<bgl_edge_iterator<...>>)` early
in development. If BGL edge iterators have behavior that doesn't satisfy C++20 requirements
(e.g., non-regular reference types), the adaptor may need to synthesize values rather than
forwarding references.

### Risk 2 (High): Descriptor Unwrapping Across CPO Tiers

**Impact:** graph-v3 CPOs auto-wrap return values. `out_edges` wraps the edge range in
`edge_descriptor_view`, which yields `edge_descriptor<EdgeIter, VertexIter>` on iteration.
The ADL overrides for `target_id(g, uv)` and `source_id(g, uv)` receive these wrapped
descriptors and must unwrap them to access the BGL edge underneath (via `.value()`
dereference).

If graph-v3's auto-wrapping changes, or if the edge_descriptor's `.value()` method does
not expose the BGL edge correctly, the extraction breaks silently.

**Mitigation:** Pin the unwrapping logic to the documented `edge_descriptor` interface
(`uv.value()` returns the edge iterator, `*uv.value()` yields the BGL edge). Add
compile-time assertions that verify the unwrapped type is the expected BGL edge_descriptor
type.

### Risk 3 (Medium): Undirected Graph Semantics

**Impact:** graph-v3 does not define an `undirected_adjacency_list` concept. BGL
undirected graphs expose each edge from both endpoints via `out_edges` and `in_edges`.
Algorithms that assume directed semantics (e.g., edgelist counting, triangle counting)
may double-count edges.

**Mitigation:** Start with directed graphs. For undirected BGL graphs, validate algorithm
results against BGL's own results on the same graph before declaring the adaptation
correct. Document any edge-counting differences.

### Risk 4 (Medium): Uniform Property Function Dependency

**Impact:** `uniform_prop_goal.md` is an in-progress effort to standardize property access
via function objects. If the `distance_fn_for` and `predecessor_fn_for` concepts change as
part of that work, the property bridge code must change too.

**Mitigation:** Build the property bridge against the current algorithm signatures. Keep
property bridge code isolated in its own header so it can be updated independently.

### Risk 5 (Low): Compile Time

**Impact:** BGL headers are template-heavy. Mixing BGL and graph-v3 template instantiations
in the same translation unit may cause long compile times, especially with deep CPO tier
resolution and descriptor wrapping.

**Mitigation:** Provide a precompiled adaptor header for common BGL configurations. Keep
BGL `#include` directives out of the core graph-v3 headers.

### Risk 6 (Low): BGL Version Sensitivity

**Impact:** BGL's internal types (e.g., `detail::edge_desc_impl`) and iterator
implementations vary across Boost versions. The adaptor code that unwraps edge descriptors
or wraps BGL iterators could break on older or newer Boost releases.

**Mitigation:** Code against BGL's public API (`boost::source`, `boost::target`,
`boost::vertices`, `boost::graph_traits`) rather than internal types. Document the
minimum Boost version tested.

---

## 11. Missing Elements in the Current Library

These are gaps in graph-v3 that may need to be addressed to fully support BGL adaptation.

### 10.1 No Iterator Compatibility Layer

graph-v3 assumes C++20 iterators throughout. There is no utility for wrapping pre-C++20
iterators. The BGL adaptor will need to build its own `bgl_edge_iterator` wrapper. If other
external graph libraries are adapted in the future, this wrapper pattern will be duplicated.

**Recommendation:** Consider a general-purpose `legacy_iterator_adaptor<LegacyIter>` utility
in `include/graph/detail/` that adds `iterator_concept` to any C++17 iterator. This is
optional for the BGL work but would reduce boilerplate for future adaptations.

### 10.2 No Undirected Graph Concept

graph-v3 defines `adjacency_list` and `bidirectional_adjacency_list` but has no concept
that distinguishes undirected from directed graphs. BGL uses `directed_category` traits
(`directed_tag`, `undirected_tag`, `bidirectional_tag`). Without a corresponding
graph-v3 concept, algorithms cannot dispatch on directedness.

**Recommendation:** For phase 1, treat undirected BGL graphs as bidirectional (both
`out_edges` and `in_edges` return the same set). Note this as a documentation item for
BGL users.

### 10.3 No Graph Type Traits Customization Point

graph-v3 derives type aliases like `vertex_id_t<G>`, `edge_t<G>`, `vertex_t<G>` from
CPO return types. There is no explicit traits struct like BGL's `graph_traits<G>`. This
is fine for native graph-v3 types, but for adapted types the aliases depend on the full
CPO → auto-wrap → descriptor chain resolving correctly. If any link breaks, the error
messages will be deep template errors with no clear indication of what went wrong.

**Recommendation:** Add `static_assert` concept checks with clear error messages in the
adaptor header. Example:
```cpp
static_assert(adjacency_list<graph_adaptor<BGL_G>>,
    "BGL graph adaptation failed: graph_adaptor does not satisfy adjacency_list. "
    "Check that vertices(), out_edges(), target_id(), and source_id() overloads are visible.");
```

### 10.4 No edge_value / vertex_value Default for Bundled Properties

graph-v3's `edge_value(g, uv)` and `vertex_value(g, u)` CPOs check for member functions
and ADL overrides, but have no way to automatically expose BGL bundled properties. The
adaptor must provide explicit ADL overrides for these.

For BGL graphs with bundled properties, the user typically expects direct access
(BGL's `g[v]` and `g[e]`). The adaptor should provide convenience overrides so that
`vertex_value(g, u)` and `edge_value(g, uv)` work without the user writing custom
function objects for basic property access.

### 10.5 Adaptor Discoverability

There is no existing example or documentation for adapting external graph types. The
transpose_view is the closest pattern (ADL friend functions), but it wraps graph-v3 types,
not foreign types.

**Recommendation:** The BGL adaptor should be accompanied by documentation covering:
1. How to wrap a BGL graph.
2. How to pass BGL properties to graph-v3 algorithms.
3. A complete end-to-end example (Dijkstra on a BGL graph).
4. Known limitations and unsupported BGL features.
