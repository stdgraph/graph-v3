# Agent Instructions: Algorithm Documentation Pages

This document provides instructions for an agent creating user-facing documentation
pages for new algorithms in the graph-v3 library. These pages live under
`docs/user-guide/algorithms/` and are linked from the algorithm catalog at
`docs/user-guide/algorithms.md`.

> **Not to be confused with** `docs/algorithm_template.md`, which is a template
> for *algorithm code* documentation (implementation internals, complexity
> proofs, supported graph properties). This guide is for the *user-guide pages*
> that serve as feature reference and educational material.

---

## Scope: User-Guide Pages and In-Header Documentation

This guide primarily targets the **user-guide pages** under
`docs/user-guide/algorithms/`. However, algorithm headers in
`include/graph/algorithm/` also carry substantial Doxygen documentation that
should follow consistent conventions. Both documentation sites serve different
purposes:

| Site | Audience | Content |
|------|----------|---------|
| **User-guide page** (`docs/`) | End users, newcomers | Progressive examples, "When to Use", educational narrative |
| **In-header Doxygen** (`include/`) | Developers, IDE tooltips | Precise signatures, concept requirements, exception specs, supported graph properties |

### Canonical style for in-header documentation

All algorithm headers must use **bold-keyword inline sections** inside the
Doxygen `/** */` block. This is the most compact style and renders well in
both Doxygen output and IDE hover tooltips:

```cpp
/**
 * @brief One-sentence description.
 *
 * Extended description (1-3 paragraphs).
 *
 * @tparam G  Graph type. Must satisfy adjacency_list concept.
 * @param  g  The graph to process.
 *
 * **Constraints:**
 * - Participates in overload resolution only if G satisfies `adjacency_list`
 *
 * **Mandates:**
 * - G must satisfy `adjacency_list` (index or mapped)
 *
 * **Preconditions:**
 * - All source vertices must be valid vertex IDs
 *
 * **Effects:**
 * - Modifies distances and predecessor; does not modify g
 *
 * **Postconditions:**
 * - distances[v] contains shortest distance for reachable v
 *
 * **Returns:**
 * - The number of vertices reached (or void if no return value)
 *
 * **Throws:**
 * - std::bad_alloc from internal allocations
 *
 * **Complexity:**
 * - Time: O(V + E)
 * - Space: O(V)
 *
 * **Remarks:**
 * - Self-loops are ignored
 *
 * **Supported Graph Properties:**
 * (see Supported Graph Properties section below)
 */
```

Do **not** use `@par` section headings or `## Markdown headings` inside Doxygen
blocks. Those styles exist in older headers and should be migrated to the
bold-keyword style during maintenance.

---

## C++ Standard Alignment (WG21 §16.3.2.4)

The function-semantics sections of every algorithm page follow the vocabulary
and ordering defined by **C++26 Standard §16.3.2.4** (see
[wg21_function_semantics.md](wg21_function_semantics.md)). The standard defines
these elements in this order:

> Constraints · Mandates · Constant When · Preconditions · Hardened
> preconditions · Effects · Synchronization · Postconditions · Result ·
> Returns · Throws · Complexity · Remarks · Error conditions

Key rules from the standard:

- **Omit inapplicable elements** (Note 135): "elements that do not apply to a
  function are omitted."
- **Complexity values are upper bounds**: "implementations that provide better
  complexity guarantees meet the requirements."
- **Constraints ≠ Mandates**: Constraints cause *silent non-viability* in
  overload resolution (e.g., `requires` clauses). Mandates render the program
  *ill-formed* if violated (e.g., `static_assert`).
- **Preconditions ≠ Hardened preconditions**: Preconditions cause undefined
  behavior when violated. Hardened preconditions trigger a contract assertion
  with checking semantics in hardened implementations.

Not all WG21 elements are commonly used:
- **Synchronization**: Omitted — graph-v3 algorithms are single-threaded.
- **Result**: Omitted unless an algorithm provides a typedef or alias whose
  type needs specification. Most algorithms document return types in Signatures
  and return values in Returns.

---

## Audience

Every algorithm page must serve **three audiences simultaneously**:

1. **Quick reference** — experienced users who know the algorithm and just need
   the signature, parameter table, and a basic example to copy-paste.
2. **Technical reference** — graph experts who need precise details: exact
   signatures with template constraints, complexity guarantees, preconditions,
   return types, visitor events, edge cases, and behavioral notes.
3. **Educational** — newcomers to the algorithm who need to understand *when* to
   use it, *why* it works, and *how* to apply it to real problems through
   progressively more complex examples.

---

## Required Section Structure

Every algorithm page follows this section order. Sections marked **(if applicable)**
are included only when the algorithm has the relevant feature.

The **function-semantics sections** (Constraints through Error Conditions) follow
the ordering defined by C++26 Standard §16.3.2.4 (see
[wg21_function_semantics.md](wg21_function_semantics.md)). As Note 135 states:
"elements that do not apply to a function are omitted."

```
# Algorithm Name

> [← Back to Algorithm Catalog](../algorithms.md)

- [Overview](#overview)
- [When to Use](#when-to-use)
- [Include](#include)
- [Signature(s)](#signatures)                  ← "Signature" (singular) if only one overload
- [Parameters](#parameters)
- [Supported Graph Properties](#supported-graph-properties)
- [Visitor Events](#visitor-events)             ← (if applicable)
- [Examples](#examples)
  ────────── WG21 §16.3.2.4 function-semantics sections (in standard order) ──────────
- [Constraints](#constraints)                   ← (if applicable)
- [Mandates](#mandates)
- [Constant When](#constant-when)               ← (if applicable)
- [Preconditions](#preconditions)
- [Hardened Preconditions](#hardened-preconditions) ← (if applicable)
- [Effects](#effects)                           ← (if applicable)
- [Postconditions](#postconditions)             ← (if applicable)
- [Returns](#returns)                           ← (if applicable)
- [Throws](#throws)                             ← (if applicable)
- [Complexity](#complexity)
- [Remarks](#remarks)                           ← (if applicable)
- [Error Conditions](#error-conditions)         ← (if applicable)
  ────────── End of function-semantics sections ──────────
- [Implementation Notes](#implementation-notes) ← (if applicable)
- [References](#references)                     ← (if applicable)
- [See Also](#see-also)
```

### Special cases

- **Multiple sub-algorithms** (e.g., MST has Kruskal + Prim, Connected
  Components has 4 algorithms): Replace the single Signature/Parameters
  sections with per-algorithm subsections under an `## Algorithms` or
  named `## Kruskal's Algorithm` / `## Prim's Algorithm` pattern.
  Include an **Algorithm Selection Guide** subsection (see below).
- **Visitor Events**: Include this section for any algorithm whose signature
  accepts a `Visitor` parameter. Check the actual header; do not rely on a
  fixed list of algorithm names.
- **Remarks**: Include when there are important behavioral details that don't
  fit in Preconditions (e.g., self-loop handling differences, in-place
  modification warnings, convergence properties).

### Algorithm Selection Guide (for multi-algorithm pages)

When a page covers multiple algorithms or variants (e.g., Kruskal vs. Prim,
Kosaraju vs. Afforest), include a subsection helping users choose:

```markdown
## Algorithm Selection

| Criterion            | Algorithm A       | Algorithm B        |
|----------------------|-------------------|--------------------|\n| Best graph density   | Sparse (E << V²)  | Dense (E ≈ V²)     |
| Concept required     | `edgelist_range`  | `adjacency_list`   |
| Extra input needed   | None              | Seed vertex        |
| Output format        | Edge list         | Predecessor array  |
```

- State when each variant is preferred.
- Highlight key complexity / capability tradeoffs.
- Note which concept requirements differ between variants.

---

## Section-by-Section Instructions

### Overview

- **First sentence**: Define the algorithm in bold, stating what it computes.
- State the **primary concept requirement** prominently. Use the exact concept
  from the header — do not assume `index_adjacency_list<G>` for every
  algorithm. The library uses several concept levels:

  | Concept | Meaning | Used by |
  |---------|---------|--------|
  | `adjacency_list<G>` | Vertices + out-edges (index or mapped) | BFS, DFS, Bellman-Ford, Dijkstra, label propagation, Jaccard, MIS, articulation points, biconnected components, topological sort |
  | `index_adjacency_list<G>` | Adjacency list with contiguous integer vertex IDs | afforest, connected_components |
  | `bidirectional_adjacency_list<G>` | Adjacency list with in-edges | Kosaraju (single-graph overload) |
  | `ordered_vertex_edges<G>` | Edges sorted by target ID | triangle_count |
  | `x_index_edgelist_range<IELR>` | Forward range of edge descriptors | Kruskal |

  Example callout:
  > The graph must satisfy `adjacency_list<G>` — any graph type with vertices
  > and out-edges, including both index-based and map-based containers.

- List key properties of the output (what the algorithm produces, how results
  are structured).
- Mention any additional concept requirements (e.g., `ordered_vertex_edges<G>`
  for triangle count).
- Keep to 1-3 short paragraphs.

### When to Use

- **Bullet list of positive use cases** — concrete scenarios where this
  algorithm is the right choice. Start each bullet with a bolded domain
  (e.g., "**Social network analysis**", "**Network reliability**").
- **"Not suitable when" or "Consider alternatives when"** — a short list of
  scenarios where another algorithm is better, with links to the alternative
  page. This helps newcomers navigate the library.
- Keep each bullet to 1-2 sentences.

### Include

- Single fenced code block with the `#include` directive.
- Use the exact header path under `include/graph/algorithm/`.

### Signatures

- Show all public overloads in a single fenced C++ code block.
- Include `constexpr`, `[[nodiscard]]`, `noexcept`, and template constraints
  if present in the actual header.
- **`[[nodiscard]]`**: Include when the return type is non-void and the
  algorithm uses this attribute. Currently used by `bellman_ford_shortest_paths`
  (returns `optional<vertex_id_t<G>>`) and `triangle_count` (returns `size_t`).
- **`constexpr`**: Include when present. Note any caveats (e.g., `constexpr`
  Dijkstra requires constexpr-capable allocators at compile time).
- **`noexcept`**: Include when present. Currently only `triangle_count` is
  `noexcept`. Also document in the Throws section (see below).
- After the code block, briefly note the **return type** if non-void. Detailed
  return-value semantics (sentinel values, edge cases) belong in the
  **Returns** section (see below), not here. A short inline note is acceptable
  for quick reference (e.g., "Returns `std::pair<EV, size_t>` — see
  [Returns](#returns) for details.").
- Use "Signature" (singular) if there is only one overload; "Signatures" (plural)
  if there are multiple.

### Parameters

- Markdown table with columns: `Parameter`, `Description`.
- Every parameter from every overload must appear.
- Note default values inline (e.g., "Default: `std::less<>{}`").
- For output parameters, state the sizing requirement
  (e.g., "Random-access range sized to `num_vertices(g)`").
- For template parameters, document the concept requirements (e.g.,
  "`G` must satisfy `adjacency_list<G>`; `WF` must be callable as
  `WF(const G&, edge_t<G>) -> DistanceValue`").

**The `vertex_property_map` pattern**: Many algorithms accept output parameters
that are **vertex property maps** — containers subscriptable by
`vertex_id_t<G>` (e.g., `std::vector<T>` for index graphs,
`std::unordered_map<VId, T>` for mapped graphs). These are constrained by
`vertex_property_map_for<Container, G>`. When documenting such parameters:

- State the concept: "Must satisfy `vertex_property_map_for<Distances, G>`."
- State the value type requirement: "Value type must be arithmetic."
- State the sizing requirement: "Must contain an entry for each vertex of `g`."
- Algorithms using this pattern: `dijkstra_shortest_paths`,
  `bellman_ford_shortest_paths`, `connected_components`, `kosaraju`,
  `afforest`, `label_propagation`, `prim`.

**The `null_predecessors` opt-out**: The shortest-path algorithms
(`dijkstra_shortest_paths`, `bellman_ford_shortest_paths`) accept a special
`_null_predecessors` sentinel as the `predecessor` parameter to skip path
reconstruction entirely. When the algorithm supports this, document it in
the Parameters table:

> `predecessor` — Vertex property map for path reconstruction. Pass
> `_null_predecessors` to skip predecessor tracking when only distances are
> needed. Detected at compile time via `is_null_range_v<Predecessors>`.

### Supported Graph Properties

Every algorithm page **must** include this section. It provides a quick
compatibility checklist using ✅/⚠️/❌ markers. Use the following template:

```markdown
## Supported Graph Properties

**Directedness:**
- ✅/❌/⚠️ Directed graphs (explain behavior if ⚠️)
- ✅/❌/⚠️ Undirected graphs (explain behavior if ⚠️)

**Edge Properties:**
- ✅/❌ Weighted edges (state if weights are used or ignored)
- ✅/❌ Unweighted edges
- ✅/❌/⚠️ Multi-edges (explain: deduplicated? all counted?)
- ✅/❌ Self-loops (explain exact behavior — see Self-Loop Documentation below)
- ✅/❌ Negative weights (if relevant to algorithm)

**Graph Structure:**
- ✅/❌ Connected graphs
- ✅/❌ Disconnected graphs (explain: all components? reachable only?)
- ✅/❌ Empty graphs (returns immediately / returns 0 / etc.)
- ✅/❌ Acyclic graphs (DAGs)
- ✅/❌ Cyclic graphs

**Container Requirements:**
- Required: `adjacency_list<G>` (or the specific concept)
- Additional: `ordered_vertex_edges<G>`, `vertex_property_map_for<T, G>`, etc.
- Compatible types: list specific container families
```

**Self-loop documentation is mandatory.** Self-loop behavior varies
significantly across algorithms and is a frequent source of user confusion.
Always state the exact behavior:

| Algorithm | Self-loop behavior |
|-----------|-------------------|
| articulation_points | Ignored — do not affect detection |
| biconnected_components | Ignored |
| triangle_count | Ignored — cannot form triangles |
| MIS | Vertices with self-loops excluded from MIS |
| Jaccard | Skipped — excluded from neighbor sets |
| label_propagation | Counted in neighbor tally |
| BFS | Examined but don't affect traversal |
| DFS | Classified as back edges (vertex is Gray) |
| Bellman-Ford / Dijkstra | Relaxation has no effect (distance cannot decrease) |
| Kruskal | Ignored by union-find (same component) |

This table is a reference; each algorithm page must state its own behavior
in the Supported Graph Properties section.

### Visitor Events (if applicable)

- Markdown table with columns: `Event`, `When invoked`, `Common uses`.
- List every visitor callback the algorithm recognizes.
- Note which events are unique to this algorithm vs. shared.

**Dual-overload pattern**: Every visitor callback in graph-v3 is checked via
two concept variants (defined in `traversal_common.hpp`):

1. **Descriptor overload**: `visitor.on_X(g, vertex_descriptor)` or
   `visitor.on_X(g, edge_ref)` — checked by `has_on_X<G, Visitor>`.
2. **Vertex ID overload**: `visitor.on_X(g, vertex_id)` — checked by
   `has_on_X_id<G, Visitor>`.

The algorithm tries the descriptor overload first. If not found, it tries the
ID overload. If neither is defined, the callback is simply not invoked (no
compilation error). When documenting visitor events, state both overload forms.

**Known visitor event sets**:

| Algorithm family | Events |
|-----------------|--------|
| BFS | `on_initialize_vertex`, `on_discover_vertex`, `on_examine_vertex`, `on_examine_edge`, `on_finish_vertex` |
| DFS | `on_initialize_vertex`, `on_start_vertex`, `on_discover_vertex`, `on_examine_edge`, `on_tree_edge`, `on_back_edge`, `on_forward_or_cross_edge`, `on_finish_edge`, `on_finish_vertex` |
| Dijkstra | `on_discover_vertex`, `on_examine_vertex`, `on_examine_edge`, `on_edge_relaxed`, `on_edge_not_relaxed`, `on_finish_vertex` |
| Bellman-Ford | `on_discover_vertex`, `on_examine_edge`, `on_edge_relaxed`, `on_edge_not_relaxed`, `on_edge_minimized`, `on_edge_not_minimized` |

Always verify against the actual header — this table is a quick reference,
not a substitute for reading the code.

### Examples

**Quantity**: At minimum 5 examples per page. This minimum ensures coverage
of: the basic happy path, at least one additional overload or mode, a
topology-specific case that illuminates algorithm properties, an advanced
feature, and one edge case or real-world scenario. More is acceptable and
encouraged when the algorithm has multiple overloads, modes, or edge cases.

**Progressive complexity**: Order examples from simplest to most advanced:

1. **Basic usage** — minimal working example, the "happy path". A newcomer
   should be able to copy this and get a result immediately.
2. **Core variant** — a second fundamental use case (e.g., different overload,
   different graph structure).
3. **Interesting graph topology** — demonstrate behavior on a specific graph
   shape (star, complete, path, cycle, disconnected) that illuminates algorithm
   properties.
4. **Advanced feature** — visitors, custom weight functions, `_id` variants,
   special parameters, etc.
5. **Edge case or real-world application** — disconnected graphs, empty graphs,
   self-loops, or a realistic domain scenario.

**Example format**:

```markdown
### Example N: Descriptive Title

One-sentence explanation of what this example demonstrates and *why* it matters.

\`\`\`cpp
#include <graph/algorithm/xxx.hpp>          // only in first example
#include <graph/container/dynamic_graph.hpp> // only in first example

// Comments explaining the graph structure or setup
// Use ASCII art for non-trivial topologies

// ... code ...

// Comments explaining expected output with concrete values
// result = {0, 2, 4}, count = 3
\`\`\`
```

**Example rules**:

- Include `#include` directives and `using` declarations **only in the first
  example**. Subsequent examples assume the same setup.
- Use concrete graph literals (not `...` or "load from file").
- Show **expected output values** in comments — never leave the reader guessing.
- Use ASCII art to diagram non-trivial graph topologies.
- When demonstrating a feature, explain *why* you'd use it in practice.
- Each example title should be descriptive enough to serve as a quick-reference
  anchor (e.g., "Kruskal — Maximum Spanning Tree", not "Example 2").

### Constraints (if applicable)

Constraints determine the function's participation in **overload resolution**
(C++26 §16.3.2.4). Failure to satisfy a constraint causes the overload to be
silently non-viable — no compiler diagnostic is issued. Constraints are
typically expressed via `requires` clauses or SFINAE.

- Bullet list of every `requires` clause or SFINAE condition.
- State when the constraint controls overload selection (e.g., choosing
  between a descriptor-based and an ID-based visitor callback).
- Example:
  > - Participates in overload resolution only if `has_on_discover_vertex<G,
  >   Visitor>` is satisfied.

**Constraints vs. Mandates**: If a `requires` clause is not selecting among
overloads but is the *only* way the function can be called, document it under
**Mandates** instead, since failure renders the call ill-formed with a
diagnostic. Reserve Constraints for genuine overload-resolution gates.

### Mandates

Mandates are **compile-time** conditions whose violation makes the program
**ill-formed** and triggers a compiler diagnostic (C++26 §16.3.2.4).
These are expressed via `static_assert`, concept requirements that are the
sole available overload, or `requires` clauses that do not merely select
among overloads.

- Bullet list of every compile-time requirement.
- State the primary concept: "`G` must satisfy `adjacency_list<G>`" (or
  whichever concept the algorithm actually requires — see the concept table
  in the Overview section).
- State concept requirements on other template parameters (e.g.,
  "`Distances` must satisfy `vertex_property_map_for<Distances, G>` with
  arithmetic value type").
- State sole-overload `requires` clause constraints (e.g.,
  "`Sources` must be `input_range` with values convertible to
  `vertex_id_t<G>`").
- If the algorithm uses `static_assert` for any condition, document that here.

### Constant When (if applicable)

Document the conditions under which a call to a `constexpr` algorithm is a
constant subexpression (C++26 §16.3.2.4). Include this section only for
algorithms declared `constexpr`.

- Bullet list of requirements for constexpr evaluation.
- Example:
  > - A constant subexpression when all of: the graph `g` is a constant
  >   expression, the allocator (if any) supports constexpr allocation, and
  >   the weight function `wf` is a constant expression.

Algorithms currently declared `constexpr`: `dijkstra_shortest_paths`,
`bellman_ford_shortest_paths`, `breadth_first_search`, `depth_first_search`.
Verify against the actual header — this list may evolve.

### Preconditions

Preconditions are **runtime** conditions whose violation causes **undefined
behavior** (C++26 §16.3.2.4). Where the implementation is generous, violation
may instead throw an exception, but callers must not rely on that.

- Bullet list of every runtime requirement the caller must satisfy.
- For undirected graph algorithms, state: "For undirected graphs, **both
  directions** of each edge must be stored (or use
  `undirected_adjacency_list`)."
- State valid vertex ID ranges and sizing requirements for output containers.
- State self-loop behavior (mandatory — see Supported Graph Properties).
- State weight-function constraints (e.g., "All edge weights must be
  non-negative" for Dijkstra).

### Hardened Preconditions (if applicable)

Hardened preconditions are conditions that the function assumes to hold and
that are enforced by **contract assertions with checking semantics** in
hardened implementations (C++26 §16.3.2.4). In non-hardened implementations,
violation is undefined behavior (same as regular Preconditions).

Include this section only if the algorithm uses C++26 contract assertions or
an equivalent runtime-checked precondition mechanism.

- Bullet list of hardened precondition conditions.
- State the checking semantic: a contract-violation handler is invoked before
  any observable side effects if the assertion fails.

### Effects (if applicable)

Include this section when the algorithm modifies caller-supplied output
parameters. Effects describe the **actions performed by the function**
(C++26 §16.3.2.4) and complement Postconditions.

- Bullet list of every observable mutation:
  - Which output parameters are modified and how.
  - Explicit statement that the graph `g` is not modified (if true).
- Example:
  > - Modifies `distances`: Sets `distances[v]` for all vertices `v`.
  > - Modifies `predecessor`: Sets `predecessor[v]` for all reachable vertices.
  > - Does not modify the graph `g`.
- Algorithms that currently document Effects in their headers:
  `bellman_ford_shortest_paths`, `dijkstra_shortest_paths`,
  `biconnected_components`, `label_propagation`.

**"Equivalent to" semantics**: When Effects specifies that the semantics are
"Equivalent to" some code sequence, the Constraints and Mandates of the
outer function are evaluated first, and the remaining elements (Preconditions,
Effects, Postconditions, Returns, Throws, Complexity, Remarks, Error
Conditions) are determined by the code sequence — unless the outer function
explicitly overrides them.

### Postconditions (if applicable)

Include this section when the algorithm writes results into caller-supplied
output ranges or produces a return value whose state needs precise
specification beyond a single sentence in Signatures. Postconditions describe
the conditions established by the function upon successful return
(C++26 §16.3.2.4).

- Bullet list of conditions established by the function upon successful return.
- Focus on output ranges: state what every element contains after the call
  (e.g., "`distances[v]` holds the shortest-path distance from `source` to `v`,
  or `numeric_limits<EV>::max()` if `v` is unreachable.").
- State the postcondition for the return value here if it is too detailed for
  the Returns section.
- Omit this section if postconditions are fully captured by the Returns
  description and output-parameter notes in Parameters.

### Returns (if applicable)

Include this section for any algorithm with a non-void return type. Returns
provides a **description of the value(s) returned by the function**
(C++26 §16.3.2.4).

- State the return type and what it represents.
- Document sentinel values (e.g., "`numeric_limits<EV>::max()` if
  unreachable").
- Document all possible return values for discriminated returns (e.g.,
  `optional<vertex_id_t<G>>` — `nullopt` if no negative cycle found).
- Example:
  > Returns `std::pair<EV, size_t>` — the total MST edge weight and the
  > number of edges included. Returns `{EV{}, 0}` if the graph has no edges.

For algorithms with `[[nodiscard]]`, state that attribute here.

**Relationship to Signatures section**: The Signatures section shows the
return type in the code block. The Returns section provides the semantic
description of what the returned value means, including edge cases and
sentinel values.

### Throws (if applicable)

Include this section when the algorithm can throw exceptions or is
`noexcept`. This corresponds to the C++ standard's **Throws** element
(C++26 §16.3.2.4).

- Bullet list of exception types and the conditions that cause each one.
- Always state the exception guarantee: **strong** (no observable effects on
  failure), **basic** (output is in a valid but unspecified state), or **none**.
- If the algorithm is `noexcept` or only propagates exceptions from
  user-supplied callables, state that explicitly (e.g., "Does not throw
  directly; exceptions from `wf` or `visitor` callbacks are propagated
  unchanged. No partial results are written (strong guarantee).").
- For `noexcept` functions: "Does not throw — declared `noexcept`."

### Complexity

Complexity states the time and/or space complexity of the function
(C++26 §16.3.2.4). Per the standard, **complexity requirements are upper
bounds** — implementations that provide better complexity guarantees meet the
requirements. If the formulation calls for a negative number of operations,
the actual requirement is zero operations.

- Markdown table with columns: `Metric`, `Value` (or `Algorithm`, `Time`,
  `Space` for multi-algorithm pages).
- Use standard variables: V = vertices, E = edges, d = degree.
- Add a brief paragraph if the complexity has interesting properties
  (e.g., "The merge-based intersection exploits sorted neighbor lists").

### Remarks (if applicable)

Remarks provide **additional semantic constraints** on the function
(C++26 §16.3.2.4) — behavioral details that don't belong in Preconditions,
Postconditions, Effects, or Throws.

- Bullet list of behavioral details, caveats, or design decisions.
- Good candidates: in-place modification warnings, self-loop counting behavior
  (especially when it differs from other algorithms), convergence properties,
  determinism guarantees, differences between similar algorithms.

### Error Conditions (if applicable)

Error conditions specify the **conditions where a function may fail**, listed
as `std::errc` constants with explanations (C++26 §16.3.2.4). Include this
section only for algorithms that report errors via `std::error_code` or
`std::expected` rather than (or in addition to) exceptions.

- Bullet list of `std::errc` constants (or domain-specific codes) the
  algorithm can set, with a brief explanation of each condition.
- Note whether errors are reported via a return value or an output parameter.
- Omit this section entirely if the algorithm never reports errors by error
  code.

### Implementation Notes (if applicable)

Include this section when the algorithm has interesting internal details that
help advanced users understand performance characteristics or make informed
choices. This content also appears in the in-header Doxygen docs.

This section is **library-specific** and not part of WG21 §16.3.2.4.

Optional subsections (include only what's relevant):

- **Algorithm Overview** — step-by-step description of the algorithm's
  strategy and phases.
- **Data Structures** — what internal structures are used and why (e.g.,
  "Queue: `std::queue` for FIFO processing; Visited: `vertex_property_map<G,
  bool>` — `vector<bool>` for index graphs, `unordered_map` for mapped
  graphs").
- **Design Decisions** — numbered rationale for API choices (e.g., "Why
  iterative DFS instead of recursive? Avoids stack overflow on deep graphs.").
- **Optimization Opportunities** — future improvement directions for
  contributors.

Keep this section factual and concise. It should not duplicate the Overview
or Complexity sections.

### References (if applicable)

Include this section when the algorithm has well-known academic provenance.

- **Academic papers**: Author(s), year, title, venue. Use standard citation
  format.
- **Textbooks**: Author(s), year, title, edition, publisher, section/chapter.
- **Online resources**: Title with URL (only for stable, authoritative
  references).

Examples:
- Tarjan, R. E. (1972). "Depth-first search and linear graph algorithms".
  *SIAM Journal on Computing*, 1(2), 146-160.
- Cormen, T. H., et al. (2009). *Introduction to Algorithms* (3rd ed.).
  MIT Press. Section 22.3.

### See Also

- Link to related algorithm pages (similar algorithms, alternatives mentioned
  in "When to Use").
- Link to the algorithm catalog: `[Algorithm Catalog](../algorithms.md)`.
- Link to the test file: `[test_xxx.cpp](../../../tests/algorithms/test_xxx.cpp)`.
- Link to any relevant example files in `examples/`.

---

## Formatting Conventions

### Markdown

- Use ATX headings (`#`, `##`, `###`). No setext headings.
- Use `**bold**` for the first mention of key terms in the Overview.
- Use backticks for all code identifiers: types, functions, parameters,
  concepts, header paths.
- Use `>` blockquotes only for the back-link and important callouts.
- Mathematical expressions: use KaTeX (`$...$` inline, `$$...$$` block).

### Code blocks

- Language tag: always `cpp`.
- Indent: 4 spaces (no tabs).
- Maximum line width: ~80 characters in code blocks.
- Use `auto` where the return type is obvious; spell out types when they aid
  understanding.

### Tables

- Always include a header separator row (`|---|---|`).
- No trailing whitespace in cells.
- Align `|` characters for readability in source.

### Links

- Relative paths from the current file's directory.
- Algorithm cross-references: `[Algorithm Name](algorithm_name.md)`.
- Test files: `[test_xxx.cpp](../../../tests/algorithms/test_xxx.cpp)`.
- Landing page: `[Algorithm Catalog](../algorithms.md)`.

---

## Checklist: Adding a New Algorithm Page

When creating a documentation page for a new algorithm:

1. **Read the header file** (`include/graph/algorithm/xxx.hpp`) to extract:
   - All public function overloads and their exact signatures
   - Template constraints and concepts (note: not all algorithms use
     `index_adjacency_list` — check the actual `requires` clause)
   - Distinguish **Constraints** (overload-resolution gates via `requires`)
     from **Mandates** (sole-overload concept requirements, `static_assert`)
   - `constexpr`, `[[nodiscard]]`, `noexcept`, and other attributes
   - For `constexpr` functions, identify **Constant When** conditions
   - Default parameter values
   - Return types and return value semantics (including sentinel values)
   - Visitor event names (if any) and both overload forms (descriptor + ID)
   - `noexcept` specifiers and any documented exception types
   - Error codes reported via `std::error_code` or `std::expected` (if any)
   - Postconditions on output ranges (from comments or contracts, if present)
   - Hardened preconditions (contract assertions, if present)
   - Supported Graph Properties (directedness, edge properties, graph
     structure, container requirements)
   - Self-loop behavior (mandatory — must be documented)
   - Whether `_null_predecessors` is supported (shortest-path algorithms)
   - Whether parameters use the `vertex_property_map_for<T, G>` pattern
   - Effects on output parameters

2. **Read the test file** (`tests/algorithms/test_xxx.cpp`) to identify:
   - Graph topologies tested (star, path, cycle, complete, disconnected, etc.)
   - Edge cases tested (empty graph, self-loops, isolated vertices, etc.)
   - Feature coverage (which overloads, which parameter combinations)
   - Example-worthy test cases to adapt into documentation examples

3. **Read any existing example files** in `examples/` for the algorithm.

4. **Create the page** at `docs/user-guide/algorithms/xxx.md` following the
   section structure above.

5. **Update the algorithm catalog** (`docs/user-guide/algorithms.md`):
   - Add a row to the "By Category" table in the appropriate category.
   - Add a row to the "Alphabetical" table.
   - Add or update the category section with a brief description paragraph.
   - Include Time and Space complexity columns in the catalog table row.

6. **Verify consistency** (sections follow WG21 §16.3.2.4 order):
   - All signatures match the actual header file exactly, including
     `constexpr`, `[[nodiscard]]`, and `noexcept` attributes.
   - All visitor events match the actual implementation (check both
     descriptor and vertex-ID overload forms).
   - Complexity values match the algorithm catalog tables.
   - The **correct** concept requirement is stated in Overview, Mandates, and
     Supported Graph Properties (not all algorithms use
     `index_adjacency_list<G>` — many use `adjacency_list<G>`).
   - Supported Graph Properties section is present with ✅/⚠️/❌ markers
     for directedness, edge properties, graph structure, and container
     requirements.
   - Self-loop behavior is explicitly documented in Supported Graph Properties.
   - All cross-links resolve (test by checking file paths).
   - At least 5 examples are provided, covering each overload and at least one
     edge case.
   - Examples progress from simple to advanced.
   - Expected output values are shown in comments for every example.
   - **Constraints section** is present if the algorithm has `requires` clauses
     that gate overload resolution (distinct from sole-overload Mandates).
   - **Mandates section** is present with compile-time concept requirements and
     `static_assert` conditions.
   - **Constant When section** is present for `constexpr` algorithms, listing
     conditions for constant evaluation.
   - **Preconditions section** is present with runtime requirements.
   - **Hardened Preconditions section** is present if the algorithm uses
     contract assertions with checking semantics.
   - **Effects section** is present if the algorithm modifies output parameters.
   - **Postconditions section** is present if output-range postconditions are
     not fully covered by the Parameters and Returns descriptions.
   - **Returns section** is present for non-void algorithms, describing the
     returned value(s) and sentinel values.
   - **Throws section** is present if the algorithm can throw, and states the
     exception guarantee; absent (or noted as `noexcept`) otherwise.
   - For `noexcept` algorithms, the Throws section states: "Does not throw —
     declared `noexcept`."
   - **Complexity section** states upper-bound complexity.
   - **Remarks section** is present for additional semantic constraints.
   - **Error Conditions section** is present if the algorithm uses
     `std::error_code`.
   - References section is present if the algorithm has well-known academic
     provenance.
   - Function-semantics sections appear in WG21 standard order: Constraints,
     Mandates, Constant When, Preconditions, Hardened Preconditions, Effects,
     Postconditions, Returns, Throws, Complexity, Remarks, Error Conditions.
   - For multi-algorithm pages, an Algorithm Selection Guide is present.

---

## Reference: Existing Pages

Use these as models. The pages vary slightly based on algorithm features but
all follow the structure above.

| Page                      | Notable features to model                                                                                                           |
| ------------------------- | ----------------------------------------------------------------------------------------------------------------------------------- |
| `dijkstra.md`             | Multiple overloads, visitor events, `constexpr` signatures, `_id` variant examples, error handling note                             |
| `bellman_ford.md`         | `[[nodiscard]]` attribute, `find_negative_cycle` helper, unique visitor events (`on_edge_minimized`)                                |
| `dfs.md`                  | Richest visitor events (9), single-source limitation note, subtree computation example                                              |
| `connected_components.md` | Multi-algorithm structure (3 sub-algorithms), `compress()` helper, per-algorithm selection guidance                                 |
| `mst.md`                  | Two distinct algorithms (Kruskal + Prim) with separate signatures, `inplace_kruskal` modification warning, cross-validation example |
| `label_propagation.md`    | RNG parameter, convergence notes, self-loop counting behavior, reproducibility example                                              |
| `triangle_count.md`       | Additional concept requirement (`ordered_vertex_edges`), pre-sorting workaround example                                             |
| `mis.md`                  | Seed sensitivity, maximal vs. maximum distinction, self-loop exclusion                                                              |

---

## Common Pitfalls to Avoid

- **Don't omit "When to Use"** — this section is critical for newcomers
  choosing between algorithms.
- **Don't show fewer than 5 examples** — comprehensive examples are the
  primary educational tool.
- **Don't leave expected output ambiguous** — always show concrete values in
  code comments.
- **Don't assume `index_adjacency_list`** — check the actual `requires`
  clause. Most algorithms use `adjacency_list<G>`, not
  `index_adjacency_list<G>`. State the correct concept in Overview, Mandates,
  and Supported Graph Properties.
- **Don't show `#include` in every example** — only the first example needs
  includes and `using` declarations.
- **Don't mix singular/plural "Signature(s)"** — use singular for one
  overload, plural for multiple.
- **Don't document features that don't exist** — read the actual header, don't
  assume based on other algorithms.
- **Don't forget to update the catalog** — the landing page at
  `docs/user-guide/algorithms.md` must list every algorithm with complexity.
- **Don't omit Throws** — if an algorithm can throw, document it with the
  exception guarantee; if it is `noexcept` or only propagates user-callable
  exceptions, state that explicitly. For `noexcept` functions, state: "Does
  not throw — declared `noexcept`."
- **Don't confuse Constraints, Mandates, and Preconditions** — per WG21
  §16.3.2.4: **Constraints** cause *silent non-viability* in overload
  resolution (e.g., `requires` clauses selecting among overloads);
  **Mandates** make the program *ill-formed* (e.g., `static_assert`, sole
  `requires` clause); **Preconditions** are *runtime* requirements
  (violation is UB). Keep them in separate sections.
- **Don't omit Error Conditions** — if the algorithm returns `std::error_code`
  or uses `std::expected`, document the error codes in their own section.
- **Don't omit Supported Graph Properties** — every algorithm page must
  document directedness, edge properties, graph structure, and container
  requirements with ✅/⚠️/❌ markers.
- **Don't omit self-loop behavior** — self-loop handling varies across
  algorithms (ignored, excluded, counted, classified as back edges, etc.).
  Always state the exact behavior.
- **Don't omit Effects** — if the algorithm modifies output parameters,
  document what is mutated and what is not (especially: "Does not modify `g`").
- **Don't use inconsistent header doc styles** — in-header Doxygen must use
  the bold-keyword inline style (`**Constraints:**`, `**Mandates:**`,
  `**Complexity:**`, etc.), not `@par` sections or `##` markdown headings.
- **Don't misororder function-semantics sections** — follow WG21 §16.3.2.4
  order: Constraints, Mandates, Constant When, Preconditions, Hardened
  Preconditions, Effects, Postconditions, Returns, Throws, Complexity,
  Remarks, Error Conditions.
- **Don't omit Returns for non-void algorithms** — every algorithm with a
  return value needs a Returns section describing the value semantics and
  sentinel values.
- **Don't omit Constant When for constexpr algorithms** — if the algorithm
  is `constexpr`, document the conditions for constant evaluation.
