# Documentation Refactor Strategy

## Goal

Transform the graph-v3 documentation from an internal development log into a polished, professional open-source library documentation that:

1. **Attracts users** with a compelling README and clear value proposition
2. **Onboards quickly** with getting-started guides and practical examples
3. **Serves as reference** with accurate, organized API documentation
4. **Supports contributors** with architecture guides, templates, and conventions
5. **Preserves history** by relocating v2 comparison content to a dedicated page

---

## Current State Assessment

### What Exists

| Location | Content | Issues |
|----------|---------|--------|
| `README.md` (555 lines) | Everything: overview, features, v2 comparison, build instructions, code examples, project structure, design principles, testing commands, contributing, status | Monolithic; mixes user/contributor content; outdated test counts and status; v2 comparison inline; project structure doesn't match reality; Quick Start references outdated types; "Next Steps" section says views/algorithms are future but they're implemented |
| `descriptor.md` (634 lines) | Detailed implementation instructions for the descriptor system | Internal agent/design doc, not user-facing |
| `docs/views.md` (878 lines) | End-user view documentation | Good quality, needs minor updates |
| `docs/container_interface.md` (387 lines) | GCI specification summary | Good quality, API reference |
| `docs/cpo.md` (1706 lines) | CPO implementation guide | Contributor doc; some duplication |
| `docs/graph_cpo_implementation.md` (1434 lines) | Complete CPO implementation guide | Contributor doc; overlaps with cpo.md |
| `docs/algorithms/README.md` (352 lines) | Algorithm catalog | Lists many unimplemented algorithms as if available |
| `docs/edge_map_analysis.md` (485 lines) | Phase 4.3 design analysis | Stale design doc; should be archived |
| `docs/common_graph_guidelines.md` (~160 lines) | Project conventions | Directory tree outdated |
| `docs/graph_cpo_order.md` (~175 lines) | CPO implementation order | Contains v2 path reference |
| `docs/view_chaining_limitations.md` (~160 lines) | Describes the solution for view chaining (not limitations) | Misleading title; content is about the solution |
| `docs/edge_value_concepts.md` (314 lines) | Edge value type concepts | Good quality, API reference |
| `docs/vertex_inner_value_patterns.md` (~210 lines) | Inner value patterns for vertices | Good quality, API reference |
| `docs/vertex_storage_concepts.md` (~260 lines) | Vertex storage pattern concepts | Good quality, API reference |
| `docs/algorithm_template.md` (469 lines) | Template for algorithm docs | Contributor template |
| `docs/view_template.md` (584 lines) | Template for view docs | Contributor template |
| `include/graph/README.md` (92 lines) | Directory structure description | Completely outdated — lists items as "Planned" that are implemented; references files that have moved |
| `include/graph/algorithm/README.md` (200 lines) | Algorithm directory conventions | Lists algorithms that don't exist |
| `include/graph/algorithm/README.md` (200 lines) | Algorithm directory conventions | Lists algorithms that don't exist |

### Key Problems

1. **No clear value proposition** — README doesn't immediately show why someone should use this library
2. **v2 comparison mixed into README** — important for existing graph-v2 users but clutters the main page
3. **Outdated information** — test counts, status badges, "future" items that are now implemented
4. **No audience separation** — users, contributors, and reference readers all share the same pages
5. **Algorithm docs overstate availability** — lists BFS, A*, Floyd-Warshall etc. as if implemented
6. **Missing landing page for docs/** — no index or navigation
7. **Stale design documents** mixed with evergreen reference docs
8. **Edge list under-documented** — the edge list ADT is a peer to the adjacency list but gets ~25% of the space in `container_interface.md`; its namespace (`graph::edge_list`) and concept names (`basic_sourced_edgelist`) don't match the doc
9. **Container variety not showcased** — `dynamic_graph` supports 26 vertex×edge container combinations (4 vertex containers × 7 edge containers) but this power isn't visible to users
10. **Orphaned files** — `descriptor.md` (top-level), `include/graph/README.md`, and several `docs/` files have no clear destination in the new structure

---

## Target Documentation Structure

```
README.md                           # Compelling landing page (NEW — rewrite)
CHANGELOG.md                        # Version history (NEW)
CONTRIBUTING.md                     # Contributor guide (NEW — extracted from README)
docs/
├── index.md                        # Documentation hub / navigation (NEW)
├── getting-started.md              # Installation, first graph, first algorithm (NEW)
├── migration-from-v2.md            # v2 comparison content + migration guide (NEW)
│
├── user-guide/                     # For library users (NEW directory)
│   ├── adjacency-lists.md          # Adjacency list ADT: concepts, CPOs, containers (NEW)
│   ├── edge-lists.md               # Edge list ADT: concepts, CPOs, patterns (NEW)
│   ├── containers.md               # All 3 graph containers + container matrix (NEW)
│   ├── views.md                    # Views documentation (MOVE from docs/views.md, update)
│   ├── algorithms.md               # Implemented algorithms only (NEW — replace algorithms/README.md)
│   └── examples.md                 # Annotated examples collection (NEW)
│
├── reference/                      # API reference (NEW directory)
│   ├── adjacency-list-interface.md # Adjacency list GCI spec (EXTRACT from container_interface.md)
│   ├── edge-list-interface.md      # Edge list GCI spec (EXTRACT from container_interface.md, sync with code)
│   ├── concepts.md                 # All concepts in one place (NEW — consolidate)
│   ├── cpo-reference.md            # CPO function signatures & behavior (NEW — extract from cpo.md)
│   ├── algorithm-complexity.md     # Complexity cheat sheet: all algorithms, O() time/space, concepts (NEW)
│   ├── edge-value-concepts.md      # (MOVE from docs/edge_value_concepts.md)
│   ├── vertex-patterns.md          # (MERGE vertex_inner_value_patterns.md + vertex_storage_concepts.md)
│   └── type-aliases.md             # Collected type alias reference (NEW)
│
├── contributing/                   # For contributors/maintainers (NEW directory)
│   ├── architecture.md             # Design principles, range-of-ranges, CPO-based interface (NEW)
│   ├── coding-guidelines.md        # (RENAME/UPDATE common_graph_guidelines.md)
│   ├── cpo-implementation.md       # How to add a CPO (CONSOLIDATE cpo.md + graph_cpo_implementation.md)
│   ├── cpo-order.md                # (MOVE/UPDATE graph_cpo_order.md)
│   ├── algorithm-template.md       # (MOVE from docs/algorithm_template.md)
│   ├── view-template.md            # (MOVE from docs/view_template.md)
│   └── view-chaining.md            # (RENAME from docs/view_chaining_limitations.md — content is the solution, not limitations)
│
├── FAQ.md                          # Common questions and answers (NEW)
│
└── archive/                        # Stale/superseded documents preserved for reference (NEW directory)
    ├── edge_map_analysis.md        # (MOVE from docs/) — Phase 4.3 design analysis
    ├── descriptor.md               # (MOVE from top-level) — early agent instruction doc
    └── include_graph_README.md     # (MOVE from include/graph/README.md) — outdated directory listing
```

### GitHub Pages Consideration

GitHub renders `docs/index.md` if linked from the repo description. Consider generating a GitHub Pages
site in the future (e.g., via MkDocs or Jekyll) to provide searchable, navigable documentation. The
`docs/` structure above is designed to be compatible with static site generators.

### Archive Policy

Any file that does not have a clear place in the `user-guide/`, `reference/`, or `contributing/` sections
should be moved to `docs/archive/` for review. This preserves content without cluttering the user-facing
documentation. Files in `docs/archive/` can be restored, merged into other docs, or deleted after review.

Candidates identified so far:

| File | Reason |
|------|--------|
| `descriptor.md` (top-level) | Early agent instruction doc referencing nonexistent `desc/` directory structure |
| `include/graph/README.md` | Completely outdated — lists implemented features as "Planned", references moved files |
| `docs/edge_map_analysis.md` | Phase 4.3 design artifact, dated Dec 2024, superseded by implementation |
| `include/graph/algorithm/README.md` | Lists algorithms (A*, Floyd-Warshall, BFS) that don't exist as headers |

---

## New README.md Design

The README is the most critical page. It should follow the structure of successful C++ open-source libraries (range-v3, {fmt}, Boost.Graph2):

### Structure

1. **Title + one-line tagline** — "A modern C++20 graph library for algorithms, views, and custom graph types."
2. **Badges (shields.io)** — Static or CI-linked badges for:
   - ![C++20](https://img.shields.io/badge/C%2B%2B-20-blue) C++ standard
   - ![License](https://img.shields.io/badge/license-BSL--1.0-green) License
   - ![Tests](https://img.shields.io/badge/tests-3900%2B-brightgreen) Test count (pin to actual `ctest` output)
   - Build status (if CI is configured)
   - Even without public CI, static badges add professionalism and signal maturity
3. **Highlights section** (bullet list, 5-7 items) — the "why use this" at a glance:
   - Header-only, C++20
   - Works with your existing containers (vector-of-vectors, maps, etc.) — zero boilerplate
   - 10+ algorithms (Dijkstra, Bellman-Ford, BFS, DFS, MST, connected components, ...)
   - Lazy views for traversal (BFS, DFS, topological sort) with `std::views` chaining
   - Customization Point Objects — adapt any graph type without modification
   - 3900+ unit tests (pin to actual number from `ctest`)
4. **Quick example** — minimal, compilable, impressive (Dijkstra on a vector-of-vectors)
5. **Two abstract data structures** — brief, equal-weight descriptions of:
   - **Adjacency Lists** — range-of-ranges model, vertex-centric
   - **Edge Lists** — flat range of sourced edges, edge-centric
6. **Feature overview** — brief sections with links to detailed docs:
   - Graph Containers (all 3: `dynamic_graph`, `compressed_graph`, `undirected_adjacency_list`)
   - Container flexibility (26 vertex×edge combinations via standard containers)
   - Views
   - Algorithms
   - CPO Architecture
7. **Supported compilers table** — Explicit compiler/version/platform matrix:
   | Compiler | Minimum Version | Platform | Status |
   |----------|----------------|----------|--------|
   | GCC | 10+ | Linux | Tested |
   | Clang | 10+ | Linux, macOS | Tested |
   | MSVC | 2019+ | Windows | Tested |
   This builds confidence and is more professional than a one-line mention.
8. **Installation / Getting Started** — CMake integration, include paths
9. **Boost.Graph comparison** (optional, brief) — 3-4 row table: "Why graph-v3 over Boost.Graph?"
   - Modern C++20 vs. C++98/03 origins
   - CPOs vs. property maps
   - Works with standard containers vs. custom graph types required
   - Header-only vs. some compiled components
   Many C++ developers know BGL; this comparison immediately anchors graph-v3's value.
10. **Documentation links** — pointer to `docs/index.md`
11. **Contributing** — pointer to `CONTRIBUTING.md`
12. **License**
13. **Status footer** — compact status line with implemented CPOs, containers, views, algorithms (pinned to actual counts)

**Explicitly removed from README:**
- v2 comparison (→ `docs/migration-from-v2.md`)
- Descriptor system deep-dive (→ `docs/reference/`)
- Design principles essay (→ `docs/contributing/architecture.md`)
- Exhaustive test command list (→ `CONTRIBUTING.md`)
- Project directory tree (→ `docs/contributing/architecture.md`)

---

## Execution Steps

### Phase 0: Truth Sync (Blocking Gate)

Before moving files or rewriting content, establish the current source-of-truth so refactoring does not preserve stale claims.

| Step | Action | Details |
|------|--------|---------|
| 0.1 | Validate public umbrella headers compile | Compile smoke tests for `#include <graph/graph.hpp>`, `#include <graph/views.hpp>`, `#include <graph/algorithms.hpp>`; fix any include drift before doc edits |
| 0.2 | Verify README consumer instructions | Confirm CMake install/consume snippet and exported target names match actual `CMakeLists.txt` export (`graph::graph3` vs historical names) |
| 0.3 | Generate implementation matrix from tests | Build a table from `tests/algorithms/CMakeLists.txt` and `tests/container/CMakeLists.txt` listing implemented algorithms, containers, and trait combinations |
| 0.4 | Reconcile algorithm/header names | Ensure docs reference real headers (`breadth_first_search.hpp`, `depth_first_search.hpp`, `bellman_ford_shortest_paths.hpp`, `mst.hpp`, `tc.hpp`, etc.) |
| 0.5 | Pin canonical metrics snapshot | Record `ctest` totals and other status metrics once, then reference that source everywhere (README footer, docs index, status summaries) |
| 0.6 | Freeze baseline status artifact | Create a single status artifact (table in `docs/index.md` or dedicated status file) used as the source for all user-facing claims |

### Phase 1: Foundation (branch: `newdoc`)

| Step | Action | Details |
|------|--------|---------|
| 1.1 | Create directory structure | `docs/user-guide/`, `docs/reference/`, `docs/contributing/`, `docs/archive/` |
| 1.2 | Write `docs/index.md` | Navigation hub with audience-based sections and links; link from repo description |
| 1.3 | Write `docs/migration-from-v2.md` | Extract v2 comparison from README, expand with migration guidance |
| 1.4 | Move `docs/edge_map_analysis.md` → `docs/archive/` | Stale design doc |
| 1.5 | Write `docs/FAQ.md` | Common questions: "Can I use my own graph type?", "How do I add edge weights?", "What's the difference between views and algorithms?", "Why descriptors instead of iterators?", "How does this compare to Boost.Graph?" |

### Phase 2: README Rewrite

| Step | Action | Details |
|------|--------|---------|
| 2.1 | Write new `README.md` | Following the design above; compelling, accurate, concise |
| 2.2 | Add shields.io badges | C++20, BSL-1.0 license, test count (pinned), build status if available |
| 2.3 | Add supported compilers table | GCC/Clang/MSVC with versions, platforms, and status |
| 2.4 | Add Boost.Graph comparison table | Brief 3-4 row table anchoring graph-v3's value proposition |
| 2.5 | Verify all code examples compile | Or mark as pseudocode |
| 2.6 | Pin all numbers to reality | Run `ctest`, count algorithms/views/containers; use one source of truth for all counts. The current README says "535 tests" in one place and "3931 tests" in another — this must not happen again |

### Phase 3: User Guide

| Step | Action | Details |
|------|--------|---------|
| 3.1 | Write `docs/getting-started.md` | Installation, CMake integration, first graph, first algorithm |
| 3.2 | Write `docs/user-guide/adjacency-lists.md` | Adjacency list ADT overview: range-of-ranges model, concepts (`adjacency_list`, `index_adjacency_list`), CPOs (`vertices`, `edges`, `target_id`, etc.), how descriptor views work |
| 3.3 | Write `docs/user-guide/edge-lists.md` | Edge list ADT overview: flat sourced-edge model, concepts (`basic_sourced_edgelist`, `basic_sourced_index_edgelist`, `has_edge_value`), CPOs (`source_id`, `target_id`, `edge_value`), supported edge patterns (pair, tuple, edge_info, edge_descriptor), non-integral vertex IDs |
| 3.4 | Write `docs/user-guide/containers.md` | All 3 graph containers with equal coverage, container selection guide, full container matrix (see below) |
| 3.5 | Move + update `docs/views.md` → `docs/user-guide/views.md` | Keep content, add navigation links |
| 3.6 | Write `docs/user-guide/algorithms.md` | Only implemented algorithms; clear "planned" section |
| 3.7 | Write `docs/user-guide/examples.md` | Curated, annotated examples |

#### Adjacency List vs Edge List: Equal-Weight Treatment

The adjacency list is ~95% of the library's surface area, but the edge list is a distinct, peer abstract
data structure — not a subset. Documentation must present them as co-equal to avoid the misconception that
edge lists are secondary. Specifically:

- Both get their own user-guide page (`adjacency-lists.md` and `edge-lists.md`)
- Both get their own reference page (`adjacency-list-interface.md` and `edge-list-interface.md`)
- The getting-started guide and README show examples of both
- `docs/index.md` navigation lists them side by side
- The key conceptual distinction is made explicit early: adjacency lists are a range of vertices where each
  vertex is a range of edges; edge lists are a flat range of edges with source and target IDs

#### Container Documentation Requirements

The `docs/user-guide/containers.md` page must include:

**All 3 graph containers:**

| Container | Storage | Mutability | Best For |
|-----------|---------|------------|----------|
| `dynamic_graph<EV,VV,GV,VId,Sourced,Traits>` | Traits-configured vertex and edge containers | Mutable | General purpose, flexible container choice |
| `compressed_graph<EV,VV,GV,VId,EIndex,Alloc>` | CSR (compressed sparse row) | Immutable after construction | Read-only, high-performance, memory-compact |
| `undirected_adjacency_list<VV,EV,GV,VId,VContainer,Alloc>` | Dual doubly-linked lists per edge | Mutable, O(1) edge removal | Undirected graphs, frequent edge insertion/removal |

**Full `dynamic_graph` container matrix (26 combinations):**

Vertex containers:

| Container | Iterator | Vertex ID | Abbreviation |
|-----------|----------|-----------|------|
| `std::vector` | Random access | Integral index | `v` |
| `std::deque` | Random access | Integral index | `d` |
| `std::map` | Bidirectional | Ordered key (any) | `m` |
| `std::unordered_map` | Forward | Hashable key (any) | `u` |

Edge containers:

| Container | Iterator | Properties | Abbreviation |
|-----------|----------|------------|------|
| `std::vector` | Random access | Cache-friendly, allows duplicates | `v` |
| `std::deque` | Random access | Efficient front/back insertion | `d` |
| `std::forward_list` | Forward | Minimal memory overhead | `fl` |
| `std::list` | Bidirectional | O(1) insertion/removal anywhere | `l` |
| `std::set` | Bidirectional | Sorted, deduplicated | `s` |
| `std::unordered_set` | Forward | Hash-based, O(1) avg | `us` |
| `std::map` | Bidirectional | Sorted by target_id key | `em` |

Traits naming convention: `{vertex}o{edge}_graph_traits` (e.g., `vov_graph_traits` = vector vertices, vector edges).

All 26 combinations listed with trait file names (vov, vod, vofl, vol, vos, vous, voem, dov, dod, dofl, dol, dos, dous, mov, mod, mofl, mol, mos, mous, moem, uov, uod, uofl, uol, uos, uous).

### Phase 4: Reference

| Step | Action | Details |
|------|--------|---------|
| 4.1 | Split `container_interface.md` → `docs/reference/adjacency-list-interface.md` + `docs/reference/edge-list-interface.md` | Give each ADT its own reference page; fix edge list namespace/concept name drift (doc says `sourced_edgelist`, code has `basic_sourced_edgelist`; doc says `graph::container::edgelist`, code uses `graph::edge_list`) |
| 4.2 | Merge vertex concept docs → `docs/reference/vertex-patterns.md` | Combine vertex_inner_value_patterns.md + vertex_storage_concepts.md |
| 4.3 | Move `edge_value_concepts.md` → `docs/reference/` | Update links |
| 4.4 | Write `docs/reference/cpo-reference.md` | Extract pure reference (signatures, behavior) from cpo.md |
| 4.5 | Write `docs/reference/concepts.md` | Consolidated concept reference — adjacency list concepts (9) AND edge list concepts (3) side by side |
| 4.6 | Write `docs/reference/algorithm-complexity.md` | Single-page cheat sheet: all implemented algorithms with time/space complexity, required concepts, one-line descriptions. Extremely useful as a quick reference for users evaluating or selecting algorithms |

### Phase 5: Contributor Documentation

| Step | Action | Details |
|------|--------|---------|
| 5.1 | Write `CONTRIBUTING.md` | Extract from README: conventions, testing, PR process |
| 5.2 | Write `docs/contributing/architecture.md` | Design principles, directory structure, range-of-ranges model |
| 5.3 | Consolidate CPO guides → `docs/contributing/cpo-implementation.md` | Merge `cpo.md` (1706 lines) + `graph_cpo_implementation.md` (1434 lines), deduplicate heavily. Target ~1500 lines total. Structure as two clear parts: Part 1 "How CPOs work" (concepts, MSVC pattern, priority tiers) and Part 2 "How to implement a CPO" (step-by-step, worked `vertex_id` example). The same pattern is currently shown 3 times across the two files — reduce to once with cross-references |
| 5.4 | Move template + convention docs into `docs/contributing/` | algorithm_template, view_template, view_chaining, guidelines, cpo_order |
| 5.5 | Update `docs/contributing/coding-guidelines.md` | Fix outdated directory tree, remove v2 references |

### Phase 6: Cleanup & Archive

| Step | Action | Details |
|------|--------|--------|
| 6.1 | Move orphaned/unclear files to `docs/archive/` | `descriptor.md` → archive, `include/graph/README.md` → archive (as `include_graph_README.md`), `include/graph/algorithm/README.md` → archive or rewrite |
| 6.2 | Remove empty original locations | Delete `docs/edge_map_analysis.md`, etc. after moves are verified |
| 6.3 | Update all internal cross-references | Grep for broken `docs/` links, fix all relative paths |
| 6.4 | Verify edge list / adjacency list parity | Ensure both ADTs appear in index.md, getting-started.md, README.md, and reference/ with comparable depth |
| 6.5 | Verify all 3 containers documented | Ensure `dynamic_graph`, `compressed_graph`, and `undirected_adjacency_list` each have clear sections |
| 6.6 | Write `CHANGELOG.md` | Initial version based on git history and phase completion notes |
| 6.7 | Final review pass | Read every doc end-to-end for consistency, accuracy, broken links |

---

## Measurable Acceptance Gates

### Global Exit Criteria (must all pass)

- **No broken public includes:** smoke-check translation units for `graph.hpp`, `views.hpp`, and `algorithms.hpp` compile.
- **No broken links:** internal markdown links under `README.md` and `docs/` resolve.
- **No stale user-facing "planned/future" drift:** landing pages only describe implemented features or clearly labeled roadmap items.
- **Single source of truth for metrics:** test counts and feature counts match the canonical status artifact.
- **ADT parity requirement met:** adjacency list and edge list each have user guide + reference coverage.

### Phase-by-Phase Gates

| Phase | Gate | Pass Condition |
|------|------|----------------|
| 0 | Truth sync complete | Public includes compile; README install target verified; implementation matrix and canonical metrics artifact created |
| 1 | Foundation complete | `docs/index.md`, `docs/migration-from-v2.md`, `docs/FAQ.md`, and archive moves exist with valid links |
| 2 | README complete | New README sections present; compiler table present; comparison table present; all snippets compile or are explicitly marked pseudocode |
| 3 | User guide complete | `adjacency-lists.md`, `edge-lists.md`, `containers.md`, `views.md`, `algorithms.md`, `examples.md` exist and cross-link correctly |
| 4 | Reference complete | Split interface docs exist and match code terminology (`graph::edge_list`, `basic_sourced_edgelist`, etc.); complexity cheat sheet present |
| 5 | Contributor docs complete | Consolidated CPO implementation doc created; duplicate guidance removed; templates/guidelines moved and link-valid |
| 6 | Cleanup complete | Orphaned docs archived or rewritten; no broken cross-references; final consistency review complete |

### Quality Gates for User-Facing Pages

- **Accuracy gate:** every algorithm/container/header named in user docs exists in source tree.
- **Consistency gate:** namespace and concept names match code declarations.
- **Navigability gate:** every user page links back to `docs/index.md` and to adjacent relevant sections.
- **Clarity gate:** each page begins with scope, audience, and "what this page covers".

---

## Guiding Principles

- **Accuracy over aspiration** — Only document what exists. Planned features go in a clearly labeled "Roadmap" section.
- **Three audiences, three paths** — Every page should be reachable from `docs/index.md` via a clear "Users | Contributors | Reference" navigation.
- **Progressive disclosure** — README → Getting Started → User Guide → Reference. Don't front-load complexity.
- **One source of truth** — Each concept, CPO, or container is documented in exactly one place. Other docs link to it.
- **Keep it maintainable** — Templates (algorithm_template.md, view_template.md) ensure new docs are consistent without manual enforcement.
- **Equal weight for peer ADTs** — Adjacency lists and edge lists are co-equal abstract data structures. Documentation must present them with comparable depth, even though the adjacency list has more surface area.
- **Archive, don't delete** — Files without a clear place go to `docs/archive/` for review rather than being deleted. This preserves history and allows content to be recovered if needed.
- **Pin numbers to reality** — Test counts, algorithm counts, and other metrics must come from a single source of truth (e.g., `ctest` output). Never hardcode the same number in multiple places — use the status footer as the canonical source and reference it elsewhere.
