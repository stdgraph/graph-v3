#pragma once

#include "container_utility.hpp"
#include <vector>
#include <concepts>
#include <functional>
#include <algorithm>
#include <ranges>
#include <cstdint>
#include <cassert>
#include <format>
#include <iostream>
#include "graph/detail/graph_using.hpp"
#include "graph/graph_info.hpp"
#include "graph/graph.hpp"
#include "graph/descriptor_traits.hpp"
#include "graph/vertex_descriptor_view.hpp"
#include "graph/edge_descriptor_view.hpp"

// NOTES
//  have public load_edges(...), load_vertices(...), and load()
//  allow separation of construction and load
//  allow multiple calls to load edges as long as subsequent edges have uid >= last vertex (append)
//  VId must be large enough for the total edges and the total vertices.
//  
// API Design:
//  - vertex_ids() returns iota view of all vertex IDs [0, size())
//  - edge_ids() returns iota view of all edge IDs [0, total_edges)
//  - edge_ids(vertex_id) returns iota view of edge IDs for specific vertex
//  - Vertex ID validity: check with id < g.size() (no find_vertex() needed)
//  - Direct access via vertex_value(id), edge_value(id), target_id(id)

namespace graph::container {

// Import types from graph::adj_list for convenience
using adj_list::vertex_descriptor;
using adj_list::edge_descriptor;
using adj_list::vertex_descriptor_view;
using adj_list::edge_descriptor_view;
using adj_list::vertex_descriptor_type;
using adj_list::edge_descriptor_type;

/**
 * @ingroup graph_containers
 * @brief Scans a range used for input for loading edges to determine the largest vertex id used.
 * 
 * @tparam VId   Vertex Id type.
 * @tparam EV    Edge value type. It may be void.
 * @tparam ERng  Edge data range type.
 * @tparam EProj Edge Projection function type to convert from an @c ERng value type to a @c copyable_edge_t<VId,EV>.
 *               If the @c ERng value type is already copyable_edge_t<VId,EV> identity can be used.
 * 
 * @param erng        The edge data range type.
 * @param eprojection The edge projection function that converts a @c ERng value type to a @c copyable_edge_t<VId,EV>.
 *                    If @c erng value type is already copyable_edge_t<VId,EV>, identity() can be used.
 * 
 * @return A @c pair<VId,size_t> with the max vertex id used and the number of edges scanned.
 *         For empty ranges, returns {0, 0}.
*/
template <class VId, class EV, forward_range ERng, class EProj = identity>
requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV>
[[nodiscard]] constexpr auto max_vertex_id(const ERng& erng, const EProj& eprojection) {
  size_t edge_count = 0;
  VId    max_id     = 0;
  for (auto&& edge_data : erng) {
    const copyable_edge_t<VId, EV>& uv = eprojection(edge_data);
    max_id = max(max_id, max(static_cast<VId>(uv.source_id), static_cast<VId>(uv.target_id)));
    ++edge_count;
  }
  return pair(max_id, edge_count);
}


//
// forward declarations
//
template <class EV        = void,            // edge value type
          class VV        = void,            // vertex value type
          class GV        = void,            // graph value type
          integral VId    = uint32_t,        // vertex id type
          integral EIndex = uint32_t,        // edge index type
          class Alloc     = std::allocator<VId>> // for internal containers
class compressed_graph;

/**
 * @ingroup graph_containers
 * @brief Wrapper struct for the row index to distinguish it from a vertex_id_type (VId).
 * 
 * @tparam EIndex  The type for storing an edge index. It must be able to store a value of |E|+1,
 *                 where |E| is the total number of edges in the graph.
*/
template <integral EIndex>
struct csr_row {
  using edge_index_type = EIndex;
  edge_index_type index = 0;
};

/**
 * @ingroup graph_containers
 * @brief Wrapper struct for the col (edge) index to distinguish it from a vertex_id_type (VId).
 * 
 * @tparam VId     Vertex id type. The type must be able to store a value of |V|+1, where |V| is the
 *                 number of vertices in the graph.
*/
template <integral VId>
struct csr_col {
  using vertex_id_type = VId;
  vertex_id_type index = 0;
};


/**
 * @ingroup graph_containers
 * @brief Holds vertex values in a vector that is the same size as @c row_index_.
 * 
 * If @c is_void_v<VV> then a specialization class is defined that is empty with a single 
 * constructor that accepts (and ignores) an allocator.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam VId     Vertex id type. The type must be able to store a value of |V|+1, where |V| is the
 *                 number of vertices in the graph.
 * @tparam EIndex  The type for storing an edge index. It must be able to store a value of |E|+1,
 *                 where |E| is the total number of edges in the graph.
 * @tparam Alloc   The allocator type.
*/
template <class EV, class VV, class GV, integral VId, integral EIndex, class Alloc>
class csr_row_values {
  using row_type           = csr_row<EIndex>; // index into col_index_
  using row_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<row_type>;
  using row_index_vector   = std::vector<row_type, row_allocator_type>;

public:
  using graph_type        = compressed_graph<EV, VV, GV, VId, EIndex, Alloc>;
  using vertex_type       = row_type;
  using vertex_value_type = VV;
  using allocator_type    = typename std::allocator_traits<Alloc>::template rebind_alloc<vertex_value_type>;
  using vector_type       = std::vector<vertex_value_type, allocator_type>;

  using value_type      = VV;
  using size_type       = size_t; //VId;
  using difference_type = typename vector_type::difference_type;
  using reference       = value_type&;
  using const_reference = const value_type&;
  using pointer         = typename vector_type::pointer;
  using const_pointer   = typename vector_type::const_pointer;
  using iterator        = typename vector_type::iterator;
  using const_iterator  = typename vector_type::const_iterator;

  constexpr csr_row_values(const Alloc& alloc) : v_(alloc) {}

  constexpr csr_row_values()                      = default;
  constexpr csr_row_values(const csr_row_values&) = default;
  constexpr csr_row_values(csr_row_values&&)      = default;
  constexpr ~csr_row_values()                     = default;

  constexpr csr_row_values& operator=(const csr_row_values&) = default;
  constexpr csr_row_values& operator=(csr_row_values&&)      = default;


public: // Properties
  [[nodiscard]] constexpr size_type size() const noexcept { return static_cast<size_type>(v_.size()); }
  [[nodiscard]] constexpr bool      empty() const noexcept { return v_.empty(); }
  [[nodiscard]] constexpr size_type capacity() const noexcept { return static_cast<size_type>(v_.capacity()); }

public: // Operations
  constexpr void reserve(size_type new_cap) { v_.reserve(new_cap); }
  constexpr void resize(size_type new_size) { v_.resize(new_size); }

  constexpr void clear() noexcept { v_.clear(); }
  constexpr void push_back(const value_type& value) { v_.push_back(value); }
  constexpr void emplace_back(value_type&& value) { v_.emplace_back(forward<value_type>(value)); }

  constexpr void swap(csr_row_values& other) noexcept { swap(v_, other.v_); }

  template <forward_range VRng, class VProj = identity>
  //requires views::copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV>
  constexpr void load_row_values(const VRng& vrng, VProj projection, size_type vertex_count) {
    // Handle empty range gracefully
    if (std::ranges::empty(vrng))
      return;
    
    if constexpr (sized_range<VRng>)
      vertex_count = max(vertex_count, std::ranges::size(vrng));
    else if (vertex_count == 0) // assume 0 means not provided and need to evaluate
      vertex_count = max(vertex_count, std::ranges::distance(vrng));
    
    // Don't shrink if already allocated - only grow if needed
    if (size() > 0)
      vertex_count = max(vertex_count, size());
    resize(vertex_count);

    for (auto&& vtx : vrng) {
      const auto& [id, value] = projection(vtx);

      // Validate vertex id is within bounds
      if (static_cast<size_t>(id) >= size()) {
        throw graph_error(std::format(
            "Invalid vertex id {} in vertex data: exceeds allocated size {}",
            id, size()));
      }

      (*this)[static_cast<size_t>(id)] = value;
    }
  }

  template <forward_range VRng, class VProj = identity>
  //requires views::copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV>
  constexpr void load_row_values(VRng&& vrng, VProj projection, size_type vertex_count) {
    // Handle empty range gracefully
    if (std::ranges::empty(vrng))
      return;
    
    if constexpr (sized_range<VRng>)
      vertex_count = max(vertex_count, std::ranges::size(vrng));
    else if (vertex_count == 0) // assume 0 means not provided and need to evaluate
      vertex_count = max(vertex_count, std::ranges::distance(vrng));
    
    // Don't shrink if already allocated - only grow if needed
    if (size() > 0)
      vertex_count = max(vertex_count, size());
    resize(vertex_count);

    for (auto&& vtx : vrng) {
      auto&& [id, value] = projection(vtx);

      // Validate vertex id is within bounds
      if (static_cast<size_t>(id) >= size()) {
        throw graph_error(std::format(
            "Invalid vertex id {} in vertex data: exceeds allocated size {}",
            id, size()));
      }

      (*this)[id] = std::move(value);
    }
  }

public:
  [[nodiscard]] constexpr reference       operator[](size_type pos) { return v_[pos]; }
  [[nodiscard]] constexpr const_reference operator[](size_type pos) const { return v_[pos]; }

private:
  vector_type v_;
};

template <class EV, class GV, integral VId, integral EIndex, class Alloc>
class csr_row_values<EV, void, GV, VId, EIndex, Alloc> {
public:
  constexpr csr_row_values([[maybe_unused]] const Alloc& alloc) {}
  constexpr csr_row_values() = default;

  using value_type = void;
  using size_type  = size_t; //VId;

public: // Properties
  [[nodiscard]] constexpr size_type size() const noexcept { return 0; }
  [[nodiscard]] constexpr bool      empty() const noexcept { return true; }
  [[nodiscard]] constexpr size_type capacity() const noexcept { return 0; }

public: // Operations
  constexpr void reserve([[maybe_unused]] size_type new_cap) {}
  constexpr void resize([[maybe_unused]] size_type new_size) {}

  constexpr void clear() noexcept {}
  constexpr void swap([[maybe_unused]] csr_row_values& other) noexcept {}

  // Load vertex values - no-op for void vertex values (ISSUE #4 fix)
  template <forward_range VRng, class VProj = identity>
  constexpr void load_row_values([[maybe_unused]] const VRng& vrng, 
                                 [[maybe_unused]] VProj projection, 
                                 [[maybe_unused]] size_type vertex_count) {
    // No-op for void vertex values - nothing to load
  }

  template <forward_range VRng, class VProj = identity>
  constexpr void load_row_values([[maybe_unused]] VRng&& vrng, 
                                 [[maybe_unused]] VProj projection, 
                                 [[maybe_unused]] size_type vertex_count) {
    // No-op for void vertex values - nothing to load
  }
};


/**
 * @ingroup graph_containers
 * @brief Class to hold vertex values in a vector that is the same size as col_index_.
 * 
 * If is_void_v<EV> then the class is empty with a single
 * constructor that accepts (and ignores) an allocator.
 *
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam VId     Vertex id type. The type must be able to store a value of |V|+1, where |V| is the
 *                 number of vertices in the graph.
 * @tparam EIndex  The type for storing an edge index. It must be able to store a value of |E|+1,
 *                 where |E| is the total number of edges in the graph.
 * @tparam Alloc   The allocator type.
*/
template <class EV, class VV, class GV, integral VId, integral EIndex, class Alloc>
class csr_col_values {
  using col_type           = csr_col<VId>; // target_id
  using col_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<col_type>;
  using col_index_vector   = std::vector<col_type, col_allocator_type>;

public:
  using graph_type      = compressed_graph<EV, VV, GV, VId, EIndex, Alloc>;
  using edge_type       = col_type; // index into v_
  using edge_value_type = EV;
  using allocator_type  = typename std::allocator_traits<Alloc>::template rebind_alloc<edge_value_type>;
  using vector_type     = std::vector<edge_value_type, allocator_type>;

  using value_type      = EV;
  using edge_id_type    = EIndex;
  using size_type       = size_t; //VId;
  using difference_type = typename vector_type::difference_type;
  using reference       = value_type&;
  using const_reference = const value_type&;
  using pointer         = typename vector_type::pointer;
  using const_pointer   = typename vector_type::const_pointer;
  using iterator        = typename vector_type::iterator;
  using const_iterator  = typename vector_type::const_iterator;

  constexpr csr_col_values(const Alloc& alloc) : v_(alloc) {}

  constexpr csr_col_values()                      = default;
  constexpr csr_col_values(const csr_col_values&) = default;
  constexpr csr_col_values(csr_col_values&&)      = default;
  constexpr ~csr_col_values()                     = default;

  constexpr csr_col_values& operator=(const csr_col_values&) = default;
  constexpr csr_col_values& operator=(csr_col_values&&)      = default;


public: // Properties
  [[nodiscard]] constexpr size_type size() const noexcept { return static_cast<size_type>(v_.size()); }
  [[nodiscard]] constexpr bool      empty() const noexcept { return v_.empty(); }
  [[nodiscard]] constexpr size_type capacity() const noexcept { return static_cast<size_type>(v_.size()); }

public: // Operations
  constexpr void reserve(size_type new_cap) { v_.reserve(new_cap); }
  constexpr void resize(size_type new_size) { v_.resize(new_size); }

  constexpr void clear() noexcept { v_.clear(); }
  constexpr void push_back(const value_type& value) { v_.push_back(value); }
  constexpr void emplace_back(value_type&& value) { v_.emplace_back(forward<value_type>(value)); }

  constexpr void swap(csr_col_values& other) noexcept { swap(v_, other.v_); }

public:
  [[nodiscard]] constexpr reference       operator[](edge_id_type pos) { return v_[static_cast<size_t>(pos)]; }
  [[nodiscard]] constexpr const_reference operator[](edge_id_type pos) const { return v_[static_cast<size_t>(pos)]; }

private:
  vector_type v_;
};

template <class VV, class GV, integral VId, integral EIndex, class Alloc>
class csr_col_values<void, VV, GV, VId, EIndex, Alloc> {
public:
  constexpr csr_col_values([[maybe_unused]] const Alloc& alloc) {}
  constexpr csr_col_values() = default;

  using value_type = void;
  using size_type  = size_t; //VId;

public: // Properties
  [[nodiscard]] constexpr size_type size() const noexcept { return 0; }
  [[nodiscard]] constexpr bool      empty() const noexcept { return true; }
  [[nodiscard]] constexpr size_type capacity() const noexcept { return 0; }

public: // Operations
  constexpr void reserve([[maybe_unused]] size_type new_cap) {}
  constexpr void resize([[maybe_unused]] size_type new_size) {}

  constexpr void clear() noexcept {}
  constexpr void swap([[maybe_unused]] csr_col_values& other) noexcept {}
};


/**
 * @ingroup graph_containers
 * @brief Base class for compressed sparse row adjacency graph
 * 
 * This is a static CSR (Compressed Sparse Row) graph structure optimized for read-heavy operations.
 * The graph is loaded once and provides efficient random access to vertices and edges.
 * 
 * Key Access Patterns:
 * - Vertex access: Use vertex_ids() for all vertices, check validity with id < size()
 * - Edge access: Use edge_ids() for all edges, edge_ids(vid) for per-vertex edges
 * - Direct data access: vertex_value(id), edge_value(id), target_id(id)
 * 
 * For constructors that accept a partition function, the function must return a partition id for a vertex id.
 * When used, the range of input edges must be ordered by the partition id they're in. Partions may be skipped
 * (empty) but the partition id must be in increasing order.
 *
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam VId     Vertex id type. The type must be able to store a value of |V|+1, where |V| is the
 *                 number of vertices in the graph.
 * @tparam EIndex  The type for storing an edge index. It must be able to store a value of |E|+1,
 *                 where |E| is the total number of edges in the graph.
 * @tparam Alloc   The allocator type.
*/
template <class EV, class VV, class GV, integral VId, integral EIndex, class Alloc>
class compressed_graph_base
      : protected csr_row_values<EV, VV, GV, VId, EIndex, Alloc>
      , protected csr_col_values<EV, VV, GV, VId, EIndex, Alloc> {
  using row_values_base = csr_row_values<EV, VV, GV, VId, EIndex, Alloc>;
  using col_values_base = csr_col_values<EV, VV, GV, VId, EIndex, Alloc>;

  using row_type           = csr_row<EIndex>; // index into col_index_
  using row_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<row_type>;
  using row_index_vector   = std::vector<row_type, row_allocator_type>;

  using col_type           = csr_col<VId>; // target_id
  using col_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<col_type>;
  using col_index_vector   = std::vector<col_type, col_allocator_type>;

public: // Types
  using graph_type = compressed_graph_base<EV, VV, GV, VId, EIndex, Alloc>;

  using partition_id_type = VId;
  using partition_vector  = std::vector<VId>;

  using vertex_id_type      = VId;
  using vertex_type         = row_type;
  using vertex_value_type   = VV;
  using vertices_type       = subrange<iterator_t<row_index_vector>>;
  using const_vertices_type = subrange<iterator_t<const row_index_vector>>;

  using edge_type        = col_type; // index into v_
  using edge_value_type  = EV;
  using edge_index_type  = EIndex;
  using edge_id_type     = EIndex; // Alias for edge_index_type (semantic name for edge identifiers)
  using column_id_type   = EIndex; // Alias for indexing into col_index_ (column array)
  using edges_type       = subrange<iterator_t<col_index_vector>>;
  using const_edges_type = subrange<iterator_t<const col_index_vector>>;

  using const_iterator = typename row_index_vector::const_iterator;
  using iterator       = typename row_index_vector::iterator;

  using size_type = range_size_t<row_index_vector>;

public: // Construction/Destruction
  constexpr compressed_graph_base()                             = default;
  constexpr compressed_graph_base(const compressed_graph_base&) = default;
  constexpr compressed_graph_base(compressed_graph_base&&)      = default;
  constexpr ~compressed_graph_base()                            = default;

  constexpr compressed_graph_base& operator=(const compressed_graph_base&) = default;
  constexpr compressed_graph_base& operator=(compressed_graph_base&&)      = default;

  constexpr compressed_graph_base(const Alloc& alloc)
        : row_values_base(alloc), col_values_base(alloc), row_index_(alloc), col_index_(alloc), partition_(alloc) {
    terminate_partitions();
  }

public: // Properties - resolve ambiguity from multiple inheritance
  /**
   * @brief Get the number of vertices in the graph.
   * 
   * Returns the count of vertices (rows) in the CSR structure. This explicitly calls
   * the row version to avoid ambiguity with the inherited col_values_base::size().
   * 
   * @return The number of vertices in the graph
  */
  [[nodiscard]] constexpr size_type size() const noexcept { 
    return row_index_.empty() ? 0 : row_index_.size() - 1; // -1 for terminating row
  }
  
  /**
   * @brief Get the number of vertices in the graph (alias for size()).
   * 
   * Returns the count of vertices in the graph. This is an alias for size() that
   * provides compatibility with the num_vertices(g) CPO.
   * 
   * @return The number of vertices in the graph
  */
  [[nodiscard]] constexpr size_type num_vertices() const noexcept { 
    return size();
  }
  
  /**
   * @brief Check if the graph has no vertices.
   * 
   * Returns true if the graph contains no vertices. This explicitly calls the row
   * version to avoid ambiguity with the inherited col_values_base::empty().
   * 
   * @return true if the graph is empty, false otherwise
  */
  [[nodiscard]] constexpr bool empty() const noexcept { 
    return row_index_.empty() || row_index_.size() <= 1; // Account for terminating row
  }
  
  /**
   * @brief Clear all graph data.
   * 
   * Removes all vertices, edges, vertex values, edge values, and partitions from the graph.
   * After calling clear(), the graph will be empty with size() == 0.
  */
  constexpr void clear() noexcept { 
    row_index_.clear();
    col_index_.clear();
    row_values_base::clear();
    col_values_base::clear();
    partition_.clear();
    terminate_partitions();
  }

  /**
   * @brief Constructor that takes a edge range to create the CSR graph.
   * 
   * Edges must be ordered by source_id (enforced by asssertion).
   * 
   * @tparam ERng    Edge range type
   * @tparam EProj   Edge projection function type
   * @tparam PartRng Range of starting vertex Ids for each partition
   * 
   * @param erng                 The input range of edges
   * @param eprojection          Projection function that creates a @c copyable_edge_t<VId,EV> from an erng value
   * @param partition_start_ids  Range of starting vertex ids for each partition. If empty, all vertices are in partition 0.
   * @param alloc                Allocator to use for internal containers
  */
  template <forward_range ERng, forward_range PartRng, class EProj = identity>
  //requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> && //
  //               convertible_to<range_value_t<PartRng>, VId>
  constexpr compressed_graph_base(const ERng&    erng,
                                  EProj          eprojection         = {},
                                  const PartRng& partition_start_ids = std::vector<VId>(),
                                  const Alloc&   alloc               = Alloc())
        : row_values_base(alloc)
        , col_values_base(alloc)
        , row_index_(alloc)
        , col_index_(alloc)
        , partition_(partition_start_ids, alloc) {
    load_edges(erng, eprojection);
    terminate_partitions();
  }

  /**
   * @brief Constructor that takes edge range and vertex range to create the CSR graph.
   * 
   * Edges must be ordered by source_id (enforced by asssertion).
   *
   * @tparam ERng    Edge Range type
   * @tparam VRng    Vetex range type
   * @tparam EProj   Edge projection function type
   * @tparam VProj   Vertex projection function type
   * @tparam PartRng Range of starting vertex Ids for each partition
   * 
   * @param erng                The input range of edges
   * @param vrng                The input range of vertices
   * @param eprojection         Projection function that creates a @c copyable_edge_t<VId,EV> from an @c erng value
   * @param vprojection         Projection function that creates a @c copyable_vertex_t<VId,VV> from a @c vrng value
   * @param partition_start_ids Range of starting vertex ids for each partition. If empty, all vertices are in partition 0.
   * @param alloc               Allocator to use for internal containers
  */
  template <forward_range ERng,
            forward_range VRng,
            forward_range PartRng,
            class EProj = identity,
            class VProj = identity>
  requires convertible_to<range_value_t<PartRng>, VId>
  //requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
  //      copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV>
  constexpr compressed_graph_base(const ERng&    erng,
                                  const VRng&    vrng,
                                  EProj          eprojection = {}, // eproj(eval) -> {source_id, target_id [,value]}
                                  VProj          vprojection = {}, // vproj(vval) -> {vertex_id [,value]}
                                  const PartRng& partition_start_ids = std::vector<VId>(),
                                  const Alloc&   alloc               = Alloc())
        : row_values_base(alloc)
        , col_values_base(alloc)
        , row_index_(alloc)
        , col_index_(alloc)
        , partition_(partition_start_ids, alloc) {

    load(erng, vrng, eprojection, vprojection);
    terminate_partitions();
  }

  /**
   * @brief Constructor for easy creation of a graph that takes an initializer list 
   *        of @c copyable_edge_t<VId,EV> -> {source_id, target_id [,value]}.
   *
   * @param ilist   Initializer list of @c copyable_edge_t<VId,EV> -> {source_id, target_id [,value]}
   * @param alloc   Allocator to use for internal containers
  */
  constexpr compressed_graph_base(const std::initializer_list<copyable_edge_t<VId, EV>>& ilist,
                                  const Alloc&                                           alloc = Alloc())
        : row_values_base(alloc), col_values_base(alloc), row_index_(alloc), col_index_(alloc), partition_(alloc) {
    load_edges(ilist, identity());
    terminate_partitions();
  }

public: // Operations
  /**
   * @brief Reserve space for vertices and edges in the graph.
   * 
   * Pre-allocates memory for the internal storage vectors to avoid reallocation
   * during graph construction. This is an optimization hint for the static CSR structure.
   * 
   * @param edge_count The number of edges to reserve space for
   * @param vertex_count The number of vertices to reserve space for (default: 0, inferred from edge data)
   * @note This is an optimization hint and does not change the graph's size.
  */
  void reserve(size_type edge_count, size_type vertex_count = 0) {
    row_index_.reserve(vertex_count + 1); // +1 for terminating row
    row_values_base::reserve(vertex_count);
    col_index_.reserve(edge_count);
    static_cast<col_values_base&>(*this).reserve(edge_count);
  }

  /**
   * @brief Load vertex values, callable either before or after @c load_edges(erng,eproj).
   *
   * If @c load_edges(vrng,vproj) has been called before this, the @c row_values_ vector will be
   * extended to match the number of @c row_index_.size()-1 to avoid out-of-bounds errors when
   * accessing vertex values.
   *
   * @tparam VRng   Vertex range type
   * @tparam VProj  Vertex projection function type
   * 
   * @param  vrng           Range of values to load for vertices. The order of the values is 
   *                        preserved in the internal vector.
   * @param  vprojection    Projection function for @c vrng values
   * @param  vertex_count   Expected vertex count (optional, inferred from range size)
  */
  template <forward_range VRng, class VProj = identity>
  //requires views::copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV>
  constexpr void load_vertices(const VRng& vrng, VProj vprojection = {}, size_type vertex_count = 0) {
    // Scan the vertex data to find the maximum vertex ID
    vertex_id_type max_id = 0;
    for (auto&& vtx_data : vrng) {
      auto&& vtx = vprojection(vtx_data);
      max_id = max(max_id, static_cast<vertex_id_type>(vtx.id));
    }
    
    // Determine required vertex count: max(provided hint, max_id + 1, existing size)
    vertex_count = max({vertex_count, static_cast<size_type>(max_id + 1), size()});
    
    // If graph structure doesn't exist yet, create it based on vertex count
    if (row_index_.empty() && vertex_count > 0) {
      row_index_.resize(vertex_count + 1, vertex_type{0}); // All vertices have 0 edges initially
    } else if (vertex_count > size()) {
      // Expand existing structure if needed
      row_index_.resize(vertex_count + 1, vertex_type{0});
    }
    
    row_values_base::load_row_values(vrng, vprojection, vertex_count);
  }

  /**
   * @brief Load the edges for the graph, callable either before or after @c load_vertices(erng,eproj).
   *
   * @c erng must be ordered by source_id (copyable_edge_t) and is enforced by assertion. target_id
   * can be unordered within a source_id.
   *
   * If @c erng is bi-directional, the source_id in the last entry is used to determine the maximum
   * number of rows and is used to reserve space in the internal row_index and row_value vectors.
   * If @c erng is an input_range or forward_range that evaluation can't be done and the internal
   * row_index vector is grown and resized normally as needed (the row_value vector is updated by
   * @c load_vertices(vrng,vproj)). If the caller knows the number of rows/vertices and edges, they
   * can call @c reserve(vertex_count, edge_count) to reserve the space.
   *
   * If @c erng is a sized_range, @c size(erng) is used to reserve space for the internal col_index and
   * v vectors. If it isn't a sized range, the vectors will be grown and resized normally as needed
   * as new indexes and values are added. If the caller knows the number of columns/edges and vertices,
   * they can call @c reserve(vertex_count, edge_count) to reserve the space.
   *
   * If row indexes have been referenced in the edges but there are no edges defined for them
   * (with source_id), rows will be added to fill out the row_index vector to avoid out-of-bounds
   * references.
   *
   * If @c load_vertices(vrng,vproj) has been called before this, the row_values_ vector will be
   * extended to match the number of @c row_index_.size()-1 to avoid out-of-bounds errors when
   * accessing vertex values.
   *
   * @todo @c ERng not a forward_range because CSV reader doesn't conform to be a forward_range
   *          10/14/2025: The library has been updated to comply with C++20 ranges. This needs revalidated.
   * 
   * @tparam ERng   Edge range type
   * @tparam EProj  Edge projection function type
   * 
   * @param erng         Input range for edges
   * @param eprojection  Edge projection function that returns a @ copyable_edge_t<VId,EV> for an element in @c erng
   * @param part_fnc     Partition function that returns a partition id for a vertex id
   * @param vertex_count The number of vertices in the graph. If 0, the number of vertices is determined by the
   *                     largest vertex id in the edge range.
   * @param edge_count   The number of edges in the graph. If 0, the number of edges is determined by the size of the
   *                     edge range.
  */
  template <class ERng, class EProj = identity>
  //requires views::copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV>
  constexpr void load_edges(ERng&& erng, EProj eprojection = {}, size_type vertex_count = 0, size_type edge_count = 0) {
    // should only be loading into an empty graph
    assert(row_index_.empty() && col_index_.empty() && static_cast<col_values_base&>(*this).empty());

    // Nothing to do?
    if (begin(erng) == end(erng)) {
      terminate_partitions();
      return;
    }

    // We can get the last vertex id from the list because erng is required to be ordered by
    // the source id. It's possible a target_id could have a larger id also, which is taken
    // care of at the end of this function.
    vertex_count =
          max(vertex_count, static_cast<size_type>(last_erng_id(erng, eprojection) + 1)); // +1 for zero-based index
    
    // Reserve space for vertices
    row_index_.reserve(vertex_count + 1); // +1 for terminating row
    row_values_base::reserve(vertex_count);

    // Eval number of input rows and reserve space for the edges, if possible
    if constexpr (sized_range<ERng>)
      edge_count = max(edge_count, std::ranges::size(erng));
    
    // Reserve space for edges
    col_index_.reserve(edge_count);
    static_cast<col_values_base&>(*this).reserve(edge_count);

    // Add edges
    vertex_id_type last_uid = 0, max_vid = 0;
    for (auto&& edge_data : erng) {
      auto&& edge = eprojection(edge_data); // compressed_graph requires EV!=void
      assert(static_cast<vertex_id_type>(edge.source_id) >= last_uid);   // ordered by uid? (requirement)
      
      // Only resize when we encounter a new source vertex (optimization)
      if (static_cast<vertex_id_type>(edge.source_id) != last_uid || row_index_.empty()) {
        row_index_.resize(static_cast<size_t>(edge.source_id) + 1,
                          vertex_type{static_cast<edge_index_type>(col_index_.size())});
        last_uid = static_cast<vertex_id_type>(edge.source_id);
      }
      
      col_index_.push_back(edge_type{static_cast<vertex_id_type>(edge.target_id)});
      if constexpr (!is_void_v<EV>)
        static_cast<col_values_base&>(*this).emplace_back(move(edge.value));
      max_vid  = max(max_vid, static_cast<vertex_id_type>(edge.target_id));
    }

    // uid and vid may refer to rows that exceed the value evaluated for vertex_count (if any)
    vertex_count = max(vertex_count, max(row_index_.size(), static_cast<size_type>(max_vid + 1)));

    // add any rows that haven't been added yet, and (+1) terminating row
    row_index_.resize(vertex_count + 1, vertex_type{static_cast<edge_index_type>(col_index_.size())});

    // If load_vertices(vrng,vproj) has been called but it doesn't have enough values for all
    // the vertices then we extend the size to remove possibility of out-of-bounds occuring when
    // getting a value for a row.
    if (row_values_base::size() > 1 && row_values_base::size() < vertex_count)
      row_values_base::resize(vertex_count);
  }

  // The only diff with this and ERng&& is v_.push_back vs. v_.emplace_back
  template <class ERng, class EProj = identity>
  //requires views::copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV>
  constexpr void
  load_edges(const ERng& erng, EProj eprojection = {}, size_type vertex_count = 0, size_type edge_count = 0) {
    // should only be loading into an empty graph
    assert(row_index_.empty() && col_index_.empty() && static_cast<col_values_base&>(*this).empty());

    // Nothing to do?
    if (begin(erng) == end(erng)) {
      terminate_partitions();
      return;
    }

    // We can get the last vertex id from the list because erng is required to be ordered by
    // the source id. It's possible a target_id could have a larger id also, which is taken
    // care of at the end of this function.
    vertex_count =
          max(vertex_count, static_cast<size_type>(last_erng_id(erng, eprojection) + 1)); // +1 for zero-based index
    
    // Reserve space for vertices
    row_index_.reserve(vertex_count + 1); // +1 for terminating row
    row_values_base::reserve(vertex_count);

    // Eval number of input rows and reserve space for the edges, if possible
    if constexpr (sized_range<ERng>)
      edge_count = max(edge_count, std::ranges::size(erng));
    
    // Reserve space for edges
    col_index_.reserve(edge_count);
    static_cast<col_values_base&>(*this).reserve(edge_count);

    // Add edges
#ifndef NDEBUG
    size_t         debug_count = 0;
#endif
    vertex_id_type last_uid = 0, max_vid = 0;
    for (auto&& edge_data : erng) {
      auto&& edge = eprojection(edge_data); // compressed_graph requires EV!=void
      if (edge.source_id < last_uid) {      // ordered by uid? (requirement)
#ifndef NDEBUG
        std::string msg = std::format(
              "source id of {} on line {} of the data input is not ordered after source id of {} on the previous line",
              edge.source_id, debug_count, last_uid);
        std::cout << std::format("\n{}\n", msg);
        throw graph_error(std::move(msg));
#else
        std::string msg = std::format(
              "source id of {} is not ordered after source id of {} in the data input",
              edge.source_id, last_uid);
        throw graph_error(std::move(msg));
#endif
      }
      
      // Only resize when we encounter a new source vertex (optimization)
      if (edge.source_id != last_uid || row_index_.empty()) {
        row_index_.resize(static_cast<size_t>(edge.source_id) + 1,
                          vertex_type{static_cast<edge_index_type>(col_index_.size())});
        last_uid = edge.source_id;
      }
      
      col_index_.push_back(edge_type{edge.target_id});
      if constexpr (!is_void_v<EV>)
        static_cast<col_values_base&>(*this).push_back(edge.value);
      max_vid  = max(max_vid, static_cast<vertex_id_type>(edge.target_id));
#ifndef NDEBUG
      ++debug_count;
#endif
    }

    // uid and vid may refer to rows that exceed the value evaluated for vertex_count (if any)
    vertex_count = max(vertex_count, max(row_index_.size(), static_cast<size_type>(max_vid + 1)));

    // add any rows that haven't been added yet, and (+1) terminating row
    row_index_.resize(vertex_count + 1, vertex_type{static_cast<edge_index_type>(col_index_.size())});

    // If load_vertices(vrng,vproj) has been called but it doesn't have enough values for all
    // the vertices then we extend the size to remove possibility of out-of-bounds occuring when
    // getting a value for a row.
    if (row_values_base::size() > 0 && row_values_base::size() < vertex_count)
      row_values_base::resize(vertex_count);
  }

  /**
   * @brief Load edges and then vertices for the graph. 
   *
   * See @c load_edges() and @c load_vertices() for more information.
   *
   * @tparam EProj   Edge Projection Function type
   * @tparam VProj   Vertex Projectiong Function type
   * @tparam ERng    Edge Range type
   * @tparam VRng    Vertex Range type
   * @tparam PartRng Range of starting vertex Ids for each partition
   * 
   * @param erng            Input edge range
   * @param vrng            Input vertex range
   * @param eprojection     Edge projection function object
   * @param vprojection     Vertex projection function object
   * @param partition_start_ids  Range of starting vertex ids for each partition. If empty, all vertices are in partition 0.
  */
  template <forward_range ERng,
            forward_range VRng,
            forward_range PartRng,
            class EProj = identity,
            class VProj = identity>
  requires convertible_to<range_value_t<PartRng>, VId>
  //requires views::copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
  //      views::copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV>
  constexpr void load(const ERng& erng, const VRng& vrng, EProj eprojection = {}, VProj vprojection = {}) {
    load_edges(erng, eprojection);
    load_vertices(vrng, vprojection); // load the values
  }

  // Overload without PartRng for simpler usage
  template <forward_range ERng,
            forward_range VRng,
            class EProj = identity,
            class VProj = identity>
  constexpr void load(const ERng& erng, const VRng& vrng, EProj eprojection = {}, VProj vprojection = {}) {
    load_edges(erng, eprojection);
    load_vertices(vrng, vprojection);
  }

protected:
  template <class ERng, class EProj>
  constexpr vertex_id_type last_erng_id(ERng&& erng, EProj eprojection) const {
    vertex_id_type last_id = vertex_id_type();
    if constexpr (bidirectional_range<ERng>) {
      if (begin(erng) != end(erng)) {
        auto lastIt = end(erng);
        --lastIt;
        auto&& e = eprojection(*lastIt); // copyable_edge
        last_id  = max(e.source_id, e.target_id);
      }
    }
    return last_id;
  }

  constexpr void terminate_partitions() {
    if (partition_.empty()) {
      partition_.push_back(0);
    } else {
      // Validate partition ordering
      if (partition_[0] != 0) {
        throw graph_error("Partition array must start with vertex_id 0");
      }
      
      // Check that partitions are in strictly increasing order
      for (size_t i = 1; i < partition_.size(); ++i) {
        if (partition_[i] <= partition_[i - 1]) {
          throw graph_error(std::format(
              "Partition ids must be in strictly increasing order: partition[{}]={} is not greater than partition[{}]={}",
              i, partition_[i], i - 1, partition_[i - 1]));
        }
      }
      
      // Additional check: all partition start ids must be valid vertex ids
      if (!partition_.empty() && partition_.back() > static_cast<partition_id_type>(row_index_.size())) {
        throw graph_error(std::format(
            "Invalid partition start id: {} exceeds number of vertices {}",
            partition_.back(), row_index_.size()));
      }
    }

    partition_.push_back(static_cast<partition_id_type>(row_index_.size()));
  }

public: // Vertex range accessors  
  /**
   * @brief Get a range of all vertex IDs in the graph.
   * 
   * Returns an iota view generating vertex IDs from 0 to size()-1. This is useful when
   * you only need vertex IDs rather than the actual vertex objects.
   * 
   * @return Iota view generating vertex IDs [0, size())
   * @note This is a lightweight view with no storage overhead
  */
  [[nodiscard]] constexpr auto vertex_ids() const noexcept {
    return std::views::iota(vertex_id_type{0}, static_cast<vertex_id_type>(size()));
  }

  /**
   * @brief Get a range of all edge IDs in the graph.
   * 
   * Returns an iota view generating edge IDs from 0 to the total number of edges.
   * Edge IDs are indices into col_index_ and the edge value array, useful for
   * direct edge access without needing vertex context.
   * 
   * @return Iota view generating edge IDs [0, total_edges)
   * @note This is a lightweight view with no storage overhead
  */
  [[nodiscard]] constexpr auto edge_ids() const noexcept {
    return std::views::iota(edge_index_type{0}, static_cast<edge_index_type>(col_index_.size()));
  }

  /**
   * @brief Get a range of edge indices for a specific vertex by ID.
   * 
   * Returns an iota view generating edge indices that can be used to index into
   * col_index_ (for target vertex IDs) and the edge value array. This is useful
   * when you need the edge index rather than the edge object itself.
   * 
   * @param id The vertex id to get edge indices for
   * @return Iota view generating edge indices [start_idx, end_idx)
   * @note Returns empty range if id is out of bounds
   * @note This is a lightweight view with no storage overhead
  */
  [[nodiscard]] constexpr auto edge_ids(vertex_id_type id) const noexcept {
    if (id >= size())
      return std::views::iota(edge_index_type{0}, edge_index_type{0});
    
    auto start_idx = row_index_[id].index;
    auto end_idx = row_index_[id + 1].index;
    return std::views::iota(start_idx, end_idx);
  }

  /**
   * @brief Get a const reference to the vertex value for a given vertex ID.
   * 
   * Returns a const reference to the user-defined value stored for the vertex
   * with the specified ID. This provides direct access to vertex data by ID
   * without needing to obtain the vertex object first.
   * 
   * @param id The vertex ID to get the value for
   * @return Const reference to the vertex value
   * @note Only available when VV is not void
   * @note No bounds checking is performed. The caller must ensure id < size()
  */
  template<typename VV_ = VV>
  [[nodiscard]] constexpr auto vertex_value(vertex_id_type id) const noexcept 
    -> std::enable_if_t<!std::is_void_v<VV_>, const VV_&>
  { 
    return row_values_base::operator[](static_cast<typename row_values_base::size_type>(id)); 
  }
  
  /**
   * @brief Get a mutable reference to the vertex value for a given vertex ID.
   * 
   * Returns a mutable reference to the user-defined value stored for the vertex
   * with the specified ID. This allows modification of vertex data by ID without
   * needing to obtain the vertex object first.
   * 
   * @param id The vertex ID to get the value for
   * @return Mutable reference to the vertex value
   * @note Only available when VV is not void
   * @note No bounds checking is performed. The caller must ensure id < size()
  */
  template<typename VV_ = VV>
  [[nodiscard]] constexpr auto vertex_value(vertex_id_type id) noexcept 
    -> std::enable_if_t<!std::is_void_v<VV_>, VV_&>
  { 
    return row_values_base::operator[](static_cast<typename row_values_base::size_type>(id)); 
  }

  /**
   * @brief Get the target vertex ID for a given edge ID.
   * 
   * Returns the target vertex ID for the edge at the specified edge index.
   * Edge IDs are indices into the col_index_ array and can be obtained from
   * edge_ids() or by iterating through edges.
   * 
   * @param edge_id The edge ID (index into col_index_)
   * @return The target vertex ID for this edge
   * @note No bounds checking is performed. The caller must ensure edge_id is valid.
  */
  [[nodiscard]] constexpr vertex_id_type target_id(edge_id_type edge_id) const noexcept {
    return col_index_[edge_id].index;
  }

  /**
   * @brief Get a const reference to the edge value for a given edge ID.
   * 
   * Returns a const reference to the user-defined value stored for the edge
   * at the specified edge index. Edge IDs are indices that can be obtained
   * from edge_ids() or by iterating through edges.
   * 
   * @param edge_id The edge ID (index into edge value array)
   * @return Const reference to the edge value
   * @note Only available when EV is not void
   * @note No bounds checking is performed. The caller must ensure edge_id is valid.
  */
  template<typename EV_ = EV>
  [[nodiscard]] constexpr auto edge_value(edge_id_type edge_id) const noexcept
    -> std::enable_if_t<!std::is_void_v<EV_>, const EV_&>
  {
    return col_values_base::operator[](static_cast<typename col_values_base::size_type>(edge_id));
  }

  /**
   * @brief Get a mutable reference to the edge value for a given edge ID.
   * 
   * Returns a mutable reference to the user-defined value stored for the edge
   * at the specified edge index. This allows modification of edge data by edge ID.
   * 
   * @param edge_id The edge ID (index into edge value array)
   * @return Mutable reference to the edge value
   * @note Only available when EV is not void
   * @note No bounds checking is performed. The caller must ensure edge_id is valid.
  */
  template<typename EV_ = EV>
  [[nodiscard]] constexpr auto edge_value(edge_id_type edge_id) noexcept
    -> std::enable_if_t<!std::is_void_v<EV_>, EV_&>
  {
    return col_values_base::operator[](static_cast<typename col_values_base::size_type>(edge_id));
  }

private:                       // Member variables
  row_index_vector row_index_; // starting index into col_index_ and v_; holds +1 extra terminating row
  col_index_vector col_index_; // col_index_[n] holds the column index (aka target)
  partition_vector partition_; // partition_[n] holds the first vertex id for each partition n
                               // holds +1 extra terminating partition

private:
  friend row_values_base;
  friend col_values_base;

public: // Friend functions
  /**
   * @brief Get a view of all vertices with their descriptors.
   * 
   * Returns a vertex_descriptor_view that iterates over all vertices in the graph,
   * yielding vertex_descriptor objects. Each descriptor provides access to the vertex's
   * ID and optionally its value (via the descriptor's methods when VV is not void).
   * 
   * The view is constructed from row_index_, providing access to vertex IDs [0, size()).
   * Constness of the graph is preserved through the iterator type.
   * 
   * @param g The graph to get vertices from (forwarding reference)
   * @return vertex_descriptor_view over all vertices with appropriate const qualification
   * @note This is a lightweight view with minimal overhead
  */
  template<typename G>
    requires std::derived_from<std::remove_cvref_t<G>, compressed_graph_base>
  [[nodiscard]] friend constexpr auto vertices(G&& g) noexcept {
    // Since row_index_ is a vector of csr_row structs (random access), 
    // we use the index-based constructor with storage_type (size_t) for vertex IDs
    using vertex_iter_type = std::conditional_t<
        std::is_const_v<std::remove_reference_t<G>>,
        typename row_index_vector::const_iterator,
        typename row_index_vector::iterator
    >;
    
    if(g.empty()) {
      return vertex_descriptor_view<vertex_iter_type>(static_cast<std::size_t>(0), static_cast<std::size_t>(0));
    }

    return vertex_descriptor_view<vertex_iter_type>(static_cast<std::size_t>(0), static_cast<std::size_t>(g.size()));
  }

  /**
   * @brief Get a view of vertices in a specific partition.
   * 
   * Returns a vertex_descriptor_view that iterates over vertices in the specified partition,
   * yielding vertex_descriptor objects. Partitions divide vertices into contiguous ranges.
   * 
   * For single-partition graphs (default), partition 0 contains all vertices.
   * For multi-partition graphs, partition_[pid] stores the first vertex ID of partition pid.
   * 
   * @param g The graph to get vertices from (forwarding reference)
   * @param pid The partition ID (0-based index)
   * @return vertex_descriptor_view over vertices in the partition
   * @note Complexity: O(1) - direct access to partition boundaries
   * @note Returns empty view if pid is out of range
   * @note This is the ADL customization point for the vertices(g, pid) CPO
  */
  template<typename G, std::integral PId>
    requires std::derived_from<std::remove_cvref_t<G>, compressed_graph_base>
  [[nodiscard]] friend constexpr auto vertices(G&& g, const PId& pid) noexcept {
    using vertex_iter_type = std::conditional_t<
        std::is_const_v<std::remove_reference_t<G>>,
        typename row_index_vector::const_iterator,
        typename row_index_vector::iterator
    >;
    
    // Handle empty graph
    if(g.empty()) {
      return vertex_descriptor_view<vertex_iter_type>(static_cast<std::size_t>(0), static_cast<std::size_t>(0));
    }
    
    // Handle single partition case (partition_ is empty or size <= 2)
    if (g.partition_.empty() || g.partition_.size() <= 2) {
      if (pid == 0) {
        // Single partition contains all vertices
        return vertex_descriptor_view<vertex_iter_type>(static_cast<std::size_t>(0), static_cast<std::size_t>(g.size()));
      } else {
        // No such partition
        return vertex_descriptor_view<vertex_iter_type>(static_cast<std::size_t>(0), static_cast<std::size_t>(0));
      }
    }
    
    // Multi-partition case
    // partition_.size() - 1 is the number of actual partitions (last element is terminator)
    const auto num_parts = g.partition_.size() - 1;
    
    if (pid < 0 || static_cast<std::size_t>(pid) >= num_parts) {
      // Partition ID out of range
      return vertex_descriptor_view<vertex_iter_type>(static_cast<std::size_t>(0), static_cast<std::size_t>(0));
    }
    
    // Return range [partition_[pid], partition_[pid+1])
    const auto begin_vid = static_cast<std::size_t>(g.partition_[pid]);
    const auto end_vid = static_cast<std::size_t>(g.partition_[pid + 1]);
    
    return vertex_descriptor_view<vertex_iter_type>(begin_vid, end_vid);
  }

  /**
   * @brief Find a vertex by its ID
   * 
   * Returns an iterator to the vertex descriptor for the given vertex ID.
   * For compressed_graph, vertex IDs are sequential indices [0, size()).
   * 
   * The returned iterator is lightweight (stores only the index) and can be
   * compared with vertices(g).end() to check validity:
   * 
   * @code
   * auto v_it = find_vertex(g, uid);
   * if (v_it != vertices(g).end()) {
   *     // Valid vertex - safe to dereference
   *     auto v_desc = *v_it;
   *     // Use descriptor...
   * }
   * @endcode
   * 
   * @param g The graph to search in (forwarding reference)
   * @param uid The vertex ID to find
   * @return Iterator to the vertex descriptor at position uid
   * @note Complexity: O(1) - direct indexed access
   * @note No bounds checking is performed; uid must be valid
   * @note The iterator stores only the vertex index and is independent of view lifetime
  */
  template<typename G, typename VId2>
    requires std::derived_from<std::remove_cvref_t<G>, compressed_graph_base>
  [[nodiscard]] friend constexpr auto find_vertex([[maybe_unused]] G&& g, const VId2& uid) noexcept {
    using vertex_iter_type = std::conditional_t<
        std::is_const_v<std::remove_reference_t<G>>,
        typename row_index_vector::const_iterator,
        typename row_index_vector::iterator
    >;
    using vertex_desc_view = vertex_descriptor_view<vertex_iter_type>;
    using vertex_desc_iterator = typename vertex_desc_view::iterator;
    
    // Construct iterator directly from the vertex ID (which is the storage_type)
    // Cast to vertex_id_type to avoid warnings when uid is a larger integral type (e.g., size_t)
    return vertex_desc_iterator{static_cast<vertex_id_type>(uid)};
  }

  /**
   * @brief Get the vertex ID from a vertex descriptor
   * 
   * Returns the vertex ID for the given vertex descriptor. For compressed_graph,
   * vertex IDs are sequential indices [0, size()).
   * 
   * @param g The graph (forwarding reference) - unused but required by CPO signature
   * @param u The vertex descriptor
   * @return The vertex ID
   * @note Complexity: O(1) - direct access to stored ID
  */
  template<typename G, vertex_descriptor_type VertexDesc>
    requires std::derived_from<std::remove_cvref_t<G>, compressed_graph_base>
  [[nodiscard]] friend constexpr auto vertex_id([[maybe_unused]] const G& g, const VertexDesc& u) noexcept
  {
    return static_cast<vertex_id_type>(u.vertex_id());
  }

  /**
   * @brief Get a view of all edges for a specific vertex.
   * 
   * Returns an edge_descriptor_view that iterates over all outgoing edges from the
   * specified vertex, yielding edge_descriptor objects. Each descriptor provides access
   * to the edge's target vertex and value (if EV is not void).
   * Constness of the graph is preserved through the iterator type.
   * 
   * @param g The graph to get edges from (forwarding reference)
   * @param u The vertex descriptor to get edges for
   * @return edge_descriptor_view over edges from vertex u with appropriate const qualification
   * @note This is a lightweight view with minimal overhead
   * @note Returns empty view if vertex descriptor is out of bounds
  */
  template<typename G, typename VertexDesc>
    requires std::derived_from<std::remove_cvref_t<G>, compressed_graph_base>
  [[nodiscard]] friend constexpr auto edges(G&& g, VertexDesc u) noexcept {
    using edge_iter_type = std::conditional_t<
        std::is_const_v<std::remove_reference_t<G>>,
        typename col_index_vector::const_iterator,
        typename col_index_vector::iterator
    >;
    using vertex_iter_type = std::conditional_t<
        std::is_const_v<std::remove_reference_t<G>>,
        typename row_index_vector::const_iterator,
        typename row_index_vector::iterator
    >;
    using edge_desc_view = edge_descriptor_view<edge_iter_type, vertex_iter_type>;
    using vertex_desc = vertex_descriptor<vertex_iter_type>;
    
    // Get the vertex ID from the descriptor
    auto vid = static_cast<std::size_t>(u.vertex_id());
    
    // Create a vertex descriptor with the correct iterator type for this graph
    vertex_desc source_vd(vid);
    
    // Check bounds
    if (vid >= g.size()) {
      // Return empty view
      return edge_desc_view(static_cast<std::size_t>(0), static_cast<std::size_t>(0), source_vd);
    }
    
    // Get the edge range for this vertex from row_index_
    auto start_idx = static_cast<std::size_t>(g.row_index_[vid].index);
    auto end_idx = static_cast<std::size_t>(g.row_index_[vid + 1].index);
    
    // Return view over the edge range
    return edge_desc_view(start_idx, end_idx, source_vd);
  }

  /**
   * @brief Get the target vertex ID from an edge descriptor
   * 
   * Returns the target vertex ID for the given edge descriptor. For compressed_graph,
   * the edge descriptor's value() method returns the edge index into col_index_,
   * which stores the target vertex IDs.
   * 
   * @param g The graph (forwarding reference)
   * @param uv The edge descriptor
   * @return The target vertex ID
   * @note Complexity: O(1) - direct indexed access
   * @note No bounds checking is performed; edge descriptor must be valid
  */
  template<typename G, typename EdgeDesc>
    requires std::derived_from<std::remove_cvref_t<G>, compressed_graph_base>
  [[nodiscard]] friend constexpr auto target_id(G&& g, const EdgeDesc& uv) noexcept {
    // Edge descriptor's value() returns the edge index into col_index_
    auto edge_idx = uv.value();
    return g.col_index_[edge_idx].index;
  }

  /**
   * @brief Get the total number of edges in the graph
   * 
   * Returns the total count of edges in the compressed sparse row structure.
   * For compressed_graph, this is simply the size of col_index_, which stores
   * one entry per edge.
   * 
   * @param g The graph (forwarding reference)
   * @return The total number of edges
   * @note Complexity: O(1) - direct size access
   * @note This is the ADL customization point for the num_edges(g) CPO
  */
  template<typename G>
    requires std::derived_from<std::remove_cvref_t<G>, compressed_graph_base>
  [[nodiscard]] friend constexpr auto num_edges(G&& g) noexcept {
    return static_cast<size_type>(g.col_index_.size());
  }

  /**
   * @brief Get the number of outgoing edges from a specific vertex
   * 
   * Returns the count of edges originating from the given vertex.
   * For compressed_graph, this is an O(1) operation that computes the difference
   * between consecutive row_index_ entries.
   * 
   * @param g The graph (forwarding reference)
   * @param u The vertex descriptor
   * @return The number of outgoing edges from vertex u
   * @note Complexity: O(1) - direct indexed access
   * @note This is the ADL customization point for the num_edges(g, u) CPO
  */
  template<typename G, typename U>
    requires std::derived_from<std::remove_cvref_t<G>, compressed_graph_base>
  [[nodiscard]] friend constexpr auto num_edges(const G& g, const U& u) noexcept {
    auto vid = static_cast<vertex_id_type>(u.vertex_id());
    if (vid >= g.size()) {
      return static_cast<size_type>(0);
    }
    return static_cast<size_type>(g.row_index_[vid + 1].index - g.row_index_[vid].index);
  }

  /**
   * @brief Check if the graph has any edges
   * 
   * Returns true if the graph contains at least one edge, false otherwise.
   * For compressed_graph, this is an O(1) operation that checks if col_index_
   * is non-empty.
   * 
   * @param g The graph (forwarding reference)
   * @return true if the graph has at least one edge, false otherwise
   * @note Complexity: O(1) - direct size check
   * @note This is the ADL customization point for the has_edge(g) CPO
  */
  template<typename G>
    requires std::derived_from<std::remove_cvref_t<G>, compressed_graph_base>
  [[nodiscard]] friend constexpr bool has_edge(const G& g) noexcept {
    return !g.col_index_.empty();
  }

  /**
   * @brief Get the user-defined value associated with a vertex
   * 
   * Returns a reference to the user-defined value stored for the given vertex.
   * For compressed_graph, extracts the vertex ID from the descriptor and uses
   * it to directly access the vertex value storage.
   * 
   * @param g The graph (forwarding reference for const preservation)
   * @param u The vertex descriptor
   * @return Reference to the vertex value (const if g is const)
   * @note Complexity: O(1) - direct array access by vertex ID
   * @note This is the ADL customization point for the vertex_value(g, u) CPO
   * @note Only available when VV is not void
  */
  template<typename G, typename U>
    requires std::derived_from<std::remove_cvref_t<G>, compressed_graph_base> && 
             (!std::is_void_v<VV>)
  [[nodiscard]] friend constexpr decltype(auto) vertex_value(G&& g, const U& u) noexcept {
    if constexpr (std::is_const_v<std::remove_reference_t<G>>) {
      return g.vertex_value(static_cast<vertex_id_type>(u.vertex_id()));
    } else {
      return g.vertex_value(static_cast<vertex_id_type>(u.vertex_id()));
    }
  }

  /**
   * @brief Get the user-defined value associated with an edge
   * 
   * Returns a reference to the user-defined value stored for the given edge.
   * For compressed_graph, extracts the edge ID from the edge descriptor and uses
   * it to directly access the edge value storage.
   * 
   * @param g The graph (forwarding reference for const preservation)
   * @param uv The edge descriptor
   * @return Reference to the edge value (const if g is const)
   * @note Complexity: O(1) - direct array access by edge ID
   * @note This is the ADL customization point for the edge_value(g, uv) CPO
   * @note Only available when EV is not void
  */
  template<typename G, typename E>
    requires std::derived_from<std::remove_cvref_t<G>, compressed_graph_base> && 
             (!std::is_void_v<EV>)
  [[nodiscard]] friend constexpr decltype(auto) edge_value(G&& g, const E& uv) noexcept {
    if constexpr (std::is_const_v<std::remove_reference_t<G>>) {
      return g.edge_value(static_cast<vertex_id_type>(uv.value()));
    } else {
      return g.edge_value(static_cast<vertex_id_type>(uv.value()));
    }
  }

  /**
   * @brief Get the partition ID for a vertex.
   * 
   * Returns the partition ID (0-based index) of the partition containing the given vertex.
   * Partitions divide vertices into contiguous ranges, typically for distributed computing
   * or NUMA-aware processing.
   * 
   * @param g The graph (forwarding reference for const preservation)
   * @param u The vertex descriptor
   * @return Partition ID (integral type, 0 if single partition)
   * @note Complexity: O(log P) where P is the number of partitions (binary search)
   * @note For single-partition graphs, returns 0
   * @note This is the ADL customization point for the partition_id(g, u) CPO
  */
  template<typename G, typename VertexDesc>
    requires std::derived_from<std::remove_cvref_t<G>, compressed_graph_base>
  [[nodiscard]] friend constexpr auto partition_id(G&& g, const VertexDesc& u) noexcept
    -> partition_id_type {
    const auto vid = u.vertex_id();
    
    // Handle empty or single partition case
    if (g.partition_.size() <= 2) {
      return 0;
    }
    
    // Binary search: find the largest partition index i where partition_[i] <= vid
    // Note: partition_.back() is the terminating element (total vertex count), not a real partition
    auto it = std::upper_bound(g.partition_.begin(), g.partition_.end() - 1, vid);
    --it;  // Move back to the last partition where partition_[i] <= vid
    
    return static_cast<partition_id_type>(std::distance(g.partition_.begin(), it));
  }

  /**
   * @brief Get the number of partitions in the graph.
   * 
   * Returns the total number of partitions that divide the vertices in the graph.
   * For single-partition graphs (the default), returns 1.
   * 
   * @param g The graph (const reference)
   * @return Number of partitions (integral type, minimum 1)
   * @note Complexity: O(1) - direct access to partition vector size
   * @note partition_ vector stores N+1 elements for N partitions (includes terminator)
   * @note This is the ADL customization point for the num_partitions(g) CPO
  */
  template<typename G>
    requires std::derived_from<std::remove_cvref_t<G>, compressed_graph_base>
  [[nodiscard]] friend constexpr auto num_partitions(const G& g) noexcept
    -> partition_id_type {
    // partition_ holds partition start IDs plus one terminating element
    // So num_partitions = partition_.size() - 1
    // For empty or single-partition: size is 0, 1, or 2, all map to 1 partition
    if (g.partition_.empty()) {
      return 1;
    }
    return static_cast<partition_id_type>(g.partition_.size() - 1);
  }
};


/**
 * @ingroup graph_containers
 * @brief Compressed Sparse Row adjacency graph container.
 *
 * When defining multiple partitions, the partition_start_ids[] must be in increasing order.
 * If the partition_start_ids[] is empty, all vertices are in partition 0. If partition_start_ids[0]!=0,
 * 0 will be inserted as the start of the first partition id.
 * 
 * @tparam EV Edge value type
 * @tparam VV Vertex value type
 * @tparam GV Graph value type
 * @tparam VI Vertex Id type. This must be large enough for the count of vertices.
 * @tparam EIndex Edge Index type. This must be large enough for the count of edges.
 * @tparam Alloc Allocator type
*/
template <class EV, class VV, class GV, integral VId, integral EIndex, class Alloc>
class compressed_graph : public compressed_graph_base<EV, VV, GV, VId, EIndex, Alloc> {
public: // Types
  using graph_type = compressed_graph<EV, VV, GV, VId, EIndex, Alloc>;
  using base_type  = compressed_graph_base<EV, VV, GV, VId, EIndex, Alloc>;

  using edge_value_type   = EV;
  using vertex_value_type = VV;
  using graph_value_type  = GV;
  using value_type        = GV;

  using vertex_id_type = VId;

public: // Graph value accessors
  /**
   * @brief Get a const reference to the graph value.
   * 
   * Returns a const reference to the user-defined graph value stored in the graph.
   * This value is independent of vertices and edges and represents metadata or
   * properties associated with the entire graph.
   * 
   * @return Const reference to the graph value
   * @note Only available when GV is not void
  */
  [[nodiscard]] constexpr const graph_value_type& graph_value() const noexcept { return value_; }
  
  /**
   * @brief Get a mutable reference to the graph value.
   * 
   * Returns a mutable reference to the user-defined graph value stored in the graph.
   * This allows modification of graph-level metadata without affecting the graph structure.
   * 
   * @return Mutable reference to the graph value
   * @note Only available when GV is not void
  */
  [[nodiscard]] constexpr graph_value_type& graph_value() noexcept { return value_; }

public: // Construction/Destruction
  constexpr compressed_graph()                        = default;
  constexpr compressed_graph(const compressed_graph&) = default;
  constexpr compressed_graph(compressed_graph&&)      = default;
  constexpr ~compressed_graph()                       = default;

  constexpr compressed_graph& operator=(const compressed_graph&) = default;
  constexpr compressed_graph& operator=(compressed_graph&&)      = default;

  // compressed_graph(      alloc)
  // compressed_graph(gv&,  alloc)
  // compressed_graph(gv&&, alloc)

  constexpr compressed_graph(const Alloc& alloc) : base_type(alloc) {}
  constexpr compressed_graph(const graph_value_type& value, const Alloc& alloc = Alloc())
        : base_type(alloc), value_(value) {}
  constexpr compressed_graph(graph_value_type&& value, const Alloc& alloc = Alloc())
        : base_type(alloc), value_(move(value)) {}

  // compressed_graph(      erng, eprojection, alloc)
  // compressed_graph(gv&,  erng, eprojection, alloc)
  // compressed_graph(gv&&, erng, eprojection, alloc)

  template <forward_range ERng, forward_range PartRng, class EProj = identity>
  requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
           convertible_to<range_value_t<PartRng>, VId>
  constexpr compressed_graph(const ERng&    erng,
                             EProj          eprojection,
                             const PartRng& partition_start_ids = std::vector<VId>(),
                             const Alloc&   alloc               = Alloc())
        : base_type(erng, eprojection, partition_start_ids, alloc) {}

  template <forward_range ERng, forward_range PartRng, class EProj = identity>
  requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
                 convertible_to<range_value_t<PartRng>, VId>
  constexpr compressed_graph(const graph_value_type& value,
                             const ERng&             erng,
                             EProj                   eprojection,
                             const PartRng&          partition_start_ids = std::vector<VId>(),
                             const Alloc&            alloc               = Alloc())
        : base_type(erng, eprojection, partition_start_ids, alloc), value_(value) {}

  template <forward_range ERng, forward_range PartRng, class EProj = identity>
  requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
                 convertible_to<range_value_t<PartRng>, VId>
  constexpr compressed_graph(graph_value_type&& value,
                             const ERng&        erng,
                             EProj              eprojection,
                             const PartRng&     partition_start_ids = std::vector<VId>(),
                             const Alloc&       alloc               = Alloc())
        : base_type(erng, eprojection, partition_start_ids, alloc), value_(move(value)) {}

  // compressed_graph(      erng, vrng, eprojection, vprojection, alloc)
  // compressed_graph(gv&,  erng, vrng, eprojection, vprojection, alloc)
  // compressed_graph(gv&&, erng, vrng, eprojection, vprojection, alloc)

  template <forward_range ERng,
            forward_range VRng,
            forward_range PartRng,
            class EProj = identity,
            class VProj = identity>
  requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
           copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV> &&
           convertible_to<range_value_t<PartRng>, VId>
  constexpr compressed_graph(const ERng&    erng,
                             const VRng&    vrng,
                             EProj          eprojection         = {},
                             VProj          vprojection         = {},
                             const PartRng& partition_start_ids = std::vector<VId>(),
                             const Alloc&   alloc               = Alloc())
        : base_type(erng, vrng, eprojection, vprojection, partition_start_ids, alloc) {}

  template <forward_range ERng,
            forward_range VRng,
            forward_range PartRng,
            class EProj = identity,
            class VProj = identity>
  requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
                 copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV> &&
                 convertible_to<range_value_t<PartRng>, VId>
  constexpr compressed_graph(const graph_value_type& value,
                             const ERng&             erng,
                             const VRng&             vrng,
                             EProj                   eprojection         = {},
                             VProj                   vprojection         = {},
                             const PartRng&          partition_start_ids = std::vector<VId>(),
                             const Alloc&            alloc               = Alloc())
        : base_type(erng, vrng, eprojection, vprojection, partition_start_ids, alloc), value_(value) {}

  template <forward_range ERng,
            forward_range VRng,
            forward_range PartRng,
            class EProj = identity,
            class VProj = identity>
  requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
                 copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV> &&
                 convertible_to<range_value_t<PartRng>, VId>
  constexpr compressed_graph(graph_value_type&& value,
                             const ERng&        erng,
                             const VRng&        vrng,
                             EProj              eprojection         = {},
                             VProj              vprojection         = {},
                             const PartRng&     partition_start_ids = std::vector<VId>(),
                             const Alloc&       alloc               = Alloc())
        : base_type(erng, vrng, eprojection, vprojection, partition_start_ids, alloc), value_(move(value)) {}

  constexpr compressed_graph(const std::initializer_list<copyable_edge_t<VId, EV>>& ilist, const Alloc& alloc = Alloc())
        : base_type(ilist, alloc) {}

private: // Member variables
  graph_value_type value_ = graph_value_type();
};

/**
 * @ingroup graph_containers
 * @brief Compressed Sparse Row adjacency graph container.
 *
 * For constructors that accept a partition function, the function must return a partition id for a vertex id.
 * When used, the range of input edges must be ordered by the partition id they're in. Partions may be skipped
 * (empty) but the partition id must be in increasing order.
 *
 * @tparam EV Edge value type
 * @tparam VV Vertex value type
 * @tparam VI Vertex Id type. This must be large enough for the count of vertices.
 * @tparam EIndex Edge Index type. This must be large enough for the count of edges.
 * @tparam Alloc Allocator type
*/
template <class EV, class VV, integral VId, integral EIndex, class Alloc>
class compressed_graph<EV, VV, void, VId, EIndex, Alloc>
      : public compressed_graph_base<EV, VV, void, VId, EIndex, Alloc> {
public: // Types
  using graph_type = compressed_graph<EV, VV, void, VId, EIndex, Alloc>;
  using base_type  = compressed_graph_base<EV, VV, void, VId, EIndex, Alloc>;

  using vertex_id_type    = VId;
  using vertex_value_type = VV;

  using graph_value_type = void;
  using value_type       = void;

public: // Construction/Destruction
  constexpr compressed_graph()                        = default;
  constexpr compressed_graph(const Alloc& alloc)      : base_type(alloc) {}
  constexpr compressed_graph(const compressed_graph&) = default;
  constexpr compressed_graph(compressed_graph&&)      = default;
  constexpr ~compressed_graph()                       = default;

  constexpr compressed_graph& operator=(const compressed_graph&) = default;
  constexpr compressed_graph& operator=(compressed_graph&&)      = default;

  // edge-only construction
  template <forward_range ERng, class EProj = identity, forward_range PartRng = std::vector<VId>>
  requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV>
  constexpr compressed_graph(const ERng&    erng,
                             EProj          eprojection         = identity(),
                             const PartRng& partition_start_ids = std::vector<VId>(),
                             const Alloc&   alloc               = Alloc())
        : base_type(erng, eprojection, partition_start_ids, alloc) {}

  // edge and vertex value construction
  template <forward_range ERng,
            forward_range VRng,
            class EProj           = identity,
            class VProj           = identity,
            forward_range PartRng = std::vector<VId>>
  requires copyable_edge<invoke_result_t<EProj, range_value_t<ERng>>, VId, EV> &&
           copyable_vertex<invoke_result_t<VProj, range_value_t<VRng>>, VId, VV> &&
           convertible_to<range_value_t<PartRng>, VId>
  constexpr compressed_graph(const ERng&    erng,
                             const VRng&    vrng,
                             EProj          eprojection         = {},
                             VProj          vprojection         = {},
                             const PartRng& partition_start_ids = std::vector<VId>(),
                             const Alloc&   alloc               = Alloc())
        : base_type(erng, vrng, eprojection, vprojection, partition_start_ids, alloc) {}

  // initializer list using edge_info<VId,true,void,EV>
  constexpr compressed_graph(const std::initializer_list<copyable_edge_t<VId, EV>>& ilist, const Alloc& alloc = Alloc())
        : base_type(ilist, alloc) {}
};

} // namespace graph::container
