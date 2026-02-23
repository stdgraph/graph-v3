#pragma once

#include <map>
#include <set>

namespace graph::container {

// Forward declarations
template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
class dynamic_out_edge;

template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
class dynamic_vertex;

template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
class dynamic_graph;

// mos_graph_traits
//  Vertices: std::map (associative; key-based lookup; bidirectional iteration)
//  Edges:    std::set (ordered; automatic deduplication by target_id/source_id)
//
//  Key characteristics:
//  - Sparse, non-contiguous vertex IDs with key-based access
//  - Vertex IDs can be any ordered type (int, string, custom struct with operator<)
//  - Edges are automatically deduplicated (no parallel edges with same endpoints)
//  - Edges are stored in sorted order (by target_id)
//  - O(log n) vertex and edge insertion, lookup, and deletion
//  - Bidirectional iterators for both vertices and edges
//  - Unlike sequential containers, vertices must be explicitly created
//
//  Template parameters: EV (edge value or void), VV (vertex value or void), GV (graph value or void),
//  VId (vertex id - any ordered type with operator<).
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Bidirectional = false>
struct mos_graph_traits {
  using edge_value_type         = EV;
  using vertex_value_type       = VV;
  using graph_value_type        = GV;
  using vertex_id_type          = VId;
  static constexpr bool bidirectional = Bidirectional;

  using edge_type   = dynamic_out_edge<EV, VV, GV, VId, Bidirectional, mos_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Bidirectional, mos_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Bidirectional, mos_graph_traits>;

  using vertices_type = std::map<VId, vertex_type>;
  using edges_type    = std::set<edge_type>;
};

} // namespace graph::container
