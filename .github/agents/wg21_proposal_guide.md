# Agent Instructions: WG21 Standards Proposal Wording

This document provides instructions for an agent writing or revising
**proposed wording** sections for WG21 (ISO C++ Standards Committee) papers
related to the graph-v3 library. These papers target LEWG and are part of
the P3126–P3337 series (see
[`docs/standardization.md`](../docs/standardization.md)).

> **Not to be confused with** `algorithm_doc_guide.md`, which targets
> user-facing documentation pages and in-header Doxygen. This guide targets
> the formal proposed-wording text that appears in WG21 papers.

---

## Scope

This guide covers:

1. **Proposed Wording** sections of WG21 papers (the normative text intended
   for inclusion in the C++ Standard draft).
2. **Function specification** blocks using the vocabulary defined in
   §16.3.2.4 of the C++26 Standard (see
   [wg21_function_semantics.md](wg21_function_semantics.md)).
3. **Clause and sub-clause structure** conforming to current C++ Standard
   editorial conventions.

It does **not** cover the non-normative sections of a paper (Motivation,
Design Discussion, Implementation Experience, etc.) — those are prose and
follow paper-author conventions.

---

## Reference Documents

| Document | Purpose |
|----------|---------|
| [wg21_function_semantics.md](wg21_function_semantics.md) | C++26 §16.3.2.4 function-semantics definitions |
| [algorithm_doc_guide.md](algorithm_doc_guide.md) | User-doc and Doxygen guide (shares §16.3.2.4 vocabulary) |
| [docs/standardization.md](../docs/standardization.md) | Paper series overview (P3126–P3337) |
| C++ Standard draft ([eel.is/c++draft](https://eel.is/c++draft)) | Current working draft for editorial style reference |

---

## Paper Structure

A complete WG21 paper typically includes these sections. Only the
**Proposed Wording** section contains normative text governed by this guide.

```
Document number:   PxxxxRn
Date:              YYYY-MM-DD
Audience:          LEWG / LWG
Reply-to:          Author Name <email>

1. Abstract
2. Revision History
3. Motivation
4. Design Discussion / Design Decisions
5. Proposed Wording              ← this guide's primary target
6. Impact on the Standard
7. Feature-Test Macros           ← (if applicable)
8. Implementation Experience
9. Acknowledgements
10. References
```

---

## Proposed Wording Conventions

### Editorial style

WG21 proposed wording uses specific typographic and editorial conventions.
Follow the current C++ Standard draft (available at
[eel.is/c++draft](https://eel.is/c++draft)) as the style reference.

- **Insertions**: Mark new text with underline or green highlighting
  (`<ins>text</ins>` in HTML papers, or `<span style="color:green">` for
  additions). In LaTeX papers, use `\added{text}`.
- **Deletions**: Mark removed text with strikethrough or red highlighting
  (`<del>text</del>` in HTML, `\removed{text}` in LaTeX).
- **Stable references**: Use stable clause names and numbers
  (e.g., "[graph.algo.dijkstra]") rather than section numbers alone, since
  section numbers shift between drafts.
- **Cross-references**: Use the form "[lib.clause.name]" for internal
  references. Use "as-if" references to existing standard clauses
  (e.g., "[algorithms.requirements]").
- **Notes and examples**: Use `[_Note N_: ... —_end note_]` and
  `[_Example N_: ... —_end example_]` formatting. Notes are non-normative.
- **Italics**: Element labels (_Constraints:_, _Mandates:_, _Preconditions:_,
  etc.) are set in italics in the C++ Standard.
- **Code font**: All code identifiers, types, and expressions use code font
  (monospace / `\tcode{}` in LaTeX).

### Clause numbering

Proposed wording should mirror existing Standard structure where possible:

```
XX.Y     General                                [graph.general]
XX.Y.1   Header <graph> synopsis                [graph.syn]
XX.Y.2   Graph concepts                         [graph.concepts]
XX.Y.3   Graph algorithms                       [graph.algo]
XX.Y.3.1   Breadth-first search                 [graph.algo.bfs]
XX.Y.3.2   Depth-first search                   [graph.algo.dfs]
XX.Y.3.3   Shortest paths                       [graph.algo.shortest]
XX.Y.3.3.1   Dijkstra's algorithm               [graph.algo.dijkstra]
XX.Y.3.3.2   Bellman-Ford algorithm              [graph.algo.bellman]
...
```

Use placeholder clause numbers (XX.Y) — the project editor assigns final
numbers during integration.

---

## Function Specification Format

Each function in proposed wording is specified using the §16.3.2.4 semantic
elements, in the standard's canonical order. Elements that do not apply are
omitted (Note 135).

### Canonical element order

1. _Constraints:_
2. _Mandates:_
3. _Constant When:_
4. _Preconditions:_
5. _Hardened preconditions:_
6. _Effects:_
7. _Synchronization:_
8. _Postconditions:_
9. _Result:_
10. _Returns:_
11. _Throws:_
12. _Complexity:_
13. _Remarks:_
14. _Error conditions:_

### Template: single function specification

```
template<class G, class WF, class Distances, class Predecessors, class Visitor>
  requires adjacency_list<G>
        && basic_edge_weight_function<G, WF, ranges::range_value_t<Distances>>
        && vertex_property_map_for<Distances, G>
        && vertex_property_map_for<Predecessors, G>
constexpr void dijkstra_shortest_paths(
    G&&           g,
    vertex_id_t<G> source,
    WF            wf,
    Distances&&   distances,
    Predecessors&& predecessor,
    Visitor&&     visitor);
```

> _Constraints:_ `adjacency_list<G>` is modeled.
>
> _Mandates:_ `basic_edge_weight_function<G, WF, ranges::range_value_t<Distances>>`
> is satisfied. `vertex_property_map_for<Distances, G>` and
> `vertex_property_map_for<Predecessors, G>` are satisfied.
>
> _Preconditions:_ `source` is a valid vertex ID in `g`.
> `distances` and `predecessor` each contain an entry for every vertex in `g`.
> All edge weights returned by `wf` are non-negative.
>
> _Effects:_ For every vertex `v` reachable from `source`, sets
> `distances[v]` to the shortest-path distance and `predecessor[v]` to the
> preceding vertex on a shortest path. For unreachable vertices `v`, sets
> `distances[v]` to `numeric_limits<ranges::range_value_t<Distances>>::max()`
> and leaves `predecessor[v]` unchanged. Does not modify `g`.
>
> _Postconditions:_ For every reachable vertex `v`,
> `distances[v] <= distances[u] + wf(g, e)` for every edge `e` from `u` to
> `v`, where `u = predecessor[v]`.
>
> _Throws:_ Any exception thrown by `wf` or `visitor` callbacks. If an
> exception is thrown, output ranges are in a valid but unspecified state
> (basic guarantee).
>
> _Complexity:_ $O((V + E) \log V)$ time, $O(V)$ additional space, where
> $V$ is the number of vertices and $E$ is the number of edges in `g`.

### Element-by-element guidance

#### _Constraints:_

- State conditions required for overload resolution participation.
- Use concept names from the library: `adjacency_list<G>`,
  `index_adjacency_list<G>`, `bidirectional_adjacency_list<G>`,
  `ordered_vertex_edges<G>`, `x_index_edgelist_range<IELR>`,
  `vertex_property_map_for<T, G>`, `basic_edge_weight_function<G, WF, EV>`.
- Use "is modeled" when the constraint is a semantic requirement; "is
  satisfied" when purely syntactic.
- Constraints map to `requires` clauses or SFINAE in the reference
  implementation (`include/graph/algorithm/`).

#### _Mandates:_

- State conditions whose violation renders the program ill-formed.
- Map to `static_assert` or sole-overload concept requirements in the
  reference implementation.
- Use "is satisfied" for syntactic concept checks.

#### _Constant When:_

- Include only for functions declared `constexpr`.
- State the conditions under which the call is a constant subexpression.
- Currently applicable to: `dijkstra_shortest_paths`,
  `bellman_ford_shortest_paths`, `breadth_first_search`,
  `depth_first_search`.

#### _Preconditions:_

- State runtime conditions whose violation causes undefined behavior.
- Always include valid-vertex-ID requirements, output-range sizing, weight
  constraints, and graph-structure assumptions (e.g., "the graph is
  connected" or "for undirected graphs, both directions of each edge are
  stored").
- Self-loop behavior may be stated here or in _Remarks:_, depending on
  whether it is a requirement or an observation.

#### _Hardened preconditions:_

- Include only when the implementation provides contract assertions with
  checking semantics (C++26 contracts).
- State the exact predicate that is checked.

#### _Effects:_

- Describe observable mutations: which output parameters are modified and how.
- State "Does not modify `g`" when applicable.
- For "Equivalent to" specifications, state the code sequence. Per
  §16.3.2.4: Constraints and Mandates of the outer function apply first;
  remaining elements are determined by the code sequence unless the outer
  function overrides them.

#### _Synchronization:_

- Omitted for graph-v3 algorithms (single-threaded).
- Include only if a future algorithm provides concurrency guarantees.

#### _Postconditions:_

- State conditions established upon successful return.
- Focus on invariants over output ranges.
- Use mathematical notation where precise (e.g.,
  "$\text{distances}[v] \leq \text{distances}[u] + w(u, v)$").

#### _Result:_

- Include only when specifying a typedef or alias type.
- Most graph algorithms do not need this element; return descriptions go in
  _Returns:_ instead.

#### _Returns:_

- Describe the returned value(s) for non-void functions.
- State sentinel values (e.g., "`nullopt` if no negative cycle is found").
- State `[[nodiscard]]` if applicable.

#### _Throws:_

- List exception types and triggering conditions.
- State the exception guarantee (strong, basic, or none).
- For `noexcept` functions: "Throws: Nothing."
- For functions that only propagate user-callable exceptions: "Any exception
  thrown by _wf_ or _visitor_ is propagated."

#### _Complexity:_

- State time and space complexity using standard asymptotic notation.
- Use $O(\cdot)$ with standard variables: $V$ (vertices), $E$ (edges),
  $d$ (degree), $\alpha$ (inverse Ackermann).
- Complexity requirements are **upper bounds** — implementations achieving
  better complexity meet the requirement.
- If the formulation yields a negative count, the requirement is zero.

#### _Remarks:_

- Additional semantic constraints that don't fit elsewhere.
- Self-loop behavior (when observational rather than a precondition).
- Convergence properties, determinism notes, implementation recommendations.

#### _Error conditions:_

- List `errc` constants and their meaning.
- Include only for algorithms reporting errors via `error_code` or
  `expected`.

---

## Concepts in Proposed Wording

State concepts using the library's defined concept names. The standard
requires that concept requirements be stated with "models" (semantic) or
"satisfies" (syntactic) as appropriate.

| Concept | When to use |
|---------|-------------|
| `adjacency_list<G>` | Most algorithms (BFS, DFS, Dijkstra, Bellman-Ford, etc.) |
| `index_adjacency_list<G>` | Algorithms requiring contiguous integer vertex IDs (afforest, connected_components) |
| `bidirectional_adjacency_list<G>` | Algorithms needing in-edges (Kosaraju single-graph overload) |
| `ordered_vertex_edges<G>` | Algorithms requiring sorted edge lists (triangle_count) |
| `x_index_edgelist_range<IELR>` | Edge-list algorithms (Kruskal) |
| `vertex_property_map_for<T, G>` | Output parameters subscriptable by `vertex_id_t<G>` |
| `basic_edge_weight_function<G, WF, EV>` | Edge weight callables |

---

## Relationship to algorithm_doc_guide.md

The two guides share the §16.3.2.4 function-semantics vocabulary but serve
different purposes:

| Aspect | algorithm_doc_guide.md | This guide (wg21_proposal_guide.md) |
|--------|----------------------|-------------------------------------|
| **Target output** | User-guide `.md` pages, in-header Doxygen | WG21 paper proposed-wording sections |
| **Audience** | Library users, contributors | WG21 committee members, LWG reviewers |
| **Style** | Bold-keyword Markdown (`**Mandates:**`) | Italic-label standard style (_Mandates:_) |
| **Content** | "When to Use", 5+ examples, educational narrative | Normative specification only |
| **Section order** | §16.3.2.4 order (same) | §16.3.2.4 order (same) |
| **Extras** | Supported Graph Properties, Visitor Events, Implementation Notes | Stable clause tags, insertion/deletion markup |

When writing proposed wording for a function already documented via
`algorithm_doc_guide.md`, use the existing documentation as a **source of
truth** for semantics, then reformat into standard editorial style.

---

## Checklist: Writing Proposed Wording for a Function

1. **Read the header** (`include/graph/algorithm/xxx.hpp`):
   - Extract signatures, constraints, concepts, attributes.
   - Identify which concepts are Constraints (overload gates) vs. Mandates
     (sole overload).
   - Note `constexpr`, `[[nodiscard]]`, `noexcept`.

2. **Read the user-guide documentation** (`docs/user-guide/algorithms/xxx.md`):
   - Verify semantics against the existing documentation.
   - Extract Preconditions, Effects, Postconditions, Returns, Throws,
     Complexity, Remarks, Error Conditions.

3. **Read the test file** (`tests/algorithms/test_xxx.cpp`):
   - Confirm edge-case behavior matches proposed semantics.
   - Verify postconditions are tested.

4. **Write the specification block**:
   - Use the template above as a starting point.
   - Include all applicable §16.3.2.4 elements in canonical order.
   - Omit elements that do not apply (Note 135).
   - Use italic labels: _Constraints:_, _Mandates:_, _Preconditions:_, etc.
   - Use stable clause tags: `[graph.algo.xxx]`.

5. **Write insertion/deletion markup** if revising an existing paper:
   - Use `<ins>` / `<del>` (HTML) or `\added{}` / `\removed{}` (LaTeX).
   - Show changes relative to the previous revision.

6. **Verify consistency**:
   - Concept names match the implementation exactly.
   - Complexity matches the algorithm_doc_guide.md and header documentation.
   - All template parameters have constraints or mandates stated.
   - Return semantics match the implementation (including sentinel values).
   - Exception behavior matches the implementation (`noexcept`, strong/basic).
   - Self-loop behavior is addressed (Preconditions or Remarks).
   - The § ordering of semantic elements is correct.

---

## Common Pitfalls

- **Don't use bold-keyword style** — proposed wording uses _italic labels_
  (_Constraints:_, not **Constraints:**). Bold-keyword is for Doxygen/user
  docs.
- **Don't include examples in proposed wording** — examples in the standard
  use `[_Example N_: ... —_end example_]` format and are non-normative. Most
  function specifications have no inline examples. Put examples in the
  paper's Design Discussion section or in separate example clauses.
- **Don't confuse "satisfies" and "models"** — "satisfies" = syntactic check;
  "models" = satisfies + semantic requirements. Use "models" when semantic
  requirements matter.
- **Don't use "UB"** — spell out "undefined behavior" in proposed wording.
- **Don't omit stable clause tags** — every sub-clause needs a `[tag.name]`.
- **Don't state complexity as exact** — complexity requirements are upper
  bounds per §16.3.2.4.
- **Don't forget Constant When** — if the function is `constexpr`, state the
  conditions for constant evaluation.
- **Don't mix Constraints and Mandates** — Constraints cause silent
  non-viability; Mandates cause ill-formed programs. These are distinct
  elements with different user-facing behavior.
- **Don't duplicate content in Effects and Postconditions** — Effects
  describes what the function *does*; Postconditions describes what is *true*
  after it returns. They complement each other.
- **Don't invent new notation** — follow existing Standard Library proposed
  wording patterns (e.g., ranges, algorithms).
