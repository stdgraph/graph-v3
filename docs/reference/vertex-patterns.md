<table><tr>
<td><img src="../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Vertex Patterns Reference


> This page documents two complementary concept families for vertex types:
**inner value patterns** (how vertex data is accessed) and
**storage patterns** (how vertex IDs are stored/extracted). Both families live
in `graph::adj_list::detail` and are re-exported via `<graph/graph.hpp>`.

</td>
</tr></table>

> [← Back to Documentation Index](../index.md) · [User Guide: Adjacency Lists](../user-guide/adjacency-lists.md)

---

## Inner Value Patterns

The `inner_value()` method on a vertex descriptor returns the vertex data
(excluding the vertex ID/key). Its behavior depends on the underlying
container type.

> These patterns enable the **default implementation** of `vertices(g)`. When a
> graph container matches one of these patterns, `vertices(g)` can automatically
> return a `vertex_descriptor_view(g)` without requiring a custom `vertices()`
> member or ADL override.

### Pattern Concepts

| Concept | Iterator Requirements | `inner_value()` Returns | Typical Containers |
|---------|----------------------|------------------------|--------------------|
| `random_access_vertex_pattern<Iter>` | Random-access iterator | `container[index]` — the whole element | `std::vector`, `std::deque` |
| `pair_value_vertex_pattern<Iter>` | Bidirectional, pair-like value type | `.second` — the mapped value | `std::map`, `std::unordered_map` |
| `whole_value_vertex_pattern<Iter>` | Bidirectional, non-pair value type | `*iter` — the whole dereferenced value | Custom bidirectional containers |

```cpp
// Disjunction of all three
template <typename Iter>
concept has_inner_value_pattern =
    random_access_vertex_pattern<Iter> ||
    pair_value_vertex_pattern<Iter>    ||
    whole_value_vertex_pattern<Iter>;
```

### Pattern Detection Traits

| Trait / Template | Purpose |
|------------------|---------|
| `vertex_inner_value_pattern<Iter>` | Struct with `::is_random_access`, `::is_pair_value`, `::is_whole_value` booleans |
| `vertex_inner_value_pattern_v<Iter>` | Variable template shortcut |
| `vertex_inner_pattern` (enum) | `random_access`, `pair_value`, `whole_value` |
| `vertex_inner_pattern_type<Iter>` | Struct with `::value` of type `vertex_inner_pattern` |
| `vertex_inner_pattern_type_v<Iter>` | Variable template returning the enum value |

### Examples

```cpp
#include <graph/graph.hpp>
#include <vector>
#include <map>

using VecIter = std::vector<int>::iterator;
using MapIter = std::map<int, double>::iterator;

// Concept checks
static_assert(random_access_vertex_pattern<VecIter>);
static_assert(pair_value_vertex_pattern<MapIter>);

// Enum-based detection
static_assert(vertex_inner_pattern_type_v<VecIter> ==
              vertex_inner_pattern::random_access);
static_assert(vertex_inner_pattern_type_v<MapIter> ==
              vertex_inner_pattern::pair_value);
```

### Integration with `vertices(g)` CPO

The `vertices(g)` CPO uses the following resolution order:

1. `g.vertices()` member
2. ADL `vertices(g)`
3. **Default:** If `g` is a forward range whose iterators satisfy
   `has_inner_value_pattern`, return `vertex_descriptor_view(g)`

This means most standard containers (vectors, maps) work out-of-the-box:

```cpp
std::vector<std::vector<int>> adj_list;
auto verts = vertices(adj_list);  // works automatically — random_access pattern
```

---

## Vertex Storage Patterns

The library defines two mutually exclusive concepts for how vertex IDs are
stored and extracted:

### Storage Concepts

| Concept | Iterator Requirements | Vertex ID Type | Access Pattern |
|---------|----------------------|---------------|----------------|
| `direct_vertex_type<Iter>` | Random-access | `std::size_t` (the index) | `container[index]` |
| `keyed_vertex_type<Iter>` | Bidirectional, pair-like value | Key type (`.first`) | `(*iter).first` for ID, `.second` for data |

```cpp
// Disjunction
template <typename Iter>
concept vertex_iterator =
    direct_vertex_type<Iter> || keyed_vertex_type<Iter>;
```

The two patterns are **mutually exclusive** — every valid vertex iterator
matches exactly one.

### Storage Detection Traits

| Trait / Template | Purpose |
|------------------|---------|
| `vertex_storage_pattern<Iter>` | Struct with `::is_direct`, `::is_keyed` booleans |
| `vertex_storage_pattern_v<Iter>` | Variable template shortcut |
| `vertex_pattern` (enum) | `direct`, `keyed` |
| `vertex_pattern_type_v<Iter>` | Variable template returning the enum value |
| `vertex_id_type_t<Iter>` | Extracts the vertex ID type: `size_t` for direct, key type for keyed |

### Examples

```cpp
using VecIter = std::vector<int>::iterator;
using MapIter = std::map<std::string, double>::iterator;

// Concept checks
static_assert(direct_vertex_type<VecIter>);
static_assert(keyed_vertex_type<MapIter>);

// ID type extraction
static_assert(std::same_as<vertex_id_type_t<VecIter>, std::size_t>);
static_assert(std::same_as<vertex_id_type_t<MapIter>, std::string>);
```

### Integration with `vertex_id()` CPO

The `vertex_id()` function in `vertex_descriptor` dispatches at compile time:

```cpp
constexpr auto vertex_id() const noexcept {
    if constexpr (std::random_access_iterator<VertexIter>) {
        return storage_;  // direct: return index (size_t)
    } else {
        return std::get<0>(*storage_);  // keyed: extract key
    }
}
```

---

## How the Two Families Relate

| Aspect | Inner Value Patterns | Storage Patterns |
|--------|---------------------|-----------------|
| **What it classifies** | How vertex *data* is accessed | How vertex *IDs* are extracted |
| **Number of patterns** | 3 | 2 |
| **Used by** | `vertices(g)` default, `vertex_descriptor_view` | `vertex_id()`, `find_vertex()` |
| **Examples** | `random_access_vertex_pattern` ↔ vector data | `direct_vertex_type` ↔ index-based ID |

Both families work together to give `vertex_descriptor` complete type safety:
- **Storage pattern** → determines how to get the vertex ID
- **Inner value pattern** → determines how to get the vertex data

---

## Compile-Time Dispatch Pattern

```cpp
template <typename VertexIter>
void process_vertices(/* ... */) {
    static_assert(vertex_iterator<VertexIter>, "Invalid vertex iterator");

    if constexpr (direct_vertex_type<VertexIter>) {
        // Optimized: index arithmetic, O(1) vertex lookup
    } else if constexpr (keyed_vertex_type<VertexIter>) {
        // Map-based: iterator dereferencing, flexible key types
    }
}
```

---

## See Also

- [User Guide: Adjacency Lists](../user-guide/adjacency-lists.md) — tutorial-style guide
- [Edge Value Concepts](edge-value-concepts.md) — edge type pattern detection
- [Adjacency List Interface](adjacency-list-interface.md) — full GCI spec
- [Concepts Reference](concepts.md) — all concepts in one place
