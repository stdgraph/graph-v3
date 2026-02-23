#pragma once

#include <cstdint>
#include <unordered_map>
#include <list>

namespace graph::container {

// Forward declarations
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_out_edge;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_vertex;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_graph;

// uol_graph_traits
//  Vertices: std::unordered_map (hash-based; O(1) average lookup; unordered iteration)
//  Edges:    std::list (doubly-linked; bidirectional iteration)
//  Notes: Supports sparse, non-contiguous vertex IDs with hash-based access.
//         Unlike sequential containers (vector/deque), vertices must be explicitly
//         created - no auto-extension when edges reference undefined vertices.
//         Vertex IDs can be any hashable type (int, string, custom struct with hash).
//         Unlike std::map, iteration order is NOT sorted - it's based on hash buckets.
//         Edges use std::list for stable iterators and bidirectional traversal.
//  Template parameters: EV (edge value or void), VV (vertex value or void), GV (graph value or void),
//  VId (vertex id - any hashable type with std::hash specialization), Sourced (store source id on edge when true).
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Sourced = false, bool Bidirectional = false>
struct uol_graph_traits {
  using edge_value_type         = EV;
  using vertex_value_type       = VV;
  using graph_value_type        = GV;
  using vertex_id_type          = VId;
  static constexpr bool sourced       = Sourced;
  static constexpr bool bidirectional = Bidirectional;

  using edge_type   = dynamic_out_edge<EV, VV, GV, VId, Sourced, Bidirectional, uol_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Sourced, Bidirectional, uol_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Sourced, Bidirectional, uol_graph_traits>;

  using vertices_type = std::unordered_map<VId, vertex_type>;
  using edges_type    = std::list<edge_type>;
};

} // namespace graph::container
