#pragma once

#include <type_traits>

namespace graph::edge_list {

// Forward declaration - actual type defined in edge_list_descriptor.hpp
template <typename VId, typename EV>
struct edge_descriptor;

// Type trait to identify edge_list descriptors
template <typename T>
struct is_edge_list_descriptor : std::false_type {};

template <typename VId, typename EV>
struct is_edge_list_descriptor<edge_descriptor<VId, EV>> : std::true_type {};

template <typename T>
inline constexpr bool is_edge_list_descriptor_v = is_edge_list_descriptor<T>::value;

} // namespace graph::edge_list
