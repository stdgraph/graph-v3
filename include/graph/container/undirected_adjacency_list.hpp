//
//	Author: J. Phillip Ratzloff
//
#include "../graph_utility.hpp"
#include "../graph_info.hpp"
#include "../vertex_descriptor_view.hpp"
#include "../edge_descriptor_view.hpp"
#include "../descriptor_traits.hpp"
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

#define CPO 1
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
///   - Provides O(1) vertex access by key/index
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
/// Per edge: ~48-64 bytes (4 list pointers, 2 vertex keys, value, allocation overhead)
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
/// - vertex_range<G>, adjacency_list<G>, sourced_adjacency_list<G>
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
/// using Graph = undirected_adjacency_list<string, int>;  // vertex_value=string, edge_value=int
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
/// @tparam VV Vertex Value type (default: void for no value)
/// @tparam EV Edge Value type (default: void for no value)  
/// @tparam GV Graph Value type (default: void for no value)
/// @tparam VId Vertex key/index type (default: uint32_t)
/// @tparam VContainer Vertex storage container template (default: std::vector)
/// @tparam Alloc Allocator type (default: std::allocator<char>)
///-------------------------------------------------------------------------------------

template <typename VV                                        = void,
          typename EV                                        = void,
          typename GV                                        = void,
          integral VId                                      = uint32_t,
          template <typename V, typename A> class VContainer = vector,
          typename Alloc                                     = allocator<char>>
class undirected_adjacency_list;

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
class ual_vertex;

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
class ual_edge;

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
class ual_const_vertex_vertex_iterator;

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
class ual_vertex_vertex_iterator;


///-------------------------------------------------------------------------------------

// designator types
struct inward_list;
struct outward_list;

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
class ual_vertex_edge_list;

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
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
template <typename EV, typename VV, typename GV, integral VId, 
          template <typename V, typename A> class VContainer, typename Alloc>
class ual_edge_value {
public:
  using value_type = EV;
  using graph_type = undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;

public:
  constexpr ual_edge_value(const value_type& val) : value(val) {}
  constexpr ual_edge_value(value_type&& val) : value(std::move(val)) {}

  constexpr ual_edge_value()                        = default;
  constexpr ual_edge_value(const ual_edge_value&)   = default;
  constexpr ual_edge_value(ual_edge_value&&)        = default;
  constexpr ~ual_edge_value()                       = default;

  constexpr ual_edge_value& operator=(const ual_edge_value&) = default;
  constexpr ual_edge_value& operator=(ual_edge_value&&)      = default;

public:
  value_type value = value_type();
};

///-------------------------------------------------------------------------------------
/// ual_edge_value<void, ...> specialization
///
/// @brief Specialization for EV=void that provides zero memory overhead.
///
template <typename VV, typename GV, integral VId, 
          template <typename V, typename A> class VContainer, typename Alloc>
class ual_edge_value<void, VV, GV, VId, VContainer, Alloc> {};

///-------------------------------------------------------------------------------------
/// ual_vertex_value
///
/// @brief Value class for undirected_adjacency_list vertices with template specialization for void.
///
/// This class stores the user-defined vertex value. A specialization for VV=void provides
/// zero memory overhead when no vertex value is needed.
///
/// @tparam VV Vertex value type
/// @tparam EV Edge value type
/// @tparam GV Graph value type
/// @tparam VId Vertex ID type
/// @tparam VContainer Container template for vertices
/// @tparam Alloc Allocator type
///
template <typename VV, typename EV, typename GV, integral VId,
          template <typename V, typename A> class VContainer, typename Alloc>
class ual_vertex_value {
public:
  using value_type = VV;
  using graph_type = undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;

public:
  constexpr ual_vertex_value(const value_type& val) : value(val) {}
  constexpr ual_vertex_value(value_type&& val) : value(std::move(val)) {}

  constexpr ual_vertex_value()                          = default;
  constexpr ual_vertex_value(const ual_vertex_value&)   = default;
  constexpr ual_vertex_value(ual_vertex_value&&)        = default;
  constexpr ~ual_vertex_value()                         = default;

  constexpr ual_vertex_value& operator=(const ual_vertex_value&) = default;
  constexpr ual_vertex_value& operator=(ual_vertex_value&&)      = default;

public:
  value_type value = value_type();
};

///-------------------------------------------------------------------------------------
/// ual_vertex_value<void, ...> specialization
///
/// @brief Specialization for VV=void that provides zero memory overhead.
///
template <typename EV, typename GV, integral VId,
          template <typename V, typename A> class VContainer, typename Alloc>
class ual_vertex_value<void, EV, GV, VId, VContainer, Alloc> {};


///-------------------------------------------------------------------------------------
/// ual_vertex_edge_list
///
/// @tparam VV     Vertex Value type. default = void.
/// @tparam EV     Edge Value type. default = void.
/// @tparam GV     Graph Value type. default = void.
/// @tparam IntexT The type used for vertex & edge index into the internal vectors.
/// @tparam A      Allocator. default = std::allocator
/// @tparam ListT  inward_list|outward_list. Which edge list this is for.
///
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
class ual_vertex_edge_list {
public:
  class iterator;
  class const_iterator;

  using graph_type = undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;

  using vertex_type           = ual_vertex<VV, EV, GV, VId, VContainer, Alloc>;
  using vertex_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set            = VContainer<vertex_type, vertex_allocator_type>;
  using vertex_iterator       = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;
  using vertex_key_type       = VId;
  using vertex_index          = VId;

  using edge_value_type = EV;
  using edge_type       = ual_edge<VV, EV, GV, VId, VContainer, Alloc>;

  using edge_list_type                    = ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>;
  using vertex_edge_list_inward_link_type = ual_vertex_edge_list_link<VV, EV, GV, VId, VContainer, Alloc, inward_list>;
  using vertex_edge_list_outward_link_type =
        ual_vertex_edge_list_link<VV, EV, GV, VId, VContainer, Alloc, outward_list>;
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

    const_iterator(const graph_type& g, vertex_key_type ukey, const edge_type* uv = nullptr) noexcept
          : vertex_key_(ukey), edge_(const_cast<edge_type*>(uv)), graph_(const_cast<graph_type*>(&g)) {}

    const_iterator() noexcept                          = default;
    const_iterator(const const_iterator& rhs) noexcept = default;
    const_iterator(const_iterator&&) noexcept          = default;
    ~const_iterator() noexcept                         = default;

    const_iterator& operator=(const const_iterator&) noexcept = default;
    const_iterator& operator=(const_iterator&&) noexcept = default;

    reference operator*() const;
    pointer   operator->() const;

    const_iterator& operator++();
    const_iterator  operator++(int);

    const_iterator& operator--();
    const_iterator  operator--(int);

    bool operator==(const const_iterator& rhs) const noexcept;
    bool operator!=(const const_iterator& rhs) const noexcept;

    friend void swap(const_iterator& lhs, const_iterator& rhs) noexcept {
      swap(lhs.vertex_key_, rhs.vertex_key_);
      swap(lhs.edge_, rhs.edge_);
    }

    graph_type&       graph() { return *graph_; }
    const graph_type& graph() const { return *graph_; }

    vertex_key_type source_key() const { return vertex_key_; }

  protected:
    void advance();
    void retreat();

  protected:
    vertex_key_type vertex_key_ = numeric_limits<vertex_key_type>::max(); // source vertex for the list we're in
    edge_type*      edge_       = nullptr;                                // current edge (==nullptr for end)
    graph_type*     graph_      = nullptr;
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
    iterator& operator=(iterator&&) = default;

    iterator(const graph_type& g, vertex_key_type ukey, const edge_type* uv = nullptr) : const_iterator(g, ukey, uv) {}

    reference operator*() const;
    pointer   operator->() const;

    iterator& operator++();
    iterator  operator++(int);

    iterator& operator--();
    iterator  operator--(int);

    friend void swap(iterator& lhs, iterator& rhs) {
      swap(lhs.vertex_key_, rhs.vertex_key_);
      swap(lhs.edge_, rhs.edge_);
    }
  }; // end iterator

public:
  ual_vertex_edge_list() noexcept                            = default;
  ual_vertex_edge_list(const ual_vertex_edge_list&) noexcept = default;
  ~ual_vertex_edge_list() noexcept                           = default;
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
  void link_front(edge_type& uv, ual_vertex_edge_list_link<VV, EV, GV, VId, VContainer, Alloc, ListT>& uv_link);

  template <typename ListT>
  void link_back(edge_type& uv, ual_vertex_edge_list_link<VV, EV, GV, VId, VContainer, Alloc, ListT>& uv_link);

  template <typename ListT>
  void unlink(edge_type& uv, ual_vertex_edge_list_link<VV, EV, GV, VId, VContainer, Alloc, ListT>& uv_link);

  iterator       begin(graph_type& g, vertex_key_type ukey) noexcept;
  const_iterator begin(const graph_type& g, vertex_key_type ukey) const noexcept;
  const_iterator cbegin(const graph_type& g, vertex_key_type ukey) const noexcept;

  iterator       end(graph_type& g, vertex_key_type ukey) noexcept;
  const_iterator end(const graph_type& g, vertex_key_type ukey) const noexcept;
  const_iterator cend(const graph_type& g, vertex_key_type ukey) const noexcept;

  edge_range       edges(graph_type& g, vertex_key_type ukey) noexcept;
  const_edge_range edges(const graph_type& g, vertex_key_type ukey) const noexcept;

private:
  edge_type* head_ = nullptr;
  edge_type* tail_ = nullptr;
  size_type  size_ = 0;
};

///-------------------------------------------------------------------------------------
/// ual_vertex_edge_list_link
///
/// @tparam VV     Vertex Value type. default = void.
/// @tparam EV     Edge Value type. default = void.
/// @tparam GV     Graph Value type. default = void.
/// @tparam IntexT The type used for vertex & edge index into the internal vectors.
/// @tparam A      Allocator. default = std::allocator
/// @tparam ListT  inward_list|outward_list. Which edge list this is for.
///
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc,
          typename ListT>
class ual_vertex_edge_list_link {
public:
  using graph_type = undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;

  using vertex_type           = ual_vertex<VV, EV, GV, VId, VContainer, Alloc>;
  using vertex_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set            = VContainer<vertex_type, vertex_allocator_type>;
  using vertex_iterator       = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;
  using vertex_key_type       = VId;
  using vertex_index          = VId;

  using edge_type = ual_edge<VV, EV, GV, VId, VContainer, Alloc>;

  using edge_list_type      = ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>;
  using edge_list_link_type = ual_vertex_edge_list_link<VV, EV, GV, VId, VContainer, Alloc, ListT>;

public:
  ual_vertex_edge_list_link(vertex_key_type ukey) noexcept : vertex_key_(ukey) {}

  ual_vertex_edge_list_link() noexcept                                 = default;
  ual_vertex_edge_list_link(const ual_vertex_edge_list_link&) noexcept = default;
  ual_vertex_edge_list_link(ual_vertex_edge_list_link&&) noexcept      = default;
  ~ual_vertex_edge_list_link() noexcept                                = default;
  ual_vertex_edge_list_link& operator=(const ual_vertex_edge_list_link&) noexcept = default;
  ual_vertex_edge_list_link& operator=(ual_vertex_edge_list_link&&) noexcept = default;

public:
  vertex_key_type       vertex_key() const noexcept { return vertex_key_; }
  const_vertex_iterator vertex(const graph_type& g) const { return g.vertices().begin() + vertex_key_; }
  vertex_iterator       vertex(graph_type& g) { return g.vertices().begin() + vertex_key_; }

  edge_type*       next() noexcept { return next_; }
  const edge_type* next() const noexcept { return next_; }
  edge_type*       prev() noexcept { return prev_; }
  const edge_type* prev() const noexcept { return prev_; }

private:
  vertex_key_type vertex_key_ = numeric_limits<vertex_key_type>::max();
  edge_type*      next_       = nullptr;
  edge_type*      prev_       = nullptr;

  friend edge_list_type;
  friend edge_type;
};


///-------------------------------------------------------------------------------------
/// ual_edge
///
/// @tparam VV     Vertex Value type. default = void.
/// @tparam EV     Edge Value type. default = void.
/// @tparam GV     Graph Value type. default = void.
/// @tparam IntexT The type used for vertex & edge index into the internal vectors.
/// @tparam A      Allocator. default = std::allocator
///
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
class ual_edge
      : public ual_edge_value<EV, VV, GV, VId, VContainer, Alloc>
      , public ual_vertex_edge_list_link<VV, EV, GV, VId, VContainer, Alloc, inward_list>
      , public ual_vertex_edge_list_link<VV, EV, GV, VId, VContainer, Alloc, outward_list> {
public:
  using graph_type       = undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;
  using graph_value_type = GV;
  using base_value_type  = ual_edge_value<EV, VV, GV, VId, VContainer, Alloc>;

  using vertex_type           = ual_vertex<VV, EV, GV, VId, VContainer, Alloc>;
  using vertex_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set            = VContainer<vertex_type, vertex_allocator_type>;
  using vertex_iterator       = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;
  using vertex_key_type       = VId;
  using vertex_index          = VId;
  using vertex_value_type     = VV;

  using edge_key_type        = pair<vertex_key_type, vertex_key_type>;
  using edge_value_type      = EV;
  using edge_type            = ual_edge<VV, EV, GV, VId, VContainer, Alloc>;
  using edge_allocator_type  = typename allocator_traits<Alloc>::template rebind_alloc<edge_type>;
  using edge_index           = VId;
  using edge_size_type       = size_t;
  using edge_difference_type = ptrdiff_t;

  using edge_list_type                    = ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>;
  using vertex_edge_list_inward_link_type = ual_vertex_edge_list_link<VV, EV, GV, VId, VContainer, Alloc, inward_list>;
  using vertex_edge_list_outward_link_type =
        ual_vertex_edge_list_link<VV, EV, GV, VId, VContainer, Alloc, outward_list>;

protected:
  // noexcept is only defined for move ctor & assignment b/c the user-defined value type could
  // throw an exception in other cases
  ual_edge() noexcept                = default;
  ual_edge(const ual_edge&) noexcept = default;
  ual_edge(ual_edge&&) noexcept      = default;
  ~ual_edge() noexcept;
  ual_edge& operator=(ual_edge&) noexcept = default;
  ual_edge& operator=(ual_edge&&) noexcept = default;

#  if 1
  ual_edge(graph_type&, vertex_key_type ukey, vertex_key_type vkey) noexcept;
  ual_edge(graph_type&, vertex_key_type ukey, vertex_key_type vkey, const edge_value_type&) noexcept;
  ual_edge(graph_type&, vertex_key_type ukey, vertex_key_type vkey, edge_value_type&&) noexcept;
#  else
  ual_edge(graph_type&, vertex_type& u, vertex_type& v) noexcept;
  ual_edge(graph_type&, vertex_type& u, vertex_type& v, const edge_value_type&) noexcept;
  ual_edge(graph_type&, vertex_type& u, vertex_type& v, edge_value_type&&) noexcept;
#  endif

  ual_edge(graph_type&, vertex_iterator ui, vertex_iterator vi) noexcept;
  ual_edge(graph_type&, vertex_iterator ui, vertex_iterator vi, const edge_value_type&) noexcept;
  ual_edge(graph_type&, vertex_iterator ui, vertex_iterator vi, edge_value_type&&) noexcept;

  void link_front(vertex_type&, vertex_type&) noexcept;
  void link_back(vertex_type&, vertex_type&) noexcept;
  void unlink(vertex_type&, vertex_type&) noexcept;

public:
  vertex_iterator       source_vertex(graph_type&) noexcept;
  const_vertex_iterator source_vertex(const graph_type&) const noexcept;
  vertex_key_type       source_vertex_key(const graph_type&) const noexcept;

  vertex_iterator       target_vertex(graph_type&) noexcept;
  const_vertex_iterator target_vertex(const graph_type&) const noexcept;
  vertex_key_type       target_vertex_key(const graph_type&) const noexcept;

  vertex_iterator       other_vertex(graph_type&, const_vertex_iterator other) noexcept;
  const_vertex_iterator other_vertex(const graph_type&, const_vertex_iterator other) const noexcept;
  vertex_iterator       other_vertex(graph_type&, vertex_key_type other_key) noexcept;
  const_vertex_iterator other_vertex(const graph_type&, vertex_key_type other_key) const noexcept;

  vertex_key_type other_vertex_key(const graph_type&, const_vertex_iterator other) const noexcept;
  vertex_key_type other_vertex_key(const graph_type&, vertex_key_type other_key) const noexcept;

  edge_key_type edge_key(const graph_type& g) const noexcept;

  friend graph_type;     // the graph is the one to create & destroy edges because it owns the allocator
  friend vertex_type;    // vertex can also destroy its own edges
  friend edge_list_type; // for delete, when clearing the list

private: // CPO support via ADL (friend functions)
  // target_id(g, e) - get target vertex id from edge (works with raw edge reference)
  friend constexpr vertex_key_type target_id(const graph_type& g, const ual_edge& e) noexcept {
    return e.target_vertex_key(g);
  }
  // source_id(g, e) - get source vertex id from edge
  friend constexpr vertex_key_type source_id(const graph_type& g, const ual_edge& e) noexcept {
    return e.source_vertex_key(g);
  }
  // edge_value(g, e) - get edge value (only when EV is not void)
  friend constexpr decltype(auto) edge_value(graph_type&, ual_edge& e) noexcept
    requires (!std::is_void_v<EV>)
  {
    return (e.base_value_type::value);
  }
  friend constexpr decltype(auto) edge_value(const graph_type&, const ual_edge& e) noexcept
    requires (!std::is_void_v<EV>)
  {
    return (e.base_value_type::value);
  }
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
public: // Type Aliases
  using graph_type       = undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;
  using graph_value_type = GV;
  using allocator_type   = Alloc;

  using vertex_type            = ual_vertex<VV, EV, GV, VId, VContainer, Alloc>;
  using vertex_value_type      = VV;
  using vertex_key_type        = VId;
  using vertex_index_type      = VId;
  using vertex_allocator_type  = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set             = VContainer<vertex_type, vertex_allocator_type>;
  using vertex_size_type       = typename vertex_set::size_type;
  using vertex_difference_type = typename vertex_set::difference_type;

  using vertex_iterator       = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;
  using vertex_range          = vertex_set&;
  using const_vertex_range    = const vertex_set&;

  using edge_type            = ual_edge<VV, EV, GV, VId, VContainer, Alloc>;
  using edge_value_type      = EV;
  using edge_allocator_type  = typename allocator_traits<Alloc>::template rebind_alloc<edge_type>;
  using edge_key_type        = pair<vertex_key_type, vertex_key_type>; // <from,to>
  using edge_size_type       = typename edge_type::edge_size_type;
  using edge_difference_type = typename edge_type::edge_difference_type;

  using vertex_edge_iterator        = typename vertex_type::vertex_edge_iterator;
  using const_vertex_edge_iterator  = typename vertex_type::const_vertex_edge_iterator;
  using vertex_edge_range           = typename vertex_type::vertex_edge_range;
  using const_vertex_edge_range     = typename vertex_type::const_vertex_edge_range;
  using vertex_edge_size_type       = typename vertex_edge_iterator::size_type;
  using vertex_edge_difference_type = typename vertex_edge_iterator::difference_type;

  using vertex_vertex_iterator       = typename vertex_type::vertex_vertex_iterator;
  using const_vertex_vertex_iterator = typename vertex_type::const_vertex_vertex_iterator;
  using vertex_vertex_range          = typename vertex_type::vertex_vertex_range;
  using const_vertex_vertex_range    = typename vertex_type::const_vertex_vertex_range;
  using vertex_vertex_size_type      = typename vertex_type::vertex_vertex_size_type;

  // Edge iterators (nested classes)
  class edge_iterator;       // (defined below)
  class const_edge_iterator; // (defined below)
  using edge_range       = ranges::subrange<edge_iterator, edge_iterator, ranges::subrange_kind::sized>;
  using const_edge_range = ranges::subrange<const_edge_iterator, const_edge_iterator, ranges::subrange_kind::sized>;

  class const_edge_iterator {
  public:
    using iterator_category = forward_iterator_tag;
    using iterator_concept  = std::forward_iterator_tag;
    using value_type        = ual_edge<VV, EV, GV, VId, VContainer, Alloc>;
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

    const_edge_iterator() noexcept                               = default;
    const_edge_iterator(const const_edge_iterator& rhs) noexcept = default;
    ~const_edge_iterator() noexcept                              = default;
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
      vertex_key_type ukey = vertex_key(*g_, u_);
      if (++uv_ != u_->edges_end(*g_, ukey))
        return;

      // find next vertex with edge(s)
      ++u_;
      advance_vertex();
    }

    void advance_vertex() {
      // at exit, if u_ != g.vertices().end() then uv_ will refer to a valid edge
      for (; u_ != g_->vertices().end(); ++u_) {
        if (u_->edges_size() > 0) {
          vertex_key_type ukey = vertex_key(*g_, u_);
          uv_                  = u_->edges_begin(*g_, ukey);
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

    edge_iterator() noexcept : const_edge_iterator(){};
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
  vertex_set vertices_;
  edge_size_type edges_size_ = 0;
  [[no_unique_address]] vertex_allocator_type vertex_alloc_;
  [[no_unique_address]] edge_allocator_type edge_alloc_;
  
  // Note: graph_value_ is NOT here - it belongs in the derived class
  
protected: // Constructors (protected - for derived class use only)
  base_undirected_adjacency_list() = default;
  
  explicit base_undirected_adjacency_list(const allocator_type& alloc)
      : vertices_(alloc)
      , edge_alloc_(alloc) {}
  
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
    requires ranges::forward_range<ERng>
          && ranges::input_range<VRng>
          && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
          && std::regular_invocable<VProj, ranges::range_reference_t<VRng>>
  base_undirected_adjacency_list(const ERng& erng, const VRng& vrng, 
                                  const EProj& eproj, const VProj& vproj, 
                                  const allocator_type& alloc);
  
  // Initializer list constructors
  base_undirected_adjacency_list(
      const initializer_list<tuple<vertex_key_type, vertex_key_type, edge_value_type>>& ilist, 
      const allocator_type& alloc);
  
  base_undirected_adjacency_list(
      const initializer_list<tuple<vertex_key_type, vertex_key_type>>& ilist, 
      const allocator_type& alloc);
  
  // Helper method for validation
  void throw_unordered_edges() const;
  
public:
  // Methods will be added in Phase 2
};

///-------------------------------------------------------------------------------------
/// ual_vertex
///
/// @tparam VV     Vertex Value type. default = void.
/// @tparam EV     Edge Value type. default = void.
/// @tparam GV     Graph Value type. default = void.
/// @tparam IntexT The type used for vertex & edge index into the internal vectors.
/// @tparam A      Allocator. default = std::allocator
///
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
class ual_vertex : public ual_vertex_value<VV, EV, GV, VId, VContainer, Alloc> {
public:
  using graph_type       = undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;
  using graph_value_type = GV;
  using base_value_type  = ual_vertex_value<VV, EV, GV, VId, VContainer, Alloc>;

  using vertex_type           = ual_vertex<VV, EV, GV, VId, VContainer, Alloc>;
  using vertex_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set            = VContainer<vertex_type, vertex_allocator_type>;
  using vertex_iterator       = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;
  using vertex_key_type       = VId;
  using vertex_index          = VId;
  using vertex_value_type     = VV;

  using edge_value_type      = EV;
  using edge_type            = ual_edge<VV, EV, GV, VId, VContainer, Alloc>;
  using edge_allocator_type  = typename allocator_traits<Alloc>::template rebind_alloc<edge_type>;
  using edge_size_type       = typename edge_type::edge_size_type;
  using edge_difference_type = typename edge_type::edge_difference_type;

  using vertex_edge_list_type             = ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>;
  using vertex_edge_list_inward_link_type = ual_vertex_edge_list_link<VV, EV, GV, VId, VContainer, Alloc, inward_list>;
  using vertex_edge_list_outward_link_type =
        ual_vertex_edge_list_link<VV, EV, GV, VId, VContainer, Alloc, outward_list>;
  using vertex_edge_size_type      = typename vertex_edge_list_type::size_type;
  using vertex_edge_iterator       = typename vertex_edge_list_type::iterator;
  using const_vertex_edge_iterator = typename vertex_edge_list_type::const_iterator;
  using vertex_edge_range          = typename vertex_edge_list_type::edge_range;
  using const_vertex_edge_range    = typename vertex_edge_list_type::const_edge_range;

  using vertex_vertex_iterator       = ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>;
  using const_vertex_vertex_iterator = ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>;
  using vertex_vertex_range =
        ranges::subrange<vertex_vertex_iterator, vertex_vertex_iterator, ranges::subrange_kind::sized>;
  using const_vertex_vertex_range =
        ranges::subrange<const_vertex_vertex_iterator, const_vertex_vertex_iterator, ranges::subrange_kind::sized>;
  using vertex_vertex_size_type       = edge_size_type;
  using vertex_vertex_difference_type = edge_difference_type;

public:
  // noexcept is only defined for move ctor & assignment b/c the user-defined value type could
  // throw an exception in other cases
  ual_vertex()                      = default;
  ual_vertex(const ual_vertex&)     = default;
  ual_vertex(ual_vertex&&) noexcept = default;
  ~ual_vertex() noexcept            = default;
  ual_vertex& operator=(const ual_vertex&) = default;
  ual_vertex& operator=(ual_vertex&&) noexcept = default;

  ual_vertex(vertex_set& vertices, vertex_index index);
  ual_vertex(vertex_set& vertices, vertex_index index, const vertex_value_type&);
  ual_vertex(vertex_set& vertices, vertex_index index, vertex_value_type&&) noexcept;

public:
#  if 0
  vertex_key_type       vertex_key(const graph_type&) const noexcept;
#  endif
  vertex_edge_size_type edges_size() const;

  vertex_edge_iterator erase_edge(graph_type&, vertex_edge_iterator);
  vertex_edge_iterator erase_edge(graph_type&, vertex_edge_iterator, vertex_edge_iterator);
  void                 clear_edges(graph_type&);

  vertex_edge_iterator       edges_begin(graph_type&, vertex_key_type ukey) noexcept;
  const_vertex_edge_iterator edges_begin(const graph_type&, vertex_key_type ukey) const noexcept;
  const_vertex_edge_iterator edges_cbegin(const graph_type&, vertex_key_type ukey) const noexcept;

  vertex_edge_iterator       edges_end(graph_type&, vertex_key_type ukey) noexcept;
  const_vertex_edge_iterator edges_end(const graph_type&, vertex_key_type ukey) const noexcept;
  const_vertex_edge_iterator edges_cend(const graph_type&, vertex_key_type ukey) const noexcept;

  vertex_edge_range       edges(graph_type&, vertex_key_type ukey);
  const_vertex_edge_range edges(const graph_type&, vertex_key_type ukey) const;

  edge_type&       edge_front(graph_type&) noexcept;
  const edge_type& edge_front(const graph_type&) const noexcept;

  edge_type&       edge_back(graph_type&) noexcept;
  const edge_type& edge_back(const graph_type&) const noexcept;

  vertex_vertex_iterator       vertices_begin(graph_type&, vertex_key_type ukey);
  const_vertex_vertex_iterator vertices_begin(const graph_type&, vertex_key_type ukey) const;
  const_vertex_vertex_iterator vertices_cbegin(const graph_type&, vertex_key_type ukey) const;

  vertex_vertex_iterator       vertices_end(graph_type&, vertex_key_type ukey);
  const_vertex_vertex_iterator vertices_end(const graph_type&, vertex_key_type ukey) const;
  const_vertex_vertex_iterator vertices_cend(const graph_type&, vertex_key_type ukey) const;

  vertex_vertex_range       vertices(graph_type&, vertex_key_type ukey);
  const_vertex_vertex_range vertices(const graph_type&, vertex_key_type ukey) const;

  vertex_vertex_size_type vertices_size(const graph_type&) const;

protected:
  void erase_edge(graph_type&, edge_type*);

  // Removed: e_begin/e_end were legacy protected methods using const_cast, replaced by public edges_begin/edges_end

private:
  vertex_edge_list_type edges_;
  friend edge_type;
  friend vertex_edge_list_inward_link_type;
  friend vertex_edge_list_outward_link_type;
};

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
class ual_const_vertex_vertex_iterator {
public:
  using this_t = ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>;

  using graph_type = undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;

  using vertex_type           = ual_vertex<VV, EV, GV, VId, VContainer, Alloc>;
  using vertex_value_type     = VV;
  using vertex_key_type       = VId;
  using vertex_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set            = VContainer<vertex_type, vertex_allocator_type>;

  using vertex_iterator       = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;

  using edge_list_type             = ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>;
  using vertex_edge_size_type      = typename edge_list_type::size_type;
  using vertex_edge_iterator       = typename edge_list_type::iterator;
  using const_vertex_edge_iterator = typename edge_list_type::const_iterator;

  using edge_type       = ual_edge<VV, EV, GV, VId, VContainer, Alloc>;
  using edge_value_type = typename edge_type::edge_value_type;
  using edge_key_type   = typename edge_type::edge_key_type; // <from,to>

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
  constexpr ual_const_vertex_vertex_iterator(vertex_edge_iterator const& uv);

  constexpr ual_const_vertex_vertex_iterator()                                        = default;
  constexpr ual_const_vertex_vertex_iterator(const ual_const_vertex_vertex_iterator&) = default;
  constexpr ual_const_vertex_vertex_iterator(ual_const_vertex_vertex_iterator&&)      = default;
  ~ual_const_vertex_vertex_iterator()                                                 = default;

  constexpr ual_const_vertex_vertex_iterator& operator=(const ual_const_vertex_vertex_iterator&) = default;
  constexpr ual_const_vertex_vertex_iterator& operator=(ual_const_vertex_vertex_iterator&&) = default;

public:
  constexpr graph_type&       graph() noexcept;
  constexpr const graph_type& graph() const noexcept;

  constexpr const_vertex_iterator other_vertex() const;
  constexpr vertex_key_type       other_vertex_key() const;

  constexpr reference operator*() const noexcept;
  constexpr pointer   operator->() const noexcept;

  constexpr ual_const_vertex_vertex_iterator& operator++() noexcept;
  constexpr ual_const_vertex_vertex_iterator  operator++(int) noexcept;

  constexpr ual_const_vertex_vertex_iterator& operator--() noexcept;
  constexpr ual_const_vertex_vertex_iterator  operator--(int) noexcept;

  constexpr bool operator==(const ual_const_vertex_vertex_iterator& rhs) const noexcept;
  constexpr bool operator!=(const ual_const_vertex_vertex_iterator& rhs) const noexcept;

  friend void swap(ual_const_vertex_vertex_iterator& lhs, ual_const_vertex_vertex_iterator& rhs) {
    swap(lhs.uv_, rhs.uv_);
  }

protected:
  vertex_edge_iterator uv_;
};

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
class ual_vertex_vertex_iterator : public ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc> {
public:
  using this_t = ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>;
  using base_t = ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>;

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
  constexpr ual_vertex_vertex_iterator(vertex_edge_iterator const& uv);

  constexpr ual_vertex_vertex_iterator()                                  = default;
  constexpr ual_vertex_vertex_iterator(const ual_vertex_vertex_iterator&) = default;
  constexpr ual_vertex_vertex_iterator(ual_vertex_vertex_iterator&&)      = default;
  ~ual_vertex_vertex_iterator()                                           = default;

  constexpr ual_vertex_vertex_iterator& operator=(const ual_vertex_vertex_iterator&) = default;
  constexpr ual_vertex_vertex_iterator& operator=(ual_vertex_vertex_iterator&&) = default;

public:
  constexpr vertex_iterator other_vertex();

  constexpr const_reference operator*() const;
  constexpr const_pointer   operator->() const;

  constexpr ual_vertex_vertex_iterator& operator++();
  constexpr ual_vertex_vertex_iterator  operator++(int);

  constexpr ual_vertex_vertex_iterator& operator--() noexcept;
  constexpr ual_vertex_vertex_iterator  operator--(int) noexcept;

  constexpr bool operator==(const ual_vertex_vertex_iterator& rhs) const noexcept;
  constexpr bool operator!=(const ual_vertex_vertex_iterator& rhs) const noexcept;

  friend void swap(ual_vertex_vertex_iterator& lhs, ual_vertex_vertex_iterator& rhs) { swap(lhs.uv_, rhs.uv_); }
};

///-------------------------------------------------------------------------------------
/// ITERATOR INVALIDATION RULES
///-------------------------------------------------------------------------------------
/// 
/// Vertex Iterators:
///   - Invalidated by: create_vertex() if reallocation occurs, clear()
///   - NOT invalidated by: create_edge(), erase_edge()
///   - Note: Use vertex keys instead of iterators for stable references
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
///   - Use vertex keys for stable references across operations
///
/// Thread Safety:
///   - NOT thread-safe for concurrent modifications
///   - Concurrent reads safe if no writes
///   - External synchronization required for write operations
///
///-------------------------------------------------------------------------------------

/// A simple undirected adjacency list (graph).
///
/// @tparam VV              Vertex Value type. default = void.
/// @tparam EV              Edge Value type. default = void.
/// @tparam GV              Graph Value type. default = void.
/// @tparam IntexT          The type used for vertex & edge index into the internal vectors.
/// @tparam VContainer<V,A> Random-access container type used to store vertices (V) with allocator (A).
/// @tparam Alloc           Allocator. default = std::allocator
///
// clang-format off
template <typename                                VV,
          typename                                EV,
          typename                                GV,
          integral                                VId,
          template <typename V, typename A> class VContainer,
          typename                                Alloc>
class undirected_adjacency_list 
    : public base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>
// clang-format on
{
public:
  using base_type = base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;
  
  // Note: Access base class members using this-> or base_type::
  // (using declarations don't work in constructor initializer lists)
  
  using graph_type       = undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>;
  using graph_value_type = GV;
  using allocator_type   = Alloc;

  using vertex_type            = ual_vertex<VV, EV, GV, VId, VContainer, Alloc>;
  using vertex_value_type      = VV;
  using vertex_key_type        = VId;
  using vertex_index_type      = VId;
  using vertex_allocator_type  = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set             = VContainer<vertex_type, vertex_allocator_type>;
  using vertex_size_type       = typename vertex_set::size_type;
  using vertex_difference_type = typename vertex_set::difference_type;

  using vertex_iterator       = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;
  using vertex_range          = vertex_set&;
  using const_vertex_range    = const vertex_set&;

  using edge_type            = ual_edge<VV, EV, GV, VId, VContainer, Alloc>;
  using edge_value_type      = EV;
  using edge_allocator_type  = typename allocator_traits<Alloc>::template rebind_alloc<edge_type>;
  using edge_key_type        = pair<vertex_key_type, vertex_key_type>; // <from,to>
  using edge_size_type       = typename edge_type::edge_size_type;
  using edge_difference_type = typename edge_type::edge_difference_type;
  // edge_set, edge_index_type n/a

  using vertex_edge_iterator        = typename vertex_type::vertex_edge_iterator;
  using const_vertex_edge_iterator  = typename vertex_type::const_vertex_edge_iterator;
  using vertex_edge_range           = typename vertex_type::vertex_edge_range;
  using const_vertex_edge_range     = typename vertex_type::const_vertex_edge_range;
  using vertex_edge_size_type       = typename vertex_edge_iterator::size_type;
  using vertex_edge_difference_type = typename vertex_edge_iterator::difference_type;

  using vertex_vertex_iterator       = typename vertex_type::vertex_vertex_iterator;
  using const_vertex_vertex_iterator = typename vertex_type::const_vertex_vertex_iterator;
  using vertex_vertex_range          = typename vertex_type::vertex_vertex_range;
  using const_vertex_vertex_range    = typename vertex_type::const_vertex_vertex_range;
  using vertex_vertex_size_type      = typename vertex_type::vertex_vertex_size_type;

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
  
  template <typename GV_ = GV>
    requires (!std::is_void_v<GV_> && !std::is_same_v<std::remove_cvref_t<GV_>, undirected_adjacency_list>)
  undirected_adjacency_list(const GV_& gv, 
                            const allocator_type& alloc = allocator_type());
  template <typename GV_ = GV>
    requires (!std::is_void_v<GV_> && !std::is_same_v<std::remove_cvref_t<GV_>, undirected_adjacency_list>)
  undirected_adjacency_list(GV_&& gv, 
                            const allocator_type& alloc = allocator_type());
  // clang-format on

  // The following constructors will load edges (and vertices) into the graph
  //
  // The number of vertices is guaranteed to match the highest vertex key in
  // the edges. Edges are scanned first to determine the highest number and
  // the vertices are resized to match the number.
  //
  // Accessor functions are used to return the edge_key_type,
  // edge_value_type and vertex_value_type.
  //
  // The order visited in the vertices determines their index (and
  // key/identity) in the internal vertices container. The edge keys use those
  // values and are also expected to be ordered by their first (in) vertex
  // key and an exception is thrown if they aren't in order. For these
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
  /// @param ilist Initializer list of tuples with source_vertex_key,
  ///              target_vertex_key and the edge value.
  /// @param alloc Allocator.
  ///
  // clang-format off
  undirected_adjacency_list(
    const initializer_list<
          tuple<vertex_key_type, vertex_key_type, edge_value_type>>& ilist,
    const Alloc&                                                     alloc = Alloc());
  // clang-format on

  /// Constructor for easy creation of a graph that takes an initializer
  /// list with edge values.
  ///
  /// @param ilist Initializer list of tuples with source_vertex_key and
  ///              target_vertex_key.
  /// @param alloc Allocator.
  ///
  // clang-format off
  undirected_adjacency_list(
    const initializer_list<tuple<vertex_key_type, vertex_key_type>>& ilist,
    const Alloc&                                                     alloc = Alloc());
  // clang-format on

  ~undirected_adjacency_list();

  undirected_adjacency_list& operator=(const undirected_adjacency_list& other);
  undirected_adjacency_list& operator=(undirected_adjacency_list&&) = default;

public: // Accessors
  /// @brief Get the edge allocator.
  /// @complexity O(1)
  constexpr edge_allocator_type edge_allocator() const noexcept;

  /// @brief Access the vertex container.
  /// @return Reference to the internal vertex container.
  /// @complexity O(1)
  constexpr vertex_set&       vertices();
  constexpr const vertex_set& vertices() const;

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

  /// @brief Find a vertex by its key (direct container access).
  /// @param key The vertex key to find.
  /// @return Iterator to the vertex, or end() if not found.
  /// @complexity O(1)
  /// @note This is distinct from the find_vertex CPO. Use this for direct container operations.
  vertex_iterator       try_find_vertex(vertex_key_type);
  const_vertex_iterator try_find_vertex(vertex_key_type) const;

  /// @brief Get the number of edges in the graph.
  /// @return Edge count (each undirected edge counted TWICE - once from each endpoint).
  /// @note For unique edge count, divide by 2.
  /// @complexity O(1)
  constexpr edge_size_type edges_size() const noexcept;

  /// @brief Get iterator to first edge in the graph.
  /// @note Iterates ALL edges from ALL vertices. Each edge visited twice (from both endpoints).
  /// @complexity O(1)
  constexpr edge_iterator       edges_begin() { return edge_iterator(*this, begin()); }
  constexpr const_edge_iterator edges_begin() const {
    return const_edge_iterator(*this, const_cast<graph_type&>(*this).begin());
  }
  constexpr const_edge_iterator edges_cbegin() const {
    return const_edge_iterator(*this, const_cast<graph_type&>(*this).cbegin());
  }

  /// @brief Get iterator to one-past-last edge.
  /// @complexity O(1)
  constexpr edge_iterator       edges_end() { return edge_iterator(*this, end()); }
  constexpr const_edge_iterator edges_end() const {
    return const_edge_iterator(*this, const_cast<graph_type&>(*this).end());
  }
  constexpr const_edge_iterator edges_cend() const {
    return const_edge_iterator(*this, const_cast<graph_type&>(*this).end());
  }

  /// @brief Get range of all edges.
  /// @note Each undirected edge appears twice in iteration (once from each endpoint).
  /// @complexity O(1) to create range, O(V+E) to iterate.
  edge_range       edges() { return {edges_begin(), edges_end(), this->edges_size_}; }
  const_edge_range edges() const { return {edges_begin(), edges_end(), this->edges_size_}; }

  // Graph value accessors
  /// @brief Access the graph-level value.
  /// @return Reference to the graph value.
  /// @complexity O(1)
  /// @note Only available when GV is not void.
  template <typename GV_ = GV>
    requires (!std::is_void_v<GV_>)
  graph_value_type& graph_value() noexcept {
    return graph_value_;
  }
  
  template <typename GV_ = GV>
    requires (!std::is_void_v<GV_>)
  const graph_value_type& graph_value() const noexcept {
    return graph_value_;
  }

public: // Vertex creation
  /// @brief Create a new vertex with default value.
  /// @return Iterator to the newly created vertex.
  /// @complexity O(1) amortized.
  /// @invalidates Vertex iterators if reallocation occurs.
  vertex_iterator create_vertex();
  
  /// @brief Create a new vertex with the given value (move).
  /// @param val The value to move into the vertex.
  /// @return Iterator to the newly created vertex.
  /// @complexity O(1) amortized.
  /// @invalidates Vertex iterators if reallocation occurs.
  vertex_iterator create_vertex(vertex_value_type&&);

  /// @brief Create a new vertex with the given value (copy).
  /// @tparam VV2 Type convertible to vertex_value_type.
  /// @param val The value to copy into the vertex.
  /// @return Iterator to the newly created vertex.
  /// @complexity O(1) amortized.
  /// @invalidates Vertex iterators if reallocation occurs.
  template <class VV2>
    requires std::constructible_from<vertex_value_type, const VV2&>
  vertex_iterator create_vertex(const VV2&);

public: // Edge creation
  /// @brief Create an edge between two vertices (by key).
  /// @param ukey Source vertex key.
  /// @param vkey Target vertex key.
  /// @return Iterator to the newly created edge.
  /// @complexity O(1).
  /// @precondition Both vertex keys must be valid.
  /// @invalidates No iterators invalidated.
  vertex_edge_iterator create_edge(vertex_key_type, vertex_key_type);
  
  /// @brief Create an edge with value between two vertices (by key, move value).
  /// @param ukey Source vertex key.
  /// @param vkey Target vertex key.
  /// @param val Edge value to move.
  /// @return Iterator to the newly created edge.
  /// @complexity O(1).
  /// @precondition Both vertex keys must be valid.
  /// @invalidates No iterators invalidated.
  vertex_edge_iterator create_edge(vertex_key_type, vertex_key_type, edge_value_type&&);

  /// @brief Create an edge with value between two vertices (by key, copy value).
  /// @tparam EV2 Type convertible to edge_value_type.
  /// @param ukey Source vertex key.
  /// @param vkey Target vertex key.
  /// @param val Edge value to copy.
  /// @return Iterator to the newly created edge.
  /// @complexity O(1).
  /// @precondition Both vertex keys must be valid.
  /// @invalidates No iterators invalidated.
  template <class EV2>
    requires std::constructible_from<edge_value_type, const EV2&>
  vertex_edge_iterator create_edge(vertex_key_type,
                                   vertex_key_type,
                                   const EV2&);

  /// @brief Create an edge between two vertices (by iterator).
  /// @param u Source vertex iterator.
  /// @param v Target vertex iterator.
  /// @return Iterator to the newly created edge.
  /// @complexity O(1).
  /// @precondition Both iterators must be valid and dereferenceable.
  /// @invalidates No iterators invalidated.
  vertex_edge_iterator create_edge(vertex_iterator, vertex_iterator);
  
  /// @brief Create an edge with value between two vertices (by iterator, move value).
  /// @param u Source vertex iterator.
  /// @param v Target vertex iterator.
  /// @param val Edge value to move.
  /// @return Iterator to the newly created edge.
  /// @complexity O(1).
  /// @precondition Both iterators must be valid and dereferenceable.
  /// @invalidates No iterators invalidated.
  vertex_edge_iterator create_edge(vertex_iterator, vertex_iterator, edge_value_type&&);

  /// @brief Create an edge with value between two vertices (by iterator, copy value).
  /// @tparam EV2 Type convertible to edge_value_type.
  /// @param u Source vertex iterator.
  /// @param v Target vertex iterator.
  /// @param val Edge value to copy.
  /// @return Iterator to the newly created edge.
  /// @complexity O(1).
  /// @precondition Both iterators must be valid and dereferenceable.
  /// @invalidates No iterators invalidated.
  template <class EV2>
    requires std::constructible_from<edge_value_type, const EV2&>
  vertex_edge_iterator create_edge(vertex_iterator,
                                   vertex_iterator,
                                   const EV2&);

public: // Edge removal
  /// @brief Erase an edge from the graph.
  /// @param pos Iterator to the edge to erase.
  /// @return Iterator to the next edge.
  /// @complexity O(1) to unlink from both vertex edge lists.
  /// @precondition Iterator must be valid and dereferenceable.
  /// @invalidates Only the erased edge iterator. Other edge iterators remain valid.
  edge_iterator erase_edge(edge_iterator);

public: // Graph operations
  /// @brief Remove all vertices and edges from the graph.
  /// @complexity O(V + E).
  /// @invalidates All iterators, pointers, and references.
  void clear();
  
  /// @brief Swap contents with another graph.
  /// @param other The graph to swap with.
  /// @complexity O(1).
  /// @invalidates All iterators, pointers, and references to both graphs.
  void swap(undirected_adjacency_list&);

protected:
  void reserve_vertices(vertex_size_type);
  void resize_vertices(vertex_size_type);
  void resize_vertices(vertex_size_type, const vertex_value_type&);
  
  //vertex_iterator finalize_outward_edges(vertex_range);

private:
  [[no_unique_address]] GV      graph_value_{};
  friend vertex_type;

private: // CPO support via ADL (friend functions)
  // These friend functions enable CPO customization via ADL
  
  // vertices(g) - returns vertex_descriptor_view for CPO compatibility
  friend constexpr auto vertices(undirected_adjacency_list& g) noexcept { 
    return vertex_descriptor_view(g.vertices_);
  }
  friend constexpr auto vertices(const undirected_adjacency_list& g) noexcept { 
    return vertex_descriptor_view(g.vertices_);
  }
  
  // num_vertices(g)
  friend constexpr auto num_vertices(const undirected_adjacency_list& g) noexcept { 
    return g.vertices_.size(); 
  }
  
  // num_edges(g) - total edge count
  friend constexpr auto num_edges(const undirected_adjacency_list& g) noexcept { 
    return g.edges_size_; 
  }
  
  // has_edge(g) - check if graph has any edges
  friend constexpr bool has_edge(const undirected_adjacency_list& g) noexcept { 
    return g.edges_size_ > 0; 
  }

  // find_vertex(g, id) - returns view iterator yielding vertex_descriptor
  friend constexpr auto find_vertex(undirected_adjacency_list& g, vertex_key_type id) noexcept {
    using container_iter = typename vertex_set::iterator;
    using view_type      = vertex_descriptor_view<container_iter>;
    using view_iterator  = typename view_type::iterator;
    using storage_type   = typename view_iterator::value_type::storage_type;

    if (id >= static_cast<vertex_key_type>(g.vertices_.size())) {
      return view_iterator{static_cast<storage_type>(g.vertices_.size())};
    }
    return view_iterator{static_cast<storage_type>(id)};
  }
  friend constexpr auto find_vertex(const undirected_adjacency_list& g, vertex_key_type id) noexcept {
    using container_iter = typename vertex_set::const_iterator;
    using view_type      = vertex_descriptor_view<container_iter>;
    using view_iterator  = typename view_type::iterator;
    using storage_type   = typename view_iterator::value_type::storage_type;

    if (id >= static_cast<vertex_key_type>(g.vertices_.size())) {
      return view_iterator{static_cast<storage_type>(g.vertices_.size())};
    }
    return view_iterator{static_cast<storage_type>(id)};
  }

  // graph_value(g) - get graph value (only when GV is not void)
  friend constexpr graph_value_type& graph_value(undirected_adjacency_list& g) noexcept
    requires (!std::is_void_v<GV>)
  {
    return g.graph_value_;
  }
  friend constexpr const graph_value_type& graph_value(const undirected_adjacency_list& g) noexcept
    requires (!std::is_void_v<GV>)
  {
    return g.graph_value_;
  }

  // vertex_value(g, u) - get vertex value (only when VV is not void)
  template <typename U>
    requires vertex_descriptor_type<U> && (!std::is_void_v<VV>)
  friend constexpr decltype(auto) vertex_value(undirected_adjacency_list& g, const U& u) noexcept {
    auto& vtx = u.inner_value(g.vertices_);
    return (vtx.base_value_type::value);
  }
  template <typename U>
    requires vertex_descriptor_type<U> && (!std::is_void_v<VV>)
  friend constexpr decltype(auto) vertex_value(const undirected_adjacency_list& g, const U& u) noexcept {
    const auto& vtx = u.inner_value(g.vertices_);
    return (vtx.base_value_type::value);
  }

  // edges(g, u) - get edges from a vertex (returns edge_descriptor_view)
  template <typename U>
    requires vertex_descriptor_type<U>
  friend constexpr auto edges(undirected_adjacency_list& g, const U& u) noexcept {
    auto uid = static_cast<vertex_key_type>(u.vertex_id());
    auto& vtx = g.vertices_[uid];
    auto edges_rng = vtx.edges(g, uid);
    using edge_iter_t = vertex_edge_iterator;
    using vertex_iter_t = typename U::iterator_type;
    return edge_descriptor_view<edge_iter_t, vertex_iter_t>(edges_rng, u);
  }
  template <typename U>
    requires vertex_descriptor_type<U>
  friend constexpr auto edges(const undirected_adjacency_list& g, const U& u) noexcept {
    auto uid = static_cast<vertex_key_type>(u.vertex_id());
    const auto& vtx = g.vertices_[uid];
    auto edges_rng = vtx.edges(g, uid);
    using edge_iter_t = const_vertex_edge_iterator;
    using vertex_iter_t = typename U::iterator_type;
    return edge_descriptor_view<edge_iter_t, vertex_iter_t>(edges_rng, u);
  }

  // degree(g, u) - get number of edges from a vertex
  template <typename U>
    requires vertex_descriptor_type<U>
  friend constexpr auto degree(const undirected_adjacency_list& g, const U& u) noexcept {
    auto uid = static_cast<vertex_key_type>(u.vertex_id());
    return g.vertices_[uid].edges_size();
  }

  // target_id(g, edge_descriptor) - get target vertex id from edge descriptor
  // For undirected graphs, the target is the "other" vertex relative to the source we're iterating from
  template <typename E>
    requires edge_descriptor_type<E>
  friend constexpr vertex_key_type target_id(const undirected_adjacency_list& g, const E& e) noexcept {
    return e.value()->other_vertex_key(g, static_cast<vertex_key_type>(e.source_id()));
  }

  // source_id(g, edge_descriptor) - get source vertex id from edge descriptor
  // For undirected graphs, the source is the vertex we're iterating from (stored in descriptor)
  template <typename E>
    requires edge_descriptor_type<E>
  friend constexpr vertex_key_type source_id([[maybe_unused]] const undirected_adjacency_list& g, const E& e) noexcept {
    return static_cast<vertex_key_type>(e.source_id());
  }

  // target(g, edge_descriptor) - get target vertex descriptor from edge descriptor
  template <typename E>
    requires edge_descriptor_type<E>
  friend constexpr auto target(undirected_adjacency_list& g, const E& e) noexcept {
    auto tid = target_id(g, e);
    return *find_vertex(g, tid);
  }
  template <typename E>
    requires edge_descriptor_type<E>
  friend constexpr auto target(const undirected_adjacency_list& g, const E& e) noexcept {
    auto tid = target_id(g, e);
    return *find_vertex(g, tid);
  }

  // source(g, edge_descriptor) - get source vertex descriptor from edge descriptor
  template <typename E>
    requires edge_descriptor_type<E>
  friend constexpr auto source(undirected_adjacency_list& g, const E& e) noexcept {
    auto sid = source_id(g, e);
    return *find_vertex(g, sid);
  }
  template <typename E>
    requires edge_descriptor_type<E>
  friend constexpr auto source(const undirected_adjacency_list& g, const E& e) noexcept {
    auto sid = source_id(g, e);
    return *find_vertex(g, sid);
  }

  // find_vertex_edge(g, u, vid) - find edge from vertex descriptor u to vertex with id vid
  // Returns an edge_descriptor
  template <typename U>
    requires vertex_descriptor_type<U>
  friend constexpr auto find_vertex_edge(undirected_adjacency_list& g, const U& u, vertex_key_type vid) noexcept {
    auto uid = static_cast<vertex_key_type>(u.vertex_id());
    auto& vtx = g.vertices_[uid];
    auto edges_rng = vtx.edges(g, uid);
    using edge_iter_t = vertex_edge_iterator;
    using vertex_iter_t = typename U::iterator_type;
    auto edge_view = edge_descriptor_view<edge_iter_t, vertex_iter_t>(edges_rng, u);
    for (auto it = edge_view.begin(); it != edge_view.end(); ++it) {
      if (target_id(g, *it) == vid) {
        return *it;
      }
    }
    // Return end edge descriptor (will be invalid if dereferenced)
    return *edge_view.end();
  }
  template <typename U>
    requires vertex_descriptor_type<U>
  friend constexpr auto find_vertex_edge(const undirected_adjacency_list& g, const U& u, vertex_key_type vid) noexcept {
    auto uid = static_cast<vertex_key_type>(u.vertex_id());
    const auto& vtx = g.vertices_[uid];
    auto edges_rng = vtx.edges(g, uid);
    using edge_iter_t = const_vertex_edge_iterator;
    using vertex_iter_t = typename U::iterator_type;
    auto edge_view = edge_descriptor_view<edge_iter_t, vertex_iter_t>(edges_rng, u);
    for (auto it = edge_view.begin(); it != edge_view.end(); ++it) {
      if (target_id(g, *it) == vid) {
        return *it;
      }
    }
    return *edge_view.end();
  }

  // find_vertex_edge(g, u, v) - find edge between two vertex descriptors
  template <typename U, typename V>
    requires vertex_descriptor_type<U> && vertex_descriptor_type<V>
  friend constexpr auto find_vertex_edge(undirected_adjacency_list& g, const U& u, const V& v) noexcept {
    return find_vertex_edge(g, u, static_cast<vertex_key_type>(v.vertex_id()));
  }
  template <typename U, typename V>
    requires vertex_descriptor_type<U> && vertex_descriptor_type<V>
  friend constexpr auto find_vertex_edge(const undirected_adjacency_list& g, const U& u, const V& v) noexcept {
    return find_vertex_edge(g, u, static_cast<vertex_key_type>(v.vertex_id()));
  }

  // find_vertex_edge(g, uid, vid) - find edge between two vertex ids
  friend constexpr auto find_vertex_edge(undirected_adjacency_list& g, vertex_key_type uid, vertex_key_type vid) noexcept {
    auto u = *find_vertex(g, uid);
    return find_vertex_edge(g, u, vid);
  }
  friend constexpr auto find_vertex_edge(const undirected_adjacency_list& g, vertex_key_type uid, vertex_key_type vid) noexcept {
    auto u = *find_vertex(g, uid);
    return find_vertex_edge(g, u, vid);
  }
};

///-------------------------------------------------------------------------------------
/// undirected_adjacency_list<VV, EV, void, ...> - Specialization for GV=void
///
/// @brief Specialization of undirected_adjacency_list when no graph value is needed.
///
/// This specialization removes the graph_value_ member and graph_value() accessor
/// to provide zero memory overhead when graph-level values aren't needed.
///
/// All other functionality remains identical to the primary template.
///-------------------------------------------------------------------------------------
template <typename VV,
          typename EV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
class undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>
    : public base_undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>
{
public:
  using base_type = base_undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>;
  
  // Note: Access base class members using this-> or base_type::
  // (using declarations don't work in constructor initializer lists)
  
  using graph_type       = undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>;
  using graph_value_type = void;
  using allocator_type   = Alloc;

  using vertex_type            = ual_vertex<VV, EV, void, VId, VContainer, Alloc>;
  using vertex_value_type      = VV;
  using vertex_key_type        = VId;
  using vertex_index_type      = VId;
  using vertex_allocator_type  = typename allocator_traits<Alloc>::template rebind_alloc<vertex_type>;
  using vertex_set             = VContainer<vertex_type, vertex_allocator_type>;
  using vertex_size_type       = typename vertex_set::size_type;
  using vertex_difference_type = typename vertex_set::difference_type;

  using vertex_iterator       = typename vertex_set::iterator;
  using const_vertex_iterator = typename vertex_set::const_iterator;
  using vertex_range          = vertex_set&;
  using const_vertex_range    = const vertex_set&;

  using edge_type            = ual_edge<VV, EV, void, VId, VContainer, Alloc>;
  using edge_value_type      = EV;
  using edge_allocator_type  = typename allocator_traits<Alloc>::template rebind_alloc<edge_type>;
  using edge_key_type        = pair<vertex_key_type, vertex_key_type>;
  using edge_size_type       = typename edge_type::edge_size_type;
  using edge_difference_type = typename edge_type::edge_difference_type;

  using vertex_edge_iterator        = typename vertex_type::vertex_edge_iterator;
  using const_vertex_edge_iterator  = typename vertex_type::const_vertex_edge_iterator;
  using vertex_edge_range           = typename vertex_type::vertex_edge_range;
  using const_vertex_edge_range     = typename vertex_type::const_vertex_edge_range;
  using vertex_edge_size_type       = typename vertex_edge_iterator::size_type;
  using vertex_edge_difference_type = typename vertex_edge_iterator::difference_type;

  using vertex_vertex_iterator       = typename vertex_type::vertex_vertex_iterator;
  using const_vertex_vertex_iterator = typename vertex_type::const_vertex_vertex_iterator;
  using vertex_vertex_range          = typename vertex_type::vertex_vertex_range;
  using const_vertex_vertex_range    = typename vertex_type::const_vertex_vertex_range;
  using vertex_vertex_size_type      = typename vertex_type::vertex_vertex_size_type;

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

  template <typename ERng, 
            typename VRng, 
            typename EProj = std::identity, 
            typename VProj = std::identity>
    requires ranges::forward_range<ERng> 
          && ranges::input_range<VRng>
          && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
          && std::regular_invocable<VProj, ranges::range_reference_t<VRng>>
  undirected_adjacency_list(const ERng&  erng,
                            const VRng&  vrng,
                            const EProj& eproj = {},
                            const VProj& vproj = {},
                            const Alloc& alloc = Alloc());

  template <typename ERng, typename EProj = std::identity>
    requires ranges::forward_range<ERng>
          && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
  undirected_adjacency_list(const ERng&  erng, 
                            const EProj& eproj = {}, 
                            const Alloc& alloc = Alloc());

  undirected_adjacency_list(
    const initializer_list<
          tuple<vertex_key_type, vertex_key_type, edge_value_type>>& ilist,
    const Alloc&                                                     alloc = Alloc());

  undirected_adjacency_list(
    const initializer_list<tuple<vertex_key_type, vertex_key_type>>& ilist,
    const Alloc&                                                     alloc = Alloc());

  ~undirected_adjacency_list();

  undirected_adjacency_list& operator=(const undirected_adjacency_list& other);
  undirected_adjacency_list& operator=(undirected_adjacency_list&&) = default;

public:
  constexpr edge_allocator_type edge_allocator() const noexcept;

  constexpr vertex_set&       vertices();
  constexpr const vertex_set& vertices() const;

  constexpr vertex_iterator       begin();
  constexpr const_vertex_iterator begin() const;
  constexpr const_vertex_iterator cbegin() const;

  constexpr vertex_iterator       end();
  constexpr const_vertex_iterator end() const;
  constexpr const_vertex_iterator cend() const;

  vertex_iterator       try_find_vertex(vertex_key_type);
  const_vertex_iterator try_find_vertex(vertex_key_type) const;

  constexpr edge_size_type edges_size() const noexcept;

  constexpr edge_iterator       edges_begin() { return edge_iterator(*this, begin()); }
  constexpr const_edge_iterator edges_begin() const {
    return const_edge_iterator(*this, const_cast<graph_type&>(*this).begin());
  }
  constexpr const_edge_iterator edges_cbegin() const {
    return const_edge_iterator(*this, const_cast<graph_type&>(*this).cbegin());
  }

  constexpr edge_iterator       edges_end() { return edge_iterator(*this, end()); }
  constexpr const_edge_iterator edges_end() const {
    return const_edge_iterator(*this, const_cast<graph_type&>(*this).end());
  }
  constexpr const_edge_iterator edges_cend() const {
    return const_edge_iterator(*this, const_cast<graph_type&>(*this).end());
  }

  edge_range       edges() { return {edges_begin(), edges_end(), this->edges_size_}; }
  const_edge_range edges() const { return {edges_begin(), edges_end(), this->edges_size_}; }

  // Note: No graph_value() methods for GV=void specialization

public:
  vertex_iterator create_vertex();
  vertex_iterator create_vertex(vertex_value_type&&);

  template <class VV2>
    requires std::constructible_from<vertex_value_type, const VV2&>
  vertex_iterator create_vertex(const VV2&);

public:
  vertex_edge_iterator create_edge(vertex_key_type, vertex_key_type);
  vertex_edge_iterator create_edge(vertex_key_type, vertex_key_type, edge_value_type&&);

  template <class EV2>
    requires std::constructible_from<edge_value_type, const EV2&>
  vertex_edge_iterator create_edge(vertex_key_type,
                                   vertex_key_type,
                                   const EV2&);

  vertex_edge_iterator create_edge(vertex_iterator, vertex_iterator);
  vertex_edge_iterator create_edge(vertex_iterator, vertex_iterator, edge_value_type&&);

  template <class EV2>
    requires std::constructible_from<edge_value_type, const EV2&>
  vertex_edge_iterator create_edge(vertex_iterator,
                                   vertex_iterator,
                                   const EV2&);

public:
  edge_iterator erase_edge(edge_iterator);

public:
  void clear();
  void swap(undirected_adjacency_list&);

protected:
  void reserve_vertices(vertex_size_type);
  void resize_vertices(vertex_size_type);
  void resize_vertices(vertex_size_type, const vertex_value_type&);

private:
  // Note: No graph_value_ member in GV=void specialization
  friend vertex_type;

private:
  friend constexpr auto vertices(undirected_adjacency_list& g) noexcept { 
    return vertex_descriptor_view(g.vertices_);
  }
  friend constexpr auto vertices(const undirected_adjacency_list& g) noexcept { 
    return vertex_descriptor_view(g.vertices_);
  }
  
  friend constexpr auto num_vertices(const undirected_adjacency_list& g) noexcept { 
    return g.vertices_.size(); 
  }
  
  friend constexpr auto num_edges(const undirected_adjacency_list& g) noexcept { 
    return g.edges_size_; 
  }
  
  friend constexpr bool has_edge(const undirected_adjacency_list& g) noexcept { 
    return g.edges_size_ > 0; 
  }

  friend constexpr auto find_vertex(undirected_adjacency_list& g, vertex_key_type id) noexcept {
    using container_iter = typename vertex_set::iterator;
    using view_type      = vertex_descriptor_view<container_iter>;
    using view_iterator  = typename view_type::iterator;
    using storage_type   = typename view_iterator::value_type::storage_type;

    if (id >= static_cast<vertex_key_type>(g.vertices_.size())) {
      return view_iterator{static_cast<storage_type>(g.vertices_.size())};
    }
    return view_iterator{static_cast<storage_type>(id)};
  }
  friend constexpr auto find_vertex(const undirected_adjacency_list& g, vertex_key_type id) noexcept {
    using container_iter = typename vertex_set::const_iterator;
    using view_type      = vertex_descriptor_view<container_iter>;
    using view_iterator  = typename view_type::iterator;
    using storage_type   = typename view_iterator::value_type::storage_type;

    if (id >= static_cast<vertex_key_type>(g.vertices_.size())) {
      return view_iterator{static_cast<storage_type>(g.vertices_.size())};
    }
    return view_iterator{static_cast<storage_type>(id)};
  }

  // Note: No graph_value(g) CPO for GV=void specialization

  template <typename U>
    requires vertex_descriptor_type<U> && (!std::is_void_v<VV>)
  friend constexpr decltype(auto) vertex_value(undirected_adjacency_list& g, const U& u) noexcept {
    auto& vtx = u.inner_value(g.vertices_);
    return (vtx.base_value_type::value);
  }
  template <typename U>
    requires vertex_descriptor_type<U> && (!std::is_void_v<VV>)
  friend constexpr decltype(auto) vertex_value(const undirected_adjacency_list& g, const U& u) noexcept {
    const auto& vtx = u.inner_value(g.vertices_);
    return (vtx.base_value_type::value);
  }

  template <typename U>
    requires vertex_descriptor_type<U>
  friend constexpr auto edges(undirected_adjacency_list& g, const U& u) noexcept {
    auto uid = static_cast<vertex_key_type>(u.vertex_id());
    auto& vtx = g.vertices_[uid];
    auto edges_rng = vtx.edges(g, uid);
    using edge_iter_t = vertex_edge_iterator;
    using vertex_iter_t = typename U::iterator_type;
    return edge_descriptor_view<edge_iter_t, vertex_iter_t>(edges_rng, u);
  }
  template <typename U>
    requires vertex_descriptor_type<U>
  friend constexpr auto edges(const undirected_adjacency_list& g, const U& u) noexcept {
    auto uid = static_cast<vertex_key_type>(u.vertex_id());
    const auto& vtx = g.vertices_[uid];
    auto edges_rng = vtx.edges(g, uid);
    using edge_iter_t = const_vertex_edge_iterator;
    using vertex_iter_t = typename U::iterator_type;
    return edge_descriptor_view<edge_iter_t, vertex_iter_t>(edges_rng, u);
  }

  template <typename U>
    requires vertex_descriptor_type<U>
  friend constexpr auto degree(const undirected_adjacency_list& g, const U& u) noexcept {
    auto uid = static_cast<vertex_key_type>(u.vertex_id());
    return g.vertices_[uid].edges_size();
  }

  template <typename E>
    requires edge_descriptor_type<E>
  friend constexpr vertex_key_type target_id(const undirected_adjacency_list& g, const E& e) noexcept {
    return e.value()->other_vertex_key(g, static_cast<vertex_key_type>(e.source_id()));
  }

  template <typename E>
    requires edge_descriptor_type<E>
  friend constexpr vertex_key_type source_id([[maybe_unused]] const undirected_adjacency_list& g, const E& e) noexcept {
    return static_cast<vertex_key_type>(e.source_id());
  }

  template <typename E>
    requires edge_descriptor_type<E>
  friend constexpr auto target(undirected_adjacency_list& g, const E& e) noexcept {
    auto tid = target_id(g, e);
    return *find_vertex(g, tid);
  }
  template <typename E>
    requires edge_descriptor_type<E>
  friend constexpr auto target(const undirected_adjacency_list& g, const E& e) noexcept {
    auto tid = target_id(g, e);
    return *find_vertex(g, tid);
  }

  template <typename E>
    requires edge_descriptor_type<E>
  friend constexpr auto source(undirected_adjacency_list& g, const E& e) noexcept {
    auto sid = source_id(g, e);
    return *find_vertex(g, sid);
  }
  template <typename E>
    requires edge_descriptor_type<E>
  friend constexpr auto source(const undirected_adjacency_list& g, const E& e) noexcept {
    auto sid = source_id(g, e);
    return *find_vertex(g, sid);
  }

  template <typename U>
    requires vertex_descriptor_type<U>
  friend constexpr auto find_vertex_edge(undirected_adjacency_list& g, const U& u, vertex_key_type vid) noexcept {
    auto uid = static_cast<vertex_key_type>(u.vertex_id());
    auto& vtx = g.vertices_[uid];
    auto edges_rng = vtx.edges(g, uid);
    using edge_iter_t = vertex_edge_iterator;
    using vertex_iter_t = typename U::iterator_type;
    auto edge_view = edge_descriptor_view<edge_iter_t, vertex_iter_t>(edges_rng, u);
    for (auto it = edge_view.begin(); it != edge_view.end(); ++it) {
      if (target_id(g, *it) == vid) {
        return *it;
      }
    }
    return *edge_view.end();
  }
  template <typename U>
    requires vertex_descriptor_type<U>
  friend constexpr auto find_vertex_edge(const undirected_adjacency_list& g, const U& u, vertex_key_type vid) noexcept {
    auto uid = static_cast<vertex_key_type>(u.vertex_id());
    const auto& vtx = g.vertices_[uid];
    auto edges_rng = vtx.edges(g, uid);
    using edge_iter_t = const_vertex_edge_iterator;
    using vertex_iter_t = typename U::iterator_type;
    auto edge_view = edge_descriptor_view<edge_iter_t, vertex_iter_t>(edges_rng, u);
    for (auto it = edge_view.begin(); it != edge_view.end(); ++it) {
      if (target_id(g, *it) == vid) {
        return *it;
      }
    }
    return *edge_view.end();
  }

  template <typename U, typename V>
    requires vertex_descriptor_type<U> && vertex_descriptor_type<V>
  friend constexpr auto find_vertex_edge(undirected_adjacency_list& g, const U& u, const V& v) noexcept {
    return find_vertex_edge(g, u, static_cast<vertex_key_type>(v.vertex_id()));
  }
  template <typename U, typename V>
    requires vertex_descriptor_type<U> && vertex_descriptor_type<V>
  friend constexpr auto find_vertex_edge(const undirected_adjacency_list& g, const U& u, const V& v) noexcept {
    return find_vertex_edge(g, u, static_cast<vertex_key_type>(v.vertex_id()));
  }

  friend constexpr auto find_vertex_edge(undirected_adjacency_list& g, vertex_key_type uid, vertex_key_type vid) noexcept {
    auto u = *find_vertex(g, uid);
    return find_vertex_edge(g, u, vid);
  }
  friend constexpr auto find_vertex_edge(const undirected_adjacency_list& g, vertex_key_type uid, vertex_key_type vid) noexcept {
    auto u = *find_vertex(g, uid);
    return find_vertex_edge(g, u, vid);
  }
};

} // namespace graph::container

#include "detail/undirected_adjacency_list_api.hpp"
#include "detail/undirected_adjacency_list_impl.hpp"

#endif // UNDIRECTED_ADJ_LIST_HPP
