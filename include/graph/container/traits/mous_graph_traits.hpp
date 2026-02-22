#pragma once

#include <map>
#include <unordered_set>

namespace graph::container {

// Forward declarations
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_edge;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_vertex;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_graph;

// mous_graph_traits
//  Vertices: std::map (associative; key-based lookup; bidirectional iteration)
//  Edges:    std::unordered_set (hash-based; automatic deduplication; unordered)
//
//  Key characteristics:
//  - Sparse, non-contiguous vertex IDs with key-based access
//  - Vertex IDs can be any ordered type (int, string, custom struct with operator<)
//  - Edges are automatically deduplicated (no parallel edges with same endpoints)
//  - Edges are stored in unordered fashion (insertion order not preserved)
//  - O(log n) vertex insertion, lookup, and deletion
//  - O(1) average edge insertion, lookup, and deletion
//  - Bidirectional iterators for vertices, forward iterators only for edges
//  - Unlike sequential containers, vertices must be explicitly created
//  - Requires operator== and std::hash on dynamic_edge (implemented in dynamic_graph.hpp)
//
//  Compared to mos_graph_traits:
//  - mos:  O(log n) edge operations, sorted order, bidirectional iterators
//  - mous: O(1) average edge operations, unordered, forward iterators only
//
//  Template parameters: EV (edge value or void), VV (vertex value or void), GV (graph value or void),
//  VId (vertex id - any ordered type with operator<), Sourced (store source id on edge when true).
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Sourced = false, bool Bidirectional = false>
struct mous_graph_traits {
  using edge_value_type         = EV;
  using vertex_value_type       = VV;
  using graph_value_type        = GV;
  using vertex_id_type          = VId;
  static constexpr bool sourced       = Sourced;
  static constexpr bool bidirectional = Bidirectional;

  using edge_type   = dynamic_edge<EV, VV, GV, VId, Sourced, Bidirectional, mous_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Sourced, Bidirectional, mous_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Sourced, Bidirectional, mous_graph_traits>;

  using vertices_type = std::map<VId, vertex_type>;
  using edges_type    = std::unordered_set<edge_type>;
};

} // namespace graph::container
