# Algorithm Documentation Revision Plan

This plan describes the changes required to bring all algorithm documentation pages
(`docs/user-guide/algorithms/*.md`) and their corresponding header files
(`include/graph/algorithm/*.hpp`) into full compliance with `algorithm_doc_guide.md`.
The revised pages will serve as the basis for the Standards proposal (P1709 / D3128).

---

## How to Use This Plan

Work through the pages in the order listed under **Per-Page Work Items**.
For each page:
1. Read the header file to verify current signatures, throws, postconditions.
2. Apply the **Global Changes** that apply to every page.
3. Apply the **Page-Specific Changes** listed for that page.
4. Verify against the **Checklist** at the end of this document.

---

## Global Changes (Apply to Every Page)

These changes are required on all 13 algorithm pages regardless of algorithm.

### G1 — Rename "Notes" → "Remarks"

Five pages already have a `## Notes` section. Rename it to `## Remarks` to match
the C++ standard vocabulary adopted in `algorithm_doc_guide.md`.

Affected: `jaccard.md`, `label_propagation.md`, `mis.md`, `mst.md`, `triangle_count.md`

Also update the TOC anchor in each affected page:
- `- [Notes](#notes)` → `- [Remarks](#remarks)`

### G2 — Add "Remarks" section to pages that currently lack it

Eight pages have no Notes/Remarks section. Add one wherever there are behavioral
details worth stating for a Standards reader (see page-specific items below).

Affected: `articulation_points.md`, `bellman_ford.md`, `bfs.md`,
`biconnected_components.md`, `connected_components.md`, `dfs.md`, `dijkstra.md`,
`topological_sort.md`

### G3 — Add "Throws" section to every page

Every algorithm header documents exception behavior. Extract it into a dedicated
`## Throws` section (between Postconditions and Remarks in the section order).
For algorithms that are `noexcept` or only propagate allocator exceptions,
state that explicitly rather than omitting the section.

See per-page items for exact exception types and guarantees.

### G4 — Add "Postconditions" section to pages with output ranges

For every algorithm that writes result values into caller-supplied ranges, add a
`## Postconditions` section (between Preconditions and Throws) that precisely states
what each output range contains after a successful return.

### G5 — Expand "Returns" descriptions in Signatures

Where the return type is non-void, the Signatures section must document both the
type and its full value semantics — including what each member means and any
sentinel/special values. See per-page items.

### G6 — Add template-parameter concept requirements to Parameters tables

Every Parameters table must document the concept constraints on template parameters
(e.g., "`G` must satisfy `index_adjacency_list<G>`"). Currently these appear only
in the Overview or Preconditions sections.

### G7 — Update TOC in every page

Add the new sections to each page's Table of Contents in the correct order:

```
- [Postconditions](#postconditions)     ← (if applicable)
- [Throws](#throws)
- [Remarks](#remarks)                   ← (if applicable)
```

---

## Per-Page Work Items

### 1. `dijkstra.md` ← `dijkstra_shortest_paths.hpp`

**Priority: High** — most complex page; canonical model for the Standards proposal.

#### Sections to add / change

**Throws** (new section, extract from inline "Error handling" note in Parameters):
- `std::out_of_range` — any source vertex id is outside `[0, num_vertices(g))`.
  Basic exception guarantee: no distances or predecessors are modified.
- `std::logic_error` — internal invariant violation (should not arise in
  correct usage); indicates a library bug.
- `std::bad_alloc` — priority queue allocation fails.
- Weight function and visitor callback exceptions are propagated unchanged.

**Postconditions** (new section):
- For every vertex `v` reachable from the source(s): `distances[v]` holds the
  shortest-path distance; `predecessors[v]` holds the id of `v`'s predecessor
  on the shortest path, or `v` itself if `v` is a source.
- For every unreachable vertex `v`: `distances[v]` is unchanged from its
  pre-call value (typically `numeric_limits<EV>::max()` after `init_shortest_paths`).
- For `dijkstra_shortest_distances` overloads: only `distances` is guaranteed;
  no predecessor information is available.

**Returns** (update in Signatures):
- All overloads are `void`. Add a note in the Signatures section: "Results are
  written into the caller-supplied `distances` and `predecessors` output ranges.
  See Postconditions."

**Remarks** (new — move "Error handling" prose out of Parameters):
- The "Error handling" paragraph currently embedded in Parameters (line 125)
  should become the Throws section. Remove it from Parameters.
- Note that `dijkstra_shortest_distances` internally uses a null-predecessor
  range, avoiding the cost of maintaining predecessor state.

**Parameters**:
- Add concept constraints for `G`, `Sources`, `WF`, `Visitor`, `Compare`,
  `Combine` template parameters.

---

### 2. `bellman_ford.md` ← `bellman_ford_shortest_paths.hpp`

**Priority: High** — non-trivial return type; negative-cycle detection.

#### Sections to add / change

**Throws** (new section):
- `std::out_of_range` — any source vertex id is outside `[0, num_vertices(g))`.
  Basic guarantee.
- `std::bad_alloc` — internal allocation fails.
- Weight function exceptions are propagated unchanged.

**Postconditions** (new section):
- Returns `nullopt`: `distances[v]` and `predecessors[v]` hold shortest-path
  values for all reachable vertices (same guarantees as Dijkstra postconditions).
- Returns `optional<vertex_id_t<G>> id`: `id` is a vertex on a negative-weight
  cycle; the cycle can be recovered by following `predecessors` from `id`.
  `distances` contents are unspecified when a negative cycle is detected.

**Returns** (expand in Signatures):
- `[[nodiscard]] constexpr optional<vertex_id_t<G>>` — returns `nullopt` if no
  negative cycle is detected, or the id of a vertex on a negative-weight cycle.

**Remarks** (new):
- Weight function must be pure (no side effects, no graph modification).
- The V·E time bound holds regardless of graph connectivity.

**Parameters**: add concept constraints.

---

### 3. `bfs.md` ← `breadth_first_search.hpp`

**Priority: Medium**

#### Sections to add / change

**Throws** (new section):
- `std::bad_alloc` — visited array or internal queue allocation fails.
  Basic guarantee: visited markers and output ranges are in a valid but
  unspecified state.
- Visitor callback exceptions are propagated unchanged.

**Postconditions** (new section):
- Each vertex reachable from the source is visited exactly once.
- Visitor callbacks are invoked in breadth-first order; `on_discover_vertex`
  precedes `on_examine_vertex` for the same vertex.

**Remarks** (new):
- Vertices unreachable from the source are never visited and no callbacks are
  invoked for them.
- For multi-source BFS, all sources are enqueued simultaneously at distance 0
  before traversal begins.

**Parameters**: add concept constraints for `G`, `Visitor`.

---

### 4. `dfs.md` ← `depth_first_search.hpp`

**Priority: Medium**

#### Sections to add / change

**Throws** (new section):
- `std::bad_alloc` — color array or internal stack allocation fails.
  Basic guarantee.
- Visitor callback exceptions are propagated unchanged.

**Postconditions** (new section):
- Every vertex is visited and colored; callbacks follow LIFO (stack-based)
  depth-first order.
- `on_finish_vertex` is called for every vertex after all its descendants
  have been explored.

**Remarks** (new):
- The implementation is iterative (stack-based), not recursive. Maximum
  nesting depth is bounded by the implementation's stack, not the graph depth.
- Single-source limitation: the single-source overload only visits vertices
  reachable from the specified source.

**Parameters**: add concept constraints.

---

### 5. `connected_components.md` ← `connected_components.hpp`

**Priority: Medium** — three sub-algorithms (Kosaraju, Union-Find, Afforest).

#### Sections to add / change

**Throws** (new section, per sub-algorithm):
- All three: `std::bad_alloc` — internal allocation fails. Basic guarantee.

**Postconditions** (new section, per sub-algorithm):
- For every vertex `v`: `component[v]` contains a non-negative integer label
  identifying its connected component. All vertices in the same component have
  the same label.
- Labels are not guaranteed to be contiguous or in any particular order unless
  `compress()` is called afterward.
- Kosaraju (directed graph): labels reflect strongly connected components.
- Union-Find / Afforest (undirected): labels reflect weakly connected components.

**Remarks** (new):
- Call `compress(component)` after the algorithm to normalize labels to the
  range `[0, k)` where `k` is the number of components.
- Afforest is designed for very large sparse graphs; it uses sampling to skip
  the most expensive union operations.

**Parameters**: add concept constraints.

---

### 6. `mst.md` ← `mst.hpp`

**Priority: Medium** — two sub-algorithms (Kruskal + Prim); `inplace_kruskal` warning.

#### Sections to add / change

**Rename** `## Notes` → `## Remarks` (Global G1).

**Throws** (new section, per sub-algorithm):
- Kruskal: `std::bad_alloc`. May propagate exceptions from comparison operators
  (recommend `noexcept` comparators).
- Prim: `std::out_of_range` — seed vertex id is outside `[0, num_vertices(g))`.
  `std::bad_alloc`. Basic guarantee.

**Postconditions** (new section, per sub-algorithm):
- Kruskal output range contains the edges of a minimum spanning tree (or forest)
  sorted by weight. The range is a subset of the graph's edges.
- Prim `weight[v]` contains the weight of the edge connecting `v` to its MST
  parent; `predecessor[v]` is the parent vertex id. `weight[seed] = EV{}`.

**Returns** (expand):
- Kruskal: iterators/range; state what the output edge descriptor contains.
- Prim: `void`.

**Remarks**: keep existing content; add note that `inplace_kruskal` modifies
the caller's edge range in-place (sort order is not preserved).

**Parameters**: add concept constraints for `G`, `WF`, `OutIter`.

---

### 7. `topological_sort.md` ← `topological_sort.hpp`

**Priority: Medium** — non-void return type (`bool`); three overloads.

#### Sections to add / change

**Throws** (new section):
- `std::bad_alloc` — internal stack or color array allocation fails.
  Basic guarantee: output range and color array are in a valid but unspecified
  state.

**Postconditions** (new section):
- Returns `true` (no cycle): the output range contains all vertex ids in
  topological order — for every directed edge `(u, v)`, `u` appears before `v`.
- Returns `false` (cycle detected): the output range is partially filled and
  its contents are unspecified; the cycle itself is not reported.

**Returns** (expand in Signatures):
- `bool` — `true` if the graph is a DAG and the sort succeeded; `false` if a
  cycle was detected. The return value is `[[nodiscard]]`; ignoring it silently
  discards cycle-detection information.

**Remarks** (new):
- Returning `false` rather than throwing on cycle detection is intentional:
  cycles are expected conditions in many applications (e.g., iterative
  dependency resolution). See header comments for rationale.
- For small graphs, recursive DFS may be slightly faster; the implementation
  uses iterative DFS to avoid stack overflow on deep graphs.

**Parameters**: add concept constraints.

---

### 8. `articulation_points.md` ← `articulation_points.hpp`

**Priority: Low-Medium**

#### Sections to add / change

**Throws** (new section):
- `std::bad_alloc` — internal `visited`, `disc`, `low`, `parent` arrays.
  Basic exception safety: output iterator may have received partial results.

**Postconditions** (new section):
- The output iterator receives every vertex id that is an articulation point
  (cut vertex) of the undirected graph exactly once, in no guaranteed order.
- If the graph is biconnected, no vertex ids are written to the output.

**Remarks** (new):
- An articulation point is a vertex whose removal increases the number of
  connected components.
- The algorithm treats the graph as undirected; for directed graphs, results
  are undefined.

**Parameters**: add concept constraints for `G`, `Iter`.

---

### 9. `biconnected_components.md` ← `biconnected_components.hpp`

**Priority: Low-Medium**

#### Sections to add / change

**Throws** (new section):
- `std::bad_alloc` — internal stack or component container allocation fails.
  Basic guarantee.

**Postconditions** (new section):
- `components` is an outer container of edge-lists. Each inner list contains
  the edges of one biconnected component. Every edge of the graph belongs to
  exactly one component.
- Bridge edges (single-edge biconnected components) appear as singleton inner
  lists.

**Remarks** (new):
- A biconnected component is a maximal subgraph with no articulation points.
- The algorithm treats the graph as undirected.

**Parameters**: add concept constraints for `G`, `OuterContainer`.

---

### 10. `jaccard.md` ← `jaccard.hpp`

**Priority: Low-Medium**

#### Sections to add / change

**Rename** `## Notes` → `## Remarks` (Global G1).

**Throws** (new section):
- `std::bad_alloc` — internal neighbor-set allocation fails. Basic guarantee.

**Postconditions** (new section):
- The output callable `out` is invoked once for every edge `(u, v)` with
  `u < v`, receiving the edge reference and the Jaccard coefficient in `[0, 1]`.
- Coefficient is `0` for edges where the endpoint neighborhoods are disjoint;
  `1` for edges where endpoints have identical neighborhoods.

**Returns** (expand in Signatures):
- `void`. Results delivered via the `out` callback. State callback signature.

**Parameters**: add concept constraints for `G`, `OutOp`.

---

### 11. `label_propagation.md` ← `label_propagation.hpp`

**Priority: Low-Medium**

#### Sections to add / change

**Rename** `## Notes` → `## Remarks` (Global G1).

**Throws** (new section):
- `std::bad_alloc` — internal label copy allocation fails. Basic guarantee.
- RNG exceptions are propagated unchanged.

**Postconditions** (new section):
- `labels[v]` contains a community label in `[0, num_vertices(g))` for every
  vertex `v`.
- Upon convergence, no vertex has a label different from the majority label
  among its neighbors. Convergence is not guaranteed in all cases (see Remarks).

**Parameters**: add concept constraints for `G`, `RNG`.

---

### 12. `mis.md` ← `mis.hpp`

**Priority: Low**

#### Sections to add / change

**Rename** `## Notes` → `## Remarks` (Global G1).

**Throws** (new section):
- `std::bad_alloc` — internal allocation fails. Basic guarantee.

**Postconditions** (new section):
- `labels[v]` is `1` if vertex `v` is in the independent set, `0` otherwise.
- The result is a **maximal** independent set: no non-member vertex can be added
  without violating independence. It is not necessarily **maximum** (largest
  possible).

**Parameters**: add concept constraints.

---

### 13. `triangle_count.md` ← `tc.hpp`

**Priority: Low**

#### Sections to add / change

**Rename** `## Notes` → `## Remarks` (Global G1).

**Throws** (new section):
- Never throws. The algorithm uses only non-throwing arithmetic and iteration.
  **Strong exception safety** (effectively `noexcept`).

**Postconditions** (new section):
- The return value is the exact number of triangles in the graph. Each triangle
  `{u, v, w}` is counted once regardless of graph representation.

**Returns** (expand in Signatures):
- `size_t` (or the algorithm's actual return type) — the total number of
  triangles. Equals zero for graphs with fewer than 3 vertices or no cycles.

**Remarks**: keep existing content (pre-sorting requirement for
`ordered_vertex_edges<G>`).

**Parameters**: add concept constraints for `G` including the additional
`ordered_vertex_edges<G>` requirement.

---

## Source File (Header) Changes

The header files contain the canonical documentation for a Standards proposal.
The following header-level changes are needed in parallel with the doc-page updates.

### H1 — Normalize exception documentation format across all headers

Current headers use inconsistent formats (`@throws`, `* - May throw`, `**Throws:**`).
Standardize to a single format compatible with the Standards proposal style.
Suggested format in header comments:

```
* Throws:
*   std::out_of_range if <condition>.
*   std::bad_alloc if internal allocation fails.
```

Headers needing normalization: all 13 (each has a different style).

### H2 — Add postcondition annotations where missing

Headers for `dijkstra_shortest_paths.hpp`, `bellman_ford_shortest_paths.hpp`,
`topological_sort.hpp`, and `connected_components.hpp` partially document
postconditions inline. Align them with the postconditions written for the doc pages.

### H3 — Verify `[[nodiscard]]` placement

- `bellman_ford_shortest_paths.hpp`: `[[nodiscard]]` ✅ present.
- `topological_sort.hpp`: verify `[[nodiscard]]` is present on all three overloads
  that return `bool`.
- `tc.hpp`: verify return-type annotation is present.
- All `void`-returning functions: `[[nodiscard]]` is not applicable; confirm absent.

### H4 — Add `noexcept` to `triangle_count` (`tc.hpp`)

The header states the function never throws and uses only non-throwing operations.
Mark the function `noexcept` to make this part of the interface contract.

---

## Revision Order (Recommended)

Process pages in this order to maximize reuse of patterns established by earlier pages:

| Order | Page                        | Reason                                                   |
| ----- | --------------------------- | -------------------------------------------------------- |
| 1     | `dijkstra.md`               | Canonical model; most complete starting point            |
| 2     | `bellman_ford.md`           | Similar structure; adds `optional` return                |
| 3     | `topological_sort.md`       | Non-void `bool` return; establishes pattern              |
| 4     | `bfs.md`                    | Simpler; establishes visitor postcondition pattern       |
| 5     | `dfs.md`                    | Mirror of BFS with richer visitor events                 |
| 6     | `connected_components.md`   | Multi-algorithm; adds postcondition-per-sub-algo pattern |
| 7     | `mst.md`                    | Multi-algorithm; Throws per sub-algo                     |
| 8     | `articulation_points.md`    | Simpler output-iterator pattern                          |
| 9     | `biconnected_components.md` | Container-output pattern                                 |
| 10    | `jaccard.md`                | Callback-output pattern                                  |
| 11    | `label_propagation.md`      | Convergence postconditions                               |
| 12    | `mis.md`                    | Simple labeling postcondition                            |
| 13    | `triangle_count.md`         | Simplest: noexcept, scalar return                        |

---

## Verification Checklist

After completing each page, verify the following against `algorithm_doc_guide.md`:

- [ ] TOC lists all sections in the correct order including Postconditions, Throws,
      Error Conditions (if applicable), and Remarks (if applicable).
- [ ] Section heading uses "Remarks" (not "Notes").
- [ ] Signatures: return type and value semantics fully described; sentinel values
      stated; `[[nodiscard]]` shown where present in header.
- [ ] Parameters table: template parameter concept constraints documented.
- [ ] Postconditions section present if algorithm writes output ranges; states
      exactly what each element contains after a successful return.
- [ ] Throws section present; exception type, triggering condition, and exception
      guarantee (strong / basic / noexcept) all stated.
- [ ] Error Conditions section present if algorithm uses `std::error_code`
      (currently none; verify this remains true).
- [ ] Remarks section present for any behavioral details, caveats, or design
      rationale beyond Preconditions / Postconditions / Throws.
- [ ] At least 5 examples; progress from basic to advanced; expected output in
      comments for every example.
- [ ] All content matches the actual header file exactly (signatures, attributes,
      visitor event names).
- [ ] `index_adjacency_list<G>` requirement stated in both Overview and
      Preconditions.
- [ ] All cross-links resolve.
- [ ] Algorithm catalog (`algorithms.md`) complexity row matches the page.
