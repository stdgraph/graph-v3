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

```
# Algorithm Name

> [← Back to Algorithm Catalog](../algorithms.md)

- [Overview](#overview)
- [When to Use](#when-to-use)
- [Include](#include)
- [Signature(s)](#signatures)        ← "Signature" (singular) if only one overload
- [Parameters](#parameters)
- [Visitor Events](#visitor-events)   ← (if applicable)
- [Examples](#examples)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [Notes](#notes)                     ← (if applicable)
- [See Also](#see-also)
```

### Special cases

- **Multiple sub-algorithms** (e.g., MST has Kruskal + Prim, Connected
  Components has 3 algorithms): Replace the single Signature/Parameters
  sections with per-algorithm subsections under an `## Algorithms` or
  named `## Kruskal's Algorithm` / `## Prim's Algorithm` pattern.
- **Visitor Events**: Include this section only for algorithms that accept a
  visitor (currently: Dijkstra, Bellman-Ford, BFS, DFS).
- **Notes**: Include when there are important behavioral details that don't
  fit in Preconditions (e.g., self-loop handling differences, in-place
  modification warnings, convergence properties).

---

## Section-by-Section Instructions

### Overview

- **First sentence**: Define the algorithm in bold, stating what it computes.
- State the graph concept requirement prominently:
  > The graph must satisfy `index_adjacency_list<G>` — vertices are stored in a
  > contiguous, integer-indexed random-access range.
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
- Include `constexpr`, `[[nodiscard]]`, and template constraints if present in
  the actual header.
- After the code block, document the **return type** in bold if non-void
  (e.g., "**Returns** `std::pair<EV, size_t>` — total weight and component count.").
- Use "Signature" (singular) if there is only one overload; "Signatures" (plural)
  if there are multiple.

### Parameters

- Markdown table with columns: `Parameter`, `Description`.
- Every parameter from every overload must appear.
- Note default values inline (e.g., "Default: `std::less<>{}`").
- For output parameters, state the sizing requirement
  (e.g., "Random-access range sized to `num_vertices(g)`").

### Visitor Events (if applicable)

- Markdown table with columns: `Event`, `When invoked`, `Common uses`.
- List every visitor callback the algorithm recognizes.
- Note which events are unique to this algorithm vs. shared.

### Examples

**Quantity**: At minimum 5 examples per page. More is acceptable and
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

### Complexity

- Markdown table with columns: `Metric`, `Value` (or `Algorithm`, `Time`,
  `Space` for multi-algorithm pages).
- Use standard variables: V = vertices, E = edges, d = degree.
- Add a brief paragraph if the complexity has interesting properties
  (e.g., "The merge-based intersection exploits sorted neighbor lists").

### Preconditions

- Bullet list of every requirement the caller must satisfy.
- Always include the `index_adjacency_list<G>` requirement.
- For undirected graph algorithms, state: "For undirected graphs, **both
  directions** of each edge must be stored (or use
  `undirected_adjacency_list`)."
- Mention self-loop behavior, valid vertex ID ranges, and sizing requirements
  for output containers.

### Notes (if applicable)

- Bullet list of behavioral details, caveats, or design decisions.
- Good candidates: in-place modification warnings, self-loop counting behavior
  (especially when it differs from other algorithms), convergence properties,
  determinism guarantees, differences between similar algorithms.

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
   - Template constraints and concepts
   - `constexpr`, `[[nodiscard]]`, and other attributes
   - Default parameter values
   - Return types
   - Visitor event names (if any)

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

6. **Verify consistency**:
   - All signatures match the actual header file exactly.
   - All visitor events match the actual implementation.
   - Complexity values match the algorithm catalog tables.
   - The `index_adjacency_list<G>` requirement is stated in Overview and
     Preconditions.
   - All cross-links resolve (test by checking file paths).
   - At least 5 examples are provided.
   - Examples progress from simple to advanced.
   - Expected output values are shown in comments for every example.

---

## Reference: Existing Pages

Use these as models. The pages vary slightly based on algorithm features but
all follow the structure above.

| Page | Notable features to model |
|------|---------------------------|
| `dijkstra.md` | Multiple overloads, visitor events, `constexpr` signatures, `_id` variant examples, error handling note |
| `bellman_ford.md` | `[[nodiscard]]` attribute, `find_negative_cycle` helper, unique visitor events (`on_edge_minimized`) |
| `dfs.md` | Richest visitor events (9), single-source limitation note, subtree computation example |
| `connected_components.md` | Multi-algorithm structure (3 sub-algorithms), `compress()` helper, per-algorithm selection guidance |
| `mst.md` | Two distinct algorithms (Kruskal + Prim) with separate signatures, `inplace_kruskal` modification warning, cross-validation example |
| `label_propagation.md` | RNG parameter, convergence notes, self-loop counting behavior, reproducibility example |
| `triangle_count.md` | Additional concept requirement (`ordered_vertex_edges`), pre-sorting workaround example |
| `mis.md` | Seed sensitivity, maximal vs. maximum distinction, self-loop exclusion |

---

## Common Pitfalls to Avoid

- **Don't omit "When to Use"** — this section is critical for newcomers
  choosing between algorithms.
- **Don't show fewer than 5 examples** — comprehensive examples are the
  primary educational tool.
- **Don't leave expected output ambiguous** — always show concrete values in
  code comments.
- **Don't forget the `index_adjacency_list` requirement** — state it in both
  Overview and Preconditions.
- **Don't show `#include` in every example** — only the first example needs
  includes and `using` declarations.
- **Don't mix singular/plural "Signature(s)"** — use singular for one
  overload, plural for multiple.
- **Don't document features that don't exist** — read the actual header, don't
  assume based on other algorithms.
- **Don't forget to update the catalog** — the landing page at
  `docs/user-guide/algorithms.md` must list every algorithm with complexity.
