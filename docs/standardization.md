# C++ Standardization

graph-v3 is the reference implementation for a proposed graph library for the **ISO C++ Standard (C++26/C++29)**.
The proposal is developed under the [WG21](https://isocpp.org/std/the-committee) umbrella and targets the Standard
Library Evolution Working Group (LEWG).

> The previous reference implementation is [graph-v2](https://github.com/stdgraph/graph-v2).
> graph-v3 refines and supersedes that work.

---

## Goals

The proposed library aims to:

1. Support creation of high-performance, state-of-the-art graph algorithms.
2. Provide syntax that is simple, expressive and easy to understand when writing algorithms.
3. Define useful concepts and traits that algorithms can use to express their requirements.
4. Support views for graph traversal commonly used by algorithms.
5. Support optional, user-defined value types for edges, vertices and the graph itself.
6. Allow the use of standard containers to define simple graphs.
7. Enable easy integration of existing graph data structures.

---

## ISO C++ Papers

The following papers collectively form the current proposal submitted to WG21.

| Paper | Title | Description |
| :---- | :---- | :---------- |
| [P3126](https://wg21.link/P3126) | Overview | Describes the big picture of what is being proposed. |
| [P3127](https://wg21.link/P3127) | Background and Terminology | Motivation and theoretical background underlying the proposal. |
| [P3128](https://wg21.link/P3128) | Algorithms | Covers the initial algorithms as well as planned future additions. |
| [P3129](https://wg21.link/P3129) | Views | Helpful views for traversing a graph. |
| [P3130](https://wg21.link/P3130) | Graph Container Interface | The core interface for uniformly accessing graph data structures and adapting to external graphs. |
| [P3131](https://wg21.link/P3131) | Graph Containers | Includes `compressed_graph` and use of standard containers to define simple graphs. |
| [P3337](https://wg21.link/P3337) | Graph Comparison | Syntax and performance comparison to the Boost Graph Library. |

---

## Relationship Between Papers and This Library

| Paper | Library location |
| :---- | :--------------- |
| P3128 Algorithms | [`include/graph/algorithm/`](../include/graph/algorithm/) |
| P3129 Views | [`include/graph/views/`](../include/graph/views/) |
| P3130 Graph Container Interface | [`include/graph/detail/`](../include/graph/detail/) and [CPO Reference](reference/cpo-reference.md) |
| P3131 Graph Containers | [`include/graph/container/`](../include/graph/container/) |

---

## How to Participate

- **WG21 mailing list and papers:** <https://isocpp.org/std/the-committee>
- **GitHub discussions:** <https://github.com/stdgraph/graph-v3/discussions>
- **GitHub issues:** <https://github.com/stdgraph/graph-v3/issues>

---

## Acknowledgements

- The [NWGraph](https://github.com/NWmath/NWgr) team for collaboration and algorithm implementations.
- The Machine Learning study group (SG19) of the ISO C++ Standards Committee (WG21) for comments and support.
