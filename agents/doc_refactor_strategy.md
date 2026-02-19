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
| Other `docs/*.md` files | Concept docs, templates | Generally good quality |
| `include/graph/algorithm/README.md` (200 lines) | Algorithm directory conventions | Lists algorithms that don't exist |

### Key Problems

1. **No clear value proposition** — README doesn't immediately show why someone should use this library
2. **v2 comparison mixed into README** — important for existing graph-v2 users but clutters the main page
3. **Outdated information** — test counts, status badges, "future" items that are now implemented
4. **No audience separation** — users, contributors, and reference readers all share the same pages
5. **Algorithm docs overstate availability** — lists BFS, A*, Floyd-Warshall etc. as if implemented
6. **Missing landing page for docs/** — no index or navigation
7. **Stale design documents** mixed with evergreen reference docs

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
│   ├── graphs-and-containers.md    # Graph types, when to use each (NEW)
│   ├── views.md                    # Views documentation (MOVE from docs/views.md, update)
│   ├── algorithms.md               # Implemented algorithms only (NEW — replace algorithms/README.md)
│   └── examples.md                 # Annotated examples collection (NEW)
│
├── reference/                      # API reference (NEW directory)
│   ├── container-interface.md      # GCI spec (MOVE from docs/container_interface.md)
│   ├── concepts.md                 # All concepts in one place (NEW — consolidate)
│   ├── cpo-reference.md            # CPO function signatures & behavior (NEW — extract from cpo.md)
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
│   └── view-chaining.md            # (MOVE from docs/view_chaining_limitations.md)
│
└── archive/                        # Stale design documents (NEW directory)
    └── edge_map_analysis.md        # (MOVE from docs/)
```

---

## New README.md Design

The README is the most critical page. It should follow the structure of successful C++ open-source libraries (range-v3, {fmt}, Boost.Graph2):

### Structure

1. **Title + one-line tagline** — "A modern C++20 graph library for algorithms, views, and custom graph types."
2. **Badges** — build status, license, C++ standard, test count
3. **Highlights section** (bullet list, 5-7 items) — the "why use this" at a glance:
   - Header-only, C++20
   - Works with your existing containers (vector-of-vectors, maps, etc.) — zero boilerplate
   - 10+ algorithms (Dijkstra, Bellman-Ford, BFS, DFS, MST, connected components, ...)
   - Lazy views for traversal (BFS, DFS, topological sort) with `std::views` chaining
   - Customization Point Objects — adapt any graph type without modification
   - 3900+ unit tests
4. **Quick example** — minimal, compilable, impressive (Dijkstra on a vector-of-vectors)
5. **Feature overview** — brief sections with links to detailed docs:
   - Graph Containers
   - Views
   - Algorithms
   - CPO Architecture
6. **Installation / Getting Started** — CMake integration, include paths
7. **Documentation links** — pointer to `docs/index.md`
8. **Contributing** — pointer to `CONTRIBUTING.md`
9. **License**
10. **Status footer** — compact status line with implemented CPOs, containers, views, algorithms

**Explicitly removed from README:**
- v2 comparison (→ `docs/migration-from-v2.md`)
- Descriptor system deep-dive (→ `docs/reference/`)
- Design principles essay (→ `docs/contributing/architecture.md`)
- Exhaustive test command list (→ `CONTRIBUTING.md`)
- Project directory tree (→ `docs/contributing/architecture.md`)

---

## Execution Steps

### Phase 1: Foundation (branch: `newdoc`)

| Step | Action | Details |
|------|--------|---------|
| 1.1 | Create directory structure | `docs/user-guide/`, `docs/reference/`, `docs/contributing/`, `docs/archive/` |
| 1.2 | Write `docs/index.md` | Navigation hub with audience-based sections and links |
| 1.3 | Write `docs/migration-from-v2.md` | Extract v2 comparison from README, expand with migration guidance |
| 1.4 | Move `docs/edge_map_analysis.md` → `docs/archive/` | Stale design doc |

### Phase 2: README Rewrite

| Step | Action | Details |
|------|--------|---------|
| 2.1 | Write new `README.md` | Following the design above; compelling, accurate, concise |
| 2.2 | Verify all code examples compile | Or mark as pseudocode |
| 2.3 | Update status line | Accurate algorithm/view/container/test counts |

### Phase 3: User Guide

| Step | Action | Details |
|------|--------|---------|
| 3.1 | Write `docs/getting-started.md` | Installation, CMake integration, first graph, first algorithm |
| 3.2 | Write `docs/user-guide/graphs-and-containers.md` | dynamic_graph, compressed_graph, undirected_adjacency_list, auto-detection |
| 3.3 | Move + update `docs/views.md` → `docs/user-guide/views.md` | Keep content, add navigation links |
| 3.4 | Write `docs/user-guide/algorithms.md` | Only implemented algorithms; clear "planned" section |
| 3.5 | Write `docs/user-guide/examples.md` | Curated, annotated examples |

### Phase 4: Reference

| Step | Action | Details |
|------|--------|---------|
| 4.1 | Move `container_interface.md` → `docs/reference/` | Update links |
| 4.2 | Merge vertex concept docs → `docs/reference/vertex-patterns.md` | Combine vertex_inner_value_patterns.md + vertex_storage_concepts.md |
| 4.3 | Move `edge_value_concepts.md` → `docs/reference/` | Update links |
| 4.4 | Write `docs/reference/cpo-reference.md` | Extract pure reference (signatures, behavior) from cpo.md |
| 4.5 | Write `docs/reference/concepts.md` | Consolidated concept reference |

### Phase 5: Contributor Documentation

| Step | Action | Details |
|------|--------|---------|
| 5.1 | Write `CONTRIBUTING.md` | Extract from README: conventions, testing, PR process |
| 5.2 | Write `docs/contributing/architecture.md` | Design principles, directory structure, range-of-ranges model |
| 5.3 | Consolidate CPO guides → `docs/contributing/cpo-implementation.md` | Merge cpo.md + graph_cpo_implementation.md, deduplicate |
| 5.4 | Move template + convention docs into `docs/contributing/` | algorithm_template, view_template, view_chaining, guidelines, cpo_order |
| 5.5 | Update `docs/contributing/coding-guidelines.md` | Fix outdated directory tree, remove v2 references |

### Phase 6: Cleanup

| Step | Action | Details |
|------|--------|---------|
| 6.1 | Remove orphaned files from `docs/` root | After all moves are complete |
| 6.2 | Update all internal cross-references | Grep for broken `docs/` links |
| 6.3 | Update `include/graph/algorithm/README.md` | Reflect only what exists |
| 6.4 | Review `descriptor.md` | Move to `docs/archive/` or `docs/contributing/` as appropriate |
| 6.5 | Write `CHANGELOG.md` | Initial version based on git history and phase completion notes |
| 6.6 | Final review pass | Read every doc end-to-end for consistency, accuracy, broken links |

---

## Additional Suggestions

1. **Add a `docs/` landing page to the repo description** — GitHub renders `docs/index.md` if linked; consider also generating a GitHub Pages site in the future.

2. **Consider a comparison table vs. Boost.Graph** — Many C++ developers know BGL. A concise "Why graph-v3 over Boost.Graph?" section (modern C++20, header-only, CPOs vs. property maps, works with standard containers) in the README or getting-started guide would be compelling.

3. **Add a "Supported Compilers" table** — Explicit compiler/version/platform matrix instead of just "GCC 10+, Clang 10+, MSVC 2019+". This builds confidence.

4. **Create a `docs/FAQ.md`** — Common questions: "Can I use my own graph type?", "How do I add edge weights?", "What's the difference between views and algorithms?", "Why descriptors instead of iterators?"

5. **Badge/shield images in README** — Even if CI isn't public, static badges for C++20, license, and test count add professionalism.

6. **Algorithm complexity cheat sheet** — A single-page table of all implemented algorithms with time/space complexity, required concepts, and one-line descriptions. Extremely useful as quick reference.

7. **Deduplicate CPO documentation** — `cpo.md` (1706 lines) and `graph_cpo_implementation.md` (1434 lines) overlap significantly. The consolidated version should be ~1500 lines with a clear split: "How CPOs work" (concepts) vs. "How to implement a CPO" (step-by-step).

8. **Pin accurate numbers** — The README currently says "535 tests" in one place and "3931 tests" in another. Run `ctest` and use the real number everywhere, maintained in one place (e.g., the status footer).

---

## Guiding Principles

- **Accuracy over aspiration** — Only document what exists. Planned features go in a clearly labeled "Roadmap" section.
- **Three audiences, three paths** — Every page should be reachable from `docs/index.md` via a clear "Users | Contributors | Reference" navigation.
- **Progressive disclosure** — README → Getting Started → User Guide → Reference. Don't front-load complexity.
- **One source of truth** — Each concept, CPO, or container is documented in exactly one place. Other docs link to it.
- **Keep it maintainable** — Templates (algorithm_template.md, view_template.md) ensure new docs are consistent without manual enforcement.
