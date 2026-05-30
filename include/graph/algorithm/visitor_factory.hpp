/**
 * @file visitor_factory.hpp
 * @brief Composable visitor utilities for graph traversal algorithms.
 *
 * This header provides a small, additive toolkit for building traversal visitors out of
 * reusable pieces. It is the graph-v3 analogue of Boost.Graph's event-visitor / event-tag
 * combinators (e.g. @c make_bfs_visitor, @c predecessor_recorder), expressed with the
 * library's duck-typed @c on_* callback convention.
 *
 * Three layers are provided:
 *
 *  1. **Single-event adaptors** (`on_discover_vertex(f)`, `on_tree_edge(f)`, ...): wrap a
 *     callable @c f so it is invoked for exactly one traversal event. These mirror BGL's
 *     event tags.
 *
 *  2. **`composite_visitor` / `make_visitor(...)`**: fan a single traversal out to several
 *     sub-visitors. Each event is forwarded to every child that implements it (by descriptor
 *     or by vertex id, whichever the child accepts). A composite only exposes an @c on_X
 *     method when at least one child handles event @c X, so it interoperates with the
 *     `has_on_*` detection used by the algorithms and with the `valid_visitor` strict check.
 *
 *  3. **Prebuilt recorders** (`predecessor_recorder`, `distance_recorder`, `time_stamper`):
 *     ready-made callables for the most common bookkeeping tasks. They return plain
 *     `(g, x) -> void` callables so the caller chooses the event to bind them to, e.g.
 *     `on_tree_edge(predecessor_recorder(pred))` for BFS/DFS or
 *     `on_edge_relaxed(predecessor_recorder(pred))` for Dijkstra/Bellman-Ford.
 *
 * Nothing here modifies the traversal algorithms; everything is built on the existing
 * visitor concepts in @ref traversal_common.hpp.
 *
 * Used with: breadth_first_search, depth_first_search, dijkstra_shortest_paths,
 *            bellman_ford_shortest_paths.
 */

#pragma once

#include <tuple>
#include <type_traits>
#include <utility>
#include <graph/graph.hpp>
#include <graph/algorithm/traversal_common.hpp>

#ifndef GRAPH_VISITOR_FACTORY_HPP
#  define GRAPH_VISITOR_FACTORY_HPP

namespace graph {

//
// Layer 1: single-event adaptors
//
// Each adaptor wraps a callable and exposes exactly one on_* method. The method is a template
// so the same adaptor works whether the algorithm passes a vertex descriptor or a vertex id
// (for vertex events) and regardless of the edge type (for edge events). The wrapped callable
// is invoked as f(g, x).
//
// GRAPH_VISITOR_EVENT_ADAPTOR(on_discover_vertex) generates:
//   - struct on_discover_vertex_fn<F>   — the adaptor type
//   - on_discover_vertex(F&&) -> on_discover_vertex_fn<decay_t<F>>  — the factory function

#  define GRAPH_VISITOR_EVENT_ADAPTOR(EVENT)                                                        \
    template <class F>                                                                              \
    struct EVENT##_fn {                                                                             \
      F f;                                                                                          \
      template <class G, class X>                                                                   \
      void EVENT(const G& g, const X& x) {                                                          \
        f(g, x);                                                                                    \
      }                                                                                             \
    };                                                                                              \
    template <class F>                                                                              \
    [[nodiscard]] EVENT##_fn<std::decay_t<F>> EVENT(F&& f) {                                        \
      return {std::forward<F>(f)};                                                                  \
    }

// Vertex events
GRAPH_VISITOR_EVENT_ADAPTOR(on_initialize_vertex)
GRAPH_VISITOR_EVENT_ADAPTOR(on_discover_vertex)
GRAPH_VISITOR_EVENT_ADAPTOR(on_examine_vertex)
GRAPH_VISITOR_EVENT_ADAPTOR(on_finish_vertex)
GRAPH_VISITOR_EVENT_ADAPTOR(on_start_vertex)

// Edge events
GRAPH_VISITOR_EVENT_ADAPTOR(on_examine_edge)
GRAPH_VISITOR_EVENT_ADAPTOR(on_edge_relaxed)
GRAPH_VISITOR_EVENT_ADAPTOR(on_edge_not_relaxed)
GRAPH_VISITOR_EVENT_ADAPTOR(on_edge_minimized)
GRAPH_VISITOR_EVENT_ADAPTOR(on_edge_not_minimized)
GRAPH_VISITOR_EVENT_ADAPTOR(on_tree_edge)
GRAPH_VISITOR_EVENT_ADAPTOR(on_back_edge)
GRAPH_VISITOR_EVENT_ADAPTOR(on_forward_or_cross_edge)
GRAPH_VISITOR_EVENT_ADAPTOR(on_finish_edge)

#  undef GRAPH_VISITOR_EVENT_ADAPTOR

//
// Layer 2: composite_visitor
//
// Holds a tuple of sub-visitors and fans every event out to each child that implements it.
//
// For a vertex event the composite receives whatever the algorithm passes (a descriptor,
// because the descriptor-form concept is checked first by the algorithms). Each child is
// then called with the form it supports: the descriptor directly, or vertex_id(g, x) for a
// child that only provides the *_id overload.
//
// Each event method is constrained by a fold expression over the child pack so that the
// method exists only when at least one child handles that event. This keeps has_on_* /
// valid_visitor detection accurate and preserves the algorithms' zero-overhead skipping of
// events that no child cares about.

template <class... Vs>
class composite_visitor {
  std::tuple<Vs...> visitors_;

public:
  explicit composite_visitor(Vs... vs) : visitors_(std::move(vs)...) {}

#  define GRAPH_COMPOSITE_VERTEX_EVENT(EVENT)                                                       \
    template <class G, class X>                                                                     \
      requires((has_##EVENT<G, Vs> || has_##EVENT##_id<G, Vs>) || ...)                              \
    void EVENT(const G& g, const X& x) {                                                            \
      std::apply(                                                                                   \
            [&](auto&... child) {                                                                   \
              auto dispatch = [&](auto& c) {                                                        \
                using C = std::remove_reference_t<decltype(c)>;                                     \
                if constexpr (has_##EVENT<G, C>)                                                    \
                  c.EVENT(g, x);                                                                    \
                else if constexpr (has_##EVENT##_id<G, C>)                                          \
                  c.EVENT(g, vertex_id(g, x));                                                      \
              };                                                                                    \
              (dispatch(child), ...);                                                               \
            },                                                                                      \
            visitors_);                                                                             \
    }

#  define GRAPH_COMPOSITE_EDGE_EVENT(EVENT)                                                         \
    template <class G, class E>                                                                     \
      requires(has_##EVENT<G, Vs> || ...)                                                           \
    void EVENT(const G& g, const E& e) {                                                            \
      std::apply(                                                                                   \
            [&](auto&... child) {                                                                   \
              auto dispatch = [&](auto& c) {                                                        \
                using C = std::remove_reference_t<decltype(c)>;                                     \
                if constexpr (has_##EVENT<G, C>)                                                    \
                  c.EVENT(g, e);                                                                    \
              };                                                                                    \
              (dispatch(child), ...);                                                               \
            },                                                                                      \
            visitors_);                                                                             \
    }

  GRAPH_COMPOSITE_VERTEX_EVENT(on_initialize_vertex)
  GRAPH_COMPOSITE_VERTEX_EVENT(on_discover_vertex)
  GRAPH_COMPOSITE_VERTEX_EVENT(on_examine_vertex)
  GRAPH_COMPOSITE_VERTEX_EVENT(on_finish_vertex)
  GRAPH_COMPOSITE_VERTEX_EVENT(on_start_vertex)

  GRAPH_COMPOSITE_EDGE_EVENT(on_examine_edge)
  GRAPH_COMPOSITE_EDGE_EVENT(on_edge_relaxed)
  GRAPH_COMPOSITE_EDGE_EVENT(on_edge_not_relaxed)
  GRAPH_COMPOSITE_EDGE_EVENT(on_edge_minimized)
  GRAPH_COMPOSITE_EDGE_EVENT(on_edge_not_minimized)
  GRAPH_COMPOSITE_EDGE_EVENT(on_tree_edge)
  GRAPH_COMPOSITE_EDGE_EVENT(on_back_edge)
  GRAPH_COMPOSITE_EDGE_EVENT(on_forward_or_cross_edge)
  GRAPH_COMPOSITE_EDGE_EVENT(on_finish_edge)

#  undef GRAPH_COMPOSITE_VERTEX_EVENT
#  undef GRAPH_COMPOSITE_EDGE_EVENT
};

/// Combine several sub-visitors into one. Each traversal event is forwarded to every
/// sub-visitor that implements it. Sub-visitors are stored by value (decayed); wrap a
/// stateful visitor you want to observe afterwards with std::ref, or read it back from the
/// returned composite.
template <class... Vs>
[[nodiscard]] composite_visitor<std::decay_t<Vs>...> make_visitor(Vs&&... vs) {
  return composite_visitor<std::decay_t<Vs>...>(std::forward<Vs>(vs)...);
}

//
// Layer 3: prebuilt recorders
//
// Each factory returns a plain (g, x) -> void callable. Bind it to an event with a
// single-event adaptor, e.g. on_tree_edge(predecessor_recorder(pred)).
//

/// Record the predecessor (parent) of each edge target: pred[target_id(g, uv)] = source_id(g, uv).
/// Bind to on_tree_edge (BFS/DFS) or on_edge_relaxed (Dijkstra/Bellman-Ford). @p pred must be
/// indexable by vertex id (e.g. a vertex_property_map or std::vector).
template <class PredMap>
[[nodiscard]] auto predecessor_recorder(PredMap& pred) {
  return [&pred](const auto& g, const auto& uv) { pred[target_id(g, uv)] = source_id(g, uv); };
}

/// Record a distance for each edge target: dist[target] = dist[source] + weight(g, uv).
/// Bind to on_tree_edge for unweighted layering or on_edge_relaxed for weighted relaxation.
/// @p dist must be indexable by vertex id and pre-seeded for the source vertices.
template <class DistMap, class WeightFn>
[[nodiscard]] auto distance_recorder(DistMap& dist, WeightFn weight) {
  return [&dist, weight](const auto& g, const auto& uv) {
    dist[target_id(g, uv)] = dist[source_id(g, uv)] + weight(g, uv);
  };
}

/// Record a hop-count distance for each edge target: dist[target] = dist[source] + 1.
/// Convenience overload of distance_recorder for unweighted BFS layering.
template <class DistMap>
[[nodiscard]] auto distance_recorder(DistMap& dist) {
  return [&dist](const auto& g, const auto& uv) {
    dist[target_id(g, uv)] = dist[source_id(g, uv)] + 1;
  };
}

/// Stamp a monotonically increasing time onto each visited vertex: time[vertex_id(g, u)] = clock++.
/// Bind to a vertex event such as on_discover_vertex or on_finish_vertex. @p clock is advanced
/// by reference so multiple stampers can share one clock for interleaved discover/finish times.
template <class TimeMap, class Counter>
[[nodiscard]] auto time_stamper(TimeMap& time, Counter& clock) {
  return [&time, &clock](const auto& g, const auto& u) { time[vertex_id(g, u)] = clock++; };
}

} // namespace graph

#endif // GRAPH_VISITOR_FACTORY_HPP
