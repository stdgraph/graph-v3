#pragma once

#include <unordered_map>
#include <set>

namespace graph::container {

// Forward declarations
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_out_edge;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_vertex;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_graph;

// uos_graph_traits
//  Vertices: std::unordered_map (hash-based; key-based lookup; forward iteration only)
//  Edges:    std::set (ordered; automatic deduplication by target_id/source_id)
//
//  Key characteristics:
//  - Sparse, non-contiguous vertex IDs with key-based access
//  - Vertex IDs can be any hashable type (int, string, custom struct with std::hash)
//  - Edges are automatically deduplicated (no parallel edges with same endpoints)
//  - Vertices stored in unordered fashion (insertion order not preserved)
//  - Edges stored in sorted order (by source_id if Sourced, then target_id)
//  - O(1) average vertex insertion, lookup, and deletion
//  - O(log n) edge insertion, lookup, and deletion
//  - Forward iterators only for vertices, bidirectional iterators for edges
//  - Unlike sequential containers, vertices must be explicitly created
//  - Requires operator== and std::hash on VId, operator< on dynamic_out_edge (implemented in dynamic_graph.hpp)
//
//  Compared to uous_graph_traits:
//  - uous: O(1) average edge operations, unordered edges, forward edge iterators
//  - uos:  O(log n) edge operations, sorted edges, bidirectional edge iterators
//
//  Template parameters: EV (edge value or void), VV (vertex value or void), GV (graph value or void),
//  VId (vertex id - any hashable type with operator== and std::hash), Sourced (store source id on edge when true).
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Sourced = false, bool Bidirectional = false>
struct uos_graph_traits {
  using edge_value_type         = EV;
  using vertex_value_type       = VV;
  using graph_value_type        = GV;
  using vertex_id_type          = VId;
  static constexpr bool sourced       = Sourced;
  static constexpr bool bidirectional = Bidirectional;

  using edge_type   = dynamic_out_edge<EV, VV, GV, VId, Sourced, Bidirectional, uos_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Sourced, Bidirectional, uos_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Sourced, Bidirectional, uos_graph_traits>;

  using vertices_type = std::unordered_map<VId, vertex_type>;
  using edges_type    = std::set<edge_type>;
};

} // namespace graph::container
