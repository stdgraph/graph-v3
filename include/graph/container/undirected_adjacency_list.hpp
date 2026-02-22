//
//	Author: J. Phillip Ratzloff
//
#include "../adj_list/graph_utility.hpp"
#include "../graph_info.hpp"
#include "../adj_list/vertex_descriptor_view.hpp"
#include "../adj_list/edge_descriptor_view.hpp"
#include "../adj_list/descriptor_traits.hpp"
#include "container_utility.hpp"
#include <vector>
#include <ranges>
#include <cassert>
#include <type_traits>
#include <cstdint>
#include <limits>
#include <initializer_list>
#include <utility>

#ifndef UNDIRECTED_ADJ_LIST_HPP
#  define UNDIRECTED_ADJ_LIST_HPP

#  define CPO 1
namespace graph::container {

// Import types from graph::adj_list for convenience
using adj_list::vertex_descriptor;
using adj_list::edge_descriptor;
using adj_list::vertex_descriptor_view;
using adj_list::edge_descriptor_view;
using adj_list::vertex_descriptor_type;
using adj_list::edge_descriptor_type;

using std::vector;
using std::allocator;
using std::numeric_limits;
using std::bidirectional_iterator_tag;
using std::initializer_list;
using std::tuple;
using std::pair;
namespace ranges = std::ranges;


///-------------------------------------------------------------------------------------
/// undirected_adjacency_list - Undirected Graph Container using Dual-List Design
///
/// @brief An efficient undirected graph implementation where edges are stored in
///        bidirectional doubly-linked lists at both vertices.
///
/// DESIGN OVERVIEW:
/// ----------------
/// This container uses a "dual-list" design where each undirected edge is physically
/// stored in two doubly-linked lists - one at each incident vertex. This provides O(1)
/// edge removal from both endpoints and efficient iteration of incident edges.
///
/// Vertices: Stored in a contiguous random-access container (default: std::vector)
///   - Provides O(1) vertex access by id/index
///   - Each vertex maintains a doubly-linked list of incident edges
///   - Vertex values stored inline (optional, use void for no value)
///
/// Edges: Each edge appears in two edge lists (one per endpoint)
///   - Allocated individually on heap (via allocator)
///   - Each edge stores pointers to form doubly-linked list at both vertices
///   - O(1) removal from both vertices' edge lists
///   - Edge values stored inline (optional, use void for no value)
///
/// MEMORY OVERHEAD:
/// ----------------
/// Per vertex: ~24-32 bytes (list head pointers, value)
/// Per edge: ~48-64 bytes (4 list pointers, 2 vertex ids, value, allocation overhead)
/// Total for edge: 2× list nodes (one at each vertex)
///
/// COMPLEXITY GUARANTEES:
/// ----------------------
/// - Vertex access: O(1)
/// - Add vertex: O(1) amortized
/// - Add edge: O(1)
/// - Remove edge: O(degree) to find + O(1) to unlink from both lists
/// - Degree query: O(1) (cached)
/// - Iterate edges from vertex: O(degree)
/// - Iterate all edges: O(V + E)
///
/// ITERATION SEMANTICS:
/// --------------------
/// - Vertex iteration: Each vertex visited exactly once
/// - Edge iteration (graph-level): Each edge visited TWICE (once from each endpoint)
///   This is fundamental to the undirected design - use `edges_size() / 2` for unique count
/// - Edge iteration (vertex-level): Each incident edge visited once
///
/// INTERFACE CONFORMANCE:
/// ----------------------
/// Satisfies Graph Container Interface requirements:
/// - vertex_range<G>, adjacency_list<G>
/// - Provides 47+ CPO functions (vertices, edges, degree, find_vertex, etc.)
/// - Full const-correctness with const/non-const overloads
/// - C++20 iterator concepts (bidirectional/forward iterators)
///
/// THREAD SAFETY:
/// --------------
/// NOT thread-safe. External synchronization required for:
/// - Concurrent writes (add/remove vertices/edges)
/// - Concurrent read + write operations
/// Concurrent reads from multiple threads are safe if no writes occur.
///
/// WHEN TO USE:
/// ------------
/// Best for:
/// - Undirected graphs with frequent edge removal
/// - Algorithms needing fast neighbor iteration
/// - Graphs where edge count dominates vertex count
/// - Graphs with moderate vertex degrees
/// - Edges with many properties (value storage amortizes overhead)
/// - Edges with property updates (changes made in one place)
///
/// Consider alternatives when:
/// - Memory overhead is critical (use compressed_graph for read-only)
/// - Vertex degrees are very high (> 1000s of edges)
/// - Graph is read-only after construction (use compressed_graph)
/// - Need directed edges (use dynamic_graph instead)
///
/// EXAMPLE USAGE:
/// --------------
/// @code
/// using Graph = undirected_adjacency_list<int, string>;  // edge_value=int, vertex_value=string
///
/// Graph g;
/// auto u = g.create_vertex("Alice");
/// auto v = g.create_vertex("Bob");
/// auto uv = g.create_edge(u, v, 100);  // edge from Alice to Bob with value 100
///
/// // Iterate neighbors
/// for (auto&& [uid, vid, uv] : g.edges(u)) {
///   cout << "Edge to " << g[vid].value << " with weight " << uv.value << "\n";
/// }
/// @endcode
///
/// @tparam EV Edge Value type (default: void for no value)
/// @tparam VV Vertex Value type (default: void for no value)
/// @tparam GV Graph Value type (default: void for no value)
/// @tparam VId Vertex id/index type (default: uint32_t)
/// @tparam VContainer Vertex storage container template (default: std::vector)
/// @tparam Alloc Allocator type (default: std::allocator<char>)
///-------------------------------------------------------------------------------------

template <typename EV                                        = void,
          typename VV                                        = void,
          typename GV                                        = void,
          integral VId                                       = uint32_t,
          template <typename V, typename A> class VContainer = vector,
          typename Alloc                                     = allocator<char>>
class undirected_adjacency_list;

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
class ual_vertex;

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
class base_undirected_adjacency_list;

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
class ual_edge;

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
class ual_const_neighbor_iterator;

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
class ual_neighbor_iterator;


///-------------------------------------------------------------------------------------

// designator types
struct inward_list;
struct outward_list;

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
class ual_vertex_edge_list;

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename A,
          typename ListT>
class ual_vertex_edge_list_link;

///-------------------------------------------------------------------------------------
/// ual_edge_value
///
/// @brief Value class for undirected_adjacency_list edges with template specialization for void.
///
/// This class stores the user-defined edge value. A specialization for EV=void provides
/// zero memory overhead when no edge value is needed.
///
/// @tparam EV Edge value type
/// @tparam VV Vertex value type
/// @tparam GV Graph value type
/// @tparam VId Vertex ID type
/// @tparam VContainer Container template for vertices
/// @tparam Alloc Allocator type
///
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
class ual_edge_value {
public:
  using edge_type  = ual_edge<EV, VV, GV, VId, VContainer, Alloc>;
  using value_type = EV;
  using graph_type = undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>;

public:
  constexpr ual_edge_value(const value_type& val) : value_(val) {}
  constexpr ual_edge_value(value_type&& val) : value_(std::move(val)) {}

  constexpr ual_edge_value()                      = default;
  constexpr ual_edge_value(const ual_edge_value&) = default;
  constexpr ual_edge_value(ual_edge_value&&)      = default;
  constexpr ~ual_edge_value()                     = default;

  constexpr ual_edge_value& operator=(const ual_edge_value&) = default;
  constexpr ual_edge_value& operator=(ual_edge_value&&)      = default;

  constexpr value_type&       value() noexcept { return value_; }
  constexpr const value_type& value() const noexcept { return value_; }

private: // CPO support via ADL (friend functions)
  // edge_value(g, e) - get edge value (only when EV is not void)
  friend constexpr decltype(auto) edge_value(graph_type&, edge_type& e) noexcept { return e.value(); }
  friend constexpr decltype(auto) edge_value(const graph_type&, const edge_type& e) noexcept { return e.value(); }

private:
  value_type value_ = value_type();
};

///-------------------------------------------------------------------------------------
/// ual_edge_value<void, ...> specialization
///
/// @brief Specialization for EV=void that provides zero memory overhead.
///
template <typename VV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
class ual_edge_value<void, VV, GV, VId, VContainer, Alloc> {};

///-------------------------------------------------------------------------------------
/// ual_vertex_value
///
/// @brief Value class for undirected_adjacency_list vertices with template specialization for void.
///
/// This class stores the user-defined vertex value. A specialization for VV=void provides
/// zero memory overhead when no vertex value is needed.
///
/// @tparam EV Edge value type
/// @tparam VV Vertex value type
/// @tparam GV Graph value type
/// @tparam VId Vertex ID type
/// @tparam VContainer Container template for vertices
/// @tparam Alloc Allocator type
///
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
class ual_vertex_value {
public:
  using vertex_type = ual_vertex<EV, VV, GV, VId, VContainer, Alloc>;
  using value_type  = VV;
  using graph_type  = undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>;

public:
  constexpr ual_vertex_value(const value_type& val) : value_(val) {}
  constexpr ual_vertex_value(value_type&& val) : value_(std::move(val)) {}

  constexpr ual_vertex_value()                        = default;
  constexpr ual_vertex_value(const ual_vertex_value&) = default;
  constexpr ual_vertex_value(ual_vertex_value&&)      = default;
  constexpr ~ual_vertex_value()                       = default;

  constexpr ual_vertex_value& operator=(const ual_vertex_value&) = default;
  constexpr ual_vertex_value& operator=(ual_vertex_value&&)      = default;

  constexpr value_type&       value() noexcept { return value_; }
  constexpr const value_type& value() const noexcept { return value_; }

private: // CPO support via ADL (friend functions)
  // vertex_value(g, u) - get vertex value (only when VV is not void)
  template <typename U>
  requires vertex_descriptor_type<U>
  friend constexpr decltype(auto) vertex_value(graph_type& g, const U& u) noexcept {
    auto& vtx = u.inner_value(g.vertices());
    return vtx.value();
  }
  template <typename U>
  requires vertex_descriptor_type<U>
  friend constexpr decltype(auto) vertex_value(const graph_type& g, const U& u) noexcept {
    const auto& vtx = u.inner_value(g.vertices());
    return vtx.value();
  }

private:
  value_type value_ = value_type();
};

///-------------------------------------------------------------------------------------
/// ual_vertex_value<void, ...> specialization
///
/// @brief Specialization for VV=void that provides zero memory overhead.
///
template <typename EV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
class ual_vertex_value<EV, void, GV, VId, VContainer, Alloc> {};


///-------------------------------------------------------------------------------------
/// ual_vertex_edge_list
///
/// @tparam EV     Edge Value type. default = void.
/// @tparam VV     Vertex Value type. default = void.
/// @tparam GV     Graph Value type. default = void.
/// @tparam IntexT The type used for vertex & edge index into the internal vectors.
/// @tparam A      Allocator. default = std::allocator
/// @tparam ListT  inward_list|outward_list. Which edge list this is for.
///
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
class ual_vertex_edge_list {
public:
  class iterator;
  class const_iterator;

  using graph_type = undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>;

  using vertex_type           = ual_vertex<EV, VV, GV, VId, VContainer, Alloc>;
  using vertex_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set            = VContainer<vertex_type, vertex_allocator_type>;
  using vertex_iterator       = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;
  using vertex_id_type        = VId;
  using vertex_index          = VId;

  using edge_value_type = EV;
  using edge_type       = ual_edge<EV, VV, GV, VId, VContainer, Alloc>;

  using edge_list_type                    = ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>;
  using vertex_edge_list_inward_link_type = ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, inward_list>;
  using vertex_edge_list_outward_link_type =
        ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, outward_list>;
  using edge_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<edge_type>;

  using value_type      = edge_type;
  using pointer         = value_type*;
  using const_pointer   = value_type const*;
  using reference       = value_type&;
  using const_reference = const value_type&;
  using size_type       = size_t;
  using difference_type = ptrdiff_t;

  using edge_range       = ranges::subrange<iterator, iterator, ranges::subrange_kind::sized>;
  using const_edge_range = ranges::subrange<const_iterator, const_iterator, ranges::subrange_kind::sized>;

public:
  class const_iterator {
  public:
    using iterator_category = bidirectional_iterator_tag;
    using iterator_concept  = std::bidirectional_iterator_tag;
    using value_type        = ual_vertex_edge_list::edge_type;
    using difference_type   = ual_vertex_edge_list::difference_type;
    using size_type         = ual_vertex_edge_list::size_type;
    using pointer           = ual_vertex_edge_list::const_pointer;
    using const_pointer     = ual_vertex_edge_list::const_pointer;
    using reference         = ual_vertex_edge_list::const_reference;
    using const_reference   = ual_vertex_edge_list::const_reference;

    using graph_type  = ual_vertex_edge_list::graph_type;
    using vertex_type = ual_vertex_edge_list::vertex_type;
    using edge_type   = ual_vertex_edge_list::edge_type;

    const_iterator(const graph_type& g, vertex_id_type uid, const edge_type* uv = nullptr) noexcept
          : vertex_id_(uid), edge_(const_cast<edge_type*>(uv)), graph_(const_cast<graph_type*>(&g)) {}

    const_iterator() noexcept                          = default;
    const_iterator(const const_iterator& rhs) noexcept = default;
    const_iterator(const_iterator&&) noexcept          = default;
    ~const_iterator() noexcept                         = default;

    const_iterator& operator=(const const_iterator&) noexcept = default;
    const_iterator& operator=(const_iterator&&) noexcept      = default;

    reference operator*() const;
    pointer   operator->() const;

    const_iterator& operator++();
    const_iterator  operator++(int);

    const_iterator& operator--();
    const_iterator  operator--(int);

    bool operator==(const const_iterator& rhs) const noexcept;
    bool operator!=(const const_iterator& rhs) const noexcept;

    friend void swap(const_iterator& lhs, const_iterator& rhs) noexcept {
      swap(lhs.vertex_id_, rhs.vertex_id_);
      swap(lhs.edge_, rhs.edge_);
    }

    graph_type&       graph() { return *graph_; }
    const graph_type& graph() const { return *graph_; }

    vertex_id_type source_id() const { return vertex_id_; }

  protected:
    void advance();
    void retreat();

  protected:
    vertex_id_type vertex_id_ = numeric_limits<vertex_id_type>::max(); // source vertex for the list we're in
    edge_type*     edge_      = nullptr;                               // current edge (==nullptr for end)
    graph_type*    graph_     = nullptr;
  }; // end const_iterator

  class iterator : public const_iterator {
  public:
    using base_t            = const_iterator;
    using iterator_category = typename base_t::iterator_category;
    using value_type        = ual_vertex_edge_list::edge_type;
    using difference_type   = ual_vertex_edge_list::difference_type;
    using size_type         = ual_vertex_edge_list::size_type;
    using pointer           = ual_vertex_edge_list::pointer;
    using const_pointer     = ual_vertex_edge_list::const_pointer;
    using reference         = ual_vertex_edge_list::reference;
    using const_reference   = ual_vertex_edge_list::const_reference;

    using graph_type  = ual_vertex_edge_list::graph_type;
    using vertex_type = ual_vertex_edge_list::vertex_type;
    using edge_type   = ual_vertex_edge_list::edge_type;

    iterator() noexcept                = default;
    iterator(const iterator&) noexcept = default;
    iterator(iterator&&) noexcept      = default;
    ~iterator() noexcept               = default;

    iterator& operator=(const iterator&) = default;
    iterator& operator=(iterator&&)      = default;

    iterator(const graph_type& g, vertex_id_type uid, const edge_type* uv = nullptr) : const_iterator(g, uid, uv) {}

    reference operator*() const;
    pointer   operator->() const;

    iterator& operator++();
    iterator  operator++(int);

    iterator& operator--();
    iterator  operator--(int);

    friend void swap(iterator& lhs, iterator& rhs) {
      swap(lhs.vertex_id_, rhs.vertex_id_);
      swap(lhs.edge_, rhs.edge_);
    }
  }; // end iterator

public:
  ual_vertex_edge_list() noexcept                                       = default;
  ual_vertex_edge_list(const ual_vertex_edge_list&) noexcept            = default;
  ~ual_vertex_edge_list() noexcept                                      = default;
  ual_vertex_edge_list& operator=(const ual_vertex_edge_list&) noexcept = default;

  ual_vertex_edge_list(ual_vertex_edge_list&& rhs) noexcept
        : head_(move(rhs.head_)), tail_(move(rhs.tail_)), size_(move(rhs.size_)) {
    rhs.head_ = rhs.tail_ = nullptr;
    rhs.size_             = 0;
  }
  ual_vertex_edge_list& operator=(ual_vertex_edge_list&& rhs) noexcept = default;

  size_type size() const noexcept;
  bool      empty() const noexcept;

  edge_type&       front();
  const edge_type& front() const;

  edge_type&       back();
  const edge_type& back() const;

public:
  template <typename ListT>
  void link_front(edge_type& uv, ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>& uv_link);

  template <typename ListT>
  void link_back(edge_type& uv, ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>& uv_link);

  template <typename ListT>
  void unlink(edge_type& uv, ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>& uv_link);

  iterator       begin(graph_type& g, vertex_id_type uid) noexcept;
  const_iterator begin(const graph_type& g, vertex_id_type uid) const noexcept;
  const_iterator cbegin(const graph_type& g, vertex_id_type uid) const noexcept;

  iterator       end(graph_type& g, vertex_id_type uid) noexcept;
  const_iterator end(const graph_type& g, vertex_id_type uid) const noexcept;
  const_iterator cend(const graph_type& g, vertex_id_type uid) const noexcept;

  edge_range       edges(graph_type& g, vertex_id_type uid) noexcept;
  const_edge_range edges(const graph_type& g, vertex_id_type uid) const noexcept;

private:
  edge_type* head_ = nullptr;
  edge_type* tail_ = nullptr;
  size_type  size_ = 0;
};

///-------------------------------------------------------------------------------------
/// ual_vertex_edge_list_link
///
/// @tparam EV     Edge Value type. default = void.
/// @tparam VV     Vertex Value type. default = void.
/// @tparam GV     Graph Value type. default = void.
/// @tparam IntexT The type used for vertex & edge index into the internal vectors.
/// @tparam A      Allocator. default = std::allocator
/// @tparam ListT  inward_list|outward_list. Which edge list this is for.
///
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc,
          typename ListT>
class ual_vertex_edge_list_link {
public:
  using graph_type = undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>;

  using vertex_type           = ual_vertex<EV, VV, GV, VId, VContainer, Alloc>;
  using vertex_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set            = VContainer<vertex_type, vertex_allocator_type>;
  using vertex_iterator       = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;
  using vertex_id_type        = VId;
  using vertex_index          = VId;

  using edge_type = ual_edge<EV, VV, GV, VId, VContainer, Alloc>;

  using edge_list_type      = ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>;
  using edge_list_link_type = ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>;

public:
  ual_vertex_edge_list_link(vertex_id_type uid) noexcept : vertex_id_(uid) {}

  ual_vertex_edge_list_link() noexcept                                            = default;
  ual_vertex_edge_list_link(const ual_vertex_edge_list_link&) noexcept            = default;
  ual_vertex_edge_list_link(ual_vertex_edge_list_link&&) noexcept                 = default;
  ~ual_vertex_edge_list_link() noexcept                                           = default;
  ual_vertex_edge_list_link& operator=(const ual_vertex_edge_list_link&) noexcept = default;
  ual_vertex_edge_list_link& operator=(ual_vertex_edge_list_link&&) noexcept      = default;

public:
  vertex_id_type        vertex_id() const noexcept { return vertex_id_; }
  const_vertex_iterator vertex(const graph_type& g) const { return g.vertices().begin() + vertex_id_; }
  vertex_iterator       vertex(graph_type& g) { return g.vertices().begin() + vertex_id_; }

  edge_type*       next() noexcept { return next_; }
  const edge_type* next() const noexcept { return next_; }
  edge_type*       prev() noexcept { return prev_; }
  const edge_type* prev() const noexcept { return prev_; }

private:
  vertex_id_type vertex_id_ = numeric_limits<vertex_id_type>::max();
  edge_type*     next_      = nullptr;
  edge_type*     prev_      = nullptr;

  friend edge_list_type;
  friend edge_type;
};


///-------------------------------------------------------------------------------------
/// ual_edge
///
/// @tparam EV     Edge Value type. default = void.
/// @tparam VV     Vertex Value type. default = void.
/// @tparam GV     Graph Value type. default = void.
/// @tparam IntexT The type used for vertex & edge index into the internal vectors.
/// @tparam A      Allocator. default = std::allocator
///
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
class ual_edge
      : public ual_edge_value<EV, VV, GV, VId, VContainer, Alloc>
      , public ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, inward_list>
      , public ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, outward_list> {
public:
  using graph_type       = undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>;
  using graph_value_type = GV;
  using base_value_type  = ual_edge_value<EV, VV, GV, VId, VContainer, Alloc>;

  using vertex_type           = ual_vertex<EV, VV, GV, VId, VContainer, Alloc>;
  using vertex_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set            = VContainer<vertex_type, vertex_allocator_type>;
  using vertex_iterator       = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;
  using vertex_id_type        = VId;
  using vertex_index          = VId;
  using vertex_value_type     = VV;

  using edge_id_type         = pair<vertex_id_type, vertex_id_type>;
  using edge_value_type      = EV;
  using edge_type            = ual_edge<EV, VV, GV, VId, VContainer, Alloc>;
  using edge_allocator_type  = typename allocator_traits<Alloc>::template rebind_alloc<edge_type>;
  using edge_index           = VId;
  using edge_size_type       = size_t;
  using edge_difference_type = ptrdiff_t;

  using edge_list_type                    = ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>;
  using vertex_edge_list_inward_link_type = ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, inward_list>;
  using vertex_edge_list_outward_link_type =
        ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, outward_list>;

protected:
  // noexcept is only defined for move ctor & assignment b/c the user-defined value type could
  // throw an exception in other cases
  ual_edge() noexcept                = default;
  ual_edge(const ual_edge&) noexcept = default;
  ual_edge(ual_edge&&) noexcept      = default;
  ~ual_edge() noexcept;
  ual_edge& operator=(ual_edge&) noexcept  = default;
  ual_edge& operator=(ual_edge&&) noexcept = default;

  ual_edge(graph_type&, vertex_id_type uid, vertex_id_type vid) noexcept;
  ual_edge(graph_type&, vertex_id_type uid, vertex_id_type vid, const edge_value_type&) noexcept;
  ual_edge(graph_type&, vertex_id_type uid, vertex_id_type vid, edge_value_type&&) noexcept;

  ual_edge(graph_type&, vertex_iterator ui, vertex_iterator vi) noexcept;
  ual_edge(graph_type&, vertex_iterator ui, vertex_iterator vi, const edge_value_type&) noexcept;
  ual_edge(graph_type&, vertex_iterator ui, vertex_iterator vi, edge_value_type&&) noexcept;

  void link_front(vertex_type&, vertex_type&) noexcept;
  void link_back(vertex_type&, vertex_type&) noexcept;
  void unlink(vertex_type&, vertex_type&) noexcept;

public:
  // Edge member methods for CPO resolution (iteration perspective)
  // These return vertex descriptors based on the edge's storage location in the graph
  vertex_iterator       source(graph_type&) noexcept;
  const_vertex_iterator source(const graph_type&) const noexcept;
  vertex_iterator       target(graph_type&) noexcept;
  const_vertex_iterator target(const graph_type&) const noexcept;

  // Internal storage-perspective methods (do NOT use for CPO resolution)
  //
  // IMPORTANT DISTINCTION:
  // - list_owner_id() returns the ID of the vertex that owns this edge in its edge list
  // - list_target_id() returns the ID of the other vertex (the target from the owner's perspective)
  //
  // These methods provide the STORAGE perspective of an edge, which is different from
  // the ITERATION perspective provided by the CPO functions source_id(g,e) and target_id(g,e).
  //
  // For undirected graphs:
  // - Each edge is stored in BOTH vertices' edge lists
  // - When iterating edges from vertex A, you get edges stored in A's list (pointing to B)
  //   AND edges stored in other vertices' lists (pointing to A)
  // - list_owner_id() tells you which list physically stores this edge instance
  // - list_target_id() tells you which vertex this instance points to
  //
  // When iterating from vertex 1:
  // - Edge (0,1) stored in vertex 0's list: list_owner_id()=0, list_target_id()=1
  // - Edge (1,2) stored in vertex 1's list: list_owner_id()=1, list_target_id()=2
  //
  // But the CPO source_id(g,e) should ALWAYS return 1 (the vertex we're iterating from),
  // regardless of which list the edge is stored in. This is why the CPO uses the edge
  // descriptor's source_id() method (iteration perspective) rather than the native edge's
  // list_owner_id() method (storage perspective).
  //
  // These methods were renamed from source_id()/target_id() to prevent the CPO's tier-1
  // resolution from incorrectly using them.
  vertex_id_type list_owner_id() const noexcept;
  vertex_id_type list_target_id() const noexcept;

  vertex_iterator       other_vertex(graph_type&, const_vertex_iterator other) noexcept;
  const_vertex_iterator other_vertex(const graph_type&, const_vertex_iterator other) const noexcept;
  vertex_iterator       other_vertex(graph_type&, vertex_id_type other_id) noexcept;
  const_vertex_iterator other_vertex(const graph_type&, vertex_id_type other_id) const noexcept;

  vertex_id_type other_vertex_id(const graph_type&, const_vertex_iterator other) const noexcept;
  vertex_id_type other_vertex_id(const graph_type&, vertex_id_type other_id) const noexcept;

  edge_id_type edge_id(const graph_type& g) const noexcept;

  friend graph_type; // the graph is the one to create & destroy edges because it owns the allocator
  friend base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>; // base class also creates edges
  friend vertex_type;                                                        // vertex can also destroy its own edges
  friend edge_list_type;                                                     // for delete, when clearing the list
};

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
/// @tparam EV Edge Value type
/// @tparam VV Vertex Value type
/// @tparam GV Graph Value type (may be void)
/// @tparam VId Vertex id/index type
/// @tparam VContainer Vertex storage container template
/// @tparam Alloc Allocator type
///-------------------------------------------------------------------------------------
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
class base_undirected_adjacency_list {
public: // Type Aliases
  using graph_type       = undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>;
  using graph_value_type = GV;
  using allocator_type   = Alloc;

  using vertex_type            = ual_vertex<EV, VV, GV, VId, VContainer, Alloc>;
  using vertex_value_type      = VV;
  using vertex_id_type         = VId;
  using vertex_index_type      = VId;
  using vertex_allocator_type  = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set             = VContainer<vertex_type, vertex_allocator_type>;
  using vertex_size_type       = typename vertex_set::size_type;
  using vertex_difference_type = typename vertex_set::difference_type;

  using vertex_iterator       = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;
  using vertex_range          = vertex_set&;
  using const_vertex_range    = const vertex_set&;

  using edge_type            = ual_edge<EV, VV, GV, VId, VContainer, Alloc>;
  using edge_value_type      = EV;
  using edge_allocator_type  = typename allocator_traits<Alloc>::template rebind_alloc<edge_type>;
  using edge_id_type         = pair<vertex_id_type, vertex_id_type>; // <from,to>
  using edge_size_type       = typename edge_type::edge_size_type;
  using edge_difference_type = typename edge_type::edge_difference_type;

  using vertex_edge_iterator        = typename vertex_type::vertex_edge_iterator;
  using const_vertex_edge_iterator  = typename vertex_type::const_vertex_edge_iterator;
  using vertex_edge_range           = typename vertex_type::vertex_edge_range;
  using const_vertex_edge_range     = typename vertex_type::const_vertex_edge_range;
  using vertex_edge_size_type       = typename vertex_edge_iterator::size_type;
  using vertex_edge_difference_type = typename vertex_edge_iterator::difference_type;

  using neighbor_iterator       = typename vertex_type::neighbor_iterator;
  using const_neighbor_iterator = typename vertex_type::const_neighbor_iterator;
  using neighbor_range          = typename vertex_type::neighbor_range;
  using const_neighbor_range    = typename vertex_type::const_neighbor_range;
  using neighbor_size_type      = typename vertex_type::neighbor_size_type;

  // Edge iterators (nested classes)
  class edge_iterator;       // (defined below)
  class const_edge_iterator; // (defined below)
  using edge_range       = ranges::subrange<edge_iterator, edge_iterator, ranges::subrange_kind::sized>;
  using const_edge_range = ranges::subrange<const_edge_iterator, const_edge_iterator, ranges::subrange_kind::sized>;

  class const_edge_iterator {
  public:
    using iterator_category = forward_iterator_tag;
    using iterator_concept  = std::forward_iterator_tag;
    using value_type        = ual_edge<EV, VV, GV, VId, VContainer, Alloc>;
    using size_type         = size_t;
    using difference_type   = ptrdiff_t;
    using pointer           = value_type const*;
    using reference         = const value_type&;

  public:
    const_edge_iterator(const graph_type& g, vertex_iterator u) : g_(&const_cast<graph_type&>(g)), u_(u) {
      advance_vertex();
    }
    const_edge_iterator(const graph_type& g, const_vertex_iterator u)
          : g_(&const_cast<graph_type&>(g)), u_(g_->vertices().begin() + (u - g.vertices().begin())) {
      advance_vertex();
    }
    const_edge_iterator(const graph_type& g, vertex_iterator u, vertex_edge_iterator uv)
          : g_(&const_cast<graph_type&>(g)), u_(u), uv_(uv) {}

    const_edge_iterator() noexcept                                 = default;
    const_edge_iterator(const const_edge_iterator& rhs) noexcept   = default;
    ~const_edge_iterator() noexcept                                = default;
    const_edge_iterator& operator=(const const_edge_iterator& rhs) = default;

    reference operator*() const { return *uv_; }
    pointer   operator->() const { return &*uv_; }

    const_edge_iterator& operator++() {
      advance_edge();
      return *this;
    }
    const_edge_iterator operator++(int) {
      const_edge_iterator tmp(*this);
      ++*this;
      return tmp;
    }

    bool operator==(const const_edge_iterator& rhs) const noexcept { return uv_ == rhs.uv_ && u_ == rhs.u_; }
    bool operator!=(const const_edge_iterator& rhs) const noexcept { return !operator==(rhs); }

  protected:
    void advance_edge() {
      // next edge for current vertex
      vertex_id_type uid = static_cast<vertex_id_type>(u_ - g_->vertices().begin());
      if (++uv_ != u_->edges_end(*g_, uid))
        return;

      // find next vertex with edge(s)
      ++u_;
      advance_vertex();
    }

    void advance_vertex() {
      // at exit, if u_ != g.vertices().end() then uv_ will refer to a valid edge
      for (; u_ != g_->vertices().end(); ++u_) {
        if (u_->num_edges() > 0) {
          vertex_id_type uid = static_cast<vertex_id_type>(u_ - g_->vertices().begin());
          uv_                = u_->edges_begin(*g_, uid);
          return;
        }
      }
    }

  protected:
    graph_type*          g_;
    vertex_iterator      u_;
    vertex_edge_iterator uv_;
  };

  class edge_iterator : public const_edge_iterator {
  public:
    using base_t            = const_edge_iterator;
    using iterator_category = typename base_t::iterator_category;
    using value_type        = typename base_t::value_type;
    using size_type         = typename base_t::size_type;
    using difference_type   = typename base_t::difference_type;
    using pointer           = value_type*;
    using reference         = value_type&;

  public:
    edge_iterator(graph_type& g, vertex_iterator u) noexcept : const_edge_iterator(g, u) {}
    edge_iterator(graph_type& g, vertex_iterator u, vertex_edge_iterator uv) : const_edge_iterator(g, u, uv) {}

    edge_iterator() noexcept : const_edge_iterator() {};
    edge_iterator(const edge_iterator& rhs) noexcept : const_edge_iterator(rhs) {}
    edge_iterator(const_edge_iterator& rhs) : const_edge_iterator(rhs) {}
    ~edge_iterator() {}
    edge_iterator& operator=(const edge_iterator& rhs) noexcept {
      const_edge_iterator::operator=(rhs);
      return *this;
    }

    reference operator*() const { return *this->uv_; }
    pointer   operator->() const { return &*this->uv_; }

    edge_iterator& operator++() {
      this->advance_edge();
      return *this;
    }
    edge_iterator operator++(int) {
      edge_iterator tmp(*this);
      ++*this;
      return tmp;
    }
  };

protected: // Data Members (protected for derived class access)
  vertex_set                                  vertices_;
  edge_size_type                              edges_size_ = 0;
  [[no_unique_address]] vertex_allocator_type vertex_alloc_;
  [[no_unique_address]] edge_allocator_type   edge_alloc_;

  // Note: graph_value_ is NOT here - it belongs in the derived class

protected: // Constructors (protected - for derived class use only)
  base_undirected_adjacency_list() = default;

  explicit base_undirected_adjacency_list(const allocator_type& alloc) : vertices_(alloc), edge_alloc_(alloc) {}

  // Copy constructor - copies vertices and edges (derived class handles graph_value_)
  base_undirected_adjacency_list(const base_undirected_adjacency_list& other);

  // Move constructor - moves vertices and edges (derived class handles graph_value_)
  base_undirected_adjacency_list(base_undirected_adjacency_list&& other) noexcept = default;

  // Destructor
  ~base_undirected_adjacency_list();

  // Copy assignment operator
  base_undirected_adjacency_list& operator=(const base_undirected_adjacency_list& other);

  // Move assignment operator
  base_undirected_adjacency_list& operator=(base_undirected_adjacency_list&& other) noexcept = default;

  // Range constructors
  template <typename ERng, typename VRng, typename EProj, typename VProj>
  requires ranges::forward_range<ERng> && ranges::input_range<VRng> &&
           std::regular_invocable<EProj, ranges::range_reference_t<ERng>> &&
           std::regular_invocable<VProj, ranges::range_reference_t<VRng>>
  base_undirected_adjacency_list(
        const ERng& erng, const VRng& vrng, const EProj& eproj, const VProj& vproj, const allocator_type& alloc);

  // Initializer list constructors
  base_undirected_adjacency_list(const initializer_list<tuple<vertex_id_type, vertex_id_type, edge_value_type>>& ilist,
                                 const allocator_type&                                                           alloc);

  base_undirected_adjacency_list(const initializer_list<tuple<vertex_id_type, vertex_id_type>>& ilist,
                                 const allocator_type&                                          alloc);

  // Helper method for validation
  void throw_unordered_edges() const;

public: // Accessors
  /// @brief Get the edge allocator.
  /// @complexity O(1)
  constexpr edge_allocator_type edge_allocator() const noexcept;

  /// @brief Access the vertex container.
  /// @return Reference to the internal vertex container.
  /// @complexity O(1)
  constexpr vertex_set&       vertices();
  constexpr const vertex_set& vertices() const;

  /// @brief Get the number of vertices in the graph (CPO-compatible name).
  /// @return Number of vertices in the graph.
  /// @complexity O(1)
  constexpr auto num_vertices() const noexcept { return vertices_.size(); }

  /// @brief Get iterator to first vertex.
  /// @complexity O(1)
  constexpr vertex_iterator       begin();
  constexpr const_vertex_iterator begin() const;
  constexpr const_vertex_iterator cbegin() const;

  /// @brief Get iterator to one-past-last vertex.
  /// @complexity O(1)
  constexpr vertex_iterator       end();
  constexpr const_vertex_iterator end() const;
  constexpr const_vertex_iterator cend() const;

  /// @brief Find a vertex by its id (direct container access).
  /// @param id The vertex id to find.
  /// @return Iterator to the vertex, or end() if not found.
  /// @complexity O(1)
  /// @note This is distinct from the find_vertex CPO. Use this for direct container operations.
  vertex_iterator       try_find_vertex(vertex_id_type);
  const_vertex_iterator try_find_vertex(vertex_id_type) const;

  /// @brief Get the number of edges in the graph.
  /// @return Edge count (each undirected edge counted TWICE - once from each endpoint).
  /// @note For unique edge count, divide by 2.
  /// @complexity O(1)
  constexpr edge_size_type num_edges() const noexcept { return edges_size_; }

  /// @brief Check if the graph has any edges (CPO-compatible name).
  /// @return true if the graph has at least one edge, false otherwise.
  /// @complexity O(1)
  constexpr bool has_edge() const noexcept { return edges_size_ > 0; }

private: // CPO support via ADL (friend functions)
  /// @brief Get edges from a vertex descriptor (CPO: edges(g, u)).
  /// @tparam U Vertex descriptor type.
  /// @param g The graph.
  /// @param u The vertex descriptor.
  /// @return Native vertex_edge_range (CPO will wrap in edge_descriptor_view).
  /// @complexity O(1)
  template <typename U>
  requires adj_list::vertex_descriptor_type<U>
  friend constexpr auto edges(graph_type& g, const U& u) noexcept {
    auto uid = static_cast<vertex_id_type>(u.vertex_id());
    return g.vertices_[uid].edges(g, uid);
  }
  template <typename U>
  requires adj_list::vertex_descriptor_type<U>
  friend constexpr auto edges(const graph_type& g, const U& u) noexcept {
    auto uid = static_cast<vertex_id_type>(u.vertex_id());
    return g.vertices_[uid].edges(g, uid);
  }

public:
  /// @brief Get range of all edges.
  /// @note Each undirected edge appears twice in iteration (once from each endpoint).
  /// @complexity O(1) to create range, O(V+E) to iterate.
  edge_range       edges() { return {edge_iterator(*this, begin()), edge_iterator(*this, end()), this->edges_size_}; }
  const_edge_range edges() const {
    return {const_edge_iterator(*this, const_cast<base_undirected_adjacency_list&>(*this).begin()),
            const_edge_iterator(*this, const_cast<base_undirected_adjacency_list&>(*this).end()), this->edges_size_};
  }

public: // Vertex Creation
  /// @brief Create a new vertex with default value.
  /// @return Iterator to the newly created vertex.
  /// @complexity O(1) amortized
  vertex_iterator create_vertex() {
    this->vertices_.push_back(vertex_type(this->vertices_, static_cast<vertex_id_type>(this->vertices_.size())));
    return this->vertices_.begin() + static_cast<vertex_difference_type>(this->vertices_.size() - 1);
}

  /// @brief Create a new vertex with moved value.
  /// @param val Value to move into the vertex.
  /// @return Iterator to the newly created vertex.
  /// @complexity O(1) amortized
  vertex_iterator create_vertex(vertex_value_type&& val) {
    this->vertices_.push_back(
          vertex_type(this->vertices_, static_cast<vertex_id_type>(this->vertices_.size()), std::move(val)));
    return this->vertices_.begin() + static_cast<vertex_difference_type>(this->vertices_.size() - 1);
  }

  /// @brief Create a new vertex with copied value.
  /// @tparam VV2 Type convertible to vertex_value_type.
  /// @param val Value to copy into the vertex.
  /// @return Iterator to the newly created vertex.
  /// @complexity O(1) amortized
  template <class VV2>
  requires std::constructible_from<VV, const VV2&>
  vertex_iterator create_vertex(const VV2& val) {
    this->vertices_.push_back(
          vertex_type(this->vertices_, static_cast<vertex_id_type>(this->vertices_.size()), val));
    return this->vertices_.begin() + static_cast<vertex_id_type>(this->vertices_.size() - 1);
  }

public: // Edge Creation
  /// @brief Create an edge between two vertices (by id).
  /// @param uid Source vertex id.
  /// @param vid Target vertex id.
  /// @return Iterator to the newly created edge.
  /// @complexity O(1).
  vertex_edge_iterator create_edge(vertex_id_type uid, vertex_id_type vid) {
    vertex_iterator ui = try_find_vertex(uid);
    vertex_iterator vi = try_find_vertex(vid);
    return create_edge(ui, vi);
  }

  /// @brief Create an edge with value between two vertices (by id, move value).
  /// @param uid Source vertex id.
  /// @param vid Target vertex id.
  /// @param val Edge value to move.
  /// @return Iterator to the newly created edge.
  /// @complexity O(1).
  vertex_edge_iterator create_edge(vertex_id_type uid, vertex_id_type vid, edge_value_type&& val) {
    vertex_iterator ui = this->vertices_.begin() + uid;
    vertex_iterator vi = this->vertices_.begin() + vid;
    return create_edge(ui, vi, std::move(val));
  }

  /// @brief Create an edge with value between two vertices (by id, copy value).
  /// @tparam EV2 Type convertible to edge_value_type.
  /// @param uid Source vertex id.
  /// @param vid Target vertex id.
  /// @param val Edge value to copy.
  /// @return Iterator to the newly created edge.
  /// @complexity O(1).
  template <class EV2>
  requires std::constructible_from<EV, const EV2&>
  vertex_edge_iterator create_edge(vertex_id_type uid, vertex_id_type vid, const EV2& val) {
    vertex_iterator ui = this->vertices_.begin() + uid;
    vertex_iterator vi = this->vertices_.begin() + vid;
    return create_edge(ui, vi, val);
  }

  /// @brief Create an edge between two vertices (by iterator).
  /// @param u Source vertex iterator.
  /// @param v Target vertex iterator.
  /// @return Iterator to the newly created edge.
  /// @complexity O(1).
  vertex_edge_iterator create_edge(vertex_iterator u, vertex_iterator v) {
    vertex_id_type uid = static_cast<vertex_id_type>(u - this->vertices_.begin());
    edge_type*     uv  = this->edge_alloc_.allocate(1);
    new (uv) edge_type(static_cast<graph_type&>(*this), u, v);
    ++this->edges_size_;
    return vertex_edge_iterator(static_cast<graph_type&>(*this), uid, uv);
  }

  /// @brief Create an edge with value between two vertices (by iterator, move value).
  /// @param u Source vertex iterator.
  /// @param v Target vertex iterator.
  /// @param val Edge value to move.
  /// @return Iterator to the newly created edge.
  /// @complexity O(1).
  vertex_edge_iterator create_edge(vertex_iterator u, vertex_iterator v, edge_value_type&& val) {
    vertex_id_type uid = static_cast<vertex_id_type>(u - this->vertices_.begin());
    edge_type*     uv  = this->edge_alloc_.allocate(1);
    new (uv) edge_type(static_cast<graph_type&>(*this), u, v, std::move(val));
    ++this->edges_size_;
    return vertex_edge_iterator(static_cast<graph_type&>(*this), uid, uv);
  }

  /// @brief Create an edge with value between two vertices (by iterator, copy value).
  /// @tparam EV2 Type convertible to edge_value_type.
  /// @param u Source vertex iterator.
  /// @param v Target vertex iterator.
  /// @param val Edge value to copy.
  /// @return Iterator to the newly created edge.
  /// @complexity O(1).
  template <class EV2>
  requires std::constructible_from<EV, const EV2&>
  vertex_edge_iterator create_edge(vertex_iterator u, vertex_iterator v, const EV2& val) {
    vertex_id_type uid = static_cast<vertex_id_type>(u - this->vertices_.begin());
    edge_type*     uv  = this->edge_alloc_.allocate(1);
    new (uv) edge_type(static_cast<graph_type&>(*this), u, v, val);
    ++this->edges_size_;
    return vertex_edge_iterator(static_cast<graph_type&>(*this), uid, uv);
  }

public: // Edge Removal
  /// @brief Erase an edge from the graph.
  /// @param pos Iterator to the edge to erase.
  /// @return Iterator to the next edge.
  /// @complexity O(1) to unlink from both vertex edge lists.
  edge_iterator erase_edge(edge_iterator pos);

public: // Graph Modification
  /// @brief Remove all vertices and edges from the graph.
  /// @complexity O(V + E).
  void clear();

  /// @brief Swap contents with another graph of the same base type.
  /// @param other The graph to swap with.
  /// @complexity O(1).
  /// @note Does NOT swap graph_value_ - that's handled by derived class.
  void swap(base_undirected_adjacency_list& other);

protected: // Utilities
  /// @brief Reserve space for vertices.
  /// @param n Number of vertices to reserve space for.
  void reserve_vertices(vertex_size_type n);

  /// @brief Resize the vertex container.
  /// @param n New size.
  void resize_vertices(vertex_size_type n);

  /// @brief Resize the vertex container with default value.
  /// @param n New size.
  /// @param val Default value for new vertices.
  void resize_vertices(vertex_size_type n, const vertex_value_type& val);
};

///-------------------------------------------------------------------------------------
/// CPO support functions for undirected_adjacency_list (non-member templates)
///
/// These functions provide CPO customization via ADL for all undirected_adjacency_list
/// specializations. They are defined once as non-member templates to avoid redefinition
/// errors when multiple instantiations of base_undirected_adjacency_list exist.
///-------------------------------------------------------------------------------------

/// find_vertex(g, id) - returns view iterator yielding vertex_descriptor
/// REQUIRED: Provides bounds checking - returns end() if id >= size()
/// The default CPO implementation lacks this bounds checking.
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr auto find_vertex(undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>& g, VId id) noexcept {
  using graph_type     = undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>;
  using vertex_set     = typename graph_type::vertex_set;
  using container_iter = typename vertex_set::iterator;
  using view_type      = vertex_descriptor_view<container_iter>;
  using view_iterator  = typename view_type::iterator;
  using storage_type   = typename view_iterator::value_type::storage_type;

  if (id >= static_cast<VId>(g.vertices().size())) {
    return view_iterator{static_cast<storage_type>(g.vertices().size())};
  }
  return view_iterator{static_cast<storage_type>(id)};
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr auto find_vertex(const undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>& g, VId id) noexcept {
  using graph_type     = undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>;
  using vertex_set     = typename graph_type::vertex_set;
  using container_iter = typename vertex_set::const_iterator;
  using view_type      = vertex_descriptor_view<container_iter>;
  using view_iterator  = typename view_type::iterator;
  using storage_type   = typename view_iterator::value_type::storage_type;

  if (id >= static_cast<VId>(g.vertices().size())) {
    return view_iterator{static_cast<storage_type>(g.vertices().size())};
  }
  return view_iterator{static_cast<storage_type>(id)};
}

/// target_id(g, edge_descriptor) - get target vertex id from edge descriptor (iteration perspective)
/// For undirected graphs, the target is the "other" vertex relative to the source we're iterating from
/// This provides the ITERATION perspective, not the STORAGE perspective.
/// See ual_edge::list_owner_id()/list_target_id() documentation for the distinction.
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc,
          typename E>
requires edge_descriptor_type<E>
constexpr VId target_id(const undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>& g, const E& e) noexcept {
  return e.value()->other_vertex_id(g, static_cast<VId>(e.source_id()));
}

/// source_id(g, edge_descriptor) - get source vertex id from edge descriptor (iteration perspective)
/// For undirected graphs, the source is the vertex we're iterating from (stored in descriptor)
/// This provides the ITERATION perspective, not the STORAGE perspective.
///
/// Uses edge_descriptor.source_id() which returns the source from iteration context,
/// NOT ual_edge.list_owner_id() which returns the storage location.
/// See ual_edge::list_owner_id()/list_target_id() documentation for the distinction.
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc,
          typename E>
requires edge_descriptor_type<E>
constexpr VId source_id([[maybe_unused]] const undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>& g,
                        const E& e) noexcept {
  return static_cast<VId>(e.source_id());
}

/// edge_value(g, edge_descriptor) - get edge value from edge descriptor
/// Extracts the edge value from the underlying edge pointed to by the descriptor.
/// Only enabled when EV is not void.
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc,
          typename E>
requires edge_descriptor_type<E> && (!std::is_void_v<EV>)
constexpr decltype(auto)
      edge_value(undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>&, const E& e) noexcept {
  return e.value()->value();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc,
          typename E>
requires edge_descriptor_type<E> && (!std::is_void_v<EV>)
constexpr decltype(auto)
      edge_value(const undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>&, const E& e) noexcept {
  return e.value()->value();
}

///-------------------------------------------------------------------------------------
/// ual_vertex
///
/// @tparam EV     Edge Value type. default = void.
/// @tparam VV     Vertex Value type. default = void.
/// @tparam GV     Graph Value type. default = void.
/// @tparam IntexT The type used for vertex & edge index into the internal vectors.
/// @tparam A      Allocator. default = std::allocator
///
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
class ual_vertex : public ual_vertex_value<EV, VV, GV, VId, VContainer, Alloc> {
public:
  using graph_type       = undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>;
  using graph_value_type = GV;
  using base_value_type  = ual_vertex_value<EV, VV, GV, VId, VContainer, Alloc>;

  using vertex_type           = ual_vertex<EV, VV, GV, VId, VContainer, Alloc>;
  using vertex_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set            = VContainer<vertex_type, vertex_allocator_type>;
  using vertex_iterator       = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;
  using vertex_id_type        = VId;
  using vertex_index          = VId;
  using vertex_value_type     = VV;

  using edge_value_type      = EV;
  using edge_type            = ual_edge<EV, VV, GV, VId, VContainer, Alloc>;
  using edge_allocator_type  = typename allocator_traits<Alloc>::template rebind_alloc<edge_type>;
  using edge_size_type       = typename edge_type::edge_size_type;
  using edge_difference_type = typename edge_type::edge_difference_type;

  using vertex_edge_list_type             = ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>;
  using vertex_edge_list_inward_link_type = ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, inward_list>;
  using vertex_edge_list_outward_link_type =
        ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, outward_list>;
  using vertex_edge_size_type      = typename vertex_edge_list_type::size_type;
  using vertex_edge_iterator       = typename vertex_edge_list_type::iterator;
  using const_vertex_edge_iterator = typename vertex_edge_list_type::const_iterator;
  using vertex_edge_range          = typename vertex_edge_list_type::edge_range;
  using const_vertex_edge_range    = typename vertex_edge_list_type::const_edge_range;

  using neighbor_iterator       = ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>;
  using const_neighbor_iterator = ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>;
  using neighbor_range          = ranges::subrange<neighbor_iterator, neighbor_iterator, ranges::subrange_kind::sized>;
  using const_neighbor_range =
        ranges::subrange<const_neighbor_iterator, const_neighbor_iterator, ranges::subrange_kind::sized>;
  using neighbor_size_type       = edge_size_type;
  using neighbor_difference_type = edge_difference_type;

public:
  // noexcept is only defined for move ctor & assignment b/c the user-defined value type could
  // throw an exception in other cases
  ual_vertex()                                 = default;
  ual_vertex(const ual_vertex&)                = default;
  ual_vertex(ual_vertex&&) noexcept            = default;
  ~ual_vertex() noexcept                       = default;
  ual_vertex& operator=(const ual_vertex&)     = default;
  ual_vertex& operator=(ual_vertex&&) noexcept = default;

  ual_vertex(vertex_set& vertices, vertex_index index);
  ual_vertex(vertex_set& vertices, vertex_index index, const vertex_value_type&);
  ual_vertex(vertex_set& vertices, vertex_index index, vertex_value_type&&) noexcept;

public:
#  if 0
  vertex_id_type       vertex_id(const graph_type&) const noexcept;
#  endif
  vertex_edge_size_type num_edges() const;

  vertex_edge_iterator erase_edge(graph_type&, vertex_edge_iterator);
  vertex_edge_iterator erase_edge(graph_type&, vertex_edge_iterator, vertex_edge_iterator);
  void                 clear_edges(graph_type&);

  vertex_edge_iterator       edges_begin(graph_type&, vertex_id_type uid) noexcept;
  const_vertex_edge_iterator edges_begin(const graph_type&, vertex_id_type uid) const noexcept;
  const_vertex_edge_iterator edges_cbegin(const graph_type&, vertex_id_type uid) const noexcept;

  vertex_edge_iterator       edges_end(graph_type&, vertex_id_type uid) noexcept;
  const_vertex_edge_iterator edges_end(const graph_type&, vertex_id_type uid) const noexcept;
  const_vertex_edge_iterator edges_cend(const graph_type&, vertex_id_type uid) const noexcept;

  vertex_edge_range       edges(graph_type&, vertex_id_type uid);
  const_vertex_edge_range edges(const graph_type&, vertex_id_type uid) const;

  edge_type&       edge_front(graph_type&) noexcept;
  const edge_type& edge_front(const graph_type&) const noexcept;

  edge_type&       edge_back(graph_type&) noexcept;
  const edge_type& edge_back(const graph_type&) const noexcept;

  neighbor_range       neighbors(graph_type&, vertex_id_type uid);
  const_neighbor_range neighbors(const graph_type&, vertex_id_type uid) const;

  neighbor_size_type neighbors_size(const graph_type&) const;

protected:
  void erase_edge(graph_type&, edge_type*);

  // Removed: e_begin/e_end were legacy protected methods using const_cast, replaced by public edges_begin/edges_end

private:
  vertex_edge_list_type edges_;
  friend edge_type;
  friend vertex_edge_list_inward_link_type;
  friend vertex_edge_list_outward_link_type;
};

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
class ual_const_neighbor_iterator {
public:
  using this_t = ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>;

  using graph_type = undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>;

  using vertex_type           = ual_vertex<EV, VV, GV, VId, VContainer, Alloc>;
  using vertex_value_type     = VV;
  using vertex_id_type        = VId;
  using vertex_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set            = VContainer<vertex_type, vertex_allocator_type>;

  using vertex_iterator       = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;

  using edge_list_type             = ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>;
  using vertex_edge_size_type      = typename edge_list_type::size_type;
  using vertex_edge_iterator       = typename edge_list_type::iterator;
  using const_vertex_edge_iterator = typename edge_list_type::const_iterator;

  using edge_type       = ual_edge<EV, VV, GV, VId, VContainer, Alloc>;
  using edge_value_type = typename edge_type::edge_value_type;
  using edge_id_type    = typename edge_type::edge_id_type; // <from,to>

  using iterator_category = bidirectional_iterator_tag;
  using iterator_concept  = std::bidirectional_iterator_tag;
  using value_type        = vertex_type;
  using size_type         = typename edge_list_type::size_type;
  using difference_type   = typename edge_list_type::difference_type;
  using pointer           = const value_type*;
  using const_pointer     = const value_type*;
  using reference         = const value_type&;
  using const_reference   = const value_type&;

public:
  constexpr ual_const_neighbor_iterator(vertex_edge_iterator const& uv);

  constexpr ual_const_neighbor_iterator()                                   = default;
  constexpr ual_const_neighbor_iterator(const ual_const_neighbor_iterator&) = default;
  constexpr ual_const_neighbor_iterator(ual_const_neighbor_iterator&&)      = default;
  ~ual_const_neighbor_iterator()                                            = default;

  constexpr ual_const_neighbor_iterator& operator=(const ual_const_neighbor_iterator&) = default;
  constexpr ual_const_neighbor_iterator& operator=(ual_const_neighbor_iterator&&)      = default;

public:
  constexpr graph_type&       graph() noexcept;
  constexpr const graph_type& graph() const noexcept;

  constexpr const_vertex_iterator other_vertex() const;
  constexpr vertex_id_type        other_vertex_id() const;

  constexpr reference operator*() const noexcept;
  constexpr pointer   operator->() const noexcept;

  constexpr ual_const_neighbor_iterator& operator++() noexcept;
  constexpr ual_const_neighbor_iterator  operator++(int) noexcept;

  constexpr ual_const_neighbor_iterator& operator--() noexcept;
  constexpr ual_const_neighbor_iterator  operator--(int) noexcept;

  constexpr bool operator==(const ual_const_neighbor_iterator& rhs) const noexcept;
  constexpr bool operator!=(const ual_const_neighbor_iterator& rhs) const noexcept;

  friend void swap(ual_const_neighbor_iterator& lhs, ual_const_neighbor_iterator& rhs) { swap(lhs.uv_, rhs.uv_); }

protected:
  vertex_edge_iterator uv_;
};

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
class ual_neighbor_iterator : public ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc> {
public:
  using this_t = ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>;
  using base_t = ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>;

  using graph_type           = typename base_t::graph_type;
  using vertex_type          = typename base_t::vertex_type;
  using vertex_edge_iterator = typename base_t::vertex_edge_iterator;
  using vertex_iterator      = typename base_t::vertex_iterator;
  using edge_type            = typename base_t::edge_type;

  using iterator_category = typename base_t::iterator_category;
  using value_type        = typename base_t::value_type;
  using size_type         = typename base_t::size_type;
  using difference_type   = typename base_t::difference_type;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;

protected:
  using base_t::uv_;

public:
  constexpr ual_neighbor_iterator(vertex_edge_iterator const& uv);

  constexpr ual_neighbor_iterator()                             = default;
  constexpr ual_neighbor_iterator(const ual_neighbor_iterator&) = default;
  constexpr ual_neighbor_iterator(ual_neighbor_iterator&&)      = default;
  ~ual_neighbor_iterator()                                      = default;

  constexpr ual_neighbor_iterator& operator=(const ual_neighbor_iterator&) = default;
  constexpr ual_neighbor_iterator& operator=(ual_neighbor_iterator&&)      = default;

public:
  constexpr vertex_iterator other_vertex();

  constexpr const_reference operator*() const;
  constexpr const_pointer   operator->() const;

  constexpr ual_neighbor_iterator& operator++();
  constexpr ual_neighbor_iterator  operator++(int);

  constexpr ual_neighbor_iterator& operator--() noexcept;
  constexpr ual_neighbor_iterator  operator--(int) noexcept;

  constexpr bool operator==(const ual_neighbor_iterator& rhs) const noexcept;
  constexpr bool operator!=(const ual_neighbor_iterator& rhs) const noexcept;

  friend void swap(ual_neighbor_iterator& lhs, ual_neighbor_iterator& rhs) { swap(lhs.uv_, rhs.uv_); }
};

///-------------------------------------------------------------------------------------
/// ITERATOR INVALIDATION RULES
///-------------------------------------------------------------------------------------
///
/// Vertex Iterators:
///   - Invalidated by: create_vertex() if reallocation occurs, clear()
///   - NOT invalidated by: create_edge(), erase_edge()
///   - Note: Use vertex ids instead of iterators for stable references
///
/// Edge Iterators (graph-level edges()):
///   - Invalidated by: erase_edge() on the same edge, clear()
///   - NOT invalidated by: erase_edge() on different edges, create_edge(), create_vertex()
///
/// Vertex-Edge Iterators (per-vertex edges):
///   - Invalidated by: erase_edge() that removes the edge, clear()
///   - NOT invalidated by: erase_edge() on different edges, create_edge(), create_vertex()
///
/// Vertex-Vertex Iterators (neighbors):
///   - Same invalidation rules as vertex-edge iterators
///
/// References and Pointers:
///   - Vertex references: Invalidated by create_vertex() if reallocation, clear()
///   - Edge references: Invalidated by erase_edge() on that edge, clear()
///   - Use vertex ids for stable references across operations
///
/// Thread Safety:
///   - NOT thread-safe for concurrent modifications
///   - Concurrent reads safe if no writes
///   - External synchronization required for write operations
///
///-------------------------------------------------------------------------------------

/// A simple undirected adjacency list (graph).
///
/// @tparam EV              Edge Value type. default = void.
/// @tparam VV              Vertex Value type. default = void.
/// @tparam GV              Graph Value type. default = void.
/// @tparam IntexT          The type used for vertex & edge index into the internal vectors.
/// @tparam VContainer<V,A> Random-access container type used to store vertices (V) with allocator (A).
/// @tparam Alloc           Allocator. default = std::allocator
///
// clang-format off
template <typename                                EV,
          typename                                VV,
          typename                                GV,
          integral                                VId,
          template <typename V, typename A> class VContainer,
          typename                                Alloc>
class undirected_adjacency_list 
    : public base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>
// clang-format on
{
public:
  using base_type = base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>;

  // Note: Access base class members using this-> or base_type::
  // (using declarations don't work in constructor initializer lists)

  using graph_type       = undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>;
  using graph_value_type = GV;
  using allocator_type   = Alloc;

  using vertex_type               = ual_vertex<EV, VV, GV, VId, VContainer, Alloc>;
  using vertex_value_type         = VV;
  using vertex_value_wrapper_type = ual_vertex_value<EV, VV, GV, VId, VContainer, Alloc>;
  using vertex_id_type            = VId;
  using vertex_index_type         = VId;
  using vertex_allocator_type     = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set                = VContainer<vertex_type, vertex_allocator_type>;
  using vertex_size_type          = typename vertex_set::size_type;
  using vertex_difference_type    = typename vertex_set::difference_type;

  using vertex_iterator       = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;
  using vertex_range          = vertex_set&;
  using const_vertex_range    = const vertex_set&;

  using edge_type            = ual_edge<EV, VV, GV, VId, VContainer, Alloc>;
  using edge_value_type      = EV;
  using edge_allocator_type  = typename allocator_traits<Alloc>::template rebind_alloc<edge_type>;
  using edge_id_type         = pair<vertex_id_type, vertex_id_type>; // <from,to>
  using edge_size_type       = typename edge_type::edge_size_type;
  using edge_difference_type = typename edge_type::edge_difference_type;
  // edge_set, edge_index_type n/a

  using vertex_edge_iterator        = typename vertex_type::vertex_edge_iterator;
  using const_vertex_edge_iterator  = typename vertex_type::const_vertex_edge_iterator;
  using vertex_edge_range           = typename vertex_type::vertex_edge_range;
  using const_vertex_edge_range     = typename vertex_type::const_vertex_edge_range;
  using vertex_edge_size_type       = typename vertex_edge_iterator::size_type;
  using vertex_edge_difference_type = typename vertex_edge_iterator::difference_type;

  // Use edge iterators from base class
  using edge_iterator       = typename base_type::edge_iterator;
  using const_edge_iterator = typename base_type::const_edge_iterator;
  using edge_range          = typename base_type::edge_range;
  using const_edge_range    = typename base_type::const_edge_range;

public:
  undirected_adjacency_list()                                         = default;
  undirected_adjacency_list(undirected_adjacency_list&& rhs) noexcept = default;
  undirected_adjacency_list(const undirected_adjacency_list& other);

  // clang-format off
  undirected_adjacency_list(const allocator_type& alloc);
  
  undirected_adjacency_list(const graph_value_type& gv, const allocator_type& alloc = allocator_type());
  undirected_adjacency_list(graph_value_type&& gv, const allocator_type& alloc = allocator_type());
  // clang-format on

  // The following constructors will load edges (and vertices) into the graph
  //
  // The number of vertices is guaranteed to match the highest vertex id in
  // the edges. Edges are scanned first to determine the highest number and
  // the vertices are resized to match the number.
  //
  // Accessor functions are used to return the edge_id_type,
  // edge_value_type and vertex_value_type.
  //
  // The order visited in the vertices determines their index (and
  // id/identity) in the internal vertices container. The edge ids use those
  // values and are also expected to be ordered by their first (in) vertex
  // id and an exception is thrown if they aren't in order. For these
  // reasons, unordered (hash) containers won't work.
  //

  /// Constructor that takes edge & vertex ranges to create the graph.
  ///
  /// The value_type of @c erng must be converted to @c copyable_edge_t<VId,EV> before it can be
  /// added to the graph, which is done using the @c eproj function. If the value_type of
  /// @c erng is already copyable_edge_t<VId,EV>, @c std::identity can be used instead. @c copyable_edge_t
  /// contains source_id, target_id and optionally a value member if an edge value type has been defined.
  ///
  /// The value_type of @c vrng must be converted to @c copyable_vertex_t<VId,VV> before it can be
  /// added to the graph, which is done using the @c vproj function. If the value_type of
  /// @c vrng is already copyable_vertex_t<VId,VV>, @c std::identity can be used instead. @c copyable_vertex_t
  /// contains id and optionally a value member if a vertex value type has been defined.
  ///
  /// @tparam ERng  The edge data range.
  /// @tparam VRng  The vertex data range.
  /// @tparam EProj A projection function type to convert the ERng value_type to copyable_edge_t<VId,EV>.
  ///               If ERng value_type is already copyable_edge_t, identity can be used.
  /// @tparam VProj A projection function type to convert the VRng value_type to copyable_vertex_t<VId,VV>.
  ///               If VRng value_type is already copyable_vertex_t, identity can be used.
  ///
  /// @param erng  The container of edge data.
  /// @param vrng  The container of vertex data.
  /// @param eproj The projection function that converts the ERng value_type to copyable_edge_t<VId,EV>,
  ///              or identity() if ERng value_type is already copyable_edge_t.
  /// @param vproj The projection function that converts the VRng value_type to copyable_vertex_t<VId,VV>,
  ///              or identity() if VRng value_type is already copyable_vertex_t.
  /// @param gv    Graph value.
  /// @param alloc The allocator to use for internal containers for vertices & edges.
  ///
  // clang-format off
  template <typename ERng, 
            typename VRng, 
            typename EProj = std::identity, 
            typename VProj = std::identity,
            typename GV_ = GV>
    requires ranges::forward_range<ERng> 
          && ranges::input_range<VRng>
          && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
          && std::regular_invocable<VProj, ranges::range_reference_t<VRng>>
          && (!std::is_void_v<GV_>)
  undirected_adjacency_list(const ERng&  erng,
                            const VRng&  vrng,
                            const EProj& eproj,
                            const VProj& vproj,
                            const GV_&   gv,
                            const Alloc& alloc = Alloc());
  
  template <typename ERng, 
            typename VRng, 
            typename EProj = std::identity, 
            typename VProj = std::identity>
    requires ranges::forward_range<ERng> 
          && ranges::input_range<VRng>
          && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
          && std::regular_invocable<VProj, ranges::range_reference_t<VRng>>
          && std::is_void_v<GV>
  undirected_adjacency_list(const ERng&  erng,
                            const VRng&  vrng,
                            const EProj& eproj = {},
                            const VProj& vproj = {},
                            const Alloc& alloc = Alloc());
  // clang-format on

  /// Constructor that takes edge range to create the graph.
  ///
  /// The value_type of @c erng must be converted to @c copyable_edge_t<VId,EV> before it can be
  /// added to the graph, which is done using the @c eproj function. If the value_type of
  /// @c erng is already copyable_edge_t<VId,EV>, @c std::identity can be used instead. @c copyable_edge_t
  /// contains source_id, target_id and optionally a value member if an edge value type has been defined.
  ///
  /// Edges are scanned to determine the largest vertex id needed.
  ///
  /// If vertices have a user-defined value (e.g. VV not void), the value must be default-constructable.
  ///
  /// @tparam ERng  The edge data range.
  /// @tparam EProj A projection function type to convert the ERng value_type to copyable_edge_t<VId,EV>.
  ///               If ERng value_type is already copyable_edge_t, identity can be used.
  ///
  /// @param erng  The container of edge data.
  /// @param eproj The projection function that converts the ERng value_type to copyable_edge_t<VId,EV>,
  ///              or identity() if ERng value_type is already copyable_edge_t.
  /// @param gv    Graph value.
  /// @param alloc The allocator to use for internal containers for vertices & edges.
  ///
  // clang-format off
  template <typename ERng, typename EProj = std::identity, typename GV_ = GV>
    requires ranges::forward_range<ERng>
          && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
          && (!std::is_void_v<GV_>)
  undirected_adjacency_list(const ERng&  erng, 
                            const EProj& eproj, 
                            const GV_&   gv, 
                            const Alloc& alloc = Alloc());
  
  template <typename ERng, typename EProj = std::identity>
    requires ranges::forward_range<ERng>
          && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
          && std::is_void_v<GV>
  undirected_adjacency_list(const ERng&  erng, 
                            const EProj& eproj = {}, 
                            const Alloc& alloc = Alloc());
  // clang-format on

  /// Constructor for easy creation of a graph that takes an initializer
  /// list with edge values.
  ///
  /// Automatically resizes the vertex container to accommodate the largest
  /// vertex ID referenced in the edge list. No need to pre-create vertices.
  ///
  /// @param ilist Initializer list of tuples with source_vertex_id,
  ///              target_vertex_id and the edge value.
  /// @param alloc Allocator.
  ///
  // clang-format off
  undirected_adjacency_list(
    const initializer_list<
          tuple<vertex_id_type, vertex_id_type, edge_value_type>>& ilist,
    const Alloc&                                                     alloc = Alloc());
  // clang-format on

  /// Constructor for easy creation of a graph that takes an initializer
  /// list without edge values.
  ///
  /// Automatically resizes the vertex container to accommodate the largest
  /// vertex ID referenced in the edge list. No need to pre-create vertices.
  ///
  /// @param ilist Initializer list of tuples with source_vertex_id and
  ///              target_vertex_id.
  /// @param alloc Allocator.
  ///
  // clang-format off
  undirected_adjacency_list(
    const initializer_list<tuple<vertex_id_type, vertex_id_type>>& ilist,
    const Alloc&                                                     alloc = Alloc());
  // clang-format on

  ~undirected_adjacency_list();

  undirected_adjacency_list& operator=(const undirected_adjacency_list& other);
  undirected_adjacency_list& operator=(undirected_adjacency_list&&) = default;

public: // Accessors
  // Base class accessors
  using base_type::edge_allocator;
  using base_type::vertices;
  using base_type::begin;
  using base_type::cbegin;
  using base_type::end;
  using base_type::cend;
  using base_type::try_find_vertex;
  using base_type::edges;

  // Graph value accessors
  /// @brief Access the graph-level value.
  /// @return Reference to the graph value.
  /// @complexity O(1)
  /// @note Only available when GV is not void.
  graph_value_type& graph_value() noexcept { return graph_value_; }

  const graph_value_type& graph_value() const noexcept { return graph_value_; }

public: // Vertex creation
  // Base class vertex creation methods
  using base_type::create_vertex;

public: // Edge creation
  // Base class edge creation methods
  using base_type::create_edge;

public: // Edge removal
  // Base class edge removal methods
  using base_type::erase_edge;

public: // Graph operations
  // Base class graph operations
  using base_type::clear;
  using base_type::reserve_vertices;
  using base_type::resize_vertices;

  /// @brief Swap contents with another graph.
  /// @param other The graph to swap with.
  /// @complexity O(1).
  /// @invalidates All iterators, pointers, and references to both graphs.
  void swap(undirected_adjacency_list&);

protected:
  // Base class utility methods

  //vertex_iterator finalize_outward_edges(vertex_range);

private:
  [[no_unique_address]] GV graph_value_{};
  friend vertex_type;

private: // CPO support via ADL (friend functions)
  // These friend functions enable CPO customization via ADL
  // Note: vertices(g) removed - CPO uses base class member function vertices()
  // Note: num_edges(g) removed - CPO uses base class member function num_edges()
  // Note: num_vertices(g) removed - CPO uses base class member function num_vertices()
  // Note: has_edge(g) removed - CPO uses base class member function has_edge()
  // Note: find_vertex(g, id) moved to base class - common implementation for all specializations
  // Note: graph_value(g) removed - CPO uses member function graph_value()
  // Note: edges(g, u) removed - CPO uses base class member function edges(u)
  // Note: degree(g, u) removed - CPO uses default implementation
  // Note: target_id(g, e) moved to base class - common implementation for all specializations
  // Note: source_id(g, e) moved to base class - common implementation for all specializations
  // Note: find_vertex_edge(g, ...) removed - CPO uses default implementation
};

///-------------------------------------------------------------------------------------
/// undirected_adjacency_list<EV, VV, void, ...> - Specialization for GV=void
///
/// @brief Specialization of undirected_adjacency_list when no graph value is needed.
///
/// This specialization removes the graph_value_ member and graph_value() accessor
/// to provide zero memory overhead when graph-level values aren't needed.
///
/// All other functionality remains identical to the primary template.
///-------------------------------------------------------------------------------------
template <typename EV, typename VV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
class undirected_adjacency_list<EV, VV, void, VId, VContainer, Alloc>
      : public base_undirected_adjacency_list<EV, VV, void, VId, VContainer, Alloc> {
public:
  using base_type = base_undirected_adjacency_list<EV, VV, void, VId, VContainer, Alloc>;

  // Note: Access base class members using this-> or base_type::
  // (using declarations don't work in constructor initializer lists)

  using graph_type       = undirected_adjacency_list<EV, VV, void, VId, VContainer, Alloc>;
  using graph_value_type = void;
  using allocator_type   = Alloc;

  using vertex_type            = ual_vertex<EV, VV, void, VId, VContainer, Alloc>;
  using vertex_value_type      = VV;
  using vertex_id_type         = VId;
  using vertex_index_type      = VId;
  using vertex_allocator_type  = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set             = VContainer<vertex_type, vertex_allocator_type>;
  using vertex_size_type       = typename vertex_set::size_type;
  using vertex_difference_type = typename vertex_set::difference_type;

  using vertex_iterator       = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;
  using vertex_range          = vertex_set&;
  using const_vertex_range    = const vertex_set&;

  using edge_type            = ual_edge<EV, VV, void, VId, VContainer, Alloc>;
  using edge_value_type      = EV;
  using edge_allocator_type  = typename allocator_traits<Alloc>::template rebind_alloc<edge_type>;
  using edge_id_type         = pair<vertex_id_type, vertex_id_type>;
  using edge_size_type       = typename edge_type::edge_size_type;
  using edge_difference_type = typename edge_type::edge_difference_type;

  using vertex_edge_iterator        = typename vertex_type::vertex_edge_iterator;
  using const_vertex_edge_iterator  = typename vertex_type::const_vertex_edge_iterator;
  using vertex_edge_range           = typename vertex_type::vertex_edge_range;
  using const_vertex_edge_range     = typename vertex_type::const_vertex_edge_range;
  using vertex_edge_size_type       = typename vertex_edge_iterator::size_type;
  using vertex_edge_difference_type = typename vertex_edge_iterator::difference_type;

  // Use edge iterators from base class
  using edge_iterator       = typename base_type::edge_iterator;
  using const_edge_iterator = typename base_type::const_edge_iterator;
  using edge_range          = typename base_type::edge_range;
  using const_edge_range    = typename base_type::const_edge_range;

public:
  // Forward declare all the same public methods as the primary template
  // (constructor declarations, accessors, create_vertex, create_edge, etc.)
  // The implementations will be shared via undirected_adjacency_list_impl.hpp

  undirected_adjacency_list()                                         = default;
  undirected_adjacency_list(undirected_adjacency_list&& rhs) noexcept = default;
  undirected_adjacency_list(const undirected_adjacency_list& other);

  undirected_adjacency_list(const allocator_type& alloc);

  template <typename ERng, typename VRng, typename EProj = std::identity, typename VProj = std::identity>
  requires ranges::forward_range<ERng> && ranges::input_range<VRng> &&
           std::regular_invocable<EProj, ranges::range_reference_t<ERng>> &&
           std::regular_invocable<VProj, ranges::range_reference_t<VRng>>
  undirected_adjacency_list(const ERng&  erng,
                            const VRng&  vrng,
                            const EProj& eproj = {},
                            const VProj& vproj = {},
                            const Alloc& alloc = Alloc());

  template <typename ERng, typename EProj = std::identity>
  requires ranges::forward_range<ERng> && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
  undirected_adjacency_list(const ERng& erng, const EProj& eproj = {}, const Alloc& alloc = Alloc());

  /// Constructor for easy creation of a graph that takes an initializer
  /// list with edge values.
  ///
  /// Automatically resizes the vertex container to accommodate the largest
  /// vertex ID referenced in the edge list. No need to pre-create vertices.
  ///
  /// @param ilist Initializer list of tuples with source_vertex_id,
  ///              target_vertex_id and the edge value.
  /// @param alloc Allocator.
  ///
  undirected_adjacency_list(const initializer_list<tuple<vertex_id_type, vertex_id_type, edge_value_type>>& ilist,
                            const Alloc& alloc = Alloc());

  /// Constructor for easy creation of a graph that takes an initializer
  /// list without edge values.
  ///
  /// Automatically resizes the vertex container to accommodate the largest
  /// vertex ID referenced in the edge list. No need to pre-create vertices.
  ///
  /// @param ilist Initializer list of tuples with source_vertex_id and
  ///              target_vertex_id.
  /// @param alloc Allocator.
  ///
  undirected_adjacency_list(const initializer_list<tuple<vertex_id_type, vertex_id_type>>& ilist,
                            const Alloc&                                                   alloc = Alloc());

  ~undirected_adjacency_list();

  undirected_adjacency_list& operator=(const undirected_adjacency_list& other);
  undirected_adjacency_list& operator=(undirected_adjacency_list&&) = default;

public: // Accessors
  // Base class accessors
  using base_type::edge_allocator;
  using base_type::vertices;
  using base_type::begin;
  using base_type::cbegin;
  using base_type::end;
  using base_type::cend;
  using base_type::try_find_vertex;
  using base_type::edges;

  // Note: No graph_value() methods for GV=void specialization

public: // Vertex creation
  // Base class vertex creation methods
  using base_type::create_vertex;

public: // Edge creation
  // Base class edge creation methods
  using base_type::create_edge;

public: // Edge removal
  // Base class edge removal methods
  using base_type::erase_edge;

public: // Graph operations
  // Base class graph operations
  using base_type::clear;
  using base_type::reserve_vertices;
  using base_type::resize_vertices;

  /// @brief Swap contents with another graph.
  /// @param rhs The graph to swap with.
  /// @complexity O(1)
  /// @invalidates All iterators, pointers, and references.
  void swap(undirected_adjacency_list&);

protected:
  // Base class utility methods

private:
  // Note: No graph_value_ member in GV=void specialization
  friend vertex_type;

private:
  // Note: vertices(g) removed - CPO uses base class member function vertices()
  // Note: num_edges(g) removed - CPO uses base class member function num_edges()
  // Note: num_vertices(g) removed - CPO uses base class member function num_vertices()
  // Note: has_edge(g) removed - CPO uses base class member function has_edge()
  // Note: find_vertex(g, id) moved to base class - common implementation for all specializations
  // Note: No graph_value(g) CPO for GV=void specialization
  // Note: edges(g, u) removed - CPO uses base class member function edges(u)
  // Note: degree(g, u) removed - CPO uses default implementation
  // Note: target_id(g, e) moved to base class - common implementation for all specializations
  // Note: source_id(g, e) moved to base class - common implementation for all specializations
  // Note: find_vertex_edge(g, ...) removed - CPO uses default implementation
};

} // namespace graph::container

#  include "detail/undirected_adjacency_list_api.hpp"
#  include "detail/undirected_adjacency_list_impl.hpp"

#endif // UNDIRECTED_ADJ_LIST_HPP
