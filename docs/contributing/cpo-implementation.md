# CPO Implementation Guide

> How Customization Point Objects work in graph-v3, how to implement new ones, and how to test them.

**References:**
- [Adjacency List Interface](../reference/adjacency-list-interface.md) — GCI spec for adjacency lists
- [Edge List Interface](../reference/edge-list-interface.md) — GCI spec for edge lists
- [CPO Reference](../reference/cpo-reference.md) — CPO signatures and behavior
- [CPO Order](cpo-order.md) — canonical implementation order and dependencies

---

## Part 1 — Understanding CPOs

### What Is a CPO?

A Customization Point Object (CPO) — sometimes called a *Niebloid* — is an `inline constexpr` function object that provides a stable API while allowing per-type customization. Every graph operation in graph-v3 is a CPO:

```cpp
auto verts = graph::vertices(g);       // Any graph type
auto tid   = graph::target_id(g, uv);  // Selects best implementation at compile time
```

Key characteristics:

1. **Niebloid** — the function object inhibits ADL at the call site, so `graph::vertices(g)` always routes through the CPO even if an unrelated `vertices` function is visible.
2. **Hierarchical dispatch** — checks member functions before ADL before built-in defaults.
3. **Constexpr-friendly** — supports compile-time evaluation.
4. **Zero-cost** — `if constexpr` dispatch compiles away; no virtual calls, no branches at runtime.
5. **Cross-compiler** — the MSVC `_Choice_t` pattern works identically on MSVC, GCC, and Clang.

### Customization Priority Order

Every CPO checks for customizations in this order:

| Priority | Mechanism | Example |
|----------|-----------|---------|
| 1 (highest) | Member function | `g.vertices()` |
| 2 | ADL free function | `vertices(g)` found via ADL |
| 3 (lowest) | Default implementation | Built-in fallback (e.g., iterate the container directly) |

For CPOs that operate through **descriptors** (vertex or edge), there is an extended *inner-value priority* pattern — see [Part 2](#inner-value-priority-pattern).

### The MSVC `_Choice_t` Pattern

graph-v3 uses the same CPO pattern as the MSVC Standard Library. The pattern has four components:

1. **Strategy enum (`_St`)** — enumerates every possible dispatch path.
2. **`_Choice_t<_Ty>` struct** — a pair of `{strategy, noexcept_flag}`, computed once per type.
3. **`_Choose<T>()` consteval function** — evaluates the best path at compile time.
4. **Single `operator()`** — uses `if constexpr` chain to dispatch. No overloads, no negation chains.

```cpp
namespace graph::_cpo {

// ── Shared across all CPOs (defined once in cpo_common.hpp) ──
template<typename _Ty>
struct _Choice_t {
    _Ty  _Strategy = _Ty{};
    bool _No_throw = false;
};

// ── Per-CPO implementation ──
namespace _my_operation {
    // 1. Strategy enum
    enum class _St { _none, _member, _adl, _default };

    // 2. Concepts for each dispatch path
    template<typename T>
    concept _has_member = requires(T&& t) {
        { std::forward<T>(t).my_operation() } -> std::convertible_to<int>;
    };

    template<typename T>
    concept _has_adl = requires(T&& t) {
        { my_operation(std::forward<T>(t)) } -> std::convertible_to<int>;
    };

    // 3. Single compile-time evaluation
    template<typename T>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_member<T>) {
            return {_St::_member, noexcept(std::declval<T>().my_operation())};
        } else if constexpr (_has_adl<T>) {
            return {_St::_adl, noexcept(my_operation(std::declval<T>()))};
        } else {
            return {_St::_default, true};
        }
    }

    // 4. CPO class — single operator() with if constexpr
    class _fn {
    private:
        template<typename T>
        static constexpr _Choice_t<_St> _Choice = _Choose<T>();

    public:
        template<typename T>
        [[nodiscard]] constexpr auto operator()(T&& t) const
            noexcept(_Choice<T>._No_throw)
            -> decltype(auto)
        {
            if constexpr (_Choice<T>._Strategy == _St::_member) {
                return std::forward<T>(t).my_operation();
            } else if constexpr (_Choice<T>._Strategy == _St::_adl) {
                return my_operation(std::forward<T>(t));
            } else {
                return 0;  // default
            }
        }
    };
} // namespace _my_operation

} // namespace graph::_cpo

// ── Public CPO object ──
inline namespace _cpos {
    inline constexpr _cpo::_my_operation::_fn my_operation{};
}
```

### Why `if constexpr` Over Multiple Overloads

The `_Choice_t` dispatch pattern is **strictly preferred** over the alternative of declaring separate `operator()` overloads with negation chains:

| `if constexpr` (recommended) | Multiple overloads (avoid) |
|-------------------------------|---------------------------|
| Priority order is clear from top-to-bottom reading | Each overload needs `(!_has_path1<E> && !_has_path2<E> && ...)` |
| Single `noexcept` spec via `_Choice<T>._No_throw` | `noexcept(...)` repeated in every overload |
| Adding a path: add one `else if constexpr` | Adding a path: update **all** lower-priority overloads' constraints |
| Single function signature → better error messages | Risk of constraint overlap causing ambiguity |

### Namespace Layout

Every CPO follows this namespace structure:

```
graph::
├── _cpo::                     # Implementation details (private)
│   ├── _Choice_t<_Ty>        # Shared struct
│   └── _my_operation::       # Per-CPO namespace
│       ├── _St (enum)
│       ├── concepts
│       ├── _Choose<T>()
│       └── _fn (class)
└── _cpos:: (inline)           # Public interface
    └── my_operation (_fn instance)
```

Users call `graph::my_operation(...)`. The `inline namespace _cpos` makes the CPO object visible in `graph::` without polluting it with implementation details.

---

## Part 2 — Implementing a CPO

### Step-by-Step Checklist

1. Choose a name and decide the signature(s).
2. Define the strategy enum `_St` with paths `_none`, `_member`, `_adl`, and optionally `_default`.
3. Write one concept per dispatch path, constraining the return type.
4. Write `_Choose<T>()` — a `consteval` function returning `_Choice_t<_St>`.
5. Write `_fn::operator()()` — a single template using `if constexpr`.
6. Create the public CPO object: `inline constexpr _fn my_cpo{};`.
7. Define any associated type aliases (e.g., `vertex_range_t<G>`).
8. Write tests for every dispatch path, plus noexcept propagation and concept satisfaction.

### File Header Template

```cpp
/**
 * @file graph_cpo.hpp
 * @brief Graph Container Interface Customization Point Objects
 *
 * Implements the P1709 Graph Container Interface using MSVC-style CPOs.
 * Each CPO follows the _Choice_t pattern for cross-compiler dispatch.
 *
 * @note All CPOs check inner_value before descriptor for element-based operations.
 * @see cpo_common.hpp for _Choice_t definition
 */
#pragma once

#include <graph/detail/cpo_common.hpp>
#include <graph/adj_list/vertex_descriptor.hpp>
#include <graph/adj_list/edge_descriptor.hpp>
#include <ranges>
#include <concepts>
```

### Inner Value Priority Pattern

For CPOs that operate on descriptors (vertices or edges), the dispatch order is *extended* to check the **inner value** — the actual element stored in the container — before the descriptor wrapper:

| Priority | What is checked | Example (`vertex_id(g, u)`) |
|----------|-----------------|----------------------------|
| 1 (highest) | Inner value member function | `u.inner_value(g).vertex_id(g)` |
| 2 | ADL with inner value | `vertex_id(g, u.inner_value(g))` |
| 3 | ADL with descriptor | `vertex_id(g, u)` |
| 4 (lowest) | Descriptor default | `u.vertex_id()` (index-based fallback) |

**Rationale:** The descriptor is a lightweight iterator/index wrapper. The actual graph element should define its own behavior; the descriptor serves only as a fallback.

```cpp
// Inner value access:
//   u.inner_value(g)    →  for vectors: g[u.value()],  for maps: (*iter).second
//   uv.inner_value(g)   →  the actual edge data (int, pair, tuple, or struct)
```

#### Worked Example: `vertex_id(g, u)` with Inner Value Priority

```cpp
namespace graph::_cpo {
namespace _vertex_id {
    enum class _St { _none, _inner_member, _adl_inner, _adl_desc, _desc_default };

    // Priority 1: inner value has .vertex_id(g) member
    template<typename G, typename U>
    concept _has_inner_member = requires(G& g, U&& u) {
        { u.inner_value(g).vertex_id(g) } -> std::integral;
    };

    // Priority 2: ADL vertex_id(g, inner_value)
    template<typename G, typename U>
    concept _has_adl_inner = requires(G& g, U&& u) {
        { vertex_id(g, u.inner_value(g)) } -> std::integral;
    };

    // Priority 3: ADL vertex_id(g, descriptor)
    template<typename G, typename U>
    concept _has_adl_desc = requires(G& g, U&& u) {
        { vertex_id(g, u) } -> std::integral;
    };

    // Priority 4: descriptor default
    template<typename U>
    concept _has_desc_default = requires(U&& u) {
        { u.vertex_id() } -> std::integral;
    };

    template<typename G, typename U>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_inner_member<G, U>) {
            return {_St::_inner_member,
                    noexcept(std::declval<U>().inner_value(std::declval<G&>())
                                 .vertex_id(std::declval<G&>()))};
        } else if constexpr (_has_adl_inner<G, U>) {
            return {_St::_adl_inner,
                    noexcept(vertex_id(std::declval<G&>(),
                                       std::declval<U>().inner_value(std::declval<G&>())))};
        } else if constexpr (_has_adl_desc<G, U>) {
            return {_St::_adl_desc,
                    noexcept(vertex_id(std::declval<G&>(), std::declval<U>()))};
        } else if constexpr (_has_desc_default<U>) {
            return {_St::_desc_default, noexcept(std::declval<U>().vertex_id())};
        } else {
            return {_St::_none, false};
        }
    }

    class _fn {
    private:
        template<typename G, typename U>
        static constexpr _Choice_t<_St> _Choice = _Choose<G, U>();

    public:
        template<typename G, typename U>
        [[nodiscard]] constexpr auto operator()(G&& g, U&& u) const
            noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<U>>._No_throw)
        {
            using _G = std::remove_cvref_t<G>;
            using _U = std::remove_cvref_t<U>;

            if constexpr (_Choice<_G, _U>._Strategy == _St::_inner_member) {
                return std::forward<U>(u).inner_value(g).vertex_id(g);
            } else if constexpr (_Choice<_G, _U>._Strategy == _St::_adl_inner) {
                return vertex_id(g, std::forward<U>(u).inner_value(g));
            } else if constexpr (_Choice<_G, _U>._Strategy == _St::_adl_desc) {
                return vertex_id(g, std::forward<U>(u));
            } else if constexpr (_Choice<_G, _U>._Strategy == _St::_desc_default) {
                return std::forward<U>(u).vertex_id();
            } else {
                static_assert(_Choice<_G, _U>._Strategy != _St::_none,
                    "vertex_id(g,u) requires inner_value.vertex_id(g), "
                    "ADL vertex_id(), or descriptor .vertex_id()");
            }
        }
    };
} // namespace _vertex_id

inline namespace _cpos {
    inline constexpr _vertex_id::_fn vertex_id{};
}
} // namespace graph::_cpo
```

### Simple CPO Example: `vertices(g)`

A graph-level CPO that does *not* involve descriptors — only 3 paths:

```cpp
namespace graph::_cpo {
namespace _vertices {
    enum class _St { _none, _member, _adl, _default };

    template<typename G>
    concept _has_member = requires(G& g) {
        { g.vertices() } -> std::ranges::forward_range;
    };

    template<typename G>
    concept _has_adl = requires(G& g) {
        { vertices(g) } -> std::ranges::forward_range;
    };

    template<typename G>
    concept _has_default = std::ranges::forward_range<std::remove_cvref_t<G>>;

    template<typename G>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_member<G>) {
            return {_St::_member, noexcept(std::declval<G&>().vertices())};
        } else if constexpr (_has_adl<G>) {
            return {_St::_adl, noexcept(vertices(std::declval<G&>()))};
        } else if constexpr (_has_default<G>) {
            return {_St::_default, true};
        } else {
            return {_St::_none, false};
        }
    }

    class _fn {
    private:
        template<typename G>
        static constexpr _Choice_t<_St> _Choice = _Choose<G>();

    public:
        template<typename G>
        [[nodiscard]] constexpr auto operator()(G&& g) const
            noexcept(_Choice<std::remove_cvref_t<G>>._No_throw)
        {
            using _G = std::remove_cvref_t<G>;
            if constexpr (_Choice<_G>._Strategy == _St::_member) {
                return std::forward<G>(g).vertices();
            } else if constexpr (_Choice<_G>._Strategy == _St::_adl) {
                return vertices(std::forward<G>(g));
            } else if constexpr (_Choice<_G>._Strategy == _St::_default) {
                // Wrap in vertex_descriptor_view for uniform interface
                return vertex_descriptor_view(std::forward<G>(g));
            } else {
                static_assert(_Choice<_G>._Strategy != _St::_none,
                    "vertices(g) requires .vertices() member, ADL vertices(), "
                    "or a forward_range");
            }
        }
    };
} // namespace _vertices

inline namespace _cpos {
    inline constexpr _vertices::_fn vertices{};
}
} // namespace graph::_cpo
```

### Multi-Argument CPO: `degree(g, u)`

Demonstrates a two-parameter CPO with a computed default:

```cpp
namespace _degree {
    enum class _St { _none, _member, _adl, _default };

    template<typename G, typename U>
    concept _has_member = requires(const G& g, const U& u) {
        { g.degree(u) } -> std::integral;
    };

    template<typename G, typename U>
    concept _has_adl = requires(const G& g, const U& u) {
        { degree(g, u) } -> std::integral;
    };

    template<typename G, typename U>
    concept _has_default = requires(G& g, U&& u) {
        { std::ranges::size(edges(g, std::forward<U>(u))) } -> std::integral;
    };

    template<typename G, typename U>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_member<G, U>) {
            return {_St::_member,
                    noexcept(std::declval<const G&>().degree(std::declval<const U&>()))};
        } else if constexpr (_has_adl<G, U>) {
            return {_St::_adl,
                    noexcept(degree(std::declval<const G&>(), std::declval<const U&>()))};
        } else if constexpr (_has_default<G, U>) {
            return {_St::_default,
                    noexcept(std::ranges::size(
                        edges(std::declval<G&>(), std::declval<U>())))};
        } else {
            return {_St::_none, false};
        }
    }

    class _fn {
    private:
        template<typename G, typename U>
        static constexpr _Choice_t<_St> _Choice = _Choose<G, U>();

    public:
        template<typename G, typename U>
        [[nodiscard]] constexpr auto operator()(const G& g, const U& u) const
            noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<U>>._No_throw)
        {
            using _G = std::remove_cvref_t<G>;
            using _U = std::remove_cvref_t<U>;

            if constexpr (_Choice<_G, _U>._Strategy == _St::_member) {
                return g.degree(u);
            } else if constexpr (_Choice<_G, _U>._Strategy == _St::_adl) {
                return degree(g, u);
            } else if constexpr (_Choice<_G, _U>._Strategy == _St::_default) {
                return std::ranges::size(edges(g, u));
            } else {
                static_assert(_Choice<_G, _U>._Strategy != _St::_none,
                    "degree(g,u) requires .degree() member, ADL degree(), "
                    "or sized edges(g,u)");
            }
        }
    };
} // namespace _degree
```

### CPO with 4+ Dispatch Paths: `edge_weight`

Sometimes more than three paths are needed. The pattern scales cleanly:

```cpp
namespace _edge_weight {
    enum class _St { _none, _member_fn, _member_var, _adl, _tuple };

    template<typename E>
    concept _has_weight_member = requires(const E& e) {
        { e.weight() } -> std::convertible_to<double>;
    };

    template<typename E>
    concept _has_weight_variable = requires(const E& e) {
        { e.weight } -> std::convertible_to<double>;
    };

    template<typename E>
    concept _has_weight_adl = requires(const E& e) {
        { weight(e) } -> std::convertible_to<double>;
    };

    template<typename E>
    concept _has_tuple_weight = requires(const E& e) {
        requires std::tuple_size<std::remove_cvref_t<E>>::value >= 2;
        { std::get<1>(e) } -> std::convertible_to<double>;
    };

    template<typename E>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_weight_member<E>) {
            return {_St::_member_fn, noexcept(std::declval<const E&>().weight())};
        } else if constexpr (_has_weight_variable<E>) {
            return {_St::_member_var, noexcept(std::declval<const E&>().weight)};
        } else if constexpr (_has_weight_adl<E>) {
            return {_St::_adl, noexcept(weight(std::declval<const E&>()))};
        } else if constexpr (_has_tuple_weight<E>) {
            return {_St::_tuple, noexcept(std::get<1>(std::declval<const E&>()))};
        } else {
            return {_St::_none, true};  // default: unit weight
        }
    }

    class _fn {
    private:
        template<typename E>
        static constexpr _Choice_t<_St> _Choice = _Choose<E>();

    public:
        template<typename E>
        [[nodiscard]] constexpr auto operator()(const E& e) const
            noexcept(_Choice<E>._No_throw)
            -> double
        {
            if constexpr (_Choice<E>._Strategy == _St::_member_fn) {
                return e.weight();
            } else if constexpr (_Choice<E>._Strategy == _St::_member_var) {
                return e.weight;
            } else if constexpr (_Choice<E>._Strategy == _St::_adl) {
                return weight(e);
            } else if constexpr (_Choice<E>._Strategy == _St::_tuple) {
                return std::get<1>(e);
            } else {
                return 1.0;  // default unit weight
            }
        }
    };
} // namespace _edge_weight
```

### Multi-Signature CPO: `find_vertex`

Some CPOs have genuinely different *signatures* (not just different dispatch paths for the same signature). Use separate `_Choose` / `_Choice` per signature:

```cpp
namespace _find_vertex {
    // ── Signature 1: find_vertex(g, id) ────────────────────
    enum class _St { _none, _member, _adl, _default };

    template<typename G, typename Id>
    concept _has_member = requires(G& g, const Id& id) {
        { g.find_vertex(id) };
    };
    template<typename G, typename Id>
    concept _has_adl = requires(G& g, const Id& id) {
        { find_vertex(g, id) };
    };

    template<typename G, typename Id>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept { /* ... */ }

    class _fn {
    private:
        template<typename G, typename Id>
        static constexpr _Choice_t<_St> _Choice = _Choose<G, Id>();

    public:
        // Signature 1: (g, id)
        template<typename G, typename Id>
        [[nodiscard]] constexpr auto operator()(G&& g, const Id& id) const
            noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<Id>>._No_throw)
        { /* if constexpr chain */ }

        // Signature 2: (g, key, value) — for compound-key graphs
        template<typename G, typename Key, typename Value>
            requires requires(G& g, const Key& k, const Value& v) {
                { g.find_vertex(k, v) };
            }
        [[nodiscard]] constexpr auto operator()(G&& g, const Key& key,
                                                 const Value& value) const
            noexcept(noexcept(std::forward<G>(g).find_vertex(key, value)))
            -> decltype(std::forward<G>(g).find_vertex(key, value))
        {
            return std::forward<G>(g).find_vertex(key, value);
        }
    };
} // namespace _find_vertex
```

### Complete GCI CPO Implementations

The following CPOs are implemented (or planned) for the Graph Container Interface. Each follows the patterns above. See [CPO Order](cpo-order.md) for the full dependency table.

| CPO | Signature | Return | Inner-value? | Default |
|-----|-----------|--------|-------------|---------|
| `vertices(g)` | `(G&)` | `vertex_descriptor_view` | no | wrap container as range |
| `edges(g, u)` | `(G&, vertex_t)` | `edge_descriptor_view` | no | wrap inner range |
| `vertex_id(g, u)` | `(G&, vertex_t)` | `vertex_id_t<G>` | **yes** (4-tier) | descriptor index |
| `target_id(g, uv)` | `(G&, edge_t)` | `vertex_id_t<G>` | **yes** | pattern extraction (integral/pair/tuple) |
| `source_id(g, uv)` | `(G&, edge_t)` | `vertex_id_t<G>` | **yes** | descriptor source |
| `find_vertex(g, uid)` | `(G&, vertex_id_t)` | `vertex_iterator_t<G>` | no | `begin(vertices(g)) + uid` |
| `num_vertices(g)` | `(const G&)` | `integral` | no | `ranges::size(vertices(g))` |
| `num_edges(g)` | `(const G&)` | `integral` | no | sum of `distance(edges(g,u))` |
| `degree(g, u)` | `(const G&, vertex_t)` | `integral` | no | `ranges::size(edges(g,u))` |
| `target(g, uv)` | `(G&, edge_t)` | `vertex_t<G>` | no | `*find_vertex(g, target_id(g,uv))` |
| `vertex_value(g, u)` | `(G&, vertex_t)` | user-defined | **yes** | none (optional) |
| `edge_value(g, uv)` | `(G&, edge_t)` | user-defined | **yes** (4-tier) | `uv.inner_value(g)` |

### `target_id` — Pattern Extraction Example

`target_id` is a good example of a CPO with inner-value priority *and* pattern-based fallbacks:

```cpp
namespace _target_id {
    enum class _St {
        _none, _inner_member, _adl_inner, _adl_desc,
        _integral, _pair_first, _tuple_first
    };

    // Priority 1: inner_value(g).target_id(g)
    template<typename G, typename E>
    concept _has_inner_member =
        is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        requires(G& g, const E& uv) {
            { uv.inner_value(g).target_id(g) } -> std::integral;
        };

    // Priority 2: ADL target_id(g, inner_value)
    template<typename G, typename E>
    concept _has_adl_inner =
        is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        requires(G& g, const E& uv) {
            { target_id(g, uv.inner_value(g)) } -> std::integral;
        };

    // Priority 3: ADL target_id(g, descriptor)
    template<typename G, typename E>
    concept _has_adl_desc = requires(G& g, const E& uv) {
        { target_id(g, uv) } -> std::integral;
    };

    // Priority 4a: inner value is integral → it IS the target id
    template<typename G, typename E>
    concept _is_integral =
        is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        std::integral<decltype(std::declval<E>().inner_value(std::declval<G&>()))>;

    // Priority 4b: inner value is a pair → .first is the target id
    template<typename G, typename E>
    concept _is_pair = requires(G& g, const E& uv) {
        { uv.inner_value(g).first } -> std::integral;
    };

    // Priority 4c: inner value is tuple-like → get<0> is the target id
    template<typename G, typename E>
    concept _is_tuple = requires(G& g, const E& uv) {
        requires std::tuple_size<decltype(uv.inner_value(g))>::value >= 1;
        { std::get<0>(uv.inner_value(g)) } -> std::integral;
    };

    // ... _Choose and _fn follow the same pattern as vertex_id
} // namespace _target_id
```

### `edge_value` — Inner-Value 4-Tier Example

`edge_value` demonstrates four inner-value tiers where the final fallback returns the edge data itself:

```cpp
namespace _edge_value {
    enum class _St { _none, _inner_value_member, _adl_inner_value,
                     _adl_descriptor, _inner_value };

    // 1. uv.inner_value(g).edge_value(g)
    template<typename G, typename E>
    concept _has_inner_value_member =
        is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        requires(G& g, const E& uv) {
            { uv.inner_value(g).edge_value(g) };
        };

    // 2. edge_value(g, uv.inner_value(g))  via ADL
    template<typename G, typename E>
    concept _has_adl_inner_value =
        is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        requires(G& g, const E& uv) {
            { edge_value(g, uv.inner_value(g)) };
        };

    // 3. edge_value(g, uv)  via ADL on descriptor
    template<typename G, typename E>
    concept _has_adl_descriptor =
        is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        requires(G& g, const E& uv) {
            { edge_value(g, uv) };
        };

    // 4. uv.inner_value(g)  — return the edge data itself
    template<typename G, typename E>
    concept _has_inner_value =
        is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        requires(G& g, const E& uv) {
            { uv.inner_value(g) };
        };

    // _Choose and _fn follow the standard pattern ...
} // namespace _edge_value
```

---

## Part 3 — Testing CPOs

### Test Every Dispatch Path

Each CPO must have tests that verify every path is selected correctly:

```cpp
TEST_CASE("vertex_id CPO - member function path") {
    struct WithMember {
        int vertex_id() const { return 42; }
    };
    WithMember v;
    REQUIRE(graph::vertex_id(v) == 42);
}

TEST_CASE("vertex_id CPO - ADL path") {
    namespace user {
        struct WithADL { int key; };
        int vertex_id(const WithADL& v) { return v.key; }
    }
    user::WithADL v{99};
    REQUIRE(graph::vertex_id(v) == 99);
}

TEST_CASE("vertex_id CPO - integral pass-through") {
    std::size_t id = 7;
    REQUIRE(graph::vertex_id(id) == 7);
}
```

### Test Noexcept Propagation

```cpp
TEST_CASE("CPO propagates noexcept") {
    struct Throwing {
        int vertex_id() const { return 1; }  // not noexcept
    };
    struct NonThrowing {
        int vertex_id() const noexcept { return 2; }
    };

    STATIC_REQUIRE(!noexcept(graph::vertex_id(Throwing{})));
    STATIC_REQUIRE(noexcept(graph::vertex_id(NonThrowing{})));
}
```

### Test Concept Constraints

Verify that types without any customization fail concept checks:

```cpp
TEST_CASE("CPO properly constrains") {
    struct Invalid {};  // No vertex_id customization

    // Should not compile:
    // auto x = graph::vertex_id(Invalid{});

    STATIC_REQUIRE(!requires(Invalid i) {
        graph::vertex_id(i);
    });
}
```

### Test Mutual Exclusion

Ensure priority order is respected when multiple paths could match:

```cpp
TEST_CASE("CPO priority - member wins over ADL") {
    namespace user {
        struct Both {
            int vertex_id() const { return 1; }      // member
        };
        int vertex_id(const Both&) { return 999; }   // ADL (should lose)
    }
    user::Both v;
    REQUIRE(graph::vertex_id(v) == 1);  // member wins
}
```

### Test with All 23 Adjacency List Types

The GCI defines 23 canonical adjacency list shapes. CPOs should be tested against a representative subset:

| Category | Types | Examples |
|----------|-------|---------|
| 1–5 | Basic patterns | `vector<vector<int>>`, `vector<vector<pair<int,double>>>` |
| 6–11 | Forward/bidirectional iterators | `vector<list<int>>`, `vector<forward_list<int>>` |
| 12–13 | Deque storage | `deque<vector<int>>` |
| 14–17 | Map-based sparse | `map<int, vector<int>>` |
| 18–21 | Hash-based sparse | `unordered_map<int, vector<int>>` |
| 22–23 | Linked storage | `list<vector<int>>` |

### Example: Catch2 Test Structure

```cpp
#include <graph/graph_cpo.hpp>
#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <map>
#include <list>

TEST_CASE("vertices(g) - basic adjacency list", "[graph_cpo][vertices]") {
    using AdjList = std::vector<std::vector<int>>;
    AdjList g = {{1, 2}, {0, 2}, {0, 1}};

    auto verts = graph::vertices(g);
    REQUIRE(std::ranges::size(verts) == 3);
}

TEST_CASE("edges(g,u) - vector of lists", "[graph_cpo][edges]") {
    using AdjList = std::vector<std::list<int>>;
    AdjList g(3);
    g[0] = {1, 2};

    auto edge_range = graph::edges(g, 0);
    REQUIRE(std::ranges::distance(edge_range) == 2);
}

TEST_CASE("target_id(g,uv) - pair pattern", "[graph_cpo][target_id]") {
    using AdjList = std::vector<std::vector<std::pair<int, double>>>;
    AdjList g = {{{1, 1.5}, {2, 2.5}}};

    auto es = graph::edges(g, 0);
    auto it = std::ranges::begin(es);
    REQUIRE(graph::target_id(g, *it) == 1);
}
```

### Cross-Compiler Testing

Use CI to validate on all three major compilers:

```yaml
strategy:
  matrix:
    compiler:
      - { name: gcc,   version: 13, cxx: g++-13 }
      - { name: clang, version: 16, cxx: clang++-16 }
      - { name: msvc,  version: 2022 }
```

---

## Part 4 — Reference

### Cross-Compiler Guidelines

1. **Prefer `if constexpr` over multiple overloads** — the `_Choice_t` pattern avoids negation chains entirely.
2. **Use `std::remove_cvref_t` in concepts** — handles cv-qualifiers and references uniformly across compilers.
3. **Constrain return types explicitly** — `{ expr } -> std::convertible_to<T>` rather than unconstrained `{ expr }`.
4. **Use trailing return types** for complex expressions — `-> decltype(...)` works reliably everywhere.
5. **Prefer standard library concepts** (`std::integral`, `std::ranges::forward_range`) over hand-rolled equivalents.
6. **Provide clear `static_assert` messages** for unsatisfied constraints.
7. **Test constraint evaluation** with `static_assert(requires(...))` and `static_assert(!requires(...))`.
8. **Mark `[[nodiscard]]`** on CPOs that return values.
9. **Mark `constexpr`** on all CPOs — enable compile-time evaluation where possible.
10. **Document compiler workarounds** if any are needed, with `#if defined(_MSC_VER)` guards.
11. **Use `decltype(auto)` for value CPOs** — the three value CPOs (`vertex_value`, `edge_value`, `graph_value`) must return `decltype(auto)` so that the exact return type of the resolved member/ADL/default function is preserved. Return-by-value (`T`), return-by-reference (`T&`), return-by-const-reference (`const T&`), and return-by-rvalue-reference (`T&&`) must all be forwarded faithfully without decay or copy.

### `_Fake_copy_init` Pattern (Advanced)

The standard library uses `_Fake_copy_init` to distinguish implicit from explicit conversions in concept checks. This is rarely needed in graph-v3 — `std::convertible_to` suffices for most CPOs.

**When to use:** Only when you specifically need implicit-only conversion semantics (matching `std::ranges::begin` behavior).

```cpp
namespace graph::_cpo::_detail {
    void _fake_copy_init(auto);  // Declared, never defined
}

template<typename VD>
concept _has_implicit_id = requires(const VD& vd) {
    _detail::_fake_copy_init<std::size_t>(vd.vertex_id());  // implicit only
    { vd.vertex_id() } -> std::convertible_to<std::size_t>;
};
```

**Recommendation:** Use `std::convertible_to` by default. Document this pattern for completeness, but reach for it only when necessary.

### Concept Subsumption (Advanced)

For CPOs with refinement hierarchies, concept subsumption can clarify overload resolution:

```cpp
template<typename T>
concept _has_operation = requires(T t) { t.operation(); };

// Refined concept subsumes base
template<typename T>
concept _has_noexcept_operation = _has_operation<T> && requires(T t) {
    { t.operation() } noexcept;
};
```

This is overkill for most CPOs. Use it only when you have legitimate refinement hierarchies.

### Common Pitfalls

| Pitfall | Wrong | Right |
|---------|-------|-------|
| ADL bypass | `using namespace graph; vertex_id(vd);` | `graph::vertex_id(vd);` |
| Missing const | `operator()(T& t)` | `operator()(const T& t)` |
| Unconstrained return | `concept C = requires(T t) { t.op(); }` | `concept C = requires(T t) { { t.op() } -> std::convertible_to<int>; }` |
| Forgetting `remove_cvref_t` | `_Choice<G>` (may vary by ref/cv) | `_Choice<std::remove_cvref_t<G>>` |

### CPO Template (Copy-Paste Starter)

```cpp
namespace graph {
namespace _cpo {
namespace _my_cpo {
    enum class _St { _none, _member, _adl, _default };

    template<typename T> concept _has_member = /* ... */;
    template<typename T> concept _has_adl    = /* ... */;

    template<typename T>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_member<T>) {
            return {_St::_member, noexcept(/* member expr */)};
        } else if constexpr (_has_adl<T>) {
            return {_St::_adl, noexcept(/* adl expr */)};
        } else {
            return {_St::_default, true};
        }
    }

    class _fn {
    private:
        template<typename T>
        static constexpr _Choice_t<_St> _Choice = _Choose<T>();

    public:
        template<typename T>
        [[nodiscard]] constexpr auto operator()(T&& t) const
            noexcept(_Choice<std::remove_cvref_t<T>>._No_throw)
            -> decltype(auto)
        {
            using _T = std::remove_cvref_t<T>;
            if constexpr (_Choice<_T>._Strategy == _St::_member) {
                return std::forward<T>(t).my_cpo();
            } else if constexpr (_Choice<_T>._Strategy == _St::_adl) {
                return my_cpo(std::forward<T>(t));
            } else {
                return /* default */;
            }
        }
    };
} // namespace _my_cpo
} // namespace _cpo

inline namespace _cpos {
    inline constexpr _cpo::_my_cpo::_fn my_cpo{};
}
} // namespace graph
```

### Implementation Checklist

For each new CPO:

- [ ] Strategy enum covers all dispatch paths plus `_none`
- [ ] One concept per dispatch path, with constrained return type
- [ ] `_Choose<T>()` is `consteval` and returns `_Choice_t<_St>`
- [ ] `_fn::operator()` uses `if constexpr` chain (no separate overloads)
- [ ] `noexcept` specification references `_Choice<T>._No_throw`
- [ ] `[[nodiscard]]` on operator()
- [ ] Value CPOs use `decltype(auto)` return type to preserve by-value/by-ref/by-const-ref/by-rvalue-ref semantics
- [ ] `static_assert` with helpful message in the `_none` branch
- [ ] `std::remove_cvref_t` used consistently for `_Choice` lookups
- [ ] Type alias defined after the CPO (if applicable)
- [ ] Tests: every dispatch path, noexcept propagation, concept satisfaction/rejection
- [ ] Tests: at least one representative container from each of the 23 adjacency list shapes
- [ ] Compiles on GCC, Clang, and MSVC

### Integration with Ranges

CPOs work as projections with standard range adaptors:

```cpp
std::vector<MyDescriptor> vertices;

auto ids = vertices
    | std::views::transform(graph::vertex_id)
    | std::ranges::to<std::vector>();
```

---

## Incoming Edge CPOs — `in_edges` Example

The `in_edges` CPO follows the same `_Choice_t` pattern as `out_edges` but
resolves to the incoming-edge list.  This is the pattern used by all four
incoming-edge CPOs (`in_edges`, `in_degree`, `find_in_edge`, `contains_in_edge`).

```cpp
namespace _in_edges {
    enum class _St { _none, _member, _adl };

    template<typename G, typename U>
    concept _has_member = requires(G& g, U& u) {
        { g.in_edges(u) } -> std::ranges::forward_range;
    };

    template<typename G, typename U>
    concept _has_adl = requires(G& g, U& u) {
        { in_edges(g, u) } -> std::ranges::forward_range;
    };

    // No default — in_edges requires explicit container support
    template<typename G, typename U>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_member<G, U>) {
            return {_St::_member, /* noexcept */ };
        } else if constexpr (_has_adl<G, U>) {
            return {_St::_adl, /* noexcept */ };
        } else {
            return {_St::_none, false};
        }
    }
    // ... operator() dispatches via if constexpr ...
};
```

**Key difference from `out_edges`:** there is no built-in default — a graph
must explicitly provide `in_edges` (via member or ADL).  The
`bidirectional_adjacency_list` concept constrains on this CPO being valid.

The `in_edge_accessor` edge-access policy (see `edge_accessor.hpp`) delegates
to `in_edges`, `source_id`, and `source` to flip any view from outgoing to
incoming iteration.

---

## See Also

- [Architecture](architecture.md) — project structure and design principles
- [CPO Order](cpo-order.md) — canonical implementation order and dependency graph
- [CPO Reference](../reference/cpo-reference.md) — public signatures and behavior
- [Coding Guidelines](coding-guidelines.md) — naming and style conventions
