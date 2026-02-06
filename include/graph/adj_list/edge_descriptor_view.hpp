/**
 * @file edge_descriptor_view.hpp
 * @brief View over edge storage that yields edge descriptors
 */

#pragma once

#include "edge_descriptor.hpp"
#include <ranges>
#include <iterator>

namespace graph::adj_list {

/**
 * @brief Forward-only view over edge storage yielding edge descriptors
 * 
 * This view wraps an underlying edge container and provides forward iteration
 * that yields edge_descriptor objects. Supports both per-vertex adjacency storage
 * and global edge storage configurations.
 * 
 * @tparam EdgeIter Iterator type of the underlying edge container
 * @tparam VertexIter Iterator type of the vertex container
 */
template<edge_iterator EdgeIter, vertex_iterator VertexIter>
class edge_descriptor_view : public std::ranges::view_interface<edge_descriptor_view<EdgeIter, VertexIter>> {
public:
    using edge_desc = edge_descriptor<EdgeIter, VertexIter>;
    using vertex_desc = vertex_descriptor<VertexIter>;
    using edge_storage_type = typename edge_desc::edge_storage_type;
    
    /**
     * @brief Forward iterator that yields edge_descriptor values
     */
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = edge_desc;
        using difference_type = std::ptrdiff_t;
        using pointer = const edge_desc*;
        using reference = edge_desc;
        
        constexpr iterator() noexcept = default;
        
        constexpr iterator(edge_storage_type edge_pos, vertex_desc source) noexcept
            : current_edge_(edge_pos), source_(source) {}
        
        // Dereference returns edge descriptor by value (synthesized on-the-fly)
        [[nodiscard]] constexpr edge_desc operator*() const noexcept {
            return edge_desc{current_edge_, source_};
        }
        
        // Pre-increment
        constexpr iterator& operator++() noexcept {
            ++current_edge_;
            return *this;
        }
        
        // Post-increment
        constexpr iterator operator++(int) noexcept {
            iterator tmp = *this;
            ++current_edge_;
            return tmp;
        }
        
        // Comparison
        [[nodiscard]] constexpr bool operator==(const iterator& other) const noexcept {
            return current_edge_ == other.current_edge_;
        }
        
    private:
        edge_storage_type current_edge_{};
        vertex_desc source_{};
    };
    
    using const_iterator = iterator;
    
    // Default constructor
    constexpr edge_descriptor_view() noexcept = default;
    
    // Copy constructor
    constexpr edge_descriptor_view(const edge_descriptor_view&) noexcept = default;
    
    /**
     * @brief Construct view from edge storage range and source vertex
     * @param begin_val Starting edge iterator/index
     * @param end_val Ending edge iterator/index
     * @param source Source vertex descriptor for all edges in this view
     */
    constexpr edge_descriptor_view(edge_storage_type begin_val, edge_storage_type end_val, vertex_desc source) noexcept
        : begin_(begin_val), end_(end_val), source_(source) {
        if constexpr (std::random_access_iterator<EdgeIter>) {
            size_ = end_val - begin_val;
        } else {
            size_ = static_cast<std::size_t>(std::distance(begin_val, end_val));
        }
    }
    
    /**
     * @brief Construct view from non-const edge container and source vertex (per-vertex adjacency)
     * @param container The underlying edge container
     * @param source The source vertex for all edges in this container
     */
    template<typename Container>
        requires requires(Container& c) {
            { c.begin() } -> std::convertible_to<EdgeIter>;
            { c.end() } -> std::convertible_to<EdgeIter>;
        }
    constexpr edge_descriptor_view(Container& container, vertex_desc source) noexcept
        : source_(source) {
        if constexpr (std::random_access_iterator<EdgeIter>) {
            begin_ = 0;
            end_ = static_cast<edge_storage_type>(container.size());
            size_ = container.size();
        } else {
            begin_ = container.begin();
            end_ = container.end();
            size_ = static_cast<std::size_t>(std::distance(begin_, end_));
        }
    }
    
    /**
     * @brief Construct view from const edge container and source vertex (per-vertex adjacency)
     * @param container The underlying const edge container
     * @param source The source vertex for all edges in this container
     * 
     * When constructed from a const container, the view will yield edge descriptors
     * that preserve const semantics through their underlying_value() and inner_value() methods.
     */
    template<typename Container>
        requires requires(const Container& c) {
            { c.begin() } -> std::convertible_to<EdgeIter>;
            { c.end() } -> std::convertible_to<EdgeIter>;
        }
    constexpr edge_descriptor_view(const Container& container, vertex_desc source) noexcept
        : source_(source) {
        if constexpr (std::random_access_iterator<EdgeIter>) {
            begin_ = 0;
            end_ = static_cast<edge_storage_type>(container.size());
            size_ = container.size();
        } else {
            begin_ = container.begin();
            end_ = container.end();
            size_ = static_cast<std::size_t>(std::distance(begin_, end_));
        }
    }
    
    [[nodiscard]] constexpr iterator begin() const noexcept {
        return iterator{begin_, source_};
    }
    
    [[nodiscard]] constexpr iterator end() const noexcept {
        return iterator{end_, source_};
    }
    
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
        return begin();
    }
    
    [[nodiscard]] constexpr const_iterator cend() const noexcept {
        return end();
    }
    
    // Size - O(1) for all iterator types (cached during construction)
    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return size_;
    }
    
    // Get the source vertex for this view
    [[nodiscard]] constexpr vertex_desc source() const noexcept {
        return source_;
    }
    
private:
    edge_storage_type begin_{};
    edge_storage_type end_{};
    vertex_desc source_{};
    std::size_t size_{0};
};

// Deduction guides for per-vertex adjacency
template<typename Container, typename VertexDesc>
    requires requires { typename Container::iterator; }
edge_descriptor_view(Container&, VertexDesc) 
    -> edge_descriptor_view<typename Container::iterator, typename VertexDesc::iterator_type>;

template<typename Container, typename VertexDesc>
    requires requires { typename Container::const_iterator; }
edge_descriptor_view(const Container&, VertexDesc) 
    -> edge_descriptor_view<typename Container::const_iterator, typename VertexDesc::iterator_type>;

// Deduction guide for ranges (including subrange which lacks ::iterator typedef)
template<typename Range, typename VertexDesc>
    requires std::ranges::range<Range> && (!requires { typename std::remove_cvref_t<Range>::iterator; })
edge_descriptor_view(Range&&, VertexDesc)
    -> edge_descriptor_view<std::ranges::iterator_t<Range>, typename VertexDesc::iterator_type>;

} // namespace graph::adj_list

// Enable borrowed_range for edge_descriptor_view to allow std::ranges operations on temporaries
template<typename EdgeIter, typename VertexIter>
inline constexpr bool std::ranges::enable_borrowed_range<graph::adj_list::edge_descriptor_view<EdgeIter, VertexIter>> = true;
