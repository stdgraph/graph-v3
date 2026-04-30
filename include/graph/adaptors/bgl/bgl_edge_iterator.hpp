#pragma once
#include <iterator>
#include <type_traits>

namespace graph::bgl {

/// Wraps a pre-C++20 BGL iterator to satisfy std::forward_iterator.
/// BGL's boost::iterator_facade defines iterator_category but not
/// iterator_concept, which C++20 ranges require.
template <typename BGL_Iter>
class bgl_edge_iterator {
  BGL_Iter it_{};

public:
  // C++20 iterator requirements
  using iterator_concept = std::forward_iterator_tag;
  using value_type       = typename std::iterator_traits<BGL_Iter>::value_type;
  using difference_type  = typename std::iterator_traits<BGL_Iter>::difference_type;
  using reference        = typename std::iterator_traits<BGL_Iter>::reference;
  using pointer          = std::conditional_t<std::is_reference_v<reference>,
                                              std::add_pointer_t<std::remove_reference_t<reference>>,
                                              void>;

  bgl_edge_iterator() = default;
  explicit bgl_edge_iterator(BGL_Iter it) : it_(it) {}

  reference operator*() const { return *it_; }

  // Only provide operator-> when dereferencing yields a stable lvalue reference.
  auto operator->() const
    requires std::is_reference_v<reference>
  {
    return std::addressof(*it_);
  }

  bgl_edge_iterator& operator++() {
    ++it_;
    return *this;
  }
  bgl_edge_iterator operator++(int) {
    auto tmp = *this;
    ++it_;
    return tmp;
  }

  friend bool operator==(const bgl_edge_iterator& a, const bgl_edge_iterator& b) {
    return a.it_ == b.it_;
  }

  /// Access the underlying BGL iterator (needed for edge property extraction).
  const BGL_Iter& base() const { return it_; }
};

} // namespace graph::bgl
