#pragma once

#include <compare>
#include <concepts>
#include <functional>
#include <utility>

namespace graph::edge_list {

template <typename VId, typename EV = void>
class edge_descriptor;

/**
 * @brief Lightweight edge descriptor for edge lists without edge values
 * 
 * This descriptor is a non-owning reference to edge data stored in an edge list.
 * It stores references to source and target IDs,
 * avoiding any copies. The descriptor is only valid as long as the referenced data exists.
 * 
 * @tparam VId Vertex ID type
 */
template <typename VId>
class edge_descriptor<VId, void> {
public:
  using vertex_id_type  = VId;
  using edge_value_type = void;

  constexpr edge_descriptor(const VId& src, const VId& tgt)
        : source_id_(src), target_id_(tgt) {}

  edge_descriptor(const edge_descriptor&)            = default;
  edge_descriptor& operator=(const edge_descriptor&) = delete;

  edge_descriptor(edge_descriptor&&)            = default;
  edge_descriptor& operator=(edge_descriptor&&) = delete;

  [[nodiscard]] constexpr const VId& source_id() const noexcept { return source_id_; }

  [[nodiscard]] constexpr const VId& target_id() const noexcept { return target_id_; }

  constexpr bool operator==(const edge_descriptor& other) const noexcept {
    return source_id_ == other.source_id_ && target_id_ == other.target_id_;
  }

  constexpr auto operator<=>(const edge_descriptor& other) const noexcept {
    if (auto cmp = source_id_ <=> other.source_id_; cmp != 0)
      return cmp;
    return target_id_ <=> other.target_id_;
  }

private:
  const VId& source_id_;
  const VId& target_id_;
};

/**
 * @brief Lightweight edge descriptor for edge lists with edge values
 *
 * This descriptor is a non-owning reference to edge data stored in an edge list.
 * It stores references to source ID, target ID, and edge value.
 * The descriptor is only valid as long as the referenced data exists.
 *
 * @tparam VId Vertex ID type
 * @tparam EV Edge value type
 */
template <typename VId, typename EV>
class edge_descriptor {
public:
  using vertex_id_type  = VId;
  using edge_value_type = EV;

  constexpr edge_descriptor(const VId& src, const VId& tgt, const EV& val)
        : source_id_(src), target_id_(tgt), value_(std::cref(val)) {}

  edge_descriptor(const edge_descriptor&)            = default;
  edge_descriptor& operator=(const edge_descriptor&) = delete;

  edge_descriptor(edge_descriptor&&)            = default;
  edge_descriptor& operator=(edge_descriptor&&) = delete;

  [[nodiscard]] constexpr const VId& source_id() const noexcept { return source_id_; }

  [[nodiscard]] constexpr const VId& target_id() const noexcept { return target_id_; }

  [[nodiscard]] constexpr const EV& value() const noexcept {
    return value_.get();
  }

  constexpr bool operator==(const edge_descriptor& other) const noexcept {
    return source_id_ == other.source_id_ && target_id_ == other.target_id_ && value_.get() == other.value_.get();
  }

  constexpr auto operator<=>(const edge_descriptor& other) const noexcept {
    if (auto cmp = source_id_ <=> other.source_id_; cmp != 0)
      return cmp;
    if (auto cmp = target_id_ <=> other.target_id_; cmp != 0)
      return cmp;
    return value_.get() <=> other.value_.get();
  }

private:
  const VId& source_id_;
  const VId& target_id_;
  std::reference_wrapper<const EV> value_;
};

// Deduction guides
template <typename VId>
edge_descriptor(VId, VId) -> edge_descriptor<VId, void>;

template <typename VId, typename EV>
edge_descriptor(VId, VId, EV) -> edge_descriptor<VId, EV>;

} // namespace graph::edge_list
