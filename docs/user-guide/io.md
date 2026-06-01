<table><tr>
<td><img src="../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Graph I/O

> Read and write graphs in DOT, GraphML, JSON, DIMACS, METIS, and adjacency-list-text formats.

</td>
</tr></table>

> [← Back to Documentation Index](../index.md)

---

## Table of Contents

- [Overview](#overview)
- [Headers](#headers)
- [DOT (GraphViz)](#dot-graphviz)
- [GraphML (XML)](#graphml-xml)
- [JSON](#json)
- [DIMACS](#dimacs)
- [METIS](#metis)
- [Adjacency List Text](#adjacency-list-text)
- [Design Philosophy](#design-philosophy)

---

## Overview

The `graph::io` namespace provides readers and writers for six graph interchange formats. The writers work with any graph satisfying the adjacency list concepts; the readers return lightweight parsed structures.

| Format | Writer | Reader | Use Case |
|--------|--------|--------|----------|
| **DOT** | `write_dot()` | `read_dot()` | Visualization (GraphViz), debugging |
| **GraphML** | `write_graphml()` | `read_graphml()` | XML-based interchange, tool ecosystems |
| **JSON** | `write_json()` | `read_json()` | Web applications, REST APIs, modern tooling |
| **DIMACS** | `write_dimacs()`, `write_dimacs_max_flow()` | `read_dimacs()` | Network-flow / shortest-path benchmark suites |
| **METIS** | `write_metis()` | `read_metis()` | Graph partitioning (METIS/ParMETIS) |
| **Adjacency List Text** | `write_adjacency_list_text()` | `read_adjacency_list_text()` | Quick structural dumps, debugging |

---

## Headers

```cpp
#include <graph/io.hpp>        // umbrella — includes all six formats

// Or include individually:
#include <graph/io/dot.hpp>
#include <graph/io/graphml.hpp>
#include <graph/io/json.hpp>
#include <graph/io/dimacs.hpp>
#include <graph/io/metis.hpp>
#include <graph/io/adjacency_list_text.hpp>
```

All functions live in `namespace graph::io`.

---

## DOT (GraphViz)

### Writing

```cpp
// Zero-config: auto-labels vertices/edges via std::format when VV/EV are formattable
template <class G>
void write_dot(std::ostream& os, const G& g, std::string_view graph_name = "G");

// Custom attributes: user-supplied functions return DOT attribute strings
template <class G, class VAttrFn, class EAttrFn>
void write_dot(std::ostream& os, const G& g, VAttrFn vertex_attr_fn, EAttrFn edge_attr_fn);
```

**Example:**

```cpp
#include <graph/io/dot.hpp>
#include <fstream>

std::ofstream ofs("output.dot");
graph::io::write_dot(ofs, my_graph);
// Then: dot -Tpng output.dot -o output.png
```

**Custom attributes:**

```cpp
graph::io::write_dot(ofs, g,
  [](const auto& g, auto uid) -> std::string {
    return std::format("[label=\"v{}\", color=\"blue\"]", uid);
  },
  [](const auto& g, auto src, auto tgt, auto uv) -> std::string {
    return std::format("[weight={:.2f}]", graph::edge_value(g, uv));
  });
```

### Reading

```cpp
auto result = graph::io::read_dot(input_stream);
// result.name        — graph name
// result.directed    — true for digraph, false for graph
// result.vertex_ids  — vector of vertex ID strings
// result.vertex_labels — parallel vector of labels
// result.edges       — vector of {source, target, label}
```

---

## GraphML (XML)

### Writing

```cpp
// Zero-config: auto-formats VV/EV as <data> elements
template <class G>
void write_graphml(std::ostream& os, const G& g, std::string_view graph_id = "G");

// Custom: user functions return map<string, string> of key→value data
template <class G, class VDataFn, class EDataFn>
void write_graphml(std::ostream& os, const G& g, VDataFn vertex_data_fn, EDataFn edge_data_fn);
```

**Example:**

```cpp
#include <graph/io/graphml.hpp>
#include <sstream>

std::ostringstream oss;
graph::io::write_graphml(oss, my_graph);
// Valid GraphML XML output
```

### Reading

```cpp
auto result = graph::io::read_graphml(input_stream);
// result.id       — graph id attribute
// result.directed — true/false
// result.keys     — vector of {id, for_what, name, type}
// result.nodes    — vector of {id, data: map<key, value>}
// result.edges    — vector of {id, source, target, data: map<key, value>}
```

---

## JSON

Uses a JGF-inspired format: `{"directed": bool, "nodes": [...], "edges": [...]}`.

### Writing

```cpp
// Zero-config with optional indentation control
template <class G>
void write_json(std::ostream& os, const G& g, int indent = 2);

// Custom: user functions return map<string, string> of extra attributes
template <class G, class VDataFn, class EDataFn>
void write_json(std::ostream& os, const G& g, VDataFn vertex_data_fn, EDataFn edge_data_fn, int indent = 2);
```

**Example:**

```cpp
#include <graph/io/json.hpp>
#include <iostream>

graph::io::write_json(std::cout, my_graph);
```

Output:
```json
{
  "directed": true,
  "nodes": [
    {"id": 0, "label": "A"},
    {"id": 1, "label": "B"}
  ],
  "edges": [
    {"source": 0, "target": 1, "label": "3.14"}
  ]
}
```

### Reading

```cpp
auto result = graph::io::read_json(input_stream);
// result.directed — true/false
// result.nodes    — vector of {id, attrs: map<string, string>}
// result.edges    — vector of {source, target, attrs: map<string, string>}
```

---

## DIMACS

The DIMACS family is line-oriented; each line starts with a kind character: `c` (comment), `p` (problem), `n` (node descriptor), `a` (arc), `e` (edge). Vertex ids in the file are **1-indexed**; the reader normalizes them to **0-indexed** ids.

### Writing

```cpp
// Generic arc list — emits "p <problem> <n> <m>" then "a u v [w]" lines.
template <class G>
void write_dimacs(std::ostream& os, const G& g, std::string_view problem = "sp");

// Max-flow problem — emits "p max", "n <src> s", "n <sink> t", and "a u v <cap>".
template <class G, class CapacityFn>
void write_dimacs_max_flow(std::ostream& os, const G& g,
                           vertex_id_t<G> source, vertex_id_t<G> sink,
                           CapacityFn capacity_fn);
```

**Example:**

```cpp
#include <graph/io/dimacs.hpp>

// Shortest-path style arc list (edge value used as weight when formattable)
graph::io::write_dimacs(std::cout, g);

// Max-flow problem with an explicit capacity accessor
graph::io::write_dimacs_max_flow(std::cout, g, /*source=*/0u, /*sink=*/3u,
  [](const auto& gr, auto uv) { return graph::edge_value(gr, uv); });
```

Output (max-flow):
```
c Generated by graph-v3
p max 4 5
n 1 s
n 4 t
a 1 2 10
...
```

### Reading

```cpp
auto result = graph::io::read_dimacs(input_stream);
// result.problem      — problem type from the "p" line (e.g. "max", "sp", "edge")
// result.num_vertices — declared vertex count
// result.num_arcs     — declared arc/edge count
// result.nodes        — vector of {id (0-indexed), designation}  e.g. {0, "s"}
// result.edges        — vector of {source, target (0-indexed), weight (text)}
```

---

## METIS

The METIS graph format describes an **undirected** graph used by the METIS/ParMETIS partitioners. A header line `<n> <m> [fmt] [ncon]` is followed by one adjacency line per vertex (1-indexed). `m` counts each undirected edge once. The optional 3-digit `fmt` flag encodes vertex sizes / vertex weights / edge weights (hundreds / tens / ones).

### Writing

```cpp
// Treats the graph as undirected: each edge (u,v) is listed under both u and v.
template <class G>
void write_metis(std::ostream& os, const G& g, bool with_weights = false);
```

**Example:**

```cpp
#include <graph/io/metis.hpp>

graph::io::write_metis(std::cout, g);               // structure only
graph::io::write_metis(std::cout, g, /*weights=*/true); // sets fmt=001
```

Output:
```
% Generated by graph-v3
3 3
2 3
1 3
1 2
```

### Reading

```cpp
auto result = graph::io::read_metis(input_stream);
// result.num_vertices   — declared vertex count
// result.num_edges      — declared (undirected) edge count
// result.fmt            — format flag (0 if absent)
// result.ncon           — number of vertex weights (0 if absent)
// result.adjacency      — adjacency[i] = vector of {neighbor (0-indexed), weight}
// result.vertex_weights — vertex_weights[i] = vector of vertex weights
```

---

## Adjacency List Text

A plain whitespace-delimited structural dump in the spirit of BGL's `operator<<` / `operator>>`. **It is not CSV.** One line per vertex: the vertex id, a `:` separator, then its out-neighbours separated by spaces. It carries structure only (no values), and vertices with no out-edges still produce a line so the vertex set round-trips.

### Writing

```cpp
template <class G>
void write_adjacency_list_text(std::ostream& os, const G& g);
```

**Example:**

```cpp
#include <graph/io/adjacency_list_text.hpp>

graph::io::write_adjacency_list_text(std::cout, g);
```

Output:
```
0: 1 2
1: 2
2: 0 3
3:
```

### Reading

```cpp
auto result = graph::io::read_adjacency_list_text(input_stream);
// result.vertex_ids — vector of vertex ID strings in declaration order
// result.edges      — vector of {source, target}
```

The reader also accepts lines with a whitespace-only separator (`0 1 2`), treating the first token as the source vertex.

---

## Design Philosophy

**`std::format`-based auto-detection.** If your vertex or edge value type has a `std::formatter` specialization, the writers automatically serialize it as a label — zero configuration needed.

**No external dependencies.** The GraphML, JSON, DIMACS, METIS, and adjacency-list-text parsers are self-contained lightweight implementations sufficient for graph interchange. They handle the core subset of each format without requiring libxml2, nlohmann/json, or Boost.

**Separation of concerns.** Writers are generic (work with any graph satisfying adjacency list concepts). Readers return simple POD-like structures that you can post-process into any graph type.

| Aspect | BGL Approach | graph-v3 Approach |
|--------|-------------|-------------------|
| Value → string | `dynamic_properties` + per-property converters | `std::format` via `std::formatter<T>` |
| Customization | Verbose `dp.property(...)` for each field | Single callable returning attributes |
| Zero-config | No — must register properties | Yes — auto-formats if formattable |
| Dependencies | Boost.Spirit (DOT), Boost.PropertyTree (GraphML) | None — self-contained |

---

> [← Back to Documentation Index](../index.md)
