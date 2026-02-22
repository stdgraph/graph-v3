#pragma once

#include <map>

namespace graph::container {

// Forward declarations
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_edge;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_vertex;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_graph;

// mom_graph_traits
//  Vertices: std::map<VId, vertex_type> (ordered map keyed by vertex ID)
//  Edges:    std::map<VId, edge_type> (ordered map keyed by target vertex ID)
//
//  Key characteristics:
//  - O(log n) vertex lookup by ID using map::find(vid)
//  - O(log n) edge lookup by target vertex ID using map::find(vid)
//  - Both vertices and edges automatically deduplicated
//  - Both stored in sorted order by ID
//  - Bidirectional iterators (not random access)
//  - Sparse graph support (non-contiguous vertex IDs)
//  - Requires operator<=> on VId (for map ordering)
//
//  Use cases:
//  - Sparse graphs with non-contiguous vertex IDs
//  - Graphs with unique edges (no parallel edges to same target)
//  - Fast lookup: O(log n) for both vertices and edges
//  - Efficient vertex insertion/deletion without affecting other IDs
//  - Memory overhead: ~24-32 bytes per vertex + ~8-16 bytes per edge
//
//  Parameter semantics mirror vom_graph_traits.
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Sourced = false, bool Bidirectional = false>
struct mom_graph_traits {
  using edge_value_type         = EV;
  using vertex_value_type       = VV;
  using graph_value_type        = GV;
  using vertex_id_type          = VId;
  static constexpr bool sourced       = Sourced;
  static constexpr bool bidirectional = Bidirectional;

  using edge_type   = dynamic_edge<EV, VV, GV, VId, Sourced, Bidirectional, mom_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Sourced, Bidirectional, mom_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Sourced, Bidirectional, mom_graph_traits>;

  using vertices_type = std::map<VId, vertex_type>; // Map keyed by vertex ID
  using edges_type    = std::map<VId, edge_type>;   // Map keyed by target vertex ID
};

} // namespace graph::container
