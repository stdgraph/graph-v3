/**
 * @file vertex_descriptor_view.hpp
 * @brief View over vertex storage that yields vertex descriptors
 */

#pragma once

#include "vertex_descriptor.hpp"
#include <ranges>
#include <iterator>

namespace graph::adj_list {

/**
 * @brief Forward-only view over vertex storage yielding vertex descriptors
 * 
 * This view wraps an underlying vertex container and provides forward iteration
 * that yields vertex_descriptor objects. Descriptors are synthesized on-the-fly,
 * making random access incompatible with the design.
 * 
 * @tparam VertexIter Iterator type of the underlying vertex container
 */
template<vertex_iterator VertexIter>
class vertex_descriptor_view : public std::ranges::view_interface<vertex_descriptor_view<VertexIter>> {
public:
    using vertex_desc = vertex_descriptor<VertexIter>;
    using storage_type = typename vertex_desc::storage_type;
    
    /**
     * @brief Forward iterator that yields vertex_descriptor values
     */
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = vertex_desc;
        using difference_type = std::ptrdiff_t;
        using pointer = const vertex_desc*;
        using reference = vertex_desc;
        
        constexpr iterator() noexcept = default;
        
        constexpr explicit iterator(storage_type pos) noexcept
            : current_(pos) {}
        
        // Dereference returns descriptor by value (synthesized on-the-fly)
        [[nodiscard]] constexpr vertex_desc operator*() const noexcept {
            return vertex_desc{current_};
        }
        
        // Pre-increment
        constexpr iterator& operator++() noexcept {
            ++current_;
            return *this;
        }
        
        // Post-increment
        constexpr iterator operator++(int) noexcept {
            iterator tmp = *this;
            ++current_;
            return tmp;
        }
        
        // Comparison
        [[nodiscard]] constexpr bool operator==(const iterator& other) const noexcept {
            return current_ == other.current_;
        }
        
    private:
        storage_type current_{};
    };
    
    using const_iterator = iterator;
    
    // Default constructor
    constexpr vertex_descriptor_view() noexcept = default;
    
    // Copy constructor
    constexpr vertex_descriptor_view(const vertex_descriptor_view&) noexcept = default;
    
    /**
     * @brief Construct view from iterator range
     * @param begin_val Starting iterator/index
     * @param end_val Ending iterator/index
     * 
     * Note: Requires random access iterator so size can be calculated.
     */
    constexpr vertex_descriptor_view(storage_type begin_val, storage_type end_val) noexcept
        requires std::random_access_iterator<VertexIter>
        : begin_(begin_val), end_(end_val), size_(end_val - begin_val) {}
    
    /**
     * @brief Construct view from non-const container with begin/end methods
     * @param container The underlying container
     */
    template<typename Container>
        requires requires(Container& c) {
            { c.begin() } -> std::convertible_to<VertexIter>;
            { c.end() } -> std::convertible_to<VertexIter>;
        } && (std::ranges::sized_range<Container> || std::random_access_iterator<VertexIter>)
    constexpr explicit vertex_descriptor_view(Container& container) noexcept {
        if constexpr (std::random_access_iterator<VertexIter>) {
            begin_ = 0;
            end_ = static_cast<storage_type>(std::ranges::size(container));
        } else {
            // Must be sized_range (enforced by requires clause)
            begin_ = container.begin();
            end_ = container.end();
        }
        size_ = std::ranges::size(container);
    }
    
    /**
     * @brief Construct view from const container with begin/end methods
     * @param container The underlying const container
     * 
     * When constructed from a const container, the view will yield descriptors
     * that preserve const semantics through their underlying_value() and inner_value() methods.
     */
    template<typename Container>
        requires requires(const Container& c) {
            { c.begin() } -> std::convertible_to<VertexIter>;
            { c.end() } -> std::convertible_to<VertexIter>;
        } && (std::ranges::sized_range<const Container> || std::random_access_iterator<VertexIter>)
    constexpr explicit vertex_descriptor_view(const Container& container) noexcept {
        if constexpr (std::random_access_iterator<VertexIter>) {
            begin_ = 0;
            end_ = static_cast<storage_type>(std::ranges::size(container));
        } else {
            // Must be sized_range (enforced by requires clause)
            begin_ = container.begin();
            end_ = container.end();
        }
        size_ = std::ranges::size(container);
    }
    
    [[nodiscard]] constexpr iterator begin() const noexcept {
        return iterator{begin_};
    }
    
    [[nodiscard]] constexpr iterator end() const noexcept {
        return iterator{end_};
    }
    
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
        return begin();
    }
    
    [[nodiscard]] constexpr const_iterator cend() const noexcept {
        return end();
    }
    
    // Size - always available (required for sized_range concept)
    // O(1) - returns cached size from construction
    [[nodiscard]] constexpr auto size() const noexcept {
        return size_;
    }
    
private:
    storage_type begin_{};
    storage_type end_{};
    std::size_t size_{};      // Cached size from sized_range containers or calculated from random_access iterators
};

// Deduction guides
template<typename Container>
vertex_descriptor_view(Container&) -> vertex_descriptor_view<typename Container::iterator>;

template<typename Container>
vertex_descriptor_view(const Container&) -> vertex_descriptor_view<typename Container::const_iterator>;

} // namespace graph::adj_list

// Enable borrowed_range for vertex_descriptor_view since it doesn't own the data
template<typename VertexIter>
inline constexpr bool std::ranges::enable_borrowed_range<graph::adj_list::vertex_descriptor_view<VertexIter>> = true;
