// Copyright (C) 2025 Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// This example demonstrates how to adapt an existing third-party graph container
// for use with the graph-v3 library using Customization Point Objects (CPOs).
//
// graph-v3 dispatches all graph operations (vertex iteration, edge access, ID
// extraction, …) through CPOs rather than through a traits specialisation or
// virtual functions.  A type satisfies graph::adjacency_list<G> once a small
// set of ADL-findable free functions has been provided in the type's own
// namespace.  No changes to the third-party type itself are required.
//
// The four functions below are all that is needed:
//
//   vertices(g)           — range of vertices, wrapped into a vertex_descriptor_view
//   edges(g, u)           — out-edge range for a vertex_descriptor u
//   target_id(g, uv)      — target vertex ID from an edge_descriptor uv
//   vertex_value(g, u)    — user-visible vertex data (MyVertex) for a vertex_descriptor u
//
// The trailing-return-type SFINAE pattern keeps the templates narrow: they are
// excluded from overload resolution automatically when the argument does not
// have the expected interface (e.g. when u is a plain integer rather than a
// vertex_descriptor, or uv is not an edge_descriptor).

#include <graph/graph.hpp>  // vertices, out_edges, vertex_value, vertexlist,
                            // vertices_dfs, index_adjacency_list, …
#include <iostream>
#include <ranges>
#include <string>
#include <vector>


namespace MyLibrary { // ── Third-party graph container (left unchanged) ────────

struct MyEdge {
  std::string content;
  int         indexOfTarget;
};

struct MyVertex {
  std::string         content;
  std::vector<MyEdge> outEdges;
};

class MyGraph {
  std::vector<MyVertex> _vertices;

public:
  MyVertex const* getVertexByIndex(int index) const { return &_vertices[static_cast<unsigned>(index)]; }

  std::vector<MyVertex> const& getAllVertices() const { return _vertices; }

  void setTopology(std::vector<MyVertex> t) { _vertices = std::move(t); }
};

} // namespace MyLibrary


namespace MyLibrary { // ── graph-v3 CPO customisation (unintrusive) ────────────

// ---------------------------------------------------------------------------
// vertices(g)
//   Returns a range over all vertices.  Because std::vector::const_iterator
//   is a random-access iterator, the CPO wraps the result in a
//   vertex_descriptor_view whose storage_type is size_t.  Vertex IDs are
//   therefore contiguous integral indices, and find_vertex / num_vertices
//   work via their built-in random-access defaults — no extra code needed.
// ---------------------------------------------------------------------------
auto vertices(const MyGraph& g) {
  return std::views::all(g.getAllVertices());
}

// ---------------------------------------------------------------------------
// edges(g, u)  — u is a graph-v3 vertex_descriptor (stores a size_t index)
//   The trailing return type makes the function SFINAE-friendly: it is
//   excluded from overload resolution for argument types that lack vertex_id()
//   (e.g. a plain size_t), so the CPO's built-in default path (which calls
//   find_vertex then edges(g, descriptor)) remains available.
//   The CPO wraps the returned range in an edge_descriptor_view automatically.
// ---------------------------------------------------------------------------
template <class VDesc>
auto edges(const MyGraph& g, const VDesc& u)
    -> decltype(std::views::all(g.getAllVertices()[u.vertex_id()].outEdges)) {
  return std::views::all(g.getAllVertices()[u.vertex_id()].outEdges);
}

// ---------------------------------------------------------------------------
// target_id(g, uv)  — uv is a graph-v3 edge_descriptor
//   uv.value() returns an iterator to the underlying MyEdge.  Dereferencing
//   it gives the MyEdge from which we read indexOfTarget.
//   The trailing return type restricts the function to edge descriptors that
//   expose a value() iterator (SFINAE).
// ---------------------------------------------------------------------------
template <class EdgeDesc>
auto target_id(const MyGraph&, const EdgeDesc& uv)
    -> decltype((*uv.value()).indexOfTarget) {
  return (*uv.value()).indexOfTarget;
}

// ---------------------------------------------------------------------------
// vertex_value(g, u)  — u is a graph-v3 vertex_descriptor
//   Returns the MyVertex data for use in display or further inspection.
//   The default CPO path (u.inner_value(g)) would call MyGraph::operator[],
//   which MyGraph does not provide, so we supply this ADL override instead.
// ---------------------------------------------------------------------------
template <class VDesc>
auto vertex_value(const MyGraph& g, const VDesc& u)
    -> decltype(g.getAllVertices()[u.vertex_id()]) {
  return g.getAllVertices()[u.vertex_id()];
}

} // namespace MyLibrary


int main() {
  // Verify concept satisfaction at compile time.
  // index_adjacency_list is required by vertices_dfs (integral vertex IDs,
  // random-access vertex storage).
  static_assert(graph::adjacency_list<MyLibrary::MyGraph>);
  static_assert(graph::index_adjacency_list<MyLibrary::MyGraph>);

  const MyLibrary::MyGraph g = []{  // ── build the graph ───────────────────
    MyLibrary::MyGraph               r;
    std::vector<MyLibrary::MyVertex> topo{
          //         A        |
          /*0*/ {"A", {{"", 1}, {"", 2}}}, //       /  \       |
          /*1*/ {"B", {{"", 3}}},          //      B    C      |
          /*2*/ {"C", {{"", 3}}},          //       \  /       |
          /*3*/ {"D", {}}                  //        D         |
    };
    r.setTopology(std::move(topo));
    return r;
  }();

  // DFS traversal starting from vertex 0.
  //
  // vertices_dfs yields one vertex_descriptor per visited vertex via a
  // structured binding [v].  graph::vertex_value(g, v) fetches the
  // associated MyVertex so we can access its 'content' field.
  //
  // graph-v3 v2 used [vid, v] where v was a raw vertex reference.
  // In v3, v is an opaque vertex_descriptor; content is accessed via the
  // vertex_value CPO.
  for (auto [v] : graph::views::vertices_dfs(g, std::size_t{0}))
    std::cout << graph::vertex_value(g, v).content << " ";

  std::cout << "\n";

  // Expected output: A B D C   (or A C D B, depending on DFS tie-breaking)
  return 0;
}
