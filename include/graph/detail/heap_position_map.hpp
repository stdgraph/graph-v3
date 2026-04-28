/**
 * @file heap_position_map.hpp
 * @brief Position-map adapters for indexed_dary_heap.
 *
 * Two adapters are provided:
 *
 *   - vector_position_map  : O(1) lookup for integral keys in a known dense
 *                            range [0, n). Backed by a caller-owned
 *                            std::vector<size_t>.
 *
 *   - assoc_position_map   : O(1) average lookup for arbitrary hashable keys.
 *                            Backed by a caller-owned std::unordered_map<Key, size_t>.
 *                            Use this when vertex ids are sparse, non-integral,
 *                            or come from a mapped graph container.
 *
 * Both adapters store a pointer to their backing storage; the storage must
 * outlive the heap. This lets the caller reuse the same map across multiple
 * Dijkstra runs (call reset() between runs).
 *
 * Concept (informal):
 *   - sentinel: static constexpr size_t npos
 *   - size_t  position(Key) const          // returns npos if not present
 *   - void    set_position(Key, size_t)    // npos means "remove"
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <unordered_map>
#include <vector>

namespace graph::detail {

// ---------------------------------------------------------------------------
// vector_position_map
//
// O(1) position map for integral keys in [0, n). The caller owns the storage
// vector, sized to n and initialised to npos. set_position(k, npos) marks k
// as absent. reset() clears the entire map in O(n).
// ---------------------------------------------------------------------------

class vector_position_map {
public:
  static constexpr std::size_t npos = static_cast<std::size_t>(-1);

  explicit vector_position_map(std::vector<std::size_t>& storage) noexcept
        : storage_(&storage) {}

  template <class Key>
  [[nodiscard]] std::size_t position(const Key& k) const noexcept {
    return (*storage_)[static_cast<std::size_t>(k)];
  }

  template <class Key>
  void set_position(const Key& k, std::size_t pos) noexcept {
    (*storage_)[static_cast<std::size_t>(k)] = pos;
  }

  /// Reset all entries to npos. O(n).
  void reset() noexcept { std::fill(storage_->begin(), storage_->end(), npos); }

  [[nodiscard]] std::size_t capacity() const noexcept { return storage_->size(); }

private:
  std::vector<std::size_t>* storage_;
};

// ---------------------------------------------------------------------------
// assoc_position_map
//
// O(1) average position map for hashable keys (e.g. when vertex ids come from
// a mapped graph and are non-contiguous, or non-integral entirely).
//
// Storage is a caller-owned std::unordered_map<Key, size_t>. set_position
// with npos erases the key, keeping the map's size equal to the heap's size
// at all times — so contains(k) reduces to a single lookup.
// ---------------------------------------------------------------------------

template <class Key,
          class Hash    = std::hash<Key>,
          class KeyEq   = std::equal_to<Key>,
          class Alloc   = std::allocator<std::pair<const Key, std::size_t>>>
class assoc_position_map {
public:
  using map_type                      = std::unordered_map<Key, std::size_t, Hash, KeyEq, Alloc>;
  static constexpr std::size_t npos   = static_cast<std::size_t>(-1);

  explicit assoc_position_map(map_type& storage) noexcept : storage_(&storage) {}

  [[nodiscard]] std::size_t position(const Key& k) const {
    auto it = storage_->find(k);
    return (it == storage_->end()) ? npos : it->second;
  }

  void set_position(const Key& k, std::size_t pos) {
    if (pos == npos) {
      storage_->erase(k);
    } else {
      // Use insert_or_assign for O(1) amortised update with no temporary.
      storage_->insert_or_assign(k, pos);
    }
  }

  /// Drop all entries. O(n).
  void reset() noexcept(noexcept(storage_->clear())) { storage_->clear(); }

  [[nodiscard]] std::size_t tracked_size() const noexcept { return storage_->size(); }

private:
  map_type* storage_;
};

} // namespace graph::detail
