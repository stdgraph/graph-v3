#pragma once

#include "graph/graph.hpp"
#include "graph/container/container_utility.hpp"

// Refactored from the graph-v2 CppCon 2022 example.
//
// An rr_adaptor wraps a random_access range-of-ranges (Outer) plus an
// optional parallel vertex-value range (VVR) and exposes the graph-v3 CPO
// interface using the descriptor-based pattern.
//
// Key v2→v3 changes:
//   • vertex_id(g, iterator) removed — vertex_descriptor::vertex_id() handles it
//   • edges/target_id/vertex_value/edge_value are now function templates whose
//     trailing return types act as SFINAE guards (same pattern as AdaptingThirdPartyGraph)
//   • find_vertex(g, uid) added — returns a vertex_descriptor_view iterator

// ─────────────────────────────────────────────────────────────────────────────
// to_tuple helpers (aggregate → tuple reflection, supports up to 4 fields)
// ─────────────────────────────────────────────────────────────────────────────

template <class T, class... TArgs>
decltype(void(T{std::declval<TArgs>()...}), std::true_type{}) test_is_braces_constructible(int);

template <class, class...>
std::false_type test_is_braces_constructible(...);

template <class T, class... TArgs>
using is_braces_constructible = decltype(test_is_braces_constructible<T, TArgs...>(0));

struct any_type {
  template <class T>
  constexpr operator T();
};

template <class T>
auto to_tuple(T&& object) noexcept {
  using type = std::decay_t<T>;
  if constexpr (is_braces_constructible<type, any_type, any_type, any_type, any_type>{}) {
    auto&& [p1, p2, p3, p4] = object;
    return std::make_tuple(p1, p2, p3, p4);
  } else if constexpr (is_braces_constructible<type, any_type, any_type, any_type>{}) {
    auto&& [p1, p2, p3] = object;
    return std::make_tuple(p1, p2, p3);
  } else if constexpr (is_braces_constructible<type, any_type, any_type>{}) {
    auto&& [p1, p2] = object;
    return std::make_tuple(p1, p2);
  } else if constexpr (is_braces_constructible<type, any_type>{}) {
    auto&& [p1] = object;
    return std::make_tuple(p1);
  } else {
    return std::make_tuple();
  }
}

template <class T>
using to_tuple_t = decltype(to_tuple(std::declval<T>()));

// ─────────────────────────────────────────────────────────────────────────────
// range_of_ranges concept
// ─────────────────────────────────────────────────────────────────────────────

template <class Outer>
concept range_of_ranges =
      std::ranges::random_access_range<Outer> &&
      std::ranges::forward_range<std::ranges::range_value_t<Outer>> &&
      std::integral<std::tuple_element_t<0, to_tuple_t<std::ranges::range_value_t<std::ranges::range_value_t<Outer>>>>>;

// ─────────────────────────────────────────────────────────────────────────────
// rr_adaptor — range-of-ranges + parallel vertex-value range
// ─────────────────────────────────────────────────────────────────────────────

template <range_of_ranges Outer, std::ranges::random_access_range VVR>
requires std::ranges::contiguous_range<Outer> && std::ranges::random_access_range<VVR>
class rr_adaptor {
public:
  using graph_type        = rr_adaptor<Outer, VVR>;
  using vertices_range    = Outer;
  using vertex_type       = std::ranges::range_value_t<vertices_range>;
  using edges_range       = std::ranges::range_value_t<vertices_range>;
  using edge_type         = std::ranges::range_value_t<edges_range>;
  using vertex_id_type    = std::remove_cv_t<std::tuple_element_t<0, to_tuple_t<edge_type>>>;
  using edge_value_type   = std::conditional_t<(std::tuple_size_v<to_tuple_t<edge_type>> <= 1),
                                               void,
                                               std::tuple_element_t<1, to_tuple_t<edge_type>>>;
  using vertex_value_type = std::ranges::range_value_t<VVR>;

  // ── Constructor ────────────────────────────────────────────────────────────

  template <std::ranges::forward_range ERng, class EProj = std::identity>
  rr_adaptor(VVR&         vertex_values,
             const ERng&  erng,
             const EProj& eproj     = EProj(),
             bool         dup_edges = false)
        : vertex_values_(vertex_values) {
    vertex_id_type max_vid = max_vertex_id(erng, eproj);

    size_t vcnt = std::max(static_cast<size_t>(max_vid + 1), vertex_values_.size());
    vertices_.resize(vcnt);
    vertex_values_.resize(vcnt);

    for (auto&& e : erng) {
      if constexpr (std::is_void_v<edge_value_type>) {
        push(vertices_[static_cast<size_t>(e.source_id)], e.target_id);
        if (dup_edges)
          push(vertices_[static_cast<size_t>(e.target_id)], e.source_id);
      } else {
        push(vertices_[static_cast<size_t>(e.source_id)], {e.target_id, e.value});
        if (dup_edges)
          push(vertices_[static_cast<size_t>(e.target_id)], {e.source_id, e.value});
      }
    }
  }

private:
  // ── Data members (declared first so friend trailing return types can see them)
  vertices_range vertices_;
  VVR&           vertex_values_;

  // ── Private helpers ───────────────────────────────────────────────────────
  template <std::ranges::forward_range ERng, class EProj>
  vertex_id_type max_vertex_id(const ERng& erng, const EProj& eproj) const {
    vertex_id_type max_vid = 0;
    for (auto&& e : erng)
      max_vid = std::max(max_vid, std::max(e.source_id, e.target_id));
    return max_vid;
  }

  void push(edges_range& edges, const edge_type& val) {
    if constexpr (graph::container::has_push_back<edges_range>)
      edges.push_back(val);
    else if constexpr (graph::container::has_push_front<edges_range>)
      edges.push_front(val);
  }

  // ── v3 graph CPO free functions ───────────────────────────────────────────

  // vertices(g) — return the raw outer container; v3 CPO wraps it in
  // vertex_descriptor_view automatically.
  friend constexpr vertices_range&       vertices(graph_type& g) { return g.vertices_; }
  friend constexpr const vertices_range& vertices(const graph_type& g) { return g.vertices_; }

  // find_vertex(g, uid) — construct a vertex_descriptor_view iterator
  // directly from the index.  For random-access backing the iterator stores
  // just a size_t, so it is safe to return from a temporary view.
  friend auto find_vertex(graph_type& g, vertex_id_type uid) {
    using Iter = std::ranges::iterator_t<vertices_range>;
    using VDV  = graph::adj_list::vertex_descriptor_view<Iter>;
    return typename VDV::iterator{static_cast<typename VDV::storage_type>(uid)};
  }
  friend auto find_vertex(const graph_type& g, vertex_id_type uid) {
    using Iter = std::ranges::iterator_t<const vertices_range>;
    using VDV  = graph::adj_list::vertex_descriptor_view<Iter>;
    return typename VDV::iterator{static_cast<typename VDV::storage_type>(uid)};
  }

  // edges(g, u) — u is a v3 vertex_descriptor; use u.vertex_id() to index.
  // Trailing return type acts as SFINAE guard.
  template <class VDesc>
  friend auto edges(graph_type& g, const VDesc& u) {
    return std::views::all(g.vertices_[u.vertex_id()]);
  }
  template <class VDesc>
  friend auto edges(const graph_type& g, const VDesc& u) {
    return std::views::all(g.vertices_[u.vertex_id()]);
  }

  // edges(g, uid) — direct UID overload kept for compatibility.
  friend constexpr edges_range&       edges(graph_type& g, const vertex_id_type uid) { return g.vertices_[uid]; }
  friend constexpr const edges_range& edges(const graph_type& g, const vertex_id_type uid) { return g.vertices_[uid]; }

  // target_id(g, uv) — uv is an edge_descriptor; *uv.value() is the raw edge.
  template <class EDesc>
  friend auto target_id(const graph_type& g, const EDesc& uv) noexcept
        -> decltype(static_cast<vertex_id_type>(std::get<0>(to_tuple(*uv.value())))) {
    return static_cast<vertex_id_type>(std::get<0>(to_tuple(*uv.value())));
  }

  // vertex_value(g, u) — u is a v3 vertex_descriptor.
  template <class VDesc>
  friend decltype(auto) vertex_value(graph_type& g, const VDesc& u) {
    return g.vertex_values_[u.vertex_id()];
  }
  template <class VDesc>
  friend decltype(auto) vertex_value(const graph_type& g, const VDesc& u) {
    return g.vertex_values_[u.vertex_id()];
  }

  // edge_value(g, uv) — uv is an edge_descriptor; *uv.value() is the raw edge.
  template <class EDesc>
  friend auto edge_value(graph_type& g, const EDesc& uv)
        -> decltype(std::get<1>(to_tuple(*uv.value()))) {
    return std::get<1>(to_tuple(*uv.value()));
  }
  template <class EDesc>
  friend auto edge_value(const graph_type& g, const EDesc& uv)
        -> decltype(std::get<1>(to_tuple(*uv.value()))) {
    return std::get<1>(to_tuple(*uv.value()));
  }
};

// ─────────────────────────────────────────────────────────────────────────────
// rr_adaptor trait helpers
// ─────────────────────────────────────────────────────────────────────────────

template <class E>
struct rr_has_edge_value : public std::integral_constant<bool, (std::tuple_size_v<to_tuple_t<E>> > 1)> {};
template <class E>
inline constexpr bool rr_has_edge_value_v = rr_has_edge_value<E>::value;

template <class E>
struct rr_vertex_id_type {
  using type = std::tuple_element_t<0, to_tuple_t<E>>;
};

template <class E>
struct rr_edge_value_type {
  using type = std::conditional<rr_has_edge_value_v<E>, std::tuple_element_t<1, to_tuple_t<E>>, void>;
};
template <class V>
using rr_edge_value_type_t = typename rr_edge_value_type<V>::type;

// ─────────────────────────────────────────────────────────────────────────────
// rr_adaptor (single-template-parameter variant, no separate vertex values)
// ─────────────────────────────────────────────────────────────────────────────

template <range_of_ranges Outer>
requires std::ranges::contiguous_range<Outer>
class rr_adaptor<Outer, Outer> {
public:
  using graph_type      = rr_adaptor<Outer, Outer>;
  using vertices_range  = Outer;
  using vertex_type     = std::ranges::range_value_t<vertices_range>;
  using edges_range     = std::ranges::range_value_t<vertices_range>;
  using edge_type       = std::ranges::range_value_t<edges_range>;
  using vertex_id_type  = std::remove_cv_t<std::tuple_element_t<0, to_tuple_t<edge_type>>>;
  using edge_value_type = std::conditional_t<(std::tuple_size_v<to_tuple_t<edge_type>> <= 1),
                                             void,
                                             std::tuple_element_t<1, to_tuple_t<edge_type>>>;

  template <std::ranges::forward_range ERng, class EProj = std::identity>
  rr_adaptor(const ERng& erng, const EProj& eproj = EProj(), bool dup_edges = false) {
    vertex_id_type max_vid = 0;
    for (auto&& e : erng)
      max_vid = std::max(max_vid, std::max(e.source_id, e.target_id));
    vertices_.resize(static_cast<size_t>(max_vid + 1));
    for (auto&& e : erng) {
      if constexpr (std::is_void_v<edge_value_type>) {
        push(vertices_[static_cast<size_t>(e.source_id)], e.target_id);
        if (dup_edges)
          push(vertices_[static_cast<size_t>(e.target_id)], e.source_id);
      } else {
        push(vertices_[static_cast<size_t>(e.source_id)], {e.target_id, e.value});
        if (dup_edges)
          push(vertices_[static_cast<size_t>(e.target_id)], {e.source_id, e.value});
      }
    }
  }

private:
  vertices_range vertices_;

  void push(edges_range& edges, const edge_type& val) {
    if constexpr (graph::container::has_push_back<edges_range>)
      edges.push_back(val);
    else if constexpr (graph::container::has_push_front<edges_range>)
      edges.push_front(val);
  }

  friend constexpr vertices_range&       vertices(graph_type& g) { return g.vertices_; }
  friend constexpr const vertices_range& vertices(const graph_type& g) { return g.vertices_; }

  friend auto find_vertex(graph_type& g, vertex_id_type uid) {
    using Iter = std::ranges::iterator_t<vertices_range>;
    using VDV  = graph::adj_list::vertex_descriptor_view<Iter>;
    return typename VDV::iterator{static_cast<typename VDV::storage_type>(uid)};
  }
  friend auto find_vertex(const graph_type& g, vertex_id_type uid) {
    using Iter = std::ranges::iterator_t<const vertices_range>;
    using VDV  = graph::adj_list::vertex_descriptor_view<Iter>;
    return typename VDV::iterator{static_cast<typename VDV::storage_type>(uid)};
  }

  template <class VDesc>
  friend auto edges(graph_type& g, const VDesc& u) {
    return std::views::all(g.vertices_[u.vertex_id()]);
  }
  template <class VDesc>
    friend auto edges(const graph_type& g, const VDesc& u) {
    return std::views::all(g.vertices_[u.vertex_id()]);
  }

  friend constexpr edges_range&       edges(graph_type& g, vertex_id_type uid) { return g.vertices_[uid]; }
  friend constexpr const edges_range& edges(const graph_type& g, vertex_id_type uid) { return g.vertices_[uid]; }

  template <class EDesc>
  friend auto target_id(const graph_type& g, const EDesc& uv) noexcept
        -> decltype(static_cast<vertex_id_type>(std::get<0>(to_tuple(*uv.value())))) {
    return static_cast<vertex_id_type>(std::get<0>(to_tuple(*uv.value())));
  }

  template <class EDesc>
  friend auto edge_value(graph_type& g, const EDesc& uv)
        -> decltype(std::get<1>(to_tuple(*uv.value()))) {
    return std::get<1>(to_tuple(*uv.value()));
  }
  template <class EDesc>
  friend auto edge_value(const graph_type& g, const EDesc& uv)
        -> decltype(std::get<1>(to_tuple(*uv.value()))) {
    return std::get<1>(to_tuple(*uv.value()));
  }

};
