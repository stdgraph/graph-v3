#pragma once

#include <type_traits>
#include <concepts>
#include <functional>
#include <utility>

namespace graph::edge_list {

namespace detail {
  // Empty type for void value optimization
  struct empty_value {
    constexpr auto operator<=>(const empty_value&) const noexcept = default;
  };
} // namespace detail

/**
 * @brief Lightweight edge descriptor for edge lists
 * 
 * This descriptor is a non-owning reference to edge data stored in an edge list.
 * It stores references to the source ID, target ID, and optionally the edge value,
 * avoiding any copies. The descriptor is only valid as long as the referenced data exists.
 * 
 * @tparam VId Vertex ID type
 * @tparam EV Edge value type (void for edges without values)
 */
template <typename VId, typename EV = void>
class edge_descriptor {
public:
  using vertex_id_type  = VId;
  using edge_value_type = EV;

  // Constructor without value (for void EV)
  constexpr edge_descriptor(const VId& src, const VId& tgt)
  requires std::is_void_v<EV>
        : source_id_(src), target_id_(tgt), value_() {}

  // Constructor with value (for non-void EV)
  template <typename E = EV>
  requires(!std::is_void_v<E>)
  constexpr edge_descriptor(const VId& src, const VId& tgt, const E& val)
        : source_id_(src), target_id_(tgt), value_(std::cref(val)) {}

  // Copy constructor and assignment - this is a reference type
  edge_descriptor(const edge_descriptor&)            = default;
  edge_descriptor& operator=(const edge_descriptor&) = delete;

  // Move constructor and assignment - this is a reference type
  edge_descriptor(edge_descriptor&&)            = default;
  edge_descriptor& operator=(edge_descriptor&&) = delete;

  // Accessors - return the stored references
  [[nodiscard]] constexpr const VId& source_id() const noexcept { return source_id_; }

  [[nodiscard]] constexpr const VId& target_id() const noexcept { return target_id_; }

  // Value accessor (only for non-void EV)
  template <typename E = EV>
  requires(!std::is_void_v<E>)
  [[nodiscard]] constexpr const E& value() const noexcept {
    return value_.get();
  }

  // Comparison operators - compare referenced values, not references themselves
  constexpr bool operator==(const edge_descriptor& other) const noexcept {
    if constexpr (std::is_void_v<EV>) {
      return source_id_ == other.source_id_ && target_id_ == other.target_id_;
    } else {
      return source_id_ == other.source_id_ && target_id_ == other.target_id_ && value_.get() == other.value_.get();
    }
  }

  constexpr auto operator<=>(const edge_descriptor& other) const noexcept {
    if (auto cmp = source_id_ <=> other.source_id_; cmp != 0)
      return cmp;
    if (auto cmp = target_id_ <=> other.target_id_; cmp != 0)
      return cmp;
    if constexpr (!std::is_void_v<EV>) {
      return value_.get() <=> other.value_.get();
    } else {
      return std::strong_ordering::equal;
    }
  }

private:
  const VId& source_id_;
  const VId& target_id_;
  [[no_unique_address]] std::conditional_t<std::is_void_v<EV>, detail::empty_value, std::reference_wrapper<const EV>>
        value_;
};

// Deduction guides
template <typename VId>
edge_descriptor(VId, VId) -> edge_descriptor<VId, void>;

template <typename VId, typename EV>
edge_descriptor(VId, VId, EV) -> edge_descriptor<VId, EV>;

} // namespace graph::edge_list
