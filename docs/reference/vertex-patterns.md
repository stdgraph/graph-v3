<table><tr>
<td><img src="../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Vertex Patterns Reference


> This page documents two complementary concept families for vertex types:
**inner value patterns** (how vertex data is accessed) and
**storage patterns** (how vertex IDs are stored/extracted). Both families live
in `graph::adj_list` and are re-exported via `<graph/graph.hpp>`.

</td>
</tr></table>

> [← Back to Documentation Index](../index.md) · [User Guide: Adjacency Lists](../user-guide/adjacency-lists.md)

---

## Foundation Concepts

These building-block concepts determine whether a type is "pair-like" and
are used internally by both the inner value and storage pattern concepts.

| Concept | Definition |
|---------|------------|
| `pair_like<T>` | Has `std::tuple_size<T>::value >= 2` and supports `std::get<0>`, `std::get<1>` (tuple protocol) |
| `has_first_second<T>` | Has `.first` and `.second` members |
| `pair_like_value<T>` | `pair_like<T> \|\| has_first_second<T>` — disjunction of both |

```cpp
// Used to constrain keyed_vertex_type, pair_value_vertex_pattern, etc.
template <typename T>
concept pair_like_value = pair_like<T> || has_first_second<T>;
```

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
| `pair_value_vertex_pattern<Iter>` | Forward, pair-like value type | `.second` — the mapped value | `std::map`, `std::unordered_map` |
| `whole_value_vertex_pattern<Iter>` | Forward, non-pair value type | `*iter` — the whole dereferenced value | Custom forward containers |

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
| `keyed_vertex_type<Iter>` | Forward, pair-like value | Key type (`.first`) | `(*iter).first` for ID, `.second` for data |

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
| `vertex_pattern_type<Iter>` | Struct with `::value` of type `vertex_pattern` |
| `vertex_pattern_type_v<Iter>` | Variable template returning the enum value |
| `vertex_id_type<Iter>` | Struct with `::type` alias; specializations for direct (`.first`/`.second`) and keyed (tuple protocol) |
| `vertex_id_type_t<Iter>` | Alias for `vertex_id_type<Iter>::type`: `size_t` for direct, key type for keyed |

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
constexpr decltype(auto) vertex_id() const noexcept {
    if constexpr (std::random_access_iterator<VertexIter>) {
        return storage_;  // direct: return index (size_t)
    } else {
        return std::get<0>(*storage_);  // keyed: return const& to key
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

## `vertex_descriptor<VertexIter>` Class Reference

Defined in `<graph/adj_list/vertex_descriptor.hpp>`.
A lightweight, type-safe handle to a vertex stored in a container.
Constrained by `vertex_iterator<VertexIter>`.

### Type Aliases

| Alias | Definition |
|-------|------------|
| `iterator_type` | `VertexIter` |
| `value_type` | `typename std::iterator_traits<VertexIter>::value_type` |
| `storage_type` | `std::size_t` for random-access iterators, `VertexIter` otherwise |

### Constructors

| Signature | Notes |
|-----------|-------|
| `constexpr vertex_descriptor() noexcept` | Default; requires `std::default_initializable<storage_type>` |
| `constexpr explicit vertex_descriptor(storage_type val) noexcept` | Construct from index or iterator |

### Member Functions

| Function | Returns | Description |
|----------|---------|-------------|
| `value()` | `storage_type` | The raw storage: index (`size_t`) or iterator |
| `vertex_id()` | `decltype(auto)` | Vertex ID — index by value for direct, `const&` to key for keyed |
| `underlying_value(Container&)` | `decltype(auto)` | Full container element (`container[i]` or `*iter`); const overload provided |
| `inner_value(Container&)` | `decltype(auto)` | Vertex data excluding the key (`.second` for maps, whole value for vectors); const overload provided |
| `operator++()` / `operator++(int)` | `vertex_descriptor&` / `vertex_descriptor` | Pre/post-increment |
| `operator<=>` / `operator==` | | Defaulted three-way and equality comparison |

### `std::hash` Specialization

A `std::hash<vertex_descriptor<VertexIter>>` specialization is provided,
hashing the index for direct storage or the `vertex_id()` for keyed storage.
This enables use in `std::unordered_set` and `std::unordered_map`.

---

## `vertex_descriptor_view<VertexIter>` Class Reference

Defined in `<graph/adj_list/vertex_descriptor_view.hpp>`.
A forward-only view over vertex storage that synthesizes `vertex_descriptor`
objects on-the-fly. Inherits from `std::ranges::view_interface`.

### Type Aliases

| Alias | Definition |
|-------|------------|
| `vertex_desc` | `vertex_descriptor<VertexIter>` |
| `storage_type` | `typename vertex_desc::storage_type` |

### Constructors

| Signature | Notes |
|-----------|-------|
| `constexpr vertex_descriptor_view() noexcept` | Default |
| `constexpr vertex_descriptor_view(storage_type begin, storage_type end) noexcept` | From index/iterator range; requires random-access |
| `constexpr explicit vertex_descriptor_view(Container&) noexcept` | From mutable container; requires `sized_range` or random-access |
| `constexpr explicit vertex_descriptor_view(const Container&) noexcept` | From const container |

### Member Functions

| Function | Description |
|----------|-------------|
| `begin()` / `end()` | Forward iterators yielding `vertex_descriptor` by value |
| `cbegin()` / `cend()` | Const equivalents |
| `size()` | O(1) — cached at construction |

### Inner `iterator` Class

| Property | Value |
|----------|-------|
| `iterator_category` | `std::forward_iterator_tag` |
| `value_type` | `vertex_descriptor<VertexIter>` |
| Dereference | Returns descriptor **by value** (synthesized) |

### Deduction Guides

```cpp
vertex_descriptor_view(Container&)       -> vertex_descriptor_view<typename Container::iterator>;
vertex_descriptor_view(const Container&) -> vertex_descriptor_view<typename Container::const_iterator>;
```

### Range Traits

`std::ranges::enable_borrowed_range` is specialized to `true` — the view
does not own the underlying data.

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
