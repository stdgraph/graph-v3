#pragma once

#include <graph/adaptors/bgl/bgl_edge_iterator.hpp>

#include <boost/graph/graph_traits.hpp>

#include <concepts>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

// ── Keyed vertex iterator adaptor ───────────────────────────────────────────
// For non-vecS BGL graphs, vertex_descriptor is void*. graph-v3's descriptor
// framework requires either random_access_iterator (direct_vertex_type) or
// forward_iterator with pair-like value_type (keyed_vertex_type).
// This adaptor wraps a BGL vertex_iterator to yield std::pair<VD, std::monostate>,
// satisfying keyed_vertex_type so vertex_descriptor_view can be constructed.

namespace graph::bgl::detail {

/// Value type for the keyed adaptor: pair<vertex_descriptor, monostate>.
/// get<0> is the vertex ID (void*); get<1> is unused padding.
template <typename VD>
using keyed_vertex_value = std::pair<VD, std::monostate>;

/// Forward iterator adaptor that wraps a BGL vertex_iterator and yields
/// keyed_vertex_value on dereference.
template <typename BGLVertexIter>
class bgl_keyed_vertex_iterator {
public:
  using bgl_vd_type       = typename std::iterator_traits<BGLVertexIter>::value_type;
  using value_type        = keyed_vertex_value<bgl_vd_type>;
  using difference_type   = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;
  using pointer           = const value_type*;
  using reference         = value_type;

  constexpr bgl_keyed_vertex_iterator() noexcept = default;
  constexpr explicit bgl_keyed_vertex_iterator(BGLVertexIter it) noexcept : it_(it) {}

  [[nodiscard]] value_type operator*() const { return value_type{*it_, std::monostate{}}; }

  bgl_keyed_vertex_iterator& operator++() {
    ++it_;
    return *this;
  }
  bgl_keyed_vertex_iterator operator++(int) {
    auto tmp = *this;
    ++it_;
    return tmp;
  }

  [[nodiscard]] bool operator==(const bgl_keyed_vertex_iterator& other) const { return it_ == other.it_; }

private:
  BGLVertexIter it_{};
};

/// Sized subrange alias for bgl_keyed_vertex_iterator.
/// The sized subrange satisfies std::ranges::sized_range, which
/// vertex_descriptor_view requires for non-random-access iterators.
template <typename BGLVertexIter>
using bgl_vertex_range = std::ranges::subrange<bgl_keyed_vertex_iterator<BGLVertexIter>,
                                               bgl_keyed_vertex_iterator<BGLVertexIter>,
                                               std::ranges::subrange_kind::sized>;

} // namespace graph::bgl::detail

// ── ADL call helpers ────────────────────────────────────────────────────────
// These MUST live outside namespace graph. graph-v3 defines CPO objects named
// out_edges, source, target, etc. in namespace graph. CPO objects are
// non-function entities; when ordinary unqualified lookup finds a non-function,
// ADL is suppressed (C++ [basic.lookup.argdep]/3). By placing these thin
// forwarders in a namespace that is not nested inside graph, ordinary lookup
// finds nothing, and ADL discovers the BGL functions in namespace boost at
// instantiation time.

namespace graph_bgl_adl {

template <typename G>
auto call_num_vertices(const G& g) -> decltype(num_vertices(g)) {
  return num_vertices(g);
}

template <typename G>
auto call_vertices(const G& g) -> decltype(vertices(g)) {
  return vertices(g);
}

template <typename V, typename G>
auto call_out_edges(V v, G& g) -> decltype(out_edges(v, g)) {
  return out_edges(v, g);
}

template <typename V, typename G>
auto call_out_edges(V v, const G& g) -> decltype(out_edges(v, g)) {
  return out_edges(v, g);
}

template <typename V, typename G>
auto call_in_edges(V v, G& g) -> decltype(in_edges(v, g)) {
  return in_edges(v, g);
}

template <typename V, typename G>
auto call_in_edges(V v, const G& g) -> decltype(in_edges(v, g)) {
  return in_edges(v, g);
}

template <typename E, typename G>
auto call_target(const E& e, const G& g) -> decltype(target(e, g)) {
  return target(e, g);
}

template <typename E, typename G>
auto call_source(const E& e, const G& g) -> decltype(source(e, g)) {
  return source(e, g);
}

} // namespace graph_bgl_adl

namespace graph::bgl {

// ── Trait: is vertex_descriptor integral (vecS) or opaque (listS/setS)? ─────

template <typename G>
inline constexpr bool has_integral_vertex_id_v =
    std::integral<typename boost::graph_traits<G>::vertex_descriptor>;

// ── graph_adaptor ───────────────────────────────────────────────────────────

/// Non-owning wrapper that adapts a BGL graph for use with graph-v3 CPOs.
///
/// For BGL graphs with integral vertex descriptors (vecS): satisfies
/// `adjacency_list` and `index_adjacency_list`.
///
/// For BGL graphs with opaque vertex descriptors (listS, setS): satisfies
/// `adjacency_list` only (vertex IDs are non-integral).
template <typename BGL_Graph>
class graph_adaptor {
  BGL_Graph* g_;

public:
  using bgl_graph_type     = BGL_Graph;
  using vertex_id_type     = typename boost::graph_traits<BGL_Graph>::vertex_descriptor;
  using bgl_edge_type      = typename boost::graph_traits<BGL_Graph>::edge_descriptor;
  using out_edge_iter_type = typename boost::graph_traits<BGL_Graph>::out_edge_iterator;

  explicit graph_adaptor(BGL_Graph& g) noexcept : g_(&g) {}

  BGL_Graph&       bgl_graph() noexcept { return *g_; }
  const BGL_Graph& bgl_graph() const noexcept { return *g_; }
};

// CTAD
template <typename G>
graph_adaptor(G&) -> graph_adaptor<G>;

// ── ADL free functions (found by graph-v3 CPOs) ────────────────────────────

/// vertices CPO (Tier 2: ADL).
/// For vecS: returns iota range (integral IDs).
/// For listS/setS: returns a bgl_vertex_range (keyed pair adaptor) that satisfies
/// the vertex_iterator concept required by vertex_descriptor_view.
template <typename G>
auto vertices(const graph_adaptor<G>& ga) {
  if constexpr (has_integral_vertex_id_v<G>) {
    using vid_t = typename boost::graph_traits<G>::vertex_descriptor;
    auto n = graph_bgl_adl::call_num_vertices(ga.bgl_graph());
    return std::views::iota(vid_t{0}, static_cast<vid_t>(n));
  } else {
    using bgl_viter = typename boost::graph_traits<G>::vertex_iterator;
    using keyed_iter = detail::bgl_keyed_vertex_iterator<bgl_viter>;
    auto [first, last] = graph_bgl_adl::call_vertices(ga.bgl_graph());
    auto n = graph_bgl_adl::call_num_vertices(ga.bgl_graph());
    return detail::bgl_vertex_range<bgl_viter>(keyed_iter(first), keyed_iter(last),
                                               static_cast<std::size_t>(n));
  }
}

/// num_vertices CPO (Tier 2: ADL).
template <typename G>
auto num_vertices(const graph_adaptor<G>& ga) {
  return graph_bgl_adl::call_num_vertices(ga.bgl_graph());
}

/// out_edges CPO (Tier 2: ADL `edges(g, u)`) — wraps BGL iterators, CPO auto-wraps in edge_descriptor_view.
template <typename G, typename U>
auto edges(graph_adaptor<G>& ga, const U& u) {
  auto [first, last] = graph_bgl_adl::call_out_edges(u.vertex_id(), ga.bgl_graph());
  using iter_t = bgl_edge_iterator<typename boost::graph_traits<G>::out_edge_iterator>;
  return std::ranges::subrange(iter_t(first), iter_t(last));
}

template <typename G, typename U>
auto edges(const graph_adaptor<G>& ga, const U& u) {
  auto [first, last] = graph_bgl_adl::call_out_edges(u.vertex_id(), ga.bgl_graph());
  using iter_t = bgl_edge_iterator<typename boost::graph_traits<G>::out_edge_iterator>;
  return std::ranges::subrange(iter_t(first), iter_t(last));
}

/// in_edges CPO (Tier 2: ADL `in_edges(g, u)`) — enabled for bidirectional and undirected BGL graphs.
/// For bidirectional: wraps BGL in_edge_iterator.
/// For undirected: BGL in_edges() is the same as out_edges(), using in_edge_iterator.
template <typename G, typename U>
auto in_edges(graph_adaptor<G>& ga, const U& u)
  requires std::is_convertible_v<typename boost::graph_traits<G>::traversal_category,
                                 boost::bidirectional_graph_tag>
{
  auto [first, last] = graph_bgl_adl::call_in_edges(u.vertex_id(), ga.bgl_graph());
  using iter_t = bgl_edge_iterator<typename boost::graph_traits<G>::in_edge_iterator>;
  return std::ranges::subrange(iter_t(first), iter_t(last));
}

template <typename G, typename U>
auto in_edges(const graph_adaptor<G>& ga, const U& u)
  requires std::is_convertible_v<typename boost::graph_traits<G>::traversal_category,
                                 boost::bidirectional_graph_tag>
{
  auto [first, last] = graph_bgl_adl::call_in_edges(u.vertex_id(), ga.bgl_graph());
  using iter_t = bgl_edge_iterator<typename boost::graph_traits<G>::in_edge_iterator>;
  return std::ranges::subrange(iter_t(first), iter_t(last));
}

/// target_id CPO (Tier 2: ADL) — extracts BGL edge from descriptor, calls BGL target.
template <typename G, typename UV>
auto target_id(const graph_adaptor<G>& ga, const UV& uv) {
  auto bgl_edge = *uv.value();  // copy — uv.value() may be a temporary iterator
  return graph_bgl_adl::call_target(bgl_edge, ga.bgl_graph());
}

/// source_id CPO (Tier 3: ADL) — extracts BGL edge from descriptor, calls BGL source.
template <typename G, typename UV>
auto source_id(const graph_adaptor<G>& ga, const UV& uv) {
  auto bgl_edge = *uv.value();  // copy — uv.value() may be a temporary iterator
  return graph_bgl_adl::call_source(bgl_edge, ga.bgl_graph());
}

} // namespace graph::bgl
