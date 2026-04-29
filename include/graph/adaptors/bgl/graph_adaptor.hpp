#pragma once

#include <graph/adaptors/bgl/bgl_edge_iterator.hpp>

#include <boost/graph/graph_traits.hpp>

#include <concepts>
#include <ranges>

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

// ── graph_adaptor ───────────────────────────────────────────────────────────

/// Non-owning wrapper that adapts a BGL graph for use with graph-v3 CPOs.
/// The adapted graph satisfies `adjacency_list` and `index_adjacency_list`
/// once the appropriate BGL graph header is included by the caller.
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

/// vertices CPO (Tier 2: ADL) — returns iota range, CPO auto-wraps in vertex_descriptor_view.
template <typename G>
auto vertices(const graph_adaptor<G>& ga) {
  using vid_t = typename boost::graph_traits<G>::vertex_descriptor;
  auto n = graph_bgl_adl::call_num_vertices(ga.bgl_graph());
  return std::views::iota(vid_t{0}, static_cast<vid_t>(n));
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
  const auto& bgl_edge = *uv.value();
  return graph_bgl_adl::call_target(bgl_edge, ga.bgl_graph());
}

/// source_id CPO (Tier 3: ADL) — extracts BGL edge from descriptor, calls BGL source.
template <typename G, typename UV>
auto source_id(const graph_adaptor<G>& ga, const UV& uv) {
  const auto& bgl_edge = *uv.value();
  return graph_bgl_adl::call_source(bgl_edge, ga.bgl_graph());
}

} // namespace graph::bgl
