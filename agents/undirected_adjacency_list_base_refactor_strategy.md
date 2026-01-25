# Strategy: Refactor undirected_adjacency_list with Base Class Pattern

## Executive Summary

Refactor `undirected_adjacency_list` to use a base class pattern similar to `dynamic_graph`, consolidating common implementation between the general template and GV=void specialization. This will eliminate code duplication, improve maintainability, and provide a clearer separation of concerns.

## Current State Analysis

### Existing Implementation Structure

The current `undirected_adjacency_list` has:

1. **General Template** (`undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>`)
   - Contains all core functionality
   - Has `graph_value_` member (type GV)
   - Has `graph_value()` accessor methods
   - ~1500 lines including nested classes

2. **GV=void Specialization** (`undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>`)
   - Duplicates all core functionality from general template
   - Excludes `graph_value_` member
   - Excludes `graph_value()` methods
   - ~800 lines of duplicated code

3. **Shared Components** (used by both versions)
   - `ual_vertex` - vertex class with edge lists
   - `ual_edge` - edge class with dual-list links
   - `ual_vertex_value` / `ual_edge_value` - value holders (with void specializations)
   - `ual_vertex_edge_list` - edge list management
   - Iterator classes: `ual_const_vertex_vertex_iterator`, `ual_vertex_vertex_iterator`
   - Nested `const_edge_iterator`, `edge_iterator`

### Code Duplication Issues

Currently duplicated between general template and GV=void specialization:

- **Type Aliases** (~25 lines): `vertex_type`, `edge_type`, `vertex_set`, `vertex_iterator`, etc.
- **Nested Classes** (~300 lines): `const_edge_iterator`, `edge_iterator`
- **Constructors** (~200 lines): Default, copy, move, allocator, range-based, initializer_list
- **Core Accessors** (~100 lines): `vertices()`, `begin()`, `end()`, `try_find_vertex()`, `edges_size()`, etc.
- **Vertex Creation** (~50 lines): `create_vertex()` overloads
- **Edge Creation** (~100 lines): `create_edge()` overloads
- **Edge Removal** (~100 lines): `erase_edge()` overloads, `clear()`
- **Utilities** (~100 lines): `reserve_vertices()`, `resize_vertices()`, `swap()`, etc.
- **Friend Functions** (~300 lines): CPO ADL customization points
- **Private Members** (~50 lines): `vertices_`, `edges_size_`, `vertex_alloc_`, `edge_alloc_`

**Total Duplication: ~1300 lines of code**

### Dynamic Graph Pattern Reference

The `dynamic_graph` uses this pattern successfully:

```cpp
template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_graph_base {
  // All common implementation
protected:
  vertices_type vertices_;
  partition_vector partition_;
  // No graph_value_ member
  
  // All core functionality:
  // - Type aliases
  // - Constructors (except graph_value constructors)
  // - Core methods: vertices(), find_vertex(), create_vertex(), create_edge()
  // - Friend functions for CPO customization
};

template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_graph : public dynamic_graph_base<EV, VV, GV, VId, Sourced, Traits> {
  using base_type = dynamic_graph_base<...>;
  
private:
  [[no_unique_address]] GV graph_value_{};  // Only member
  
public:
  // Constructors that take GV parameter
  dynamic_graph(const GV& gv, allocator_type alloc = allocator_type());
  dynamic_graph(GV&& gv, allocator_type alloc = allocator_type());
  
  // Constructors that forward to base
  dynamic_graph(allocator_type alloc) : base_type(alloc) {}
  template <class ERng, class VRng, ...>
  dynamic_graph(const ERng& erng, const VRng& vrng, ...) 
    : base_type(erng, vrng, ...) {}
  
  // Graph value accessors (only methods in derived class)
  constexpr GV& graph_value() noexcept { return graph_value_; }
  constexpr const GV& graph_value() const noexcept { return graph_value_; }
};

template <class EV, class VV, class VId, bool Sourced, class Traits>
class dynamic_graph<EV, VV, void, VId, Sourced, Traits> 
    : public dynamic_graph_base<EV, VV, void, VId, Sourced, Traits> {
  using base_type = dynamic_graph_base<...>;
  
  // No graph_value_ member
  
public:
  // Constructors that forward to base
  dynamic_graph(allocator_type alloc) : base_type(alloc) {}
  template <class ERng, class VRng, ...>
  dynamic_graph(const ERng& erng, const VRng& vrng, ...) 
    : base_type(erng, vrng, ...) {}
  
  // No graph_value() methods
};
```

**Key Observations:**
- Base class contains ALL implementation (types, methods, friends)
- Derived classes are THIN wrappers (typically 50-100 lines each)
- Only difference: `graph_value_` member and `graph_value()` accessors
- No virtual functions (zero runtime overhead)
- `[[no_unique_address]]` eliminates space overhead for empty GV types

## Proposed Architecture

### New Structure

```
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>
├── All type aliases
├── All nested classes (const_edge_iterator, edge_iterator)
├── All private members (vertices_, edges_size_, allocators)
├── All constructors (EXCEPT those taking GV parameter)
├── All core methods (vertices, begin, end, create_vertex, create_edge, etc.)
├── All friend functions (CPO ADL customization points)
└── No graph_value_ member

undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>  [General]
├── Inherits from base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>
├── Private: [[no_unique_address]] GV graph_value_
├── Constructors taking GV parameter (2-3 constructors)
├── Forwarding constructors to base (8-10 constructors)
├── graph_value() accessor methods (2 methods)
└── Total: ~60-80 lines

undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>  [GV=void]
├── Inherits from base_undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>
├── No graph_value_ member
├── Forwarding constructors to base (8-10 constructors)
├── No graph_value() methods
└── Total: ~50-60 lines
```

## Detailed Design Decisions

### What Goes in Base Class

#### 1. Type Aliases (~25 lines)
```cpp
template <class VV, class EV, class GV, class VId, 
          template <typename V, typename A> class VContainer, typename Alloc>
class base_undirected_adjacency_list {
public:
  using graph_type = undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;
  
  using vertex_type = ual_vertex<VV, EV, GV, VId, VContainer, Alloc>;
  using vertex_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set = VContainer<vertex_type, vertex_allocator_type>;
  using vertex_iterator = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;
  using vertex_id_type = VId;
  using vertex_value_type = VV;
  
  using edge_type = ual_edge<VV, EV, GV, VId, VContainer, Alloc>;
  using edge_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<edge_type>;
  using edge_value_type = EV;
  using edge_size_type = size_t;
  
  // ... all other type aliases
};
```

#### 2. Nested Iterator Classes (~300 lines)
```cpp
template <class VV, class EV, class GV, class VId, 
          template <typename V, typename A> class VContainer, typename Alloc>
class base_undirected_adjacency_list {
  // ...
  
  class const_edge_iterator {
    // Full implementation (~150 lines)
  };
  
  class edge_iterator : public const_edge_iterator {
    // Full implementation (~150 lines)
  };
};
```

**Rationale:** These iterators depend on `graph_type` but don't need `graph_value_`. They operate on vertices and edges, which are the same for both versions.

#### 3. Private Data Members (~10 lines)
```cpp
template <class VV, class EV, class GV, class VId, 
          template <typename V, typename A> class VContainer, typename Alloc>
class base_undirected_adjacency_list {
protected:  // Protected so derived classes can access if needed
  vertex_set vertices_;
  edge_size_type edges_size_ = 0;
  [[no_unique_address]] vertex_allocator_type vertex_alloc_;
  [[no_unique_address]] edge_allocator_type edge_alloc_;
  
  // Note: No graph_value_ member - that goes in derived class
};
```

**Rationale:** All core state except `graph_value_` is shared. Use `protected` for potential derived class access (following `dynamic_graph` pattern).

#### 4. Constructors (~200 lines, EXCEPT GV-taking constructors)
```cpp
template <class VV, class EV, class GV, class VId, 
          template <typename V, typename A> class VContainer, typename Alloc>
class base_undirected_adjacency_list {
public:
  // Default/copy/move
  base_undirected_adjacency_list() = default;
  base_undirected_adjacency_list(const base_undirected_adjacency_list& other);
  base_undirected_adjacency_list(base_undirected_adjacency_list&&) noexcept = default;
  
  // Allocator constructor
  base_undirected_adjacency_list(const allocator_type& alloc);
  
  // Range-based constructors (WITHOUT GV parameter)
  template <typename ERng, typename VRng, typename EProj, typename VProj>
  base_undirected_adjacency_list(const ERng& erng, const VRng& vrng, 
                                  const EProj& eproj, const VProj& vproj,
                                  const Alloc& alloc = Alloc());
  
  template <typename ERng, typename EProj>
  base_undirected_adjacency_list(const ERng& erng, const EProj& eproj,
                                  const Alloc& alloc = Alloc());
  
  // Initializer_list constructors
  base_undirected_adjacency_list(
    const initializer_list<tuple<vertex_id_type, vertex_id_type, edge_value_type>>& ilist,
    const Alloc& alloc = Alloc());
  
  base_undirected_adjacency_list(
    const initializer_list<tuple<vertex_id_type, vertex_id_type>>& ilist,
    const Alloc& alloc = Alloc());
};
```

**Note:** Constructors that take GV parameter will be in the general template derived class only.

#### 5. Core Accessors (~100 lines)
```cpp
template <class VV, class EV, class GV, class VId, 
          template <typename V, typename A> class VContainer, typename Alloc>
class base_undirected_adjacency_list {
public:
  constexpr vertex_set& vertices() { return vertices_; }
  constexpr const vertex_set& vertices() const { return vertices_; }
  
  constexpr vertex_iterator begin() { return vertices_.begin(); }
  constexpr const_vertex_iterator begin() const { return vertices_.begin(); }
  constexpr const_vertex_iterator cbegin() const { return vertices_.cbegin(); }
  
  constexpr vertex_iterator end() { return vertices_.end(); }
  constexpr const_vertex_iterator end() const { return vertices_.end(); }
  constexpr const_vertex_iterator cend() const { return vertices_.cend(); }
  
  vertex_iterator try_find_vertex(vertex_id_type key);
  const_vertex_iterator try_find_vertex(vertex_id_type key) const;
  
  constexpr edge_size_type edges_size() const noexcept { return edges_size_; }
  
  constexpr edge_allocator_type edge_allocator() const noexcept { return edge_alloc_; }
  
  // Edge iterators
  constexpr edge_iterator edges_begin() { return edge_iterator(*this, begin()); }
  constexpr const_edge_iterator edges_begin() const;
  // ... all edge iterator methods
  
  edge_range edges() { return {edges_begin(), edges_end(), edges_size_}; }
  const_edge_range edges() const;
};
```

**Rationale:** None of these methods depend on `graph_value_`. They all operate on the shared state in base class.

#### 6. Vertex & Edge Creation (~150 lines)
```cpp
template <class VV, class EV, class GV, class VId, 
          template <typename V, typename A> class VContainer, typename Alloc>
class base_undirected_adjacency_list {
public:
  vertex_iterator create_vertex();
  vertex_iterator create_vertex(vertex_value_type&&);
  template <class VV2>
  vertex_iterator create_vertex(const VV2&);
  
  vertex_edge_iterator create_edge(vertex_id_type, vertex_id_type);
  vertex_edge_iterator create_edge(vertex_id_type, vertex_id_type, edge_value_type&&);
  template <class EV2>
  vertex_edge_iterator create_edge(vertex_id_type, vertex_id_type, const EV2&);
  
  edge_type* create_edge(vertex_iterator, vertex_iterator);
  edge_type* create_edge(vertex_iterator, vertex_iterator, edge_value_type&&);
  template <class EV2>
  edge_type* create_edge(vertex_iterator, vertex_iterator, const EV2&);
};
```

**Rationale:** Edge and vertex creation is identical for both versions. No dependency on `graph_value_`.

#### 7. Edge Removal & Utilities (~200 lines)
```cpp
template <class VV, class EV, class GV, class VId, 
          template <typename V, typename A> class VContainer, typename Alloc>
class base_undirected_adjacency_list {
public:
  vertex_edge_iterator erase_edge(vertex_id_type, vertex_id_type);
  void erase_edge(edge_type*);
  
  void clear();
  void swap(base_undirected_adjacency_list&);
  
  void reserve_vertices(size_t);
  void resize_vertices(size_t);
  
protected:
  void throw_unordered_edges(const char* msg) const;
};
```

**Rationale:** All graph modification operations are shared. `swap()` in base swaps `vertices_`, `edges_size_`, and allocators (but NOT `graph_value_`).

#### 8. Friend Functions for CPO Customization (~300 lines)
```cpp
template <class VV, class EV, class GV, class VId, 
          template <typename V, typename A> class VContainer, typename Alloc>
class base_undirected_adjacency_list {
  // CPO friend functions (ADL customization points)
  
  // Vertex operations
  template <typename G>
  requires std::derived_from<std::remove_cvref_t<G>, base_undirected_adjacency_list>
  friend constexpr auto vertices(G&& g) noexcept {
    return vertex_descriptor_view<G>(std::forward<G>(g).vertices());
  }
  
  template <typename G>
  requires std::derived_from<std::remove_cvref_t<G>, base_undirected_adjacency_list>
  friend constexpr auto find_vertex(G&& g, vertex_id_type key) {
    auto it = std::forward<G>(g).try_find_vertex(key);
    return vertex_descriptor<G>(it, std::forward<G>(g).vertices());
  }
  
  // Edge operations
  template <typename G, typename VDesc>
  requires std::derived_from<std::remove_cvref_t<G>, base_undirected_adjacency_list>
  friend constexpr auto edges(G&& g, VDesc&& u) noexcept {
    // ...
  }
  
  template <typename G, typename VDesc>
  requires std::derived_from<std::remove_cvref_t<G>, base_undirected_adjacency_list>
  friend constexpr auto degree(G&& g, VDesc&& u) noexcept {
    // ...
  }
  
  // Edge value access
  template <typename G, typename EDesc>
  requires std::derived_from<std::remove_cvref_t<G>, base_undirected_adjacency_list>
           && (!std::is_void_v<EV>)
  friend constexpr decltype(auto) edge_value(G&& g, EDesc&& uv) noexcept {
    // ...
  }
  
  // Vertex value access
  template <typename G, typename VDesc>
  requires std::derived_from<std::remove_cvref_t<G>, base_undirected_adjacency_list>
           && (!std::is_void_v<VV>)
  friend constexpr decltype(auto) vertex_value(G&& g, VDesc&& u) noexcept {
    // ...
  }
  
  // Note: graph_value() friend function NOT here - goes in derived class
  
  // ... 30+ more CPO friend functions
};
```

**Critical Design Decision:** All CPO friend functions go in base class, EXCEPT `graph_value()` CPO.

**Rationale:**
1. The friend functions use `std::derived_from<std::remove_cvref_t<G>, base_undirected_adjacency_list>` constraint
2. This allows them to work with both `undirected_adjacency_list<..., GV>` and `undirected_adjacency_list<..., void>`
3. CPOs like `edge_value`, `vertex_value` already have `requires (!std::is_void_v<EV>)` guards
4. Only `graph_value()` CPO needs to be in the general template derived class

### What Goes in Derived Classes

#### General Template (with GV) (~60-80 lines)

```cpp
template <typename VV, typename EV, typename GV, integral VId,
          template <typename V, typename A> class VContainer, typename Alloc>
class undirected_adjacency_list 
    : public base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc> {
public:
  using base_type = base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;
  using graph_type = undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;
  using graph_value_type = GV;
  using allocator_type = typename base_type::vertex_allocator_type;
  
private:
  [[no_unique_address]] GV graph_value_{};  // Only additional member
  
public:
  // Default/copy/move (default implementations call base)
  undirected_adjacency_list() = default;
  undirected_adjacency_list(const undirected_adjacency_list&) = default;
  undirected_adjacency_list(undirected_adjacency_list&&) noexcept = default;
  ~undirected_adjacency_list() = default;
  
  undirected_adjacency_list& operator=(const undirected_adjacency_list&) = default;
  undirected_adjacency_list& operator=(undirected_adjacency_list&&) = default;
  
  // Constructors with GV parameter (UNIQUE to general template)
  undirected_adjacency_list(const GV& gv, const allocator_type& alloc = allocator_type())
      : base_type(alloc), graph_value_(gv) {}
  
  undirected_adjacency_list(GV&& gv, const allocator_type& alloc = allocator_type())
      : base_type(alloc), graph_value_(std::move(gv)) {}
  
  // Forwarding constructor: allocator only
  undirected_adjacency_list(const allocator_type& alloc) : base_type(alloc) {}
  
  // Forwarding constructors: range-based with GV
  template <typename ERng, typename VRng, typename EProj = std::identity, 
            typename VProj = std::identity>
  undirected_adjacency_list(const ERng& erng, const VRng& vrng,
                            const EProj& eproj, const VProj& vproj,
                            const GV& gv, const Alloc& alloc = Alloc())
      : base_type(erng, vrng, eproj, vproj, alloc), graph_value_(gv) {}
  
  template <typename ERng, typename EProj = std::identity>
  undirected_adjacency_list(const ERng& erng, const EProj& eproj,
                            const GV& gv, const Alloc& alloc = Alloc())
      : base_type(erng, eproj, alloc), graph_value_(gv) {}
  
  // Forwarding constructors: range-based WITHOUT GV (forward to base)
  template <typename ERng, typename VRng, typename EProj = std::identity, 
            typename VProj = std::identity>
  undirected_adjacency_list(const ERng& erng, const VRng& vrng,
                            const EProj& eproj = {}, const VProj& vproj = {},
                            const Alloc& alloc = Alloc())
      : base_type(erng, vrng, eproj, vproj, alloc) {}
  
  // ... other forwarding constructors
  
public:
  // Graph value accessors (UNIQUE to general template)
  constexpr GV& graph_value() noexcept { return graph_value_; }
  constexpr const GV& graph_value() const noexcept { return graph_value_; }
  
  // Graph value CPO friend function (UNIQUE to general template)
  template <typename G>
  requires std::derived_from<std::remove_cvref_t<G>, undirected_adjacency_list>
  friend constexpr decltype(auto) graph_value(G&& g) noexcept {
    return std::forward<G>(g).graph_value();
  }
};
```

**Key Points:**
- Only ~60-80 lines of code
- Only contains: `graph_value_` member, constructors, `graph_value()` methods, `graph_value()` CPO
- All other functionality inherited from base

#### GV=void Specialization (~50-60 lines)

```cpp
template <typename VV, typename EV, integral VId,
          template <typename V, typename A> class VContainer, typename Alloc>
class undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>
    : public base_undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc> {
public:
  using base_type = base_undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>;
  using graph_type = undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>;
  using graph_value_type = void;
  using allocator_type = typename base_type::vertex_allocator_type;
  
  // No graph_value_ member
  
public:
  // Default/copy/move
  undirected_adjacency_list() = default;
  undirected_adjacency_list(const undirected_adjacency_list&) = default;
  undirected_adjacency_list(undirected_adjacency_list&&) noexcept = default;
  ~undirected_adjacency_list() = default;
  
  undirected_adjacency_list& operator=(const undirected_adjacency_list&) = default;
  undirected_adjacency_list& operator=(undirected_adjacency_list&&) = default;
  
  // Forwarding constructor: allocator only
  undirected_adjacency_list(const allocator_type& alloc) : base_type(alloc) {}
  
  // Forwarding constructors: range-based (all forward to base)
  template <typename ERng, typename VRng, typename EProj = std::identity, 
            typename VProj = std::identity>
  undirected_adjacency_list(const ERng& erng, const VRng& vrng,
                            const EProj& eproj = {}, const VProj& vproj = {},
                            const Alloc& alloc = Alloc())
      : base_type(erng, vrng, eproj, vproj, alloc) {}
  
  template <typename ERng, typename EProj = std::identity>
  undirected_adjacency_list(const ERng& erng, const EProj& eproj = {},
                            const Alloc& alloc = Alloc())
      : base_type(erng, eproj, alloc) {}
  
  // ... other forwarding constructors
  
  // No graph_value() methods
  // No graph_value() CPO friend function
};
```

**Key Points:**
- Only ~50-60 lines of code
- No `graph_value_` member, no `graph_value()` methods, no `graph_value()` CPO
- All constructors simply forward to base
- All other functionality inherited from base

## Implementation Plan

### Phase 1: Create Base Class (Low Risk)

**Goal:** Extract common code into `base_undirected_adjacency_list` without breaking existing code.

**Steps:**

1. **Create base class declaration** in `undirected_adjacency_list.hpp` (before existing class)
   ```cpp
   template <typename VV, typename EV, typename GV, integral VId,
             template <typename V, typename A> class VContainer, typename Alloc>
   class base_undirected_adjacency_list {
     // Empty for now
   };
   ```

2. **Move type aliases** from general template to base class
   - Copy all `using` declarations
   - Change `graph_type = undirected_adjacency_list<...>` (keep in both places for now)

3. **Move nested iterator classes** to base class
   - Move `const_edge_iterator` and `edge_iterator` class definitions
   - Update any references to use base class types

4. **Move private members** to base class as `protected`
   - Move `vertices_`, `edges_size_`, `vertex_alloc_`, `edge_alloc_`
   - Remove from general template (will cause compilation errors - expected)

5. **Make general template inherit from base**
   ```cpp
   template <typename VV, typename EV, typename GV, integral VId,
             template <typename V, typename A> class VContainer, typename Alloc>
   class undirected_adjacency_list 
       : public base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc> {
   public:
     using base_type = base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;
     // ... rest of class
   };
   ```

6. **Fix member access in general template**
   - Prefix base class members with `this->` or `base_type::`
   - Or add `using base_type::vertices_;` etc.

7. **Compile and test general template**
   - Run all tests to ensure nothing broke
   - Fix any compilation errors

### Phase 2: Move Core Methods to Base (Medium Risk)

**Goal:** Move all shared implementation to base class.

**Steps:**

1. **Move constructor implementations** to base class
   - Keep declarations in general template for now
   - Move implementations from `undirected_adjacency_list_impl.hpp` to new section for base class
   - Constructors taking GV parameter remain in general template

2. **Move method implementations** to base class
   - Move all accessor implementations (`vertices()`, `begin()`, `edges_size()`, etc.)
   - Move all creation methods (`create_vertex()`, `create_edge()`)
   - Move all modification methods (`erase_edge()`, `clear()`, `swap()`)
   - Move utilities (`reserve_vertices()`, `resize_vertices()`)

3. **Move friend functions** to base class (except `graph_value()` CPO)
   - Update `requires` clauses to use `std::derived_from<..., base_undirected_adjacency_list>`
   - Keep `graph_value()` CPO in general template

4. **Compile and test**
   - Ensure all tests still pass for general template

### Phase 3: Update GV=void Specialization (Medium Risk)

**Goal:** Replace duplicated code in specialization with inheritance.

**Steps:**

1. **Make GV=void specialization inherit from base**
   ```cpp
   template <typename VV, typename EV, integral VId,
             template <typename V, typename A> class VContainer, typename Alloc>
   class undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>
       : public base_undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc> {
   public:
     using base_type = base_undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>;
     // ... minimal code
   };
   ```

2. **Remove all duplicated code** from GV=void specialization
   - Remove all type aliases (inherited from base)
   - Remove nested iterator classes (inherited from base)
   - Remove all private members (inherited from base)
   - Remove all method implementations (inherited from base)
   - Remove all friend functions (inherited from base)

3. **Keep only forwarding constructors** in GV=void specialization
   - Thin wrappers that call base class constructors

4. **Remove implementations** for GV=void from `undirected_adjacency_list_impl.hpp`
   - All implementation now comes from base class

5. **Compile and test GV=void specialization**
   - Run all tests to ensure nothing broke

### Phase 4: Final Cleanup & Optimization (Low Risk)

**Goal:** Polish the implementation and verify correctness.

**Steps:**

1. **Review base class for any GV dependencies**
   - Ensure no code in base class references `graph_value_`
   - Confirm all GV-specific code is in derived classes

2. **Verify `[[no_unique_address]]` is used** for `graph_value_` in general template
   - Ensures zero overhead for empty GV types

3. **Update documentation**
   - Add comments explaining base/derived structure
   - Document which class contains which functionality

4. **Run full test suite**
   - All 3866 tests should pass
   - Verify both general and GV=void versions work correctly

5. **Check for binary size changes**
   - Inheritance should not increase binary size (no vtables)
   - May actually decrease due to code deduplication

6. **Performance testing** (if available)
   - Ensure no performance regression
   - Static inheritance has zero runtime cost

## Expected Benefits

### Code Maintenance
- **Eliminate 1300 lines of duplication** - Single source of truth for all common code
- **Easier bug fixes** - Fix once in base, applies to both versions
- **Simpler additions** - New features go in base class automatically available to both versions

### Code Clarity
- **Clear separation** - Base class = shared, derived classes = GV-specific
- **Follows established pattern** - Consistent with `dynamic_graph` design
- **Easier to understand** - Less code to read and maintain

### Testing
- **Reduced test burden** - Common code tested once
- **Better coverage** - Easier to ensure both versions are tested equally

### Future Extensions
- **Easy to add new specializations** - e.g., GV+VV=void, GV+EV=void
- **Consistent interface** - All versions have identical base functionality

## Risks & Mitigations

### Risk 1: Template Complexity
**Risk:** Base class template may be harder to instantiate/debug.

**Mitigation:**
- Use clear type aliases (`using base_type = ...`)
- Add static_asserts for common errors
- Document template parameter requirements

### Risk 2: Friend Function Resolution
**Risk:** Friend functions in base class may not resolve correctly for derived types.

**Mitigation:**
- Use `std::derived_from` constraints (already planned)
- Test ADL resolution thoroughly
- CPOs already have this pattern working in `dynamic_graph`

### Risk 3: Compilation Time
**Risk:** Additional template instantiation layers might slow compilation.

**Mitigation:**
- Base class is instantiated once, not twice (same as before)
- Forward declarations reduce recompilation
- Measure compilation time before/after

### Risk 4: Breakage of Existing Code
**Risk:** User code might break if it depends on implementation details.

**Mitigation:**
- Public interface remains unchanged
- Type aliases maintained in derived classes
- Incremental testing after each phase

## Testing Strategy

### Phase 1 Testing
- Compile general template after inheritance
- Run subset of tests (10-20) covering core functionality
- Fix any compilation errors

### Phase 2 Testing
- Full test suite after moving each category of methods
- Test constructors, accessors, modifiers separately
- Verify friend function ADL resolution

### Phase 3 Testing
- Full test suite for GV=void specialization
- Compare behavior against general template
- Test edge cases (empty graph, single vertex, etc.)

### Phase 4 Testing
- Complete test suite (all 3866 tests)
- Performance benchmarks (if available)
- Memory usage checks
- Binary size comparison

### Continuous Testing
- Run tests after EACH significant change
- Use git commits to checkpoint working state
- Rollback if tests fail (keep known-good state)

## Success Criteria

1. **All tests pass** - 3866 tests, 36004 assertions, 100% pass rate
2. **No duplicate code** - <50 lines of duplication between general and GV=void versions
3. **Consistent interface** - Both versions have identical public API (except `graph_value()`)
4. **Zero runtime overhead** - No virtual functions, no performance regression
5. **Code reduction** - ~1300 lines eliminated, ~80% reduction in derived class code
6. **Maintainability** - Single source for all common functionality

## Estimated Effort

- **Phase 1:** 2-3 hours (create base, move types/members, update general template)
- **Phase 2:** 3-4 hours (move methods and friend functions)
- **Phase 3:** 2-3 hours (update GV=void specialization)
- **Phase 4:** 1-2 hours (cleanup, testing, documentation)

**Total:** 8-12 hours of focused development time

## Alternative Approaches Considered

### Alternative 1: CRTP (Curiously Recurring Template Pattern)
**Rejected because:** More complex than inheritance, no benefit here (no need for static polymorphism).

### Alternative 2: Mixins/Multiple Inheritance
**Rejected because:** Adds complexity without benefit. Single base class is clearer.

### Alternative 3: Preprocessor Macros
**Rejected because:** Unmaintainable, hard to debug, doesn't follow modern C++ best practices.

### Alternative 4: Keep Current Duplication
**Rejected because:** ~1300 lines of duplication is a maintenance burden and violates DRY principle.

## Conclusion

The base class refactoring follows the proven pattern from `dynamic_graph`, eliminates significant code duplication, and improves maintainability without affecting performance or public API. The phased implementation approach minimizes risk and allows for incremental testing. This refactoring is a worthwhile investment that will pay dividends in future maintenance and feature development.

## Next Steps

1. **Review this strategy document** - Get feedback from stakeholders
2. **Create feature branch** - `feature/undirected-adjacency-list-base-refactor`
3. **Begin Phase 1** - Create base class and update general template
4. **Incremental commits** - Commit after each successful phase
5. **Full testing** - Run complete test suite before merging to main
