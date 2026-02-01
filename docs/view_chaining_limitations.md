# View Chaining Limitations with std::views

## Overview

Graph views in this library support C++20 range operations and can chain with `std::views` adaptors. However, there is an important limitation when using views with **capturing lambda value functions**.

## The Problem

When a view is created with a capturing lambda as a value function, it **cannot be chained** with `std::views` adaptors like `filter`, `transform`, `take`, etc.

### Example That FAILS

```cpp
std::vector<std::vector<int>> g = {{1, 2}, {0, 2}, {0, 1}};

// Capturing lambda as value function
auto vvf = [&g](auto v) { return vertex_id(g, v) * 10; };

// This FAILS to compile:
auto view = g | vertexlist(vvf) 
              | std::views::take(2);  // ❌ Compilation error
```

### Compilation Error

```
error: no match for 'operator|' 
  (operand types are 'vertexlist_view<..., lambda>' and 
   'std::ranges::views::__adaptor::_Partial<...>')
note: constraints not satisfied
note: the required expression '...__is_range_adaptor_closure_fn(...)' is invalid
```

## Why This Happens

The `std::ranges::view` concept requires types to be **semiregular**, which means:
1. **Default constructible**: Can create without arguments
2. **Copyable**: Can be copied
3. **Movable**: Can be moved

### The Lambda Problem

Lambdas with captures are **NOT default constructible**:

```cpp
int data = 42;
auto lambda = [data](int x) { return data + x; };  // Captures 'data'

// Attempting to default construct this type fails:
decltype(lambda) default_lambda;  // ❌ Error: no default constructor
```

**Why?** The compiler doesn't know what value to give the captured variable `data` when default-constructing.

### Impact on Views

When `vertexlist_view<G, VVF>` stores a capturing lambda as `VVF`:
- The view inherits the lambda's properties
- The view becomes **not default_initializable**
- Therefore, the view **doesn't satisfy `std::ranges::view`**
- `std::views` adaptors **refuse to work** with non-view types

## Concept Check Results

```cpp
// Lambda with capture
auto lambda = [&g](auto v) { return vertex_id(g, v); };
using ViewType = vertexlist_view<G, decltype(lambda)>;

std::ranges::view<ViewType>            // ❌ false
std::ranges::range<ViewType>           // ✅ true
std::ranges::input_range<ViewType>     // ✅ true
std::semiregular<ViewType>             // ❌ false - this is the problem!
std::default_initializable<ViewType>   // ❌ false - root cause
```

## Working Patterns

### ✅ Pattern 1: No Value Function + std::views::transform (RECOMMENDED)

Instead of passing a value function to the view, use `std::views::transform`:

```cpp
auto view = g | vertexlist()  // No value function
              | std::views::transform([&g](auto vi) {
                  return vertex_id(g, vi.vertex) * 10;
                })
              | std::views::take(2);  // ✅ Works perfectly!
```

**Why this works**: `vertexlist()` without VVF is fully semiregular, and capturing lambdas work fine **inside** `std::views::transform`.

### ✅ Pattern 2: Value Functions Without Chaining

Use value functions when you don't need to chain with `std::views`:

```cpp
auto vvf = [&g](auto v) { return vertex_id(g, v) * 10; };
auto view = g | vertexlist(vvf);  // ✅ Works fine

for (auto [vertex, value] : view) {
    // Use vertex and value
}
// Just don't try to chain std::views after this
```

### ✅ Pattern 3: Extract to Container First

Extract view results to a container, then chain `std::views`:

```cpp
std::vector<vertex_info<void, vertex_t<G>, void>> vertices;
for (auto vi : g | vertexlist()) {
    vertices.push_back(vi);
}

// Now chain std::views on the vector (capturing allowed)
auto view = vertices 
    | std::views::transform([&g](auto vi) { return vertex_id(g, vi.vertex); })
    | std::views::filter([](auto id) { return id > 0; });
```

### ✅ Pattern 4: Stateless Lambdas Work

Lambdas **without** captures are default constructible and work for chaining:

```cpp
// No captures - this is default constructible
auto vvf = [](auto v) { return 42; };  // Stateless

auto view = g | vertexlist(vvf) 
              | std::views::take(2);  // ✅ Works!
```

## C++26 Solution

### The Fix: std::copyable_function (P2548)

C++26 will introduce `std::copyable_function`, a type-erased function wrapper that is **always semiregular**, even when wrapping capturing lambdas.

```cpp
// C++26 (future)
std::copyable_function<int(vertex_t)> vvf = [&g](auto v) { 
    return vertex_id(g, v) * 10; 
};

// This wrapper IS default_initializable
std::default_initializable<decltype(vvf)>  // ✅ true!
```

### Future Implementation Strategy

Once C++26 is available, views could internally wrap VVF in `std::copyable_function`:

```cpp
template <adj_list::adjacency_list G, class VVF>
class vertexlist_view {
private:
    G* g_ = nullptr;
    
    // Wrap VVF in copyable_function for C++26+
    #if __cplusplus >= 202600L
        std::copyable_function<std::invoke_result_t<VVF, vertex_t<G>>(vertex_t<G>)> vvf_;
    #else
        VVF vvf_;  // C++20 - has limitations
    #endif
};
```

This would enable full chaining with capturing lambdas while maintaining backward compatibility.

## Related C++ Proposals

- **P2548R6**: `std::copyable_function` - Type-erased copyable function wrapper
- **P0792R14**: `function_ref` - Non-owning function reference (C++26)
- **P3312R0**: Relaxed `std::function` requirements for views

## Summary Table

| Pattern | Chaining | Captures | Default Init | Works? |
|---------|----------|----------|--------------|--------|
| `vertexlist()` | Yes | In `std::views` | ✅ Yes | ✅ Yes |
| `vertexlist(stateless_lambda)` | Yes | No | ✅ Yes | ✅ Yes |
| `vertexlist(capturing_lambda)` | **No** | Yes | ❌ No | ❌ No |
| `vertexlist(capturing_lambda)` | No | Yes | ❌ No | ✅ Yes |

## Testing Guidelines

When writing tests for views:

1. **Test basic iteration** - All patterns work
2. **Test chaining without value functions** - Always works
3. **Test value functions standalone** - Always works
4. **Don't test chaining with capturing value functions** - Won't compile (this is expected)

### Example Test Structure

```cpp
TEST_CASE("vertexlist - basic iteration") {
    auto g = make_test_graph();
    auto view = g | vertexlist();
    // Test basic iteration
}

TEST_CASE("vertexlist - with value function (no chaining)") {
    auto g = make_test_graph();
    auto vvf = [&g](auto v) { return vertex_id(g, v) * 10; };
    auto view = g | vertexlist(vvf);  // Don't chain after this
    // Test iteration with values
}

TEST_CASE("vertexlist - chaining with std::views") {
    auto g = make_test_graph();
    auto view = g | vertexlist()  // No value function
                  | std::views::transform([&g](auto vi) {
                      return vertex_id(g, vi.vertex);
                    });
    // Test chained view
}
```

## References

- [C++20 Ranges](https://en.cppreference.com/w/cpp/ranges)
- [std::ranges::view concept](https://en.cppreference.com/w/cpp/ranges/view)
- [std::semiregular concept](https://en.cppreference.com/w/cpp/concepts/semiregular)
- [P2548R6 - copyable_function](https://wg21.link/p2548r6)
