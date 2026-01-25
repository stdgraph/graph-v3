//
//	Author: J. Phillip Ratzloff
//

#ifndef UNDIRECTED_ADJ_LIST_API_HPP
#define UNDIRECTED_ADJ_LIST_API_HPP

#include <stdexcept>

namespace graph::container {

// Type trait aliases for undirected_adjacency_list API
template<typename G> using graph_value_t = typename G::graph_value_type;
template<typename G> using vertex_type_t = typename G::vertex_type;
template<typename G> using vertex_value_t = typename G::vertex_value_type;
template<typename G> using vertex_id_t = typename G::vertex_id_type;
template<typename G> using vertex_size_t = typename G::vertex_size_type;
template<typename G> using vertex_iterator_t = typename G::vertex_iterator;
template<typename G> using const_vertex_iterator_t = typename G::const_vertex_iterator;
template<typename G> using vertex_range_t = typename G::vertex_range;
template<typename G> using const_vertex_range_t = typename G::const_vertex_range;
template<typename G> using edge_t = typename G::edge_type;
template<typename G> using edge_value_t = typename G::edge_value_type;
template<typename G> using edge_id_t = typename G::edge_id_type;
template<typename G> using edge_size_t = typename G::edge_size_type;
template<typename G> using edge_iterator_t = typename G::edge_iterator;
template<typename G> using const_edge_iterator_t = typename G::const_edge_iterator;
template<typename G> using edge_range_t = typename G::edge_range;
template<typename G> using const_edge_range_t = typename G::const_edge_range;
template<typename G> using vertex_edge_iterator_t = typename G::vertex_edge_iterator;
template<typename G> using const_vertex_edge_iterator_t = typename G::const_vertex_edge_iterator;
template<typename G> using vertex_edge_range_t = typename G::vertex_edge_range;
template<typename G> using const_vertex_edge_range_t = typename G::const_vertex_edge_range;
template<typename G> using vertex_vertex_iterator_t = typename G::vertex_vertex_iterator;
template<typename G> using const_vertex_vertex_iterator_t = typename G::const_vertex_vertex_iterator;
template<typename G> using vertex_vertex_range_t = typename G::vertex_vertex_range;
template<typename G> using const_vertex_vertex_range_t = typename G::const_vertex_vertex_range;

#ifdef CPO
///-------------------------------------------------------------------------------------
/// undirected_adjacency_list graph API
///

// Helper functions

//
// Uniform API: Common functions (accepts graph, vertex and edge)
//
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto graph_value(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>& g)
      -> const graph_value_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>& {
  return user_value(g);
}

//
// API vertex functions
//
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto vertex_id(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                   g,
                          const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> u)
      -> vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return static_cast<vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>>(
        u - g.vertices().begin());
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto vertex_value(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                   g,
                            vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> u)
      -> vertex_value_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>& {
  return user_value(*u);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto vertex_value(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                   g,
                            const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> u)
      -> const vertex_value_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>& {
  return user_value(*u);
}

// (clear, create_vertex, erase_vertex not supported because the graph is immutable)

//
// Uniform API: Edge functions
//

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto edge_id(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&         g,
                      const_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> edge_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv.edge_id(g);
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto edge_id(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                   g,
                      const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> u,
                      const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> v)
      -> edge_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return edge_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>(vertex_id(g, u), vertex_id(g, v));
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto edge_value(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                 g,
                          edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> edge_value_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>& {
  return user_value(*uv);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto edge_value(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                 g,
                          const_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> const edge_value_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>& {
  return user_value(*uv);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto vertex(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                         g,
                      edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>         uv,
                      const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> source)
      -> vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->other_vertex(g, source);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto vertex(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                   g,
                      const_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>   uv,
                      const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> source)
      -> const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->other_vertex(g, source);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto vertex(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                 g,
                      edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv,
                      vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>    source_id)
      -> vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->other_vertex(g, source_id);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto vertex(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                 g,
                      const_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv,
                      vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>          source_id)
      -> const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->other_vertex(g, source_id);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto
vertex_id(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                   g,
           const_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>   uv,
           const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> source)
      -> vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->other_vertex_id(g, source);
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto vertex_id(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                 g,
                          const_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv,
                          vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> source_id)
      -> vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->other_vertex_id(g, source_id);
}

// (create_edge & erase_edge)

//
// Uniform API: Graph-Vertex range functions
//

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto vertices(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>& g)
      -> vertex_range_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return g.vertices();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto vertices(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>& g)
      -> const_vertex_range_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return g.vertices();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto try_find_vertex(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&              g,
                           vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> id)
      -> vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return g.try_find_vertex(id);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto try_find_vertex(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&        g,
                           vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> id)
      -> const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return g.try_find_vertex(id);
}


template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void reserve_vertices(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&               g,
                      vertex_size_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> n) {
  g.reserve_vertices(n);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void resize_vertices(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&               g,
                     vertex_size_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> n) {
  g.resize(n);
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void resize_vertices(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                       g,
                     vertex_size_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>         n,
                     const vertex_value_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>& val) {
  g.resize(n, val);
}


//
// Uniform API: Graph-Edge range functions
//

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto edges(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>& g)
      -> edge_range_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return g.edges();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto edges(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>& g)
      -> const_edge_range_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return g.edges();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto find_edge(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                   g,
                         vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> u,
                         vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> v)
      -> edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  auto e = edges(g);
  for (auto uv = e.begin(); uv != e.end(); ++uv)
    if (vertex(g, uv, u) == v)
      return uv;
  return e.end();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto find_edge(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                   g,
                         const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> u,
                         const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> v)
      -> const_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  auto e = edges(g);
  for (auto uv = e.begin(); uv != e.end(); ++uv)
    if (vertex(g, uv, u) == v)
      return uv;
  return e.end();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto find_edge(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&              g,
                         vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uid,
                         vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> vid)
      -> edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return find_edge(g, try_find_vertex(g, uid), try_find_vertex(g, vid));
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto find_edge(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&        g,
                         vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uid,
                         vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> vid)
      -> const_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return find_edge(g, try_find_vertex(g, uid), try_find_vertex(g, vid));
}


//
// Uniform API: Vertex-Edge range functions
//

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto vertex(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                         g,
                      vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>  uv,
                      const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> source)
      -> vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->other_vertex(g, source);
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto vertex(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                        g,
                      const_vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv,
                      const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> source)
      -> const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->other_vertex(g, source);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto vertex(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                        g,
                      vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv,
                      vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>           source_id)
      -> vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->other_vertex(g, source_id);
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto vertex(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                        g,
                      const_vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv,
                      vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> source_id)
      -> const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->other_vertex(g, source_id);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto
vertex_id(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                        g,
           const_vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv,
           const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>      source)
      -> vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->other_vertex_id(g, source);
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto
vertex_id(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                        g,
           const_vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv,
           vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>                 source_id)
      -> vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->other_vertex_id(g, source_id);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto edge_value(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                        g,
                          vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> edge_value_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>& {
  return user_value(*uv);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto
edge_value(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                        g,
           const_vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> const edge_value_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>& {
  return user_value(*uv);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto edges(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                   g,
                     vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> u)
      -> vertex_edge_range_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return u->edges(g, vertex_id(g, u));
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto edges(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                   g,
                     const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> u)
      -> const_vertex_edge_range_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return u->edges(g, vertex_id(g, u));
}


template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto try_find_vertex_edge(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                   g,
                                    vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> u,
                                    vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> v)
      -> vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  auto e = edges(g, u);
  for (auto uv = begin(e); uv != end(e); ++uv)
    if (vertex(g, uv, u) == v)
      return uv;
  return end(e);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto
try_find_vertex_edge(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                    g,
                     const vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>& u,
                     const vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>& v)
      -> const_vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  auto e = edges(g, u);
  for (auto uv = begin(e); uv != end(e); ++uv)
    if (vertex(g, uv, u) == v)
      return uv;
  return end(e);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto try_find_vertex_edge(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&              g,
                                    vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uid,
                                    vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> vid)
      -> vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return try_find_vertex_edge(g, try_find_vertex(g, uid), try_find_vertex(g, vid));
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto try_find_vertex_edge(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&        g,
                                    vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uid,
                                    vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> vid)
      -> const_vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return try_find_vertex_edge(g, try_find_vertex(g, uid), try_find_vertex(g, vid));
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto erase_edge(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                        g,
                          vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  edge_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>* uv_ptr = &*uv;
  ++uv;
  delete uv_ptr;
  return uv;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto erase_edges(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                     g,
                           vertex_edge_range_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv_rng)
      -> vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {

  vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv = ranges::begin(uv_rng);
  while (uv != ranges::end(uv_rng))
    uv = erase_edge(g, uv);

  return uv;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr void clear_edges(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                   g,
                           vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> u) {
  u->clear_edges(g);
}


//
// Uniform API: Vertex-Vertex range functions
//
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto
vertex_id(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                          g,
           const_vertex_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> u)
      -> vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return u.other_vertex_id();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto vertices(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                   g,
                        vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> u)
      -> vertex_vertex_range_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return u->vertices(g, vertex_id(g, u));
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto vertices(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                   g,
                        const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> u)
      -> const_vertex_vertex_range_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return u->vertices(g, vertex_id(g, u));
}


//
// API edge functions
//

//
// Directed API (inward & outward)
//

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto target_vertex(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                 g,
                             edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->target_vertex(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto target_vertex(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                 g,
                             const_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->target_vertex(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto target_vertex(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                        g,
                             vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->target_vertex(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto
target_vertex(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                        g,
              const_vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->target_vertex(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto
target_vertex_id(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                 g,
                  const_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->target_vertex_id(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto
target_vertex_id(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                        g,
                  const_vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->target_vertex_id(g);
}


template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto source_vertex(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                 g,
                             edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->source_vertex(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto source_vertex(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                 g,
                             const_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->source_vertex(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto source_vertex(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                        g,
                             vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->source_vertex(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto
source_vertex(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                        g,
              const_vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->source_vertex(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto
source_vertex_id(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                 g,
                  const_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->list_owner_id();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr auto
source_vertex_id(const undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&                        g,
                  const_vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uv)
      -> vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> {
  return uv->list_owner_id();
}


//
// Directed API (outward): Vertex-Edge functions
//

// todo: outward_vertices


#  if 0
template <typename VV, typename EV, typename GV, typename IndexT, typename A>
constexpr auto vertex(undirected_adjacency_list<VV, EV, GV, IndexT, A>& g, vertex_edge_iterator_t<undirected_adjacency_list<VV, EV, GV, IndexT, A>> uv)
      -> vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, IndexT, A>> {
  return uv->target_vertex(g);
}

template <typename VV, typename EV, typename GV, typename IndexT, typename A>
constexpr auto vertex(const undirected_adjacency_list<VV, EV, GV, IndexT, A>&         g,
                      const_vertex_edge_t<undirected_adjacency_list<VV, EV, GV, IndexT, A>> uv)
      -> const_vertex_iterator_t<undirected_adjacency_list<VV, EV, GV, IndexT, A>> {
  return uv->target_vertex(g);
}

template <typename VV, typename EV, typename GV, typename IndexT, typename A>
constexpr auto vertex_id(const undirected_adjacency_list<VV, EV, GV, IndexT, A>&         g,
                          const_vertex_edge_t<undirected_adjacency_list<VV, EV, GV, IndexT, A>> uv)
      -> vertex_id_t<undirected_adjacency_list<VV, EV, GV, IndexT, A>> {
  return uv->target_vertex_id(g);
}
#  endif


//
// API graph functions
//

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
bool contains_vertex(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&              g,
                     vertex_id_t<undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>> uid) {
  return uid >= 0 && uid < g.vertices().size();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void clear(undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>& g) {
  g.clear();
}


//
// API: common container functions
//

#endif // CPO

} // namespace graph::container

#endif // UNDIRECTED_ADJ_LIST_API_HPP
