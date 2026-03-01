<table><tr>
<td><img src="assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Frequently Asked Questions

</td>
</tr></table>

> [← Back to Documentation Index](index.md)

---

### Can I use my own graph type?

Yes. graph-v3 is built on **Customization Point Objects (CPOs)** — the same mechanism the
C++ standard library uses for `std::ranges`. Any type that satisfies the graph concepts
(e.g. a `vector<vector<int>>`) works automatically. For types that don't model the concepts
out of the box, you can specialize the CPOs.

See [CPO Reference](reference/cpo-reference.md) for the full list of customization points.

---

### How do I add edge weights?

Edge weights are modeled as **edge values**. You can attach values to edges in several ways:

- Use a container that stores values directly (e.g. `vector<vector<pair<int, double>>>`)
- Provide an edge value function (`EVF`) to views and algorithms
- Use `dynamic_graph` with an edge value template parameter

See [Edge Value Concepts](reference/edge-value-concepts.md) for the concept definitions.

---

### What's the difference between views and algorithms?

**Views** are lazy ranges — they don't compute anything until you iterate. They adapt the
graph into a different traversal order (BFS, DFS, topological sort) or projection (vertexlist,
edgelist, neighbors, incidence). You can chain them with `|` pipe syntax.

**Algorithms** eagerly compute a result — shortest paths, connected components, MST, etc.
They typically take output parameters (distance vectors, predecessor maps) and return
upon completion.

- [Views documentation](user-guide/views.md)
- [Algorithms documentation](user-guide/algorithms.md)

---

### Why descriptors instead of iterators?

Descriptors are lightweight value types that identify a vertex or edge. Compared to iterators:

| | Descriptors | Iterators |
|-|-------------|-----------|
| **Size** | Typically one index or pointer | Often two pointers |
| **Stability** | Remain valid across many mutations | Invalidated by container mutations |
| **Composability** | Easy to store, pass, return | Tied to a specific range |
| **Concept count** | Enabled reducing from 18 to 9 concepts | Required separate sourced/unsourced variants |

See [Architecture](contributing/architecture.md) for the design rationale.

---

### How does this compare to Boost.Graph?

| | graph-v3 | Boost.Graph |
|-|----------|-------------|
| **C++ standard** | C++20 (concepts, ranges) | C++98/03 origins |
| **Interface** | CPOs (like `std::ranges`) | Property maps + traits classes |
| **Container support** | Works with `vector<vector<int>>`, maps, etc. | Requires `adjacency_list<>` or custom adaptors |
| **Library type** | Header-only | Some compiled components |

graph-v3 is designed for modern C++ codebases that want standard-library-style ergonomics.
Boost.Graph remains a mature, battle-tested choice with a larger algorithm catalog.

---

### Which container should I use?

**Start here:**

1. **Read-only, high-performance?** → `compressed_graph` (CSR layout, minimal memory)
2. **Mutable, general-purpose?** → `dynamic_graph` (26 vertex×edge container combinations)
3. **Undirected with frequent edge removal?** → `undirected_adjacency_list` (O(1) edge removal)
4. **Already have a `vector<vector<T>>`?** → Use it directly — no container needed

See [Containers](user-guide/containers.md) for the full selection guide and trait matrix.

---

### Does this work on Windows/macOS?

Yes. Supported compilers:

| Compiler | Minimum Version | Platform | Status |
|----------|----------------|----------|--------|
| GCC | 10+ | Linux | Tested |
| Clang | 10+ | Linux, macOS | Tested |
| MSVC | 2019+ | Windows | Tested |

CMake presets are provided for all platforms — see the [README](../README.md) or
[Getting Started](getting-started.md).

---

> [← Back to Documentation Index](index.md)
