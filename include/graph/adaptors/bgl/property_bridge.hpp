#pragma once

#include <concepts>
#include <functional>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace graph::bgl {

// ── Readable property-map wrapper ───────────────────────────────────────────
//
// Wraps any BGL readable property map into a graph-v3 edge_value_function or
// vertex_value_function. The KeyExtractor converts from the graph-v3 descriptor
// to the BGL property-map key.

template <typename PropertyMap, typename KeyExtractor>
struct bgl_readable_property_map_fn {
  PropertyMap  property_map;
  KeyExtractor key_extractor;

  template <typename AdaptedGraph, typename Descriptor>
  decltype(auto) operator()(const AdaptedGraph&, const Descriptor& descriptor) const {
    return get(property_map, key_extractor(descriptor));
  }
};

template <typename PropertyMap, typename KeyExtractor>
auto make_bgl_readable_property_map_fn(PropertyMap pm, KeyExtractor key_extractor) {
  return bgl_readable_property_map_fn<PropertyMap, KeyExtractor>{pm, key_extractor};
}

// ── Lvalue (writable) property-map wrapper ──────────────────────────────────
//
// Same as the readable wrapper but intended for BGL property maps whose get()
// returns a stable lvalue reference. graph-v3's writable property functions
// must return a reference so the caller can assign through it.

template <typename PropertyMap, typename KeyExtractor>
struct bgl_lvalue_property_map_fn {
  PropertyMap  property_map;
  KeyExtractor key_extractor;

  template <typename AdaptedGraph, typename Descriptor>
  decltype(auto) operator()(const AdaptedGraph&, const Descriptor& descriptor) const {
    auto&& result = get(property_map, key_extractor(descriptor));
    static_assert(std::is_lvalue_reference_v<decltype(result)>,
                  "bgl_lvalue_property_map_fn requires get(pm, key) to return an lvalue reference");
    return result;
  }
};

template <typename PropertyMap, typename KeyExtractor>
auto make_bgl_lvalue_property_map_fn(PropertyMap pm, KeyExtractor key_extractor) {
  return bgl_lvalue_property_map_fn<PropertyMap, KeyExtractor>{pm, key_extractor};
}

// ── Edge key extractor ──────────────────────────────────────────────────────
//
// Extracts the BGL edge_descriptor from a graph-v3 edge_descriptor by
// dereferencing the wrapped bgl_edge_iterator stored in value().

struct edge_key_extractor {
  template <typename EdgeDescriptor>
  auto operator()(const EdgeDescriptor& uv) const {
    return *uv.value();  // return by value — uv.value() may be a temporary iterator
  }
};

// ── Convenience: make edge weight function from BGL property map ────────────

template <typename PropertyMap>
auto make_bgl_edge_weight_fn(PropertyMap pm) {
  return make_bgl_readable_property_map_fn(pm, edge_key_extractor{});
}

// ── Vertex-indexed vector property function ─────────────────────────────────
//
// Wraps a std::vector<T>& into a function object that satisfies
// vertex_property_fn_for: (const G&, vertex_id_t<G>) -> T&.
// Used for distance and predecessor storage with Dijkstra.

template <typename T>
struct vertex_vector_property_fn {
  std::vector<T>* storage;

  template <typename G, typename VId>
  T& operator()(const G&, const VId& uid) const {
    return (*storage)[static_cast<std::size_t>(uid)];
  }
};

template <typename T>
auto make_vertex_id_property_fn(std::vector<T>& vec) {
  return vertex_vector_property_fn<T>{&vec};
}

// ── Vertex-keyed map property function ──────────────────────────────────────
//
// For graphs with non-integral vertex IDs (e.g. BGL listS/setS where
// vertex_descriptor is void*), wraps any associative container (map,
// unordered_map, etc.) into a function object usable as a graph-v3 vertex
// property function.

template <typename Map>
struct vertex_map_property_fn {
  Map* storage;

  template <typename G, typename VId>
  auto& operator()(const G&, const VId& uid) const {
    return (*storage)[uid];
  }
};

/// Create a vertex property function from any associative container
/// (std::map, std::unordered_map, etc.) keyed by vertex ID.
template <typename Map>
auto make_vertex_map_property_fn(Map& map) {
  return vertex_map_property_fn<Map>{&map};
}

} // namespace graph::bgl
