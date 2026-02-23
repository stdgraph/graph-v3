#pragma once

#include <unordered_map>
#include <unordered_set>

namespace graph::container {

// Forward declarations
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_out_edge;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_vertex;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_graph;

// uous_graph_traits
//  Vertices: std::unordered_map (hash-based; key-based lookup; forward iteration only)
//  Edges:    std::unordered_set (hash-based; automatic deduplication; unordered)
//
//  Key characteristics:
//  - Sparse, non-contiguous vertex IDs with key-based access
//  - Vertex IDs can be any hashable type (int, string, custom struct with std::hash)
//  - Edges are automatically deduplicated (no parallel edges with same endpoints)
//  - Both vertices and edges stored in unordered fashion (insertion order not preserved)
//  - O(1) average vertex insertion, lookup, and deletion
//  - O(1) average edge insertion, lookup, and deletion
//  - Forward iterators only for both vertices and edges
//  - Unlike sequential containers, vertices must be explicitly created
//  - Requires operator== and std::hash on both VId and dynamic_out_edge (implemented in dynamic_graph.hpp)
//
//  Compared to mous_graph_traits:
//  - mous: O(log n) vertex operations, sorted vertex order, bidirectional vertex iterators
//  - uous: O(1) average vertex operations, unordered, forward iterators only
//
//  Template parameters: EV (edge value or void), VV (vertex value or void), GV (graph value or void),
//  VId (vertex id - any hashable type with operator== and std::hash), Sourced (store source id on edge when true).
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Sourced = false, bool Bidirectional = false>
struct uous_graph_traits {
  using edge_value_type         = EV;
  using vertex_value_type       = VV;
  using graph_value_type        = GV;
  using vertex_id_type          = VId;
  static constexpr bool sourced       = Sourced;
  static constexpr bool bidirectional = Bidirectional;

  using edge_type   = dynamic_out_edge<EV, VV, GV, VId, Sourced, Bidirectional, uous_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Sourced, Bidirectional, uous_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Sourced, Bidirectional, uous_graph_traits>;

  using vertices_type = std::unordered_map<VId, vertex_type>;
  using edges_type    = std::unordered_set<edge_type>;
};

} // namespace graph::container
