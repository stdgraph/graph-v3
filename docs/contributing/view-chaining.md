# View Chaining — How It Works

## Overview

Graph views in graph-v3 support **full C++20 range chaining with `std::views`**, including
when using value functions. This was achieved by changing value function signatures to receive the
graph as a parameter rather than requiring lambda capture.

## The Solution: Parameter-Based Value Functions

Value functions now receive the graph as a parameter instead of requiring capture:

```cpp
// Graph is passed as a parameter — lambda is stateless
auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 10; };

// Full chaining support!
auto view = g | vertexlist(vvf) 
              | std::views::take(2);        // ✅ Works perfectly!
              | std::views::filter(...);    // ✅ Chains freely!
```

### Why This Works

Stateless lambdas (empty capture list `[]`) are:
1. **Default constructible**: No captured state to initialize
2. **Copyable**: No captured references to worry about
3. **Semiregular**: Satisfies `std::semiregular` concept

These properties allow views storing stateless lambdas to satisfy `std::ranges::view`,
which is what `std::views` adaptors require.

### Concept Checks

```cpp
auto vvf = [](const auto& g, auto v) { return vertex_id(g, v); };
using ViewType = vertexlist_view<G, decltype(vvf)>;

std::ranges::view<ViewType>            // ✅ true
std::ranges::range<ViewType>           // ✅ true
std::ranges::input_range<ViewType>     // ✅ true
std::semiregular<ViewType>             // ✅ true
std::default_initializable<ViewType>   // ✅ true
```

## Usage Patterns

### ✅ Pattern 1: Value Functions with Full Chaining (Recommended)

```cpp
auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 10; };
auto view = g | vertexlist(vvf) 
              | std::views::take(2);  // ✅ Works!
```

### ✅ Pattern 2: No Value Function + std::views::transform

```cpp
auto view = g | vertexlist()  // No value function
              | std::views::transform([&g](auto vi) {
                  auto [v] = vi;
                  return vertex_id(g, v) * 10;
                })
              | std::views::take(2);  // ✅ Works
```

Note: Capturing lambdas are fine **inside** `std::views::transform` and other standard
adaptors — only the graph view itself needs to be semiregular.

### ✅ Pattern 3: Search Views with Value Functions and Chaining

```cpp
auto vvf = [](const auto& g, auto v) { return vertex_id(g, v); };
auto view = g | vertices_dfs(0, vvf)
              | std::views::take(5);   // ✅ Works!

auto evf = [](const auto& g, auto e) { return target_id(g, e); };
auto edges = g | edges_bfs(0, evf)
               | std::views::take(3);  // ✅ Works!
```

## Historical Context: The Capture Problem

### Previous Signatures (Before This Change)

Previously, value functions took only a descriptor:
- VVF: `vvf(v)` — vertex descriptor only
- EVF: `evf(e)` — edge descriptor only

This required users to **capture** the graph reference:

```cpp
// OLD pattern — capturing lambda breaks chaining
auto vvf = [&g](auto v) { return vertex_id(g, v) * 10; };
//          ^^^^ Capture makes lambda stateful
```

### Why Capturing Lambdas Failed

Lambdas with captures are **NOT default constructible**:

```cpp
int data = 42;
auto lambda = [data](int x) { return data + x; };

// This fails:
decltype(lambda) default_lambda;  // ❌ Error: no default constructor
```

**Why?** The compiler doesn't know what value to give the captured variable `data` when
default-constructing.

When a view stored a capturing lambda as its VVF:
- The view inherited the lambda's properties
- The view became **not default_initializable**
- Therefore, the view **didn't satisfy `std::ranges::view`**
- `std::views` adaptors **refused to work** with it

```cpp
// OLD: This FAILED to compile
auto vvf = [&g](auto v) { return vertex_id(g, v) * 10; };
auto view = g | vertexlist(vvf) 
              | std::views::take(2);  // ❌ Compilation error
```

### Current Signatures (After This Change)

Value functions now receive the graph as a first parameter:
- VVF: `vvf(g, v)` — graph and vertex descriptor
- EVF: `evf(g, e)` — graph and edge descriptor

The view passes `std::as_const(*g_)` as the graph argument, ensuring const-correctness.
Since the graph is a **parameter** rather than a **capture**, the lambda remains stateless
and the view satisfies `std::ranges::view`.

## C++26 Alternatives

C++26 will introduce `std::copyable_function` (P2548) which could also solve this problem
by wrapping capturing lambdas in a semiregular type-erased wrapper. However, the parameter-based
approach adopted here:
1. Works in C++20 today (no C++26 dependency)
2. Is more explicit about graph access
3. Avoids type-erasure overhead
4. Enables more flexible value functions

## Summary Table

| Pattern | Chaining | Captures | Semiregular | Works? |
|---------|----------|----------|-------------|--------|
| `vertexlist()` (no VVF) | Yes | N/A | ✅ Yes | ✅ Yes |
| `vertexlist(stateless_vvf)` | Yes | No | ✅ Yes | ✅ Yes |
| `vertexlist(capturing_vvf)` | **No** | Yes | ❌ No | ❌ No |

**Note**: With parameter-based value functions, the typical usage is always the "stateless" row.
Capturing lambdas would only occur if a user intentionally captured additional non-graph state.

## References

- [C++20 Ranges](https://en.cppreference.com/w/cpp/ranges)
- [std::ranges::view concept](https://en.cppreference.com/w/cpp/ranges/view)
- [std::semiregular concept](https://en.cppreference.com/w/cpp/concepts/semiregular)
- [P2548R6 - copyable_function](https://wg21.link/p2548r6)
- [Value Function Goal](../agents/value_function_goal.md) - Design rationale
