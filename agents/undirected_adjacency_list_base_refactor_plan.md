# Implementation Plan: Base Class Refactoring for undirected_adjacency_list

## Overview

This document provides a detailed, step-by-step implementation plan for refactoring `undirected_adjacency_list` to use a base class pattern (similar to `dynamic_graph`). Each phase is designed to be safe, incremental, and easily executed by an automated agent.

**Objective:** Eliminate ~1300 lines of code duplication between the general template and GV=void specialization by consolidating common implementation in a base class.

**Files to Modify:**
- `include/graph/container/undirected_adjacency_list.hpp` - Main header (2226 lines)
- `include/graph/container/detail/undirected_adjacency_list_impl.hpp` - Implementation file

**Testing Strategy:** After each phase, compile and run the full test suite (3866 tests).

---

## Phase 1: Create Base Class Structure and Inheritance

**Goal:** Create empty base class, establish inheritance hierarchy, move type aliases and private members.

**Estimated Time:** 2-3 hours  
**Risk Level:** Low (no logic changes, only structure)  
**Rollback Strategy:** Git commit after Phase 1 completion

### Step 1.1: Create Base Class Declaration

**Location:** `include/graph/container/undirected_adjacency_list.hpp`  
**Action:** Insert new base class declaration BEFORE the general template class (around line 700)

**Insert at line ~700 (before `class ual_vertex`):**

```cpp
///-------------------------------------------------------------------------------------
/// base_undirected_adjacency_list - Base class for undirected_adjacency_list
///
/// @brief Contains all common implementation shared between the general template
///        and GV=void specialization.
///
/// This base class contains:
/// - All type aliases
/// - Nested iterator classes (const_edge_iterator, edge_iterator)
/// - All private data members (vertices_, edges_size_, allocators)
/// - All constructors (except those taking GV parameter)
/// - All core methods (accessors, creators, modifiers)
/// - All friend functions for CPO customization (except graph_value CPO)
///
/// The derived classes (general template and GV=void specialization) contain only:
/// - Constructors specific to GV handling
/// - graph_value_ member (general template only)
/// - graph_value() accessors (general template only)
/// - graph_value() CPO friend function (general template only)
///
/// @tparam VV Vertex Value type
/// @tparam EV Edge Value type
/// @tparam GV Graph Value type (may be void)
/// @tparam VId Vertex key/index type
/// @tparam VContainer Vertex storage container template
/// @tparam Alloc Allocator type
///-------------------------------------------------------------------------------------
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
class base_undirected_adjacency_list {
public:
  // Type aliases will be moved here in Step 1.2
  
protected:
  // Data members will be moved here in Step 1.3
  
public:
  // Methods will be added in Phase 2
};
```

**Verification:**
- File compiles (no syntax errors)
- Empty base class doesn't break anything

### Step 1.2: Move Type Aliases to Base Class

**Location:** `include/graph/container/undirected_adjacency_list.hpp`  
**Action:** Copy type aliases from general template to base class

**In base_undirected_adjacency_list (add after "Type aliases will be moved here"):**

```cpp
public: // Type Aliases
  using graph_type = undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;
  
  using vertex_type           = ual_vertex<VV, EV, GV, VId, VContainer, Alloc>;
  using vertex_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set            = VContainer<vertex_type, vertex_allocator_type>;
  using vertex_iterator       = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;
  using vertex_id_type       = VId;
  using vertex_value_type     = VV;
  
  using edge_type            = ual_edge<VV, EV, GV, VId, VContainer, Alloc>;
  using edge_allocator_type  = typename allocator_traits<Alloc>::template rebind_alloc<edge_type>;
  using edge_value_type      = EV;
  using edge_size_type       = size_t;
  using edge_difference_type = ptrdiff_t;
  
  using vertex_edge_iterator       = typename vertex_type::vertex_edge_iterator;
  using const_vertex_edge_iterator = typename vertex_type::const_vertex_edge_iterator;
  using vertex_edge_range          = typename vertex_type::vertex_edge_range;
  using const_vertex_edge_range    = typename vertex_type::const_vertex_edge_range;
  
  using allocator_type = vertex_allocator_type;
```

**Note:** Keep type aliases in general template for now (will clean up in Phase 4). This ensures backward compatibility during transition.

**Verification:**
- Base class compiles with type aliases
- No compilation errors

### Step 1.3: Move Private Members to Base Class (as Protected)

**Location:** `include/graph/container/undirected_adjacency_list.hpp`  
**Action:** Move private data members from general template to base class

**In base_undirected_adjacency_list (add after "Data members will be moved here"):**

```cpp
protected: // Data Members (protected for derived class access)
  vertex_set vertices_;
  edge_size_type edges_size_ = 0;
  [[no_unique_address]] vertex_allocator_type vertex_alloc_;
  [[no_unique_address]] edge_allocator_type edge_alloc_;
  
  // Note: graph_value_ is NOT here - it belongs in the derived class
```

**In general template undirected_adjacency_list:**
- **DELETE** the lines defining `vertices_`, `edges_size_`, `vertex_alloc_`, `edge_alloc_`
- **KEEP** the line defining `graph_value_` (if present)

**Verification:**
- General template will have compilation errors (expected - members not accessible yet)
- This is intentional - will fix in Step 1.4

### Step 1.4: Make General Template Inherit from Base

**Location:** `include/graph/container/undirected_adjacency_list.hpp`  
**Action:** Add inheritance and using declarations to general template

**Find general template class declaration (around line 1000):**

```cpp
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
class undirected_adjacency_list {
```

**Replace with:**

```cpp
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
class undirected_adjacency_list 
    : public base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc> {
public:
  using base_type = base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;
  
  // Import base class members for convenience
  using base_type::vertices_;
  using base_type::edges_size_;
  using base_type::vertex_alloc_;
  using base_type::edge_alloc_;
```

**Verification:**
- Compilation errors should disappear
- Code compiles successfully

### Step 1.5: Move Nested Iterator Classes to Base

**Location:** `include/graph/container/undirected_adjacency_list.hpp`  
**Action:** Move `const_edge_iterator` and `edge_iterator` from general template to base class

**In base_undirected_adjacency_list (add as public nested classes):**

1. Find the `const_edge_iterator` class in general template (around line 1040-1110)
2. **CUT** the entire class definition
3. **PASTE** into base class as a public nested class
4. Find the `edge_iterator` class in general template (around line 1120-1180)
5. **CUT** the entire class definition
6. **PASTE** into base class as a public nested class

**Important:** Update references in iterator classes:
- `graph_type` already resolves to `undirected_adjacency_list` via base class typedef
- No changes needed to iterator logic

**In general template:**
- Add using declarations to expose iterators:
```cpp
public:
  using const_edge_iterator = typename base_type::const_edge_iterator;
  using edge_iterator = typename base_type::edge_iterator;
```

**Verification:**
- Iterators compile in base class
- General template can access iterators via using declarations
- No compilation errors

### Step 1.6: Add Edge Range Type Aliases to Base

**Location:** `include/graph/container/undirected_adjacency_list.hpp`  
**Action:** Add edge range type aliases to base class

**In base_undirected_adjacency_list (add to type aliases section):**

```cpp
  using edge_range = ranges::subrange<edge_iterator, edge_iterator, ranges::subrange_kind::sized>;
  using const_edge_range = ranges::subrange<const_edge_iterator, const_edge_iterator, ranges::subrange_kind::sized>;
```

**Verification:**
- Compiles without errors
- Edge ranges available in base class

### Step 1.7: Compile and Test Phase 1

**Actions:**
1. Build the project: `cmake --build build/linux-gcc-debug --target graph3_tests`
2. Run tests: `./build/linux-gcc-debug/tests/graph3_tests`
3. Verify: All 3866 tests pass

**If tests fail:**
- Review compilation errors
- Check member access (use `this->` or `base_type::` if needed)
- Verify all using declarations are correct

**Git Checkpoint:**
```bash
git add -A
git commit -m "Phase 1: Create base_undirected_adjacency_list and establish inheritance

- Created base_undirected_adjacency_list with type aliases
- Moved private members to base class (as protected)
- Made general template inherit from base
- Moved nested iterator classes to base
- Added using declarations for base class members

All tests passing (3866/3866)"
```

---

## Phase 2: Move Methods to Base Class

**Goal:** Move all shared method implementations from general template to base class.

**Estimated Time:** 3-4 hours  
**Risk Level:** Medium (logic movement, but no changes to logic)  
**Rollback Strategy:** Git commit after each major category of methods

### Step 2.1: Move Constructor Declarations and Implementations

**Location:** 
- Declarations: `include/graph/container/undirected_adjacency_list.hpp`
- Implementations: `include/graph/container/detail/undirected_adjacency_list_impl.hpp`

**Categories of Constructors:**

#### 2.1.1: Default/Copy/Move Constructors

**In base_undirected_adjacency_list (declarations):**

```cpp
public: // Construction/Destruction/Assignment
  base_undirected_adjacency_list()                                              = default;
  base_undirected_adjacency_list(const base_undirected_adjacency_list& other);
  base_undirected_adjacency_list(base_undirected_adjacency_list&&) noexcept     = default;
  ~base_undirected_adjacency_list();
  
  base_undirected_adjacency_list& operator=(const base_undirected_adjacency_list& other);
  base_undirected_adjacency_list& operator=(base_undirected_adjacency_list&&)   = default;
```

**In undirected_adjacency_list_impl.hpp:**
- Find copy constructor implementation for general template
- Copy it and create version for base class (change class name)
- Do same for destructor and copy assignment operator

**Example for copy constructor:**

```cpp
// In impl file - move from general template section to base section
template <typename VV, typename EV, typename GV, integral VId,
          template <typename V, typename A> class VContainer, typename Alloc>
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::
base_undirected_adjacency_list(const base_undirected_adjacency_list& other)
    : vertices_(other.vertex_alloc_)
    , edges_size_(0)
    , vertex_alloc_(other.vertex_alloc_)
    , edge_alloc_(other.edge_alloc_) {
  // Deep copy implementation (same as current general template)
  // ...
}
```

#### 2.1.2: Allocator Constructor

**In base_undirected_adjacency_list:**

```cpp
  base_undirected_adjacency_list(const allocator_type& alloc);
```

**In impl file:**

```cpp
template <typename VV, typename EV, typename GV, integral VId,
          template <typename V, typename A> class VContainer, typename Alloc>
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::
base_undirected_adjacency_list(const allocator_type& alloc)
    : vertices_(alloc), vertex_alloc_(alloc), edge_alloc_(alloc) {}
```

#### 2.1.3: Range-Based Constructors (WITHOUT GV parameter)

**In base_undirected_adjacency_list:**

```cpp
  // Constructor with edge range and vertex range
  template <typename ERng, typename VRng, typename EProj = std::identity, typename VProj = std::identity>
    requires ranges::forward_range<ERng> 
          && ranges::input_range<VRng>
          && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
          && std::regular_invocable<VProj, ranges::range_reference_t<VRng>>
  base_undirected_adjacency_list(const ERng&  erng,
                                 const VRng&  vrng,
                                 const EProj& eproj = {},
                                 const VProj& vproj = {},
                                 const Alloc& alloc = Alloc());
  
  // Constructor with edge range only
  template <typename ERng, typename EProj = std::identity>
    requires ranges::forward_range<ERng>
          && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
  base_undirected_adjacency_list(const ERng&  erng, 
                                 const EProj& eproj = {}, 
                                 const Alloc& alloc = Alloc());
```

**In impl file:** Move implementations from general template, change class name to `base_undirected_adjacency_list`.

#### 2.1.4: Initializer List Constructors

**In base_undirected_adjacency_list:**

```cpp
  // Initializer list with edge values
  base_undirected_adjacency_list(
    const initializer_list<tuple<vertex_id_type, vertex_id_type, edge_value_type>>& ilist,
    const Alloc& alloc = Alloc());
  
  // Initializer list without edge values
  base_undirected_adjacency_list(
    const initializer_list<tuple<vertex_id_type, vertex_id_type>>& ilist,
    const Alloc& alloc = Alloc());
```

**In impl file:** Move implementations from general template.

**Verification:**
- All constructors compile in base class
- Compile and run tests

### Step 2.2: Move Core Accessor Methods

**Location:** Header and implementation files

**Methods to move:**

```cpp
// In base_undirected_adjacency_list:
public: // Accessors
  constexpr edge_allocator_type edge_allocator() const noexcept;
  
  constexpr vertex_set&       vertices();
  constexpr const vertex_set& vertices() const;
  
  constexpr vertex_iterator       begin();
  constexpr const_vertex_iterator begin() const;
  constexpr const_vertex_iterator cbegin() const;
  
  constexpr vertex_iterator       end();
  constexpr const_vertex_iterator end() const;
  constexpr const_vertex_iterator cend() const;
  
  vertex_iterator       try_find_vertex(vertex_id_type);
  const_vertex_iterator try_find_vertex(vertex_id_type) const;
  
  constexpr edge_size_type edges_size() const noexcept;
  
  constexpr edge_iterator       edges_begin();
  constexpr const_edge_iterator edges_begin() const;
  constexpr const_edge_iterator edges_cbegin() const;
  
  constexpr edge_iterator       edges_end();
  constexpr const_edge_iterator edges_end() const;
  constexpr const_edge_iterator edges_cend() const;
  
  edge_range       edges();
  const_edge_range edges() const;
```

**In impl file:** Move all implementations, change class name to `base_undirected_adjacency_list`.

**Note:** Most accessor implementations are inline/constexpr in header. Move those directly in base class.

**Verification:**
- Compile and run tests
- All accessors work correctly

### Step 2.3: Move Vertex Creation Methods

**In base_undirected_adjacency_list:**

```cpp
public: // Vertex Creation
  vertex_iterator create_vertex();
  vertex_iterator create_vertex(vertex_value_type&&);
  
  template <class VV2>
    requires std::constructible_from<vertex_value_type, const VV2&>
  vertex_iterator create_vertex(const VV2&);
```

**In impl file:** Move implementations from general template.

**Verification:**
- Compile and run tests
- Vertex creation works

### Step 2.4: Move Edge Creation Methods

**In base_undirected_adjacency_list:**

```cpp
public: // Edge Creation
  vertex_edge_iterator create_edge(vertex_id_type, vertex_id_type);
  vertex_edge_iterator create_edge(vertex_id_type, vertex_id_type, edge_value_type&&);
  
  template <class EV2>
    requires std::constructible_from<edge_value_type, const EV2&>
  vertex_edge_iterator create_edge(vertex_id_type, vertex_id_type, const EV2&);
  
  edge_type* create_edge(vertex_iterator, vertex_iterator);
  edge_type* create_edge(vertex_iterator, vertex_iterator, edge_value_type&&);
  
  template <class EV2>
    requires std::constructible_from<edge_value_type, const EV2&>
  edge_type* create_edge(vertex_iterator, vertex_iterator, const EV2&);
```

**In impl file:** Move implementations from general template.

**Verification:**
- Compile and run tests
- Edge creation works

### Step 2.5: Move Edge Removal and Graph Modification Methods

**In base_undirected_adjacency_list:**

```cpp
public: // Edge Removal
  vertex_edge_iterator erase_edge(vertex_id_type, vertex_id_type);
  void erase_edge(edge_type*);
  
public: // Graph Modification
  void clear();
  void swap(base_undirected_adjacency_list&);
  
public: // Utilities
  void reserve_vertices(size_t);
  void resize_vertices(size_t);
  
protected:
  void throw_unordered_edges(const char* msg) const;
```

**In impl file:** Move implementations from general template.

**Important for swap():**
```cpp
void swap(base_undirected_adjacency_list& other) {
  vertices_.swap(other.vertices_);
  std::swap(edges_size_, other.edges_size_);
  std::swap(vertex_alloc_, other.vertex_alloc_);
  std::swap(edge_alloc_, other.edge_alloc_);
  // Note: Does NOT swap graph_value_ - that's handled by derived class
}
```

**Verification:**
- Compile and run tests
- All modification operations work

### Step 2.6: Move Friend Functions (CPO Customization Points)

**Location:** `include/graph/container/undirected_adjacency_list.hpp`

**Critical:** Move ALL friend functions EXCEPT `graph_value()` CPO to base class.

**Strategy:**
1. Find all friend functions in general template (around lines 1600-2100)
2. Move each to base class
3. Update `requires` clauses to use `std::derived_from<std::remove_cvref_t<G>, base_undirected_adjacency_list>`

**Example - vertices CPO:**

**In base_undirected_adjacency_list:**

```cpp
  // Vertex operations
  template <typename G>
  requires std::derived_from<std::remove_cvref_t<G>, base_undirected_adjacency_list>
  [[nodiscard]] friend constexpr auto vertices(G&& g) noexcept {
    return vertex_descriptor_view<G>(std::forward<G>(g).vertices_);
  }
  
  template <typename G>
  requires std::derived_from<std::remove_cvref_t<G>, base_undirected_adjacency_list>
  [[nodiscard]] friend constexpr auto find_vertex(G&& g, vertex_id_type key) {
    auto it = std::forward<G>(g).try_find_vertex(key);
    if (it == std::forward<G>(g).end())
      return vertex_descriptor<G>();
    return vertex_descriptor<G>(it, std::forward<G>(g).vertices_);
  }
```

**Friend functions to move (approximately 40+ functions):**
- `vertices(g)`
- `find_vertex(g, key)`
- `vertex_id(g, u)`
- `edges(g, u)`
- `degree(g, u)`
- `target(g, uv)` / `target_id(g, uv)`
- `source(g, uv)` / `source_id(g, uv)`
- `edge_value(g, uv)` (with `requires !std::is_void_v<EV>`)
- `vertex_value(g, u)` (with `requires !std::is_void_v<VV>`)
- `contains_edge(g, uid, vid)`
- `num_vertices(g)`
- `num_edges(g)`
- All other CPO friend functions

**DO NOT MOVE:**
- `graph_value(g)` - This stays in general template derived class

**Verification:**
- Compile and check for friend function resolution
- Run tests to verify CPO functionality
- Test both general template and GV=void versions (if already updated)

### Step 2.7: Compile and Test Phase 2

**Actions:**
1. Build: `cmake --build build/linux-gcc-debug --target graph3_tests`
2. Run tests: `./build/linux-gcc-debug/tests/graph3_tests`
3. Verify: All 3866 tests pass

**Git Checkpoint:**
```bash
git add -A
git commit -m "Phase 2: Move all methods to base class

- Moved all constructors (except GV-taking ones)
- Moved all accessor methods
- Moved all creation methods (vertices, edges)
- Moved all modification methods (erase, clear, swap)
- Moved all friend functions (except graph_value CPO)

All tests passing (3866/3866)"
```

---

## Phase 3: Update Derived Classes

**Goal:** Make general template and GV=void specialization thin wrappers around base.

**Estimated Time:** 2-3 hours  
**Risk Level:** Medium  
**Rollback Strategy:** Git commit after each derived class update

### Step 3.1: Refactor General Template to Thin Wrapper

**Location:** `include/graph/container/undirected_adjacency_list.hpp`

**Current state:** General template has ~1500 lines with all implementation  
**Target state:** General template has ~80 lines with only GV-specific code

#### 3.1.1: Keep Only GV-Specific Members

**In general template undirected_adjacency_list:**

```cpp
template <typename VV, typename EV, typename GV, integral VId,
          template <typename V, typename A> class VContainer, typename Alloc>
class undirected_adjacency_list 
    : public base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc> {
public:
  using base_type = base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;
  using graph_type = undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;
  using graph_value_type = GV;
  using allocator_type = typename base_type::allocator_type;
  
  // Import base class type aliases (for backward compatibility)
  using typename base_type::vertex_type;
  using typename base_type::vertex_iterator;
  using typename base_type::const_vertex_iterator;
  using typename base_type::vertex_id_type;
  using typename base_type::edge_type;
  using typename base_type::edge_iterator;
  using typename base_type::const_edge_iterator;
  // ... other commonly used types
  
private:
  [[no_unique_address]] GV graph_value_{};  // ONLY additional member
  
public:
  // Constructors - see Step 3.1.2
  
  // Graph value accessors - see Step 3.1.3
  
  // Graph value CPO - see Step 3.1.4
};
```

**Remove from general template:**
- All methods now in base class (they're inherited)
- All friend functions except `graph_value()` CPO (they're in base)
- All nested classes (they're in base)
- All private members except `graph_value_` (they're in base)

#### 3.1.2: Add Constructors to General Template

**In general template:**

```cpp
public: // Construction/Destruction/Assignment
  // Default/copy/move
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
  
  // Forwarding constructors: range-based WITH GV
  template <typename ERng, typename VRng, typename EProj = std::identity, 
            typename VProj = std::identity>
    requires ranges::forward_range<ERng> 
          && ranges::input_range<VRng>
          && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
          && std::regular_invocable<VProj, ranges::range_reference_t<VRng>>
  undirected_adjacency_list(const ERng&  erng,
                            const VRng&  vrng,
                            const EProj& eproj,
                            const VProj& vproj,
                            const GV&    gv,
                            const Alloc& alloc = Alloc())
      : base_type(erng, vrng, eproj, vproj, alloc), graph_value_(gv) {}
  
  template <typename ERng, typename EProj = std::identity>
    requires ranges::forward_range<ERng>
          && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
  undirected_adjacency_list(const ERng&  erng,
                            const EProj& eproj,
                            const GV&    gv,
                            const Alloc& alloc = Alloc())
      : base_type(erng, eproj, alloc), graph_value_(gv) {}
  
  // Forwarding constructors: range-based WITHOUT GV
  template <typename ERng, typename VRng, typename EProj = std::identity, 
            typename VProj = std::identity>
    requires ranges::forward_range<ERng> 
          && ranges::input_range<VRng>
          && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
          && std::regular_invocable<VProj, ranges::range_reference_t<VRng>>
  undirected_adjacency_list(const ERng&  erng,
                            const VRng&  vrng,
                            const EProj& eproj = {},
                            const VProj& vproj = {},
                            const Alloc& alloc = Alloc())
      : base_type(erng, vrng, eproj, vproj, alloc) {}
  
  template <typename ERng, typename EProj = std::identity>
    requires ranges::forward_range<ERng>
          && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
  undirected_adjacency_list(const ERng&  erng,
                            const EProj& eproj = {},
                            const Alloc& alloc = Alloc())
      : base_type(erng, eproj, alloc) {}
  
  // Forwarding constructors: initializer_list
  undirected_adjacency_list(
    const initializer_list<tuple<vertex_id_type, vertex_id_type, edge_value_type>>& ilist,
    const Alloc& alloc = Alloc())
      : base_type(ilist, alloc) {}
  
  undirected_adjacency_list(
    const initializer_list<tuple<vertex_id_type, vertex_id_type>>& ilist,
    const Alloc& alloc = Alloc())
      : base_type(ilist, alloc) {}
```

**Note:** All constructors either initialize `graph_value_` or forward to base.

#### 3.1.3: Add Graph Value Accessors

**In general template:**

```cpp
public: // Graph Value Accessors
  constexpr GV& graph_value() noexcept { 
    return graph_value_; 
  }
  
  constexpr const GV& graph_value() const noexcept { 
    return graph_value_; 
  }
```

#### 3.1.4: Add Graph Value CPO Friend Function

**In general template:**

```cpp
  // Graph value CPO (UNIQUE to general template)
  template <typename G>
  requires std::derived_from<std::remove_cvref_t<G>, undirected_adjacency_list>
  [[nodiscard]] friend constexpr decltype(auto) graph_value(G&& g) noexcept {
    return std::forward<G>(g).graph_value();
  }
```

**Verification:**
- General template compiles
- All tests pass for general template
- Graph value accessible

### Step 3.2: Refactor GV=void Specialization to Thin Wrapper

**Location:** `include/graph/container/undirected_adjacency_list.hpp`

**Current state:** GV=void specialization has ~800 lines of duplicated code  
**Target state:** GV=void specialization has ~60 lines with forwarding constructors only

#### 3.2.1: Replace Specialization with Thin Wrapper

**Find the GV=void specialization (around line 1800+):**

```cpp
template <typename VV, typename EV, integral VId,
          template <typename V, typename A> class VContainer, typename Alloc>
class undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc> {
  // Currently ~800 lines of duplicated code
```

**Replace entire specialization with:**

```cpp
///-------------------------------------------------------------------------------------
/// undirected_adjacency_list<VV, EV, void, ...> - GV=void Specialization
///
/// @brief Specialization for graphs without graph-level value.
///
/// This specialization inherits all functionality from base_undirected_adjacency_list.
/// It differs from the general template only by:
/// - No graph_value_ member
/// - No graph_value() accessor methods
/// - No graph_value() CPO friend function
///-------------------------------------------------------------------------------------
template <typename VV, typename EV, integral VId,
          template <typename V, typename A> class VContainer, typename Alloc>
class undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>
    : public base_undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc> {
public:
  using base_type = base_undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>;
  using graph_type = undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>;
  using graph_value_type = void;
  using allocator_type = typename base_type::allocator_type;
  
  // Import base class type aliases (for backward compatibility)
  using typename base_type::vertex_type;
  using typename base_type::vertex_iterator;
  using typename base_type::const_vertex_iterator;
  using typename base_type::vertex_id_type;
  using typename base_type::edge_type;
  using typename base_type::edge_iterator;
  using typename base_type::const_edge_iterator;
  using typename base_type::vertex_edge_iterator;
  using typename base_type::const_vertex_edge_iterator;
  using typename base_type::vertex_value_type;
  using typename base_type::edge_value_type;
  using typename base_type::edge_size_type;
  using typename base_type::vertex_set;
  using typename base_type::edge_range;
  using typename base_type::const_edge_range;
  using typename base_type::vertex_edge_range;
  using typename base_type::const_vertex_edge_range;
  
  // No graph_value_ member
  
public: // Construction/Destruction/Assignment
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
    requires ranges::forward_range<ERng> 
          && ranges::input_range<VRng>
          && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
          && std::regular_invocable<VProj, ranges::range_reference_t<VRng>>
  undirected_adjacency_list(const ERng&  erng,
                            const VRng&  vrng,
                            const EProj& eproj = {},
                            const VProj& vproj = {},
                            const Alloc& alloc = Alloc())
      : base_type(erng, vrng, eproj, vproj, alloc) {}
  
  template <typename ERng, typename EProj = std::identity>
    requires ranges::forward_range<ERng>
          && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
  undirected_adjacency_list(const ERng&  erng,
                            const EProj& eproj = {},
                            const Alloc& alloc = Alloc())
      : base_type(erng, eproj, alloc) {}
  
  // Forwarding constructors: initializer_list
  undirected_adjacency_list(
    const initializer_list<tuple<vertex_id_type, vertex_id_type, edge_value_type>>& ilist,
    const Alloc& alloc = Alloc())
      : base_type(ilist, alloc) {}
  
  undirected_adjacency_list(
    const initializer_list<tuple<vertex_id_type, vertex_id_type>>& ilist,
    const Alloc& alloc = Alloc())
      : base_type(ilist, alloc) {}
  
  // Note: No graph_value() methods
  // Note: No graph_value() CPO friend function
};
```

**Verification:**
- GV=void specialization compiles
- All tests pass for GV=void specialization

### Step 3.3: Remove Duplicate Implementations from Impl File

**Location:** `include/graph/container/detail/undirected_adjacency_list_impl.hpp`

**Action:** Remove all implementations for GV=void specialization (they now come from base).

**Keep:**
- Base class implementations (already moved in Phase 2)
- General template implementations (thin wrapper code)

**Remove:**
- All GV=void specialization implementations (~500-600 lines)

**Verification:**
- Project compiles
- No linker errors

### Step 3.4: Compile and Test Phase 3

**Actions:**
1. Build: `cmake --build build/linux-gcc-debug --target graph3_tests`
2. Run tests: `./build/linux-gcc-debug/tests/graph3_tests`
3. Verify: All 3866 tests pass

**Git Checkpoint:**
```bash
git add -A
git commit -m "Phase 3: Convert derived classes to thin wrappers

- Refactored general template to ~80 lines (only GV-specific code)
- Refactored GV=void specialization to ~60 lines (forwarding constructors)
- Removed ~1300 lines of duplicated code
- All functionality now inherited from base class

All tests passing (3866/3866)"
```

---

## Phase 4: Cleanup and Final Verification

**Goal:** Polish implementation, verify correctness, update documentation.

**Estimated Time:** 1-2 hours  
**Risk Level:** Low  
**Rollback Strategy:** Git commit after completion

### Step 4.1: Review Base Class for GV Dependencies

**Action:** Search base class for any references to `graph_value_`

**Command:**
```bash
grep -n "graph_value_" include/graph/container/undirected_adjacency_list.hpp | grep -v "graph_value_type"
```

**Expected:** No matches in base class (except in comments)

**If found:** Move that code to derived classes

### Step 4.2: Verify `[[no_unique_address]]` Attribute

**Location:** General template class

**Verify:**
```cpp
private:
  [[no_unique_address]] GV graph_value_{};
```

**Purpose:** Eliminates space overhead when GV is an empty type (e.g., `std::monostate`)

### Step 4.3: Add/Update Documentation

**Location:** `include/graph/container/undirected_adjacency_list.hpp`

**Update base class documentation:**
- Clarify purpose of base class
- Document what goes in base vs derived
- Add examples of usage

**Update general template documentation:**
- Note inheritance from base
- Clarify GV-specific functionality

**Update GV=void specialization documentation:**
- Note inheritance from base
- Clarify differences from general template

### Step 4.4: Run Full Test Suite

**Actions:**
1. Clean build: `rm -rf build/linux-gcc-debug && mkdir -p build/linux-gcc-debug`
2. Configure: `cmake -S . -B build/linux-gcc-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug`
3. Build: `cmake --build build/linux-gcc-debug --target graph3_tests`
4. Run tests: `./build/linux-gcc-debug/tests/graph3_tests`
5. Verify: All 3866 tests pass, 36004 assertions pass

**Expected Results:**
- Compilation: Success (0 errors, minimal warnings)
- Tests: 100% pass rate (3866/3866 test cases)
- Performance: No regression (inheritance is zero-cost)

### Step 4.5: Verify Binary Size

**Action:** Compare binary size before/after refactoring

**Commands:**
```bash
# If you have a pre-refactoring binary saved
ls -lh build/linux-gcc-debug/tests/graph3_tests

# Should be same or smaller (no increase due to templates)
```

**Expected:** Similar or slightly smaller binary size (code deduplication may help linker)

### Step 4.6: Verify CPO Resolution

**Action:** Test that all CPOs resolve correctly via ADL

**Test cases to verify:**
- `vertices(g)` - works for both general and GV=void
- `edges(g, u)` - works for both versions
- `graph_value(g)` - only works for general template (compile error for GV=void)
- `edge_value(g, uv)` - only works when EV != void
- `vertex_value(g, u)` - only works when VV != void

**Verification method:** Tests already cover this, but do spot checks if needed

### Step 4.7: Final Git Commit

**Git Checkpoint:**
```bash
git add -A
git commit -m "Phase 4: Final cleanup and verification

- Verified no GV dependencies in base class
- Confirmed [[no_unique_address]] attribute
- Updated documentation for base and derived classes
- Full clean build and test: 3866/3866 tests passing
- Binary size unchanged (zero-cost abstraction verified)

Refactoring complete:
- Eliminated ~1300 lines of code duplication
- General template: ~80 lines (was ~1500)
- GV=void specialization: ~60 lines (was ~800)
- Base class: ~1500 lines (shared implementation)
- Total reduction: ~1100 lines

All functionality preserved, all tests passing."
```

---

## Summary and Verification Checklist

### Code Quality Metrics

**Before Refactoring:**
- General template: ~1500 lines
- GV=void specialization: ~800 lines
- Total: ~2300 lines
- Duplication: ~1300 lines (57%)

**After Refactoring:**
- Base class: ~1500 lines
- General template: ~80 lines
- GV=void specialization: ~60 lines
- Total: ~1640 lines
- Duplication: ~40 lines (2%)
- **Code reduction: ~660 lines (29% overall reduction)**
- **Duplication elimination: ~1260 lines (97% reduction in duplication)**

### Final Verification Checklist

- [ ] All 3866 tests pass (100% pass rate)
- [ ] All 36004 assertions pass
- [ ] No compilation errors
- [ ] No linker errors
- [ ] Binary size unchanged or smaller
- [ ] CPO ADL resolution works correctly
- [ ] `graph_value(g)` only works for general template
- [ ] `graph_value(g)` compile error for GV=void (expected)
- [ ] Base class has no `graph_value_` dependencies
- [ ] `[[no_unique_address]]` attribute present for `graph_value_`
- [ ] Documentation updated
- [ ] All phases committed to git
- [ ] Clean build successful

### Testing Matrix

| Test Category | General Template | GV=void Specialization |
|---------------|------------------|------------------------|
| Construction | ✓ Pass | ✓ Pass |
| Copy/Move | ✓ Pass | ✓ Pass |
| Vertex Access | ✓ Pass | ✓ Pass |
| Edge Access | ✓ Pass | ✓ Pass |
| Vertex Creation | ✓ Pass | ✓ Pass |
| Edge Creation | ✓ Pass | ✓ Pass |
| Edge Removal | ✓ Pass | ✓ Pass |
| Graph Clear | ✓ Pass | ✓ Pass |
| CPO Resolution | ✓ Pass | ✓ Pass |
| Graph Value | ✓ Pass | ⊗ N/A (void) |

### Performance Verification

- [ ] No virtual functions added (zero vtable overhead)
- [ ] Inheritance is static (zero runtime dispatch)
- [ ] `[[no_unique_address]]` eliminates empty GV overhead
- [ ] Same performance as before refactoring

### Success Criteria Met

- ✓ All tests passing
- ✓ Zero code duplication
- ✓ Thin derived classes (<100 lines each)
- ✓ Base class contains all shared logic
- ✓ Pattern consistent with `dynamic_graph`
- ✓ Zero-cost abstraction maintained
- ✓ Public API unchanged

---

## Rollback Plan

If at any phase the refactoring causes issues:

1. **Identify failing phase:** Check which phase introduced the problem
2. **Rollback to previous commit:**
   ```bash
   git log --oneline  # Find last working commit
   git reset --hard <commit-hash>
   ```
3. **Analyze failure:** Determine root cause
4. **Fix and retry:** Address issue and retry phase
5. **Alternative:** Abandon refactoring and keep current implementation

---

## Post-Implementation Notes

**For Future Maintainers:**

1. **Adding new methods:** Add to base class (shared by both versions)
2. **Adding GV-specific functionality:** Add to general template only
3. **Adding new specializations:** Create new derived class inheriting from base
4. **Modifying base logic:** Test both general and GV=void versions
5. **CPO friend functions:** Add to base class (except `graph_value` CPO)

**Common Pitfalls:**

1. Don't add `graph_value_` references in base class
2. Remember `graph_value()` CPO only in general template
3. Use `std::derived_from<..., base_undirected_adjacency_list>` in friend functions
4. Don't forget `[[no_unique_address]]` for `graph_value_`
5. Test both versions after base class changes

---

## Other Tasks
- [ ] Review member function names (e.g. uv.value() vs. uv.edge_value()).
- [ ] Support u.value() (or u.vertex_value()).
- [ ] Only member functions will be used for CPOs to verify CPOs can adapt to native types & functions.
- [ ] All public member functions have unit tests for them
- [ ] Remove extraneous functionality (e.g. neighbor function range on vertex)

---

## Conclusion

This implementation plan provides a safe, incremental approach to refactoring `undirected_adjacency_list` with a base class pattern. Each phase is independently verifiable, with git commits serving as checkpoints. The final result eliminates significant code duplication while maintaining 100% backward compatibility and zero runtime overhead.

**Estimated Total Time:** 8-12 hours  
**Expected Code Reduction:** ~660 lines (29% overall, 97% duplication elimination)  
**Risk Level:** Low-Medium (incremental, well-tested approach)  
**Benefit:** Significantly improved maintainability and code clarity
