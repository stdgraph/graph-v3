/**
 * @file edgelist.hpp
 * @brief Edge list container and utilities
 * 
 * This file contains the edge list container - a simple, flexible
 * representation of a graph as a list of edges.
 */

#pragma once

#include "adj_list/descriptor.hpp"
#include "detail/graph_using.hpp"
#include <vector>
#include <utility>
#include <ranges>

namespace graph {

/**
 * @brief Simple edge representation with source and target vertices
 * 
 * @tparam VId Vertex identifier type
 * @tparam EV Edge value type (default: empty)
 */
template<typename VId, typename EV = void>
struct edge {
    VId source;
    VId target;
    
    // Edge value storage (when EV is not void)
    [[no_unique_address]] 
    std::conditional_t<std::is_void_v<EV>, std::monostate, EV> value;
    
    constexpr edge() = default;
    
    // Constructor for edges without values
    constexpr edge(VId src, VId tgt) 
        requires std::is_void_v<EV>
        : source(src), target(tgt) {}
    
    // Constructor for edges with values
    constexpr edge(VId src, VId tgt, const EV& val)
        requires (!std::is_void_v<EV>)
        : source(src), target(tgt), value(val) {}
    
    constexpr edge(VId src, VId tgt, EV&& val)
        requires (!std::is_void_v<EV>)
        : source(src), target(tgt), value(std::move(val)) {}
};

// Deduction guides
template<typename VId>
edge(VId, VId) -> edge<VId, void>;

template<typename VId, typename EV>
edge(VId, VId, EV) -> edge<VId, EV>;

/**
 * @brief Edge list graph container
 * 
 * A simple graph representation as a vector of edges.
 * Suitable for algorithms that need to iterate over all edges.
 * 
 * @tparam VId Vertex identifier type
 * @tparam EV Edge value type
 * @tparam Alloc Allocator type
 */
template<typename VId = size_t, 
         typename EV = void,
         typename Alloc = std::allocator<edge<VId, EV>>>
class edgelist {
public:
    using vertex_id_type = VId;
    using edge_value_type = EV;
    using edge_type = edge<VId, EV>;
    using container_type = std::vector<edge_type, Alloc>;
    using size_type = typename container_type::size_type;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;
    
private:
    container_type edges_;
    size_type num_vertices_ = 0;
    
public:
    // Constructors
    edgelist() = default;
    explicit edgelist(size_type num_verts) : num_vertices_(num_verts) {}
    
    // Edge access
    auto begin() { return edges_.begin(); }
    auto end() { return edges_.end(); }
    auto begin() const { return edges_.begin(); }
    auto end() const { return edges_.end(); }
    auto cbegin() const { return edges_.cbegin(); }
    auto cend() const { return edges_.cend(); }
    
    // Size queries
    size_type num_edges() const { return edges_.size(); }
    size_type num_vertices() const { return num_vertices_; }
    bool empty() const { return edges_.empty(); }
    
    // Modifiers
    void add_edge(VId src, VId tgt) requires std::is_void_v<EV> {
        edges_.emplace_back(src, tgt);
        num_vertices_ = std::max(num_vertices_, 
                                 static_cast<size_type>(std::max(src, tgt) + 1));
    }
    
    void add_edge(VId src, VId tgt, const EV& val) requires (!std::is_void_v<EV>) {
        edges_.emplace_back(src, tgt, val);
        num_vertices_ = std::max(num_vertices_, 
                                 static_cast<size_type>(std::max(src, tgt) + 1));
    }
    
    void add_edge(VId src, VId tgt, EV&& val) requires (!std::is_void_v<EV>) {
        edges_.emplace_back(src, tgt, std::move(val));
        num_vertices_ = std::max(num_vertices_, 
                                 static_cast<size_type>(std::max(src, tgt) + 1));
    }
    
    void reserve(size_type n) { edges_.reserve(n); }
    void clear() { edges_.clear(); }
    
    void set_num_vertices(size_type n) { num_vertices_ = n; }
};

} // namespace graph
