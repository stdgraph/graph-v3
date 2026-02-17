/**
 * @file adaptors.hpp
 * @brief Range adaptor closures for graph views enabling pipe syntax
 * 
 * Implements C++20 range adaptor pattern for graph views:
 * 
 * Basic views:
 * - g | vertexlist() - view of all vertices
 * - g | incidence(uid) - view of edges from a vertex
 * - g | neighbors(uid) - view of neighbor vertices
 * - g | edgelist() - view of all edges
 * 
 * Search views:
 * - g | vertices_dfs(seed) - depth-first vertex traversal
 * - g | edges_dfs(seed) - depth-first edge traversal
 * - g | vertices_bfs(seed) - breadth-first vertex traversal
 * - g | edges_bfs(seed) - breadth-first edge traversal
 * - g | vertices_topological_sort() - topological order vertex traversal
 * - g | edges_topological_sort() - topological order edge traversal
 * 
 * Supports chaining with standard range adaptors:
 * - g | vertexlist() | std::views::take(5)
 * - g | incidence(0) | std::views::filter(predicate)
 * - g | vertices_dfs(0) | std::views::transform(fn)
 */

#pragma once

#include <graph/views/vertexlist.hpp>
#include <graph/views/incidence.hpp>
#include <graph/views/neighbors.hpp>
#include <graph/views/edgelist.hpp>
#include <graph/views/dfs.hpp>
#include <graph/views/bfs.hpp>
#include <graph/views/topological_sort.hpp>
#include <type_traits>

// Bring CPOs into graph namespace for convenience when using views
namespace graph {
using adj_list::vertices;
using adj_list::vertex_id;
using adj_list::find_vertex;
using adj_list::edges;
using adj_list::target_id;
using adj_list::target;
using adj_list::num_vertices;
using adj_list::num_edges;
using adj_list::degree;
using adj_list::find_vertex_edge;
using adj_list::contains_edge;
using adj_list::has_edge;
using adj_list::vertex_value;
using adj_list::edge_value;
using adj_list::graph_value;
using adj_list::source_id;
using adj_list::source;
using adj_list::partition_id;
using adj_list::num_partitions;
} // namespace graph

namespace graph::views {

// Empty placeholder for void template parameters
struct monostate {};

//=============================================================================
// vertexlist adaptor
//=============================================================================

template <class VVF = void>
struct vertexlist_adaptor_closure {
  [[no_unique_address]] std::conditional_t<std::is_void_v<VVF>, monostate, VVF> vvf;

  template <adj_list::index_adjacency_list G>
  friend auto operator|(G&& g, vertexlist_adaptor_closure adaptor) {
    if constexpr (std::is_void_v<VVF>) {
      return vertexlist(std::forward<G>(g));
    } else {
      return vertexlist(std::forward<G>(g), std::move(adaptor.vvf));
    }
  }
};

// Factory function object for creating vertexlist adaptors
struct vertexlist_adaptor_fn {
  // No arguments: g | vertexlist()
  auto operator()() const { return vertexlist_adaptor_closure<void>{monostate{}}; }

  // With value function: g | vertexlist(vvf)
  template <class VVF>
  auto operator()(VVF&& vvf) const {
    return vertexlist_adaptor_closure<std::decay_t<VVF>>{std::forward<VVF>(vvf)};
  }

  // Direct call: vertexlist(g)
  template <adj_list::adjacency_list G>
  auto operator()(G&& g) const {
    return graph::views::vertexlist(std::forward<G>(g));
  }

  // Direct call with value function: vertexlist(g, vvf)
  template <adj_list::adjacency_list G, class VVF>
  auto operator()(G&& g, VVF&& vvf) const {
    return graph::views::vertexlist(std::forward<G>(g), std::forward<VVF>(vvf));
  }
};

//=============================================================================
// incidence adaptor
//=============================================================================

template <class UID, class EVF = void>
struct incidence_adaptor_closure {
  UID                                                                           uid;
  [[no_unique_address]] std::conditional_t<std::is_void_v<EVF>, monostate, EVF> evf;

  template <adj_list::index_adjacency_list G>
  friend auto operator|(G&& g, incidence_adaptor_closure adaptor) {
    auto u = *adj_list::find_vertex(g, std::move(adaptor.uid));
    if constexpr (std::is_void_v<EVF>) {
      return incidence(std::forward<G>(g), u);
    } else {
      return incidence(std::forward<G>(g), u, std::move(adaptor.evf));
    }
  }
};

// Factory function object for creating incidence adaptors
struct incidence_adaptor_fn {
  // With vertex id: g | incidence(uid)
  template <class UID>
  auto operator()(UID&& uid) const {
    return incidence_adaptor_closure<std::decay_t<UID>, void>{std::forward<UID>(uid), monostate{}};
  }

  // With vertex id and value function: g | incidence(uid, evf)
  template <class UID, class EVF>
  auto operator()(UID&& uid, EVF&& evf) const {
    return incidence_adaptor_closure<std::decay_t<UID>, std::decay_t<EVF>>{std::forward<UID>(uid),
                                                                           std::forward<EVF>(evf)};
  }

  // Direct call: incidence(g, uid)
  template <class G, class UID>
  requires adj_list::index_adjacency_list<std::remove_cvref_t<G>>
  auto operator()(G&& g, UID&& uid) const {
    return graph::views::incidence(std::forward<G>(g), adj_list::vertex_id_t<std::remove_cvref_t<G>>(std::forward<UID>(uid)));
  }

  // Direct call with value function: incidence(g, uid, evf)
  template <class G, class UID, class EVF>
  requires adj_list::index_adjacency_list<std::remove_cvref_t<G>>
  auto operator()(G&& g, UID&& uid, EVF&& evf) const {
    return graph::views::incidence(std::forward<G>(g), adj_list::vertex_id_t<std::remove_cvref_t<G>>(std::forward<UID>(uid)), std::forward<EVF>(evf));
  }
};

//=============================================================================
// neighbors adaptor
//=============================================================================

template <class UID, class VVF = void>
struct neighbors_adaptor_closure {
  UID                                                                           uid;
  [[no_unique_address]] std::conditional_t<std::is_void_v<VVF>, monostate, VVF> vvf;

  template <adj_list::index_adjacency_list G>
  friend auto operator|(G&& g, neighbors_adaptor_closure adaptor) {
    if constexpr (std::is_void_v<VVF>) {
      return neighbors(std::forward<G>(g), std::move(adaptor.uid));
    } else {
      return neighbors(std::forward<G>(g), std::move(adaptor.uid), std::move(adaptor.vvf));
    }
  }
};

// Factory function object for creating neighbors adaptors
struct neighbors_adaptor_fn {
  // With vertex id: g | neighbors(uid)
  template <class UID>
  auto operator()(UID&& uid) const {
    return neighbors_adaptor_closure<std::decay_t<UID>, void>{std::forward<UID>(uid), monostate{}};
  }

  // With vertex id and value function: g | neighbors(uid, vvf)
  template <class UID, class VVF>
  auto operator()(UID&& uid, VVF&& vvf) const {
    return neighbors_adaptor_closure<std::decay_t<UID>, std::decay_t<VVF>>{std::forward<UID>(uid),
                                                                           std::forward<VVF>(vvf)};
  }

  // Direct call: neighbors(g, uid)
  template <adj_list::adjacency_list G, class UID>
  auto operator()(G&& g, UID&& uid) const {
    return graph::views::neighbors(std::forward<G>(g), std::forward<UID>(uid));
  }

  // Direct call with value function: neighbors(g, uid, vvf)
  template <adj_list::adjacency_list G, class UID, class VVF>
  auto operator()(G&& g, UID&& uid, VVF&& vvf) const {
    return graph::views::neighbors(std::forward<G>(g), std::forward<UID>(uid), std::forward<VVF>(vvf));
  }
};

//=============================================================================
// edgelist adaptor
//=============================================================================

template <class EVF = void>
struct edgelist_adaptor_closure {
  [[no_unique_address]] std::conditional_t<std::is_void_v<EVF>, monostate, EVF> evf;

  template <adj_list::index_adjacency_list G>
  friend auto operator|(G&& g, edgelist_adaptor_closure adaptor) {
    if constexpr (std::is_void_v<EVF>) {
      return edgelist(std::forward<G>(g));
    } else {
      return edgelist(std::forward<G>(g), std::move(adaptor.evf));
    }
  }
};

// Factory function object for creating edgelist adaptors
struct edgelist_adaptor_fn {
  // No arguments: g | edgelist()
  auto operator()() const { return edgelist_adaptor_closure<void>{monostate{}}; }

  // With value function: g | edgelist(evf)
  template <class EVF>
  auto operator()(EVF&& evf) const {
    return edgelist_adaptor_closure<std::decay_t<EVF>>{std::forward<EVF>(evf)};
  }

  // Direct call: edgelist(g)
  template <adj_list::adjacency_list G>
  auto operator()(G&& g) const {
    return graph::views::edgelist(std::forward<G>(g));
  }

  // Direct call with value function: edgelist(g, evf)
  template <adj_list::adjacency_list G, class EVF>
  auto operator()(G&& g, EVF&& evf) const {
    return graph::views::edgelist(std::forward<G>(g), std::forward<EVF>(evf));
  }
};

//=============================================================================
// vertices_dfs adaptor
//=============================================================================

template <class Seed, class VVF = void, class Alloc = std::allocator<bool>>
struct vertices_dfs_adaptor_closure {
  Seed                                                                          seed;
  [[no_unique_address]] std::conditional_t<std::is_void_v<VVF>, monostate, VVF> vvf;
  [[no_unique_address]] Alloc                                                   alloc;

  template <adj_list::index_adjacency_list G>
  friend auto operator|(G&& g, vertices_dfs_adaptor_closure adaptor) {
    if constexpr (std::is_void_v<VVF>) {
      return vertices_dfs(std::forward<G>(g), std::move(adaptor.seed), std::move(adaptor.alloc));
    } else {
      return vertices_dfs(std::forward<G>(g), std::move(adaptor.seed), std::move(adaptor.vvf),
                          std::move(adaptor.alloc));
    }
  }
};

// Factory function object for creating vertices_dfs adaptors
struct vertices_dfs_adaptor_fn {
  // With seed: g | vertices_dfs(seed)
  template <class Seed>
  auto operator()(Seed&& seed) const {
    return vertices_dfs_adaptor_closure<std::decay_t<Seed>, void, std::allocator<bool>>{
          std::forward<Seed>(seed), monostate{}, std::allocator<bool>{}};
  }

  // With seed and value function: g | vertices_dfs(seed, vvf)
  template <class Seed, class VVF>
  auto operator()(Seed&& seed, VVF&& vvf) const {
    return vertices_dfs_adaptor_closure<std::decay_t<Seed>, std::decay_t<VVF>, std::allocator<bool>>{
          std::forward<Seed>(seed), std::forward<VVF>(vvf), std::allocator<bool>{}};
  }

  // With seed, value function, and allocator: g | vertices_dfs(seed, vvf, alloc)
  template <class Seed, class VVF, class Alloc>
  auto operator()(Seed&& seed, VVF&& vvf, Alloc&& alloc) const {
    return vertices_dfs_adaptor_closure<std::decay_t<Seed>, std::decay_t<VVF>, std::decay_t<Alloc>>{
          std::forward<Seed>(seed), std::forward<VVF>(vvf), std::forward<Alloc>(alloc)};
  }

  // Direct call: vertices_dfs(g, seed)
  template <adj_list::index_adjacency_list G, class Seed>
  auto operator()(G&& g, Seed&& seed) const {
    return graph::views::vertices_dfs(std::forward<G>(g), std::forward<Seed>(seed));
  }

  // Direct call with value function: vertices_dfs(g, seed, vvf)
  template <adj_list::index_adjacency_list G, class Seed, class VVF>
  auto operator()(G&& g, Seed&& seed, VVF&& vvf) const {
    return graph::views::vertices_dfs(std::forward<G>(g), std::forward<Seed>(seed), std::forward<VVF>(vvf));
  }

  // Direct call with value function and allocator: vertices_dfs(g, seed, vvf, alloc)
  template <adj_list::index_adjacency_list G, class Seed, class VVF, class Alloc>
  auto operator()(G&& g, Seed&& seed, VVF&& vvf, Alloc&& alloc) const {
    return graph::views::vertices_dfs(std::forward<G>(g), std::forward<Seed>(seed), std::forward<VVF>(vvf),
                                      std::forward<Alloc>(alloc));
  }
};

//=============================================================================
// edges_dfs adaptor
//=============================================================================

template <class Seed, class EVF = void, class Alloc = std::allocator<bool>>
struct edges_dfs_adaptor_closure {
  Seed                                                                          seed;
  [[no_unique_address]] std::conditional_t<std::is_void_v<EVF>, monostate, EVF> evf;
  [[no_unique_address]] Alloc                                                   alloc;

  template <adj_list::index_adjacency_list G>
  friend auto operator|(G&& g, edges_dfs_adaptor_closure adaptor) {
    if constexpr (std::is_void_v<EVF>) {
      return edges_dfs(std::forward<G>(g), std::move(adaptor.seed), std::move(adaptor.alloc));
    } else {
      return edges_dfs(std::forward<G>(g), std::move(adaptor.seed), std::move(adaptor.evf), std::move(adaptor.alloc));
    }
  }
};

// Factory function object for creating edges_dfs adaptors
struct edges_dfs_adaptor_fn {
  // With seed: g | edges_dfs(seed)
  template <class Seed>
  auto operator()(Seed&& seed) const {
    return edges_dfs_adaptor_closure<std::decay_t<Seed>, void, std::allocator<bool>>{
          std::forward<Seed>(seed), monostate{}, std::allocator<bool>{}};
  }

  // With seed and value function: g | edges_dfs(seed, evf)
  template <class Seed, class EVF>
  auto operator()(Seed&& seed, EVF&& evf) const {
    return edges_dfs_adaptor_closure<std::decay_t<Seed>, std::decay_t<EVF>, std::allocator<bool>>{
          std::forward<Seed>(seed), std::forward<EVF>(evf), std::allocator<bool>{}};
  }

  // With seed, value function, and allocator: g | edges_dfs(seed, evf, alloc)
  template <class Seed, class EVF, class Alloc>
  auto operator()(Seed&& seed, EVF&& evf, Alloc&& alloc) const {
    return edges_dfs_adaptor_closure<std::decay_t<Seed>, std::decay_t<EVF>, std::decay_t<Alloc>>{
          std::forward<Seed>(seed), std::forward<EVF>(evf), std::forward<Alloc>(alloc)};
  }

  // Direct call: edges_dfs(g, seed)
  template <adj_list::index_adjacency_list G, class Seed>
  auto operator()(G&& g, Seed&& seed) const {
    return graph::views::edges_dfs(std::forward<G>(g), std::forward<Seed>(seed));
  }

  // Direct call with value function: edges_dfs(g, seed, evf)
  template <adj_list::index_adjacency_list G, class Seed, class EVF>
  auto operator()(G&& g, Seed&& seed, EVF&& evf) const {
    return graph::views::edges_dfs(std::forward<G>(g), std::forward<Seed>(seed), std::forward<EVF>(evf));
  }

  // Direct call with value function and allocator: edges_dfs(g, seed, evf, alloc)
  template <adj_list::index_adjacency_list G, class Seed, class EVF, class Alloc>
  auto operator()(G&& g, Seed&& seed, EVF&& evf, Alloc&& alloc) const {
    return graph::views::edges_dfs(std::forward<G>(g), std::forward<Seed>(seed), std::forward<EVF>(evf),
                                   std::forward<Alloc>(alloc));
  }
};

//=============================================================================
// vertices_bfs adaptor
//=============================================================================

template <class Seed, class VVF = void, class Alloc = std::allocator<bool>>
struct vertices_bfs_adaptor_closure {
  Seed                                                                          seed;
  [[no_unique_address]] std::conditional_t<std::is_void_v<VVF>, monostate, VVF> vvf;
  [[no_unique_address]] Alloc                                                   alloc;

  template <adj_list::index_adjacency_list G>
  friend auto operator|(G&& g, vertices_bfs_adaptor_closure adaptor) {
    if constexpr (std::is_void_v<VVF>) {
      return vertices_bfs(std::forward<G>(g), std::move(adaptor.seed), std::move(adaptor.alloc));
    } else {
      return vertices_bfs(std::forward<G>(g), std::move(adaptor.seed), std::move(adaptor.vvf),
                          std::move(adaptor.alloc));
    }
  }
};

// Factory function object for creating vertices_bfs adaptors
struct vertices_bfs_adaptor_fn {
  // With seed: g | vertices_bfs(seed)
  template <class Seed>
  auto operator()(Seed&& seed) const {
    return vertices_bfs_adaptor_closure<std::decay_t<Seed>, void, std::allocator<bool>>{
          std::forward<Seed>(seed), monostate{}, std::allocator<bool>{}};
  }

  // With seed and value function: g | vertices_bfs(seed, vvf)
  template <class Seed, class VVF>
  auto operator()(Seed&& seed, VVF&& vvf) const {
    return vertices_bfs_adaptor_closure<std::decay_t<Seed>, std::decay_t<VVF>, std::allocator<bool>>{
          std::forward<Seed>(seed), std::forward<VVF>(vvf), std::allocator<bool>{}};
  }

  // With seed, value function, and allocator: g | vertices_bfs(seed, vvf, alloc)
  template <class Seed, class VVF, class Alloc>
  auto operator()(Seed&& seed, VVF&& vvf, Alloc&& alloc) const {
    return vertices_bfs_adaptor_closure<std::decay_t<Seed>, std::decay_t<VVF>, std::decay_t<Alloc>>{
          std::forward<Seed>(seed), std::forward<VVF>(vvf), std::forward<Alloc>(alloc)};
  }

  // Direct call: vertices_bfs(g, seed)
  template <adj_list::index_adjacency_list G, class Seed>
  auto operator()(G&& g, Seed&& seed) const {
    return graph::views::vertices_bfs(std::forward<G>(g), std::forward<Seed>(seed));
  }

  // Direct call with value function: vertices_bfs(g, seed, vvf)
  template <adj_list::index_adjacency_list G, class Seed, class VVF>
  auto operator()(G&& g, Seed&& seed, VVF&& vvf) const {
    return graph::views::vertices_bfs(std::forward<G>(g), std::forward<Seed>(seed), std::forward<VVF>(vvf));
  }

  // Direct call with value function and allocator: vertices_bfs(g, seed, vvf, alloc)
  template <adj_list::index_adjacency_list G, class Seed, class VVF, class Alloc>
  auto operator()(G&& g, Seed&& seed, VVF&& vvf, Alloc&& alloc) const {
    return graph::views::vertices_bfs(std::forward<G>(g), std::forward<Seed>(seed), std::forward<VVF>(vvf),
                                      std::forward<Alloc>(alloc));
  }
};

//=============================================================================
// edges_bfs adaptor
//=============================================================================

template <class Seed, class EVF = void, class Alloc = std::allocator<bool>>
struct edges_bfs_adaptor_closure {
  Seed                                                                          seed;
  [[no_unique_address]] std::conditional_t<std::is_void_v<EVF>, monostate, EVF> evf;
  [[no_unique_address]] Alloc                                                   alloc;

  template <adj_list::index_adjacency_list G>
  friend auto operator|(G&& g, edges_bfs_adaptor_closure adaptor) {
    if constexpr (std::is_void_v<EVF>) {
      return edges_bfs(std::forward<G>(g), std::move(adaptor.seed), std::move(adaptor.alloc));
    } else {
      return edges_bfs(std::forward<G>(g), std::move(adaptor.seed), std::move(adaptor.evf), std::move(adaptor.alloc));
    }
  }
};

// Factory function object for creating edges_bfs adaptors
struct edges_bfs_adaptor_fn {
  // With seed: g | edges_bfs(seed)
  template <class Seed>
  auto operator()(Seed&& seed) const {
    return edges_bfs_adaptor_closure<std::decay_t<Seed>, void, std::allocator<bool>>{
          std::forward<Seed>(seed), monostate{}, std::allocator<bool>{}};
  }

  // With seed and value function: g | edges_bfs(seed, evf)
  template <class Seed, class EVF>
  auto operator()(Seed&& seed, EVF&& evf) const {
    return edges_bfs_adaptor_closure<std::decay_t<Seed>, std::decay_t<EVF>, std::allocator<bool>>{
          std::forward<Seed>(seed), std::forward<EVF>(evf), std::allocator<bool>{}};
  }

  // With seed, value function, and allocator: g | edges_bfs(seed, evf, alloc)
  template <class Seed, class EVF, class Alloc>
  auto operator()(Seed&& seed, EVF&& evf, Alloc&& alloc) const {
    return edges_bfs_adaptor_closure<std::decay_t<Seed>, std::decay_t<EVF>, std::decay_t<Alloc>>{
          std::forward<Seed>(seed), std::forward<EVF>(evf), std::forward<Alloc>(alloc)};
  }

  // Direct call: edges_bfs(g, seed)
  template <adj_list::index_adjacency_list G, class Seed>
  auto operator()(G&& g, Seed&& seed) const {
    return graph::views::edges_bfs(std::forward<G>(g), std::forward<Seed>(seed));
  }

  // Direct call with value function: edges_bfs(g, seed, evf)
  template <adj_list::index_adjacency_list G, class Seed, class EVF>
  auto operator()(G&& g, Seed&& seed, EVF&& evf) const {
    return graph::views::edges_bfs(std::forward<G>(g), std::forward<Seed>(seed), std::forward<EVF>(evf));
  }

  // Direct call with value function and allocator: edges_bfs(g, seed, evf, alloc)
  template <adj_list::index_adjacency_list G, class Seed, class EVF, class Alloc>
  auto operator()(G&& g, Seed&& seed, EVF&& evf, Alloc&& alloc) const {
    return graph::views::edges_bfs(std::forward<G>(g), std::forward<Seed>(seed), std::forward<EVF>(evf),
                                   std::forward<Alloc>(alloc));
  }
};

//=============================================================================
// vertices_topological_sort adaptor
//=============================================================================

template <class VVF = void, class Alloc = std::allocator<bool>>
struct vertices_topological_sort_adaptor_closure {
  [[no_unique_address]] std::conditional_t<std::is_void_v<VVF>, monostate, VVF> vvf;
  [[no_unique_address]] Alloc                                                   alloc;

  template <adj_list::index_adjacency_list G>
  friend auto operator|(G&& g, vertices_topological_sort_adaptor_closure adaptor) {
    if constexpr (std::is_void_v<VVF>) {
      return graph::views::vertices_topological_sort(std::forward<G>(g), adaptor.alloc);
    } else {
      return graph::views::vertices_topological_sort(std::forward<G>(g), adaptor.vvf, adaptor.alloc);
    }
  }
};

struct vertices_topological_sort_adaptor_fn {
  // Basic: g | vertices_topological_sort()
  auto operator()() const { return vertices_topological_sort_adaptor_closure<void, std::allocator<bool>>{}; }

  // With value function: g | vertices_topological_sort(vvf)
  template <class VVF>
  auto operator()(VVF&& vvf) const {
    return vertices_topological_sort_adaptor_closure<std::decay_t<VVF>, std::allocator<bool>>{std::forward<VVF>(vvf),
                                                                                              std::allocator<bool>{}};
  }

  // With value function and allocator: g | vertices_topological_sort(vvf, alloc)
  template <class VVF, class Alloc>
  auto operator()(VVF&& vvf, Alloc&& alloc) const {
    return vertices_topological_sort_adaptor_closure<std::decay_t<VVF>, std::decay_t<Alloc>>{
          std::forward<VVF>(vvf), std::forward<Alloc>(alloc)};
  }

  // Direct call: vertices_topological_sort(g)
  template <adj_list::index_adjacency_list G>
  auto operator()(G&& g) const {
    return graph::views::vertices_topological_sort(std::forward<G>(g));
  }

  // Direct call with value function: vertices_topological_sort(g, vvf)
  template <adj_list::index_adjacency_list G, class VVF>
  auto operator()(G&& g, VVF&& vvf) const {
    return graph::views::vertices_topological_sort(std::forward<G>(g), std::forward<VVF>(vvf));
  }

  // Direct call with value function and allocator: vertices_topological_sort(g, vvf, alloc)
  template <adj_list::index_adjacency_list G, class VVF, class Alloc>
  auto operator()(G&& g, VVF&& vvf, Alloc&& alloc) const {
    return graph::views::vertices_topological_sort(std::forward<G>(g), std::forward<VVF>(vvf),
                                                   std::forward<Alloc>(alloc));
  }
};

//=============================================================================
// edges_topological_sort adaptor
//=============================================================================

template <class EVF = void, class Alloc = std::allocator<bool>>
struct edges_topological_sort_adaptor_closure {
  [[no_unique_address]] std::conditional_t<std::is_void_v<EVF>, monostate, EVF> evf;
  [[no_unique_address]] Alloc                                                   alloc;

  template <adj_list::index_adjacency_list G>
  friend auto operator|(G&& g, edges_topological_sort_adaptor_closure adaptor) {
    if constexpr (std::is_void_v<EVF>) {
      return graph::views::edges_topological_sort(std::forward<G>(g), adaptor.alloc);
    } else {
      return graph::views::edges_topological_sort(std::forward<G>(g), adaptor.evf, adaptor.alloc);
    }
  }
};

struct edges_topological_sort_adaptor_fn {
  // Basic: g | edges_topological_sort()
  auto operator()() const { return edges_topological_sort_adaptor_closure<void, std::allocator<bool>>{}; }

  // With value function: g | edges_topological_sort(evf)
  template <class EVF>
  auto operator()(EVF&& evf) const {
    return edges_topological_sort_adaptor_closure<std::decay_t<EVF>, std::allocator<bool>>{std::forward<EVF>(evf),
                                                                                           std::allocator<bool>{}};
  }

  // With value function and allocator: g | edges_topological_sort(evf, alloc)
  template <class EVF, class Alloc>
  auto operator()(EVF&& evf, Alloc&& alloc) const {
    return edges_topological_sort_adaptor_closure<std::decay_t<EVF>, std::decay_t<Alloc>>{std::forward<EVF>(evf),
                                                                                          std::forward<Alloc>(alloc)};
  }

  // Direct call: edges_topological_sort(g)
  template <adj_list::index_adjacency_list G>
  auto operator()(G&& g) const {
    return graph::views::edges_topological_sort(std::forward<G>(g));
  }

  // Direct call with value function: edges_topological_sort(g, evf)
  template <adj_list::index_adjacency_list G, class EVF>
  auto operator()(G&& g, EVF&& evf) const {
    return graph::views::edges_topological_sort(std::forward<G>(g), std::forward<EVF>(evf));
  }

  // Direct call with value function and allocator: edges_topological_sort(g, evf, alloc)
  template <adj_list::index_adjacency_list G, class EVF, class Alloc>
  auto operator()(G&& g, EVF&& evf, Alloc&& alloc) const {
    return graph::views::edges_topological_sort(std::forward<G>(g), std::forward<EVF>(evf), std::forward<Alloc>(alloc));
  }
};

} // namespace graph::views

//=============================================================================
// Adaptor objects for pipe syntax
//=============================================================================

namespace graph::views::adaptors {
// Basic views
inline constexpr vertexlist_adaptor_fn vertexlist{};
inline constexpr incidence_adaptor_fn  incidence{};
inline constexpr neighbors_adaptor_fn  neighbors{};
inline constexpr edgelist_adaptor_fn   edgelist{};

// Search views
inline constexpr vertices_dfs_adaptor_fn vertices_dfs{};
inline constexpr edges_dfs_adaptor_fn    edges_dfs{};
inline constexpr vertices_bfs_adaptor_fn vertices_bfs{};
inline constexpr edges_bfs_adaptor_fn    edges_bfs{};

// Topological sort views
inline constexpr vertices_topological_sort_adaptor_fn vertices_topological_sort{};
inline constexpr edges_topological_sort_adaptor_fn    edges_topological_sort{};
} // namespace graph::views::adaptors
