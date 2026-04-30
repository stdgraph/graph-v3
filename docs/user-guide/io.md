<table><tr>
<td><img src="../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Graph I/O

> Read and write graphs in DOT, GraphML, and JSON formats.

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
- [Design Philosophy](#design-philosophy)

---

## Overview

The `graph::io` namespace provides readers and writers for three graph interchange formats. The writers work with any graph satisfying the adjacency list concepts; the readers return lightweight parsed structures.

| Format | Writer | Reader | Use Case |
|--------|--------|--------|----------|
| **DOT** | `write_dot()` | `read_dot()` | Visualization (GraphViz), debugging |
| **GraphML** | `write_graphml()` | `read_graphml()` | XML-based interchange, tool ecosystems |
| **JSON** | `write_json()` | `read_json()` | Web applications, REST APIs, modern tooling |

---

## Headers

```cpp
#include <graph/io.hpp>        // umbrella — includes all three formats

// Or include individually:
#include <graph/io/dot.hpp>
#include <graph/io/graphml.hpp>
#include <graph/io/json.hpp>
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

## Design Philosophy

**`std::format`-based auto-detection.** If your vertex or edge value type has a `std::formatter` specialization, the writers automatically serialize it as a label — zero configuration needed.

**No external dependencies.** The GraphML and JSON parsers are self-contained lightweight implementations sufficient for graph interchange. They handle the core subset of each format without requiring libxml2 or nlohmann/json.

**Separation of concerns.** Writers are generic (work with any graph satisfying adjacency list concepts). Readers return simple POD-like structures that you can post-process into any graph type.

| Aspect | BGL Approach | graph-v3 Approach |
|--------|-------------|-------------------|
| Value → string | `dynamic_properties` + per-property converters | `std::format` via `std::formatter<T>` |
| Customization | Verbose `dp.property(...)` for each field | Single callable returning attributes |
| Zero-config | No — must register properties | Yes — auto-formats if formattable |
| Dependencies | Boost.Spirit (DOT), Boost.PropertyTree (GraphML) | None — self-contained |

---

> [← Back to Documentation Index](../index.md)
