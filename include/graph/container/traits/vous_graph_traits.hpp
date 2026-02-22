#pragma once

#include <vector>
#include <unordered_set>

namespace graph::container {

// Forward declarations
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_edge;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_vertex;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_graph;

// vous_graph_traits
//  Vertices: std::vector (contiguous; random access by vertex ID)
//  Edges:    std::unordered_set (hash-based; automatic deduplication; unordered)
//
//  Key characteristics:
//  - Edges are automatically deduplicated (no parallel edges with same endpoints)
//  - Edges are stored in unordered fashion (insertion order not preserved)
//  - O(1) average edge insertion, lookup, and deletion
//  - Forward iterators only (no bidirectional or random access)
//  - Requires operator== and std::hash on dynamic_edge (implemented in dynamic_graph.hpp)
//
//  Compared to vos_graph_traits:
//  - vos:  O(log n) operations, sorted order, bidirectional iterators
//  - vous: O(1) average operations, unordered, forward iterators only
//
//  Parameter semantics mirror vofl_graph_traits.
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Sourced = false, bool Bidirectional = false>
struct vous_graph_traits {
  using edge_value_type         = EV;
  using vertex_value_type       = VV;
  using graph_value_type        = GV;
  using vertex_id_type          = VId;
  static constexpr bool sourced       = Sourced;
  static constexpr bool bidirectional = Bidirectional;

  using edge_type   = dynamic_edge<EV, VV, GV, VId, Sourced, Bidirectional, vous_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Sourced, Bidirectional, vous_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Sourced, Bidirectional, vous_graph_traits>;

  using vertices_type = std::vector<vertex_type>;
  using edges_type    = std::unordered_set<edge_type>;
};

} // namespace graph::container
