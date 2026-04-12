/**
 * @file edge_descriptor.hpp
 * @brief Edge descriptor template for graph edges
 */

#pragma once

#include "vertex_descriptor.hpp"
#include <compare>
#include <functional>
#include <numeric>

namespace graph::adj_list {

/**
 * @brief Descriptor for edges in a graph
 * 
 * Provides a lightweight, type-safe handle to edges stored in various container types.
 * Maintains both the edge location (as an iterator) and the source vertex descriptor.
 * 
 * Unlike vertex_descriptor which supports both index and iterator storage,
 * edge_descriptor always stores the edge iterator directly. There is no need for
 * an index-only edge path because edges are always backed by a physical container.
 * 
 * The EdgeDirection tag controls source/target semantics:
 * - out_edge_tag (default): source_ is the source vertex, target navigates .edges()
 * - in_edge_tag: source_ is the target vertex, source navigates .in_edges()
 * 
 * @tparam EdgeIter Iterator type of the underlying edge container
 * @tparam VertexIter Iterator type of the vertex container
 * @tparam EdgeDirection Direction tag: out_edge_tag or in_edge_tag
 */
template <edge_iterator EdgeIter, vertex_iterator VertexIter, class EdgeDirection = out_edge_tag>
class edge_descriptor {
public:
  using edge_iterator_type   = EdgeIter;
  using vertex_iterator_type = VertexIter;
  using vertex_desc          = vertex_descriptor<VertexIter>;
  using edge_direction       = EdgeDirection;

  /// True when this descriptor wraps an in-edge (source/target semantics are swapped).
  static constexpr bool is_in_edge = std::is_same_v<EdgeDirection, in_edge_tag>;

  /// True when this descriptor wraps an out-edge (default direction).
  static constexpr bool is_out_edge = std::is_same_v<EdgeDirection, out_edge_tag>;

  // Edge storage is always the iterator itself
  using edge_storage_type = EdgeIter;

  // Default constructor
  constexpr edge_descriptor() noexcept
  requires std::default_initializable<edge_storage_type> && std::default_initializable<vertex_desc>
        : edge_storage_{}, source_{} {}

  // Constructor from edge iterator and source vertex descriptor
  constexpr edge_descriptor(edge_storage_type edge_val, vertex_desc source) noexcept
        : edge_storage_(edge_val), source_(source) {}

  /**
     * @brief Get the underlying edge iterator
     * @return The stored edge iterator
     */
  [[nodiscard]] constexpr edge_storage_type value() const noexcept { return edge_storage_; }

  /**
     * @brief Get the source vertex descriptor
     * @return The vertex descriptor representing the source of this edge
     */
  [[nodiscard]] constexpr vertex_desc source() const noexcept { return source_; }

  /**
     * @brief Get the source vertex ID
     * @return The vertex ID of the source vertex (by value for integral, const& for map keys)
     */
  [[nodiscard]] constexpr decltype(auto) source_id() const noexcept { return source_.vertex_id(); }

  /**
     * @brief Get the source vertex ID by navigating the edge container (in-edge only)
     * @param vertex_data The vertex/edge data structure passed from the CPO
     * @return The source vertex identifier extracted from the native in-edge
     * 
     * For in-edge descriptors, source_id() (no args) returns the owning vertex ID
     * which is actually the target. This overload navigates the in_edges container
     * to retrieve the native edge's source_id — the actual source vertex.
     * 
     * Only available when EdgeDirection is in_edge_tag.
     */
  template <typename VertexData>
  [[nodiscard]] constexpr auto source_id(const VertexData& vertex_data) const noexcept
  requires(is_in_edge) {
    const auto& edge_val = *edge_storage_;

    // Extract source_id from the native edge
    if constexpr (requires { edge_val.second.source_id(); }) {
      return edge_val.second.source_id();
    } else if constexpr (requires { edge_val.source_id(); }) {
      return edge_val.source_id();
    } else if constexpr (requires { edge_val.first; }) {
      return edge_val.first;
    } else if constexpr (requires { std::get<0>(edge_val); }) {
      return std::get<0>(edge_val);
    } else {
      return edge_val;
    }
  }

  /**
     * @brief Get the target vertex ID for an in-edge descriptor
     * @param vertex_data The vertex/edge data structure passed from the CPO (unused)
     * @return The target vertex identifier (the owning vertex)
     * 
     * For in-edges, the owning vertex IS the target — no container navigation needed.
     * 
     * Only available when EdgeDirection is in_edge_tag.
     */
  template <typename VertexData>
  [[nodiscard]] constexpr auto target_id(const VertexData& /*vertex_data*/) const noexcept
  requires(is_in_edge) {
    return source_.vertex_id();
  }

  /**
     * @brief Get the target vertex ID for an out-edge descriptor
     * @param vertex_data The vertex/edge data structure passed from the CPO
     * @return The target vertex identifier extracted from the edge
     * 
     * Dereferences the stored edge iterator to extract the target vertex ID.
     * 
     * Only available when EdgeDirection is out_edge_tag.
     */
  template <typename VertexData>
  [[nodiscard]] constexpr auto target_id(const VertexData& /*vertex_data*/) const noexcept
  requires(is_out_edge) {
    using edge_value_type = typename std::iterator_traits<EdgeIter>::value_type;

    const auto& edge_val = *edge_storage_;

    // Extract target ID from edge value based on its type
    if constexpr (std::integral<edge_value_type>) {
      return edge_val;
    } else if constexpr (requires { edge_val.second.target_id(); }) {
      return edge_val.second.target_id();
    } else if constexpr (requires { edge_val.target_id(); }) {
      return edge_val.target_id();
    } else if constexpr (requires { edge_val.first; }) {
      return (edge_val.first);
    } else if constexpr (requires { std::get<0>(edge_val); }) {
      return std::get<0>(edge_val);
    } else {
      return edge_val;
    }
  }

  /**
     * @brief Get the underlying container value (the actual edge data)
     * @param vertex_data The vertex/edge data structure passed from the CPO
     * @return Reference to the edge data from the container
     * 
     * Dereferences the stored edge iterator.
     */
  template <typename VertexData>
  [[nodiscard]] constexpr decltype(auto) underlying_value(VertexData& /*vertex_data*/) const noexcept {
    return (*edge_storage_);
  }

  /**
     * @brief Get the underlying container value (const version)
     * @param vertex_data The vertex/edge data structure passed from the CPO
     * @return Const reference to the edge data from the container
     */
  template <typename VertexData>
  [[nodiscard]] constexpr decltype(auto) underlying_value(const VertexData& /*vertex_data*/) const noexcept {
    return (*edge_storage_);
  }

  /**
     * @brief Get the inner/property value (excluding the target ID)
     * @param vertex_data The vertex/edge data structure passed from the CPO
     * @return Reference to the edge properties (excluding target vertex ID)
     * 
     * Behavior based on edge data type:
     * - Simple integral type (int): Returns the value itself (since it's just the target ID)
     * - Pair<target, property>: Returns .second (the property part)
     * - Tuple<target, prop1, prop2, ...>: Returns reference to tuple of remaining elements (excluding first)
     * - Struct/Custom type: Returns the whole value (user manages which fields are properties)
     * 
     * For tuples with 3+ elements, this creates a tuple of references to elements [1, N).
     */
  template <typename VertexData>
  [[nodiscard]] constexpr decltype(auto) inner_value(VertexData& /*vertex_data*/) const noexcept {
    using edge_value_type = typename std::iterator_traits<EdgeIter>::value_type;

    auto& edge_val = *edge_storage_;

    if constexpr (std::integral<edge_value_type>) {
      return (edge_val);
    } else if constexpr (requires {
                           std::declval<edge_value_type>().first;
                           std::declval<edge_value_type>().second;
                         }) {
      return (edge_val.second);
    } else if constexpr (requires { std::tuple_size<edge_value_type>::value; }) {
      constexpr size_t tuple_size = std::tuple_size<edge_value_type>::value;
      if constexpr (tuple_size == 1) {
        return (std::get<0>(edge_val));
      } else if constexpr (tuple_size == 2) {
        return (std::get<1>(edge_val));
      } else {
        return [&]<size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
          return std::forward_as_tuple(std::get<Is + 1>(edge_val)...);
        }(std::make_index_sequence<tuple_size - 1>{});
      }
    } else {
      return (edge_val);
    }
  }

  /**
     * @brief Get the inner/property value (const version)
     * @param vertex_data The vertex/edge data structure passed from the CPO
     * @return Const reference to the edge properties
     */
  template <typename VertexData>
  [[nodiscard]] constexpr decltype(auto) inner_value(const VertexData& /*vertex_data*/) const noexcept {
    using edge_value_type = typename std::iterator_traits<EdgeIter>::value_type;

    const auto& edge_val = *edge_storage_;

    if constexpr (std::integral<edge_value_type>) {
      return (edge_val);
    } else if constexpr (requires {
                           std::declval<edge_value_type>().first;
                           std::declval<edge_value_type>().second;
                         }) {
      return (edge_val.second);
    } else if constexpr (requires { std::tuple_size<edge_value_type>::value; }) {
      constexpr size_t tuple_size = std::tuple_size<edge_value_type>::value;
      if constexpr (tuple_size == 1) {
        return (std::get<0>(edge_val));
      } else if constexpr (tuple_size == 2) {
        return (std::get<1>(edge_val));
      } else {
        return [&]<size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
          return std::forward_as_tuple(std::get<Is + 1>(edge_val)...);
        }(std::make_index_sequence<tuple_size - 1>{});
      }
    } else {
      return (edge_val);
    }
  }

  // Pre-increment: advances edge position, keeps source unchanged
  constexpr edge_descriptor& operator++() noexcept {
    ++edge_storage_;
    return *this;
  }

  // Post-increment
  constexpr edge_descriptor operator++(int) noexcept {
    edge_descriptor tmp = *this;
    ++edge_storage_;
    return tmp;
  }

  // Comparison operators
  [[nodiscard]] auto operator<=>(const edge_descriptor&) const noexcept = default;
  [[nodiscard]] bool operator==(const edge_descriptor&) const noexcept  = default;

private:
  edge_storage_type edge_storage_;
  vertex_desc       source_;
};

} // namespace graph::adj_list

// Hash specialization for std::unordered containers
namespace std {
template <graph::adj_list::edge_iterator EdgeIter, graph::adj_list::vertex_iterator VertexIter, class Dir>
struct hash<graph::adj_list::edge_descriptor<EdgeIter, VertexIter, Dir>> {
  [[nodiscard]] size_t operator()(const graph::adj_list::edge_descriptor<EdgeIter, VertexIter, Dir>& ed) const noexcept {
    // Hash using the address of the referenced element.
    // This assumes the iterator is dereferenceable (not end).
    size_t h1 = std::hash<std::size_t>{}(reinterpret_cast<std::size_t>(&(*ed.value())));
    size_t h2 = std::hash<graph::adj_list::vertex_descriptor<VertexIter>>{}(ed.source());

    // Combine hashes using a simple mixing function
    return h1 ^ (h2 << 1);
  }
};
} // namespace std
