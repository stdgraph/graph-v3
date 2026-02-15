#pragma once

#ifndef CONTAINER_UTILITY_HPP
#  define CONTAINER_UTILITY_HPP

#  include "graph/detail/graph_using.hpp"

namespace graph::container {

using namespace graph; // Bring in the using declarations from parent namespace

//--------------------------------------------------------------------------------------------------
// utility functions
//

template <class C>
concept reservable = requires(C& container, typename C::size_type n) {
  { container.reserve(n) };
};
template <class C>
concept resizable = requires(C& container, typename C::size_type n) {
  { container.resize(n) };
};

template <class C>
concept has_emplace_back = requires(C& container, typename C::value_type&& value) {
  { container.emplace_back(move(value)) };
};
template <class C>
concept has_push_back = requires(C& container, std::ranges::range_reference_t<C> val) {
  { container.push_back(val) };
};
template <class C>
concept has_emplace_front = requires(C& container, typename C::value_type&& value) {
  { container.emplace_front(move(value)) };
};
template <class C>
concept has_push_front = requires(C& container, const typename C::value_type& value) {
  { container.push_front(value) };
};
template <class C>
concept has_emplace = requires(C& container, typename C::value_type&& value) {
  { container.emplace(move(value)) };
};
template <class C>
concept has_insert = requires(C& container, const typename C::value_type& value) {
  { container.insert(value) };
};

template <class C, class Idx>
concept has_array_operator = requires(C&& container, Idx idx) {
  { container[idx] }; //->is_lvalue_reference_v;
};

// Concepts for detecting associative containers (map/unordered_map)
template <class C>
concept has_key_type = requires { typename C::key_type; };

template <class C>
concept has_mapped_type = requires { typename C::mapped_type; };

template <class C>
concept is_associative_container = has_key_type<C> && has_mapped_type<C>;

// Concept for detecting map-based edge containers (std::map<VId, edge_type>)
// These containers use vertex IDs as keys and edges as values
template <class C>
concept is_map_based_edge_container =
      is_associative_container<C> &&
      requires {
        typename C::mapped_type::vertex_id_type; // edge_type has vertex_id_type
      } &&
      (std::same_as<typename C::key_type, typename C::mapped_type::vertex_id_type> ||
       std::is_same_v<typename C::key_type,
                      std::pair<typename C::mapped_type::vertex_id_type, typename C::mapped_type::vertex_id_type>>);

// Concept for detecting map-based vertex containers (std::map<VId, vertex_type>)
// These containers use vertex IDs as keys and vertices as values
template <class C>
concept is_map_based_vertex_container = is_associative_container<C> && requires {
  typename C::mapped_type;                                   // has mapped_type (vertex_type)
  typename C::key_type;                                      // has key_type (VId)
} && !requires { typename C::mapped_type::vertex_id_type; }; // NOT an edge container

// return a lambda to push/insert/emplace an element in a container
template <class C>
constexpr auto push_or_insert(C& container) {
  // favor pushing to the back over the front for things like list & deque
  if constexpr (has_emplace_back<C>)
    return [&container](typename C::value_type&& value) { container.emplace_back(std::move(value)); };
  else if constexpr (has_push_back<C>) {
    return [&container](const typename C::value_type& value) { container.push_back(value); };
  } else if constexpr (has_emplace_front<C>)
    return [&container](typename C::value_type&& value) { container.emplace_front(std::move(value)); };
  else if constexpr (has_push_front<C>) {
    return [&container](const typename C::value_type& value) { container.push_front(value); };
  } else if constexpr (has_emplace<C>)
    return [&container](typename C::value_type&& value) { container.emplace(std::move(value)); };
  else if constexpr (has_insert<C>) {
    return [&container](const typename C::value_type& value) { container.insert(value); };
  }
#  ifdef _MSC_VER
  // This didn't assert if a previous if was true until MSVC 1931; gcc has always asserted
  // We need the ability to put constexpr on an else
  //else {
  //    static_assert(false,
  //              "The container doesn't have emplace_back, push_back, emplace_front, push_front, emplace or insert");
  //}
#  endif
}

// Helper to insert edges into map-based or sequential edge containers
// For map-based containers (std::map<VId, edge_type>), wraps edge in a pair
// For sequential containers (vector, set, etc.), inserts edge directly
template <class EdgeContainer, class Edge>
constexpr void emplace_edge(EdgeContainer& edges, const typename Edge::vertex_id_type& target_id, Edge&& edge) {
  if constexpr (is_map_based_edge_container<EdgeContainer>) {
    // Map-based: key is target_id, value is edge
    edges.emplace(target_id, std::forward<Edge>(edge));
  } else {
    // Sequential/set-based: insert edge directly
    if constexpr (has_emplace<EdgeContainer>) {
      edges.emplace(std::forward<Edge>(edge));
    } else if constexpr (has_insert<EdgeContainer>) {
      edges.insert(std::forward<Edge>(edge));
    } else if constexpr (has_emplace_back<EdgeContainer>) {
      edges.emplace_back(std::forward<Edge>(edge));
    } else if constexpr (has_push_back<EdgeContainer>) {
      edges.push_back(std::forward<Edge>(edge));
    } else if constexpr (has_emplace_front<EdgeContainer>) {
      // For forward_list and similar containers that only support front insertion
      edges.emplace_front(std::forward<Edge>(edge));
    }
  }
}

// return a lambda to assign/insert an element in a container
// assignment applies to random_access containers that have elements pre-allocated
// insert     applies to other containers that can insert elements (e.g. map, unordered_map, ...)
template <class C, class K>
requires has_array_operator<C, K>
constexpr auto assign_or_insert(C& container) {
  if constexpr (random_access_range<C>) {
    static_assert(sized_range<C>, "random_access container is assumed to have size()");
    return [&container](const K& id, typename C::value_type&& value) {
      typename C::size_type k = static_cast<typename C::size_type>(id);
      assert(k < container.size());
      container[k] = move(value);
    };
  } else if constexpr (has_array_operator<C, K>) {
    return [&container](const K& id, typename C::value_type&& value) { container[id] = move(value); };
  }
}


namespace detail {
  //--------------------------------------------------------------------------------------
  // graph_value<> - wraps scaler, union & reference user values for graph, vertex & edge
  //
  template <class T>
  struct graph_value_wrapper {
    constexpr graph_value_wrapper()                            = default;
    graph_value_wrapper(const graph_value_wrapper&)            = default;
    graph_value_wrapper& operator=(const graph_value_wrapper&) = default;
    graph_value_wrapper(graph_value_wrapper&& v) : value(move(v.value)) {}
    graph_value_wrapper(const T& v) : value(v) {}
    graph_value_wrapper(T&& v) : value(move(v)) {}

    T value = T();
  };

  template <class T>
  struct graph_value_needs_wrap
        : std::integral_constant<bool,
                                 std::is_scalar<T>::value || std::is_array<T>::value || std::is_union<T>::value ||
                                       std::is_reference<T>::value> {};

  template <class T>
  constexpr auto user_value(T& v) -> T& {
    return v;
  }
  template <class T>
  constexpr auto user_value(const T& v) -> const T& {
    return v;
  }
} // namespace detail

//
// Common Property Values
//
struct empty_value {}; // empty graph|vertex|edge value

struct weight_value {
  int weight = 0;

  constexpr weight_value()                     = default;
  weight_value(const weight_value&)            = default;
  weight_value& operator=(const weight_value&) = default;
  weight_value(const int& w) : weight(w) {}
};

struct name_value {
  std::string name;

  name_value()                             = default;
  name_value(const name_value&)            = default;
  name_value& operator=(const name_value&) = default;
  name_value(const std::string& s) : name(s) {}
  name_value(std::string&& s) : name(std::move(s)) {}
};

} // namespace graph::container

#endif //CONTAINER_UTILITY_HPP
