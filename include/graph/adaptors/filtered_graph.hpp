#pragma once

#include <graph/graph.hpp>
#include <concepts>
#include <functional>
#include <optional>
#include <ranges>
#include <type_traits>

// ── ADL helpers ─────────────────────────────────────────────────────────────
// Must live outside namespace graph to avoid CPO object ADL suppression.
// These call the underlying graph's CPO-level functions via ADL.
//
// NOTE: Unlike graph_bgl_adl (which bypasses CPOs to get raw results), these
// call through the graph-v3 CPOs because we cannot generically access raw
// ADL functions for all graph types.  The filtered_graph returns the CPO's
// edge_descriptor_view from its edges() function; the out_edges CPO detects
// the is_edge_descriptor_view_v and passes it through without re-wrapping.

namespace filtered_graph_adl {

template <typename G>
auto call_vertices(G& g) -> decltype(graph::vertices(g)) {
  return graph::vertices(g);
}
template <typename G>
auto call_vertices(const G& g) -> decltype(graph::vertices(g)) {
  return graph::vertices(g);
}

template <typename G>
auto call_num_vertices(const G& g) -> decltype(graph::num_vertices(g)) {
  return graph::num_vertices(g);
}

template <typename G, typename U>
auto call_out_edges(G& g, const U& u) -> decltype(graph::out_edges(g, u)) {
  return graph::out_edges(g, u);
}
template <typename G, typename U>
auto call_out_edges(const G& g, const U& u) -> decltype(graph::out_edges(g, u)) {
  return graph::out_edges(g, u);
}

template <typename G, typename UV>
auto call_target_id(const G& g, const UV& uv) -> decltype(graph::target_id(g, uv)) {
  return graph::target_id(g, uv);
}

template <typename G, typename UV>
auto call_source_id(const G& g, const UV& uv) -> decltype(graph::source_id(g, uv)) {
  return graph::source_id(g, uv);
}

template <typename G, typename VId>
auto call_find_vertex(G& g, const VId& uid) -> decltype(graph::find_vertex(g, uid)) {
  return graph::find_vertex(g, uid);
}
template <typename G, typename VId>
auto call_find_vertex(const G& g, const VId& uid) -> decltype(graph::find_vertex(g, uid)) {
  return graph::find_vertex(g, uid);
}

} // namespace filtered_graph_adl


namespace graph::adaptors {

// ── Predicate that accepts everything ───────────────────────────────────────

struct keep_all {
  template <typename... Args>
  constexpr bool operator()(Args&&...) const noexcept {
    return true;
  }
};

// ── Self-contained filtering iterator ───────────────────────────────────────
// Unlike std::views::filter iterators, these don't reference a parent view.
// They store the predicate and end sentinel by value, so they remain valid
// after the function that created them returns.

template <typename BaseIter, typename Pred>
class filtering_iterator {
  BaseIter current_{};
  BaseIter end_{};
  std::optional<Pred> pred_{};

  void advance_to_valid() {
    while (current_ != end_ && !(*pred_)(*current_))
      ++current_;
  }

public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = std::iter_value_t<BaseIter>;
  using difference_type = std::iter_difference_t<BaseIter>;
  using reference = std::iter_reference_t<BaseIter>;

  filtering_iterator() = default;
  filtering_iterator(BaseIter begin, BaseIter end, Pred pred)
      : current_(begin), end_(end), pred_(std::move(pred)) {
    advance_to_valid();
  }

  // Custom copy/move assignment — lambdas aren't assignable, so we
  // destroy-and-reconstruct the optional.
  filtering_iterator(const filtering_iterator&) = default;
  filtering_iterator(filtering_iterator&&) = default;

  filtering_iterator& operator=(const filtering_iterator& other) {
    if (this != &other) {
      current_ = other.current_;
      end_ = other.end_;
      pred_.reset();
      if (other.pred_) pred_.emplace(*other.pred_);
    }
    return *this;
  }
  filtering_iterator& operator=(filtering_iterator&& other) noexcept {
    current_ = std::move(other.current_);
    end_ = std::move(other.end_);
    pred_.reset();
    if (other.pred_) pred_.emplace(std::move(*other.pred_));
    return *this;
  }

  reference operator*() const { return *current_; }

  filtering_iterator& operator++() {
    ++current_;
    advance_to_valid();
    return *this;
  }
  filtering_iterator operator++(int) {
    auto tmp = *this;
    ++*this;
    return tmp;
  }

  bool operator==(const filtering_iterator& other) const { return current_ == other.current_; }
};

// ── filtered_graph ──────────────────────────────────────────────────────────
//
// Non-owning adaptor that wraps any graph-v3 graph and filters edges during
// traversal. Delegates begin()/end()/size() to the underlying graph so that
// it satisfies forward_range and the CPO's _has_inner_value_pattern path
// handles vertex iteration automatically.
//
// VertexPredicate: (vertex_id_t<G>) → bool
//   Controls which vertices are "included". Edges to/from excluded vertices
//   are skipped during out_edges iteration.
//
// EdgePredicate: (vertex_id_t<G> source, vertex_id_t<G> target) → bool
//   Controls which edges are included, given their endpoint IDs.

template <typename G, typename VertexPredicate = keep_all, typename EdgePredicate = keep_all>
class filtered_graph {
  G*              g_;
  [[no_unique_address]] VertexPredicate vpred_;
  [[no_unique_address]] EdgePredicate   epred_;

public:
  using graph_type = G;
  using value_type = std::ranges::range_value_t<G>;
  using iterator = std::ranges::iterator_t<G>;
  using const_iterator = std::ranges::iterator_t<const G>;

  explicit filtered_graph(G& g, VertexPredicate vp = {}, EdgePredicate ep = {})
        : g_(&g), vpred_(std::move(vp)), epred_(std::move(ep)) {}

  G&                      graph() noexcept { return *g_; }
  const G&                graph() const noexcept { return *g_; }
  const VertexPredicate&  vertex_pred() const noexcept { return vpred_; }
  const EdgePredicate&    edge_pred() const noexcept { return epred_; }

  // Forward range delegation — makes filtered_graph satisfy forward_range
  // and _has_inner_value_pattern for the vertices CPO.
  auto begin() { return g_->begin(); }
  auto begin() const { return g_->begin(); }
  auto end() { return g_->end(); }
  auto end() const { return g_->end(); }
  auto size() const { return g_->size(); }

  // Random access — needed by vertex_descriptor::inner_value(container)
  auto& operator[](std::size_t idx) { return (*g_)[idx]; }
  const auto& operator[](std::size_t idx) const { return (*g_)[idx]; }
};

// CTAD
template <typename G, typename VP, typename EP>
filtered_graph(G&, VP, EP) -> filtered_graph<G, VP, EP>;

template <typename G, typename VP>
filtered_graph(G&, VP) -> filtered_graph<G, VP>;

template <typename G>
filtered_graph(G&) -> filtered_graph<G>;

// ── ADL free functions (found by graph-v3 CPOs) ────────────────────────────

/// edges(fg, u) — ADL function for out_edges CPO.
/// Returns a filtered range of the raw edge storage using self-contained
/// filtering_iterator (no dangling reference issues).
template <typename G, typename VP, typename EP, typename U>
auto edges(filtered_graph<G, VP, EP>& fg, const U& u) {
  auto uid = adj_list::vertex_id(fg.graph(), u);
  auto& raw_edges = u.inner_value(fg.graph());
  using base_iter_t = std::ranges::iterator_t<std::remove_cvref_t<decltype(raw_edges)>>;
  using vid_t = adj_list::vertex_id_t<G>;

  auto pred = [vpred = fg.vertex_pred(), epred = fg.edge_pred(), uid](const auto& edge_val) {
    auto tid = static_cast<vid_t>(std::get<0>(edge_val));
    return vpred(tid) && epred(uid, tid);
  };
  using pred_t = decltype(pred);
  using filter_iter = filtering_iterator<base_iter_t, pred_t>;

  return std::ranges::subrange<filter_iter>(
    filter_iter(raw_edges.begin(), raw_edges.end(), pred),
    filter_iter(raw_edges.end(), raw_edges.end(), pred));
}

template <typename G, typename VP, typename EP, typename U>
auto edges(const filtered_graph<G, VP, EP>& fg, const U& u) {
  auto uid = adj_list::vertex_id(fg.graph(), u);
  auto& raw_edges = u.inner_value(fg.graph());
  using base_iter_t = std::ranges::iterator_t<std::remove_cvref_t<decltype(raw_edges)>>;
  using vid_t = adj_list::vertex_id_t<G>;

  auto pred = [vpred = fg.vertex_pred(), epred = fg.edge_pred(), uid](const auto& edge_val) {
    auto tid = static_cast<vid_t>(std::get<0>(edge_val));
    return vpred(tid) && epred(uid, tid);
  };
  using pred_t = decltype(pred);
  using filter_iter = filtering_iterator<base_iter_t, pred_t>;

  return std::ranges::subrange<filter_iter>(
    filter_iter(raw_edges.begin(), raw_edges.end(), pred),
    filter_iter(raw_edges.end(), raw_edges.end(), pred));
}

// ── Convenience: filtered vertex iteration ──────────────────────────────────

template <typename G, typename VP, typename EP>
auto filtered_vertices(const filtered_graph<G, VP, EP>& fg) {
  return filtered_graph_adl::call_vertices(fg.graph())
    | std::views::filter([&fg](const auto& u) {
        return fg.vertex_pred()(vertex_id(fg.graph(), u));
      });
}

} // namespace graph::adaptors
