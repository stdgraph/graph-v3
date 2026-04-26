/**
 * @file indexed_dary_heap.hpp
 * @brief External-key, indirect-comparison d-ary min-heap with O(log_d N)
 *        decrease-key.
 *
 * Designed for Dijkstra and Prim where:
 *   - Vertex ids serve as stable external keys.
 *   - Distances live in a user-supplied container, accessed via a callable
 *     @c DistanceFn(Key) -> const Distance& .
 *   - The relax step needs O(log_d N) `decrease(key)` rather than O(N) re-push.
 *
 * The heap stores keys only. Distances are read live through @c DistanceFn so
 * the heap never goes stale: when the algorithm updates a distance and calls
 * @c decrease(k), the heap re-orders k against the current distance values.
 *
 * Position bookkeeping (key → heap index) is delegated to a @c PositionMap.
 * A heap of N entries always has exactly one position recorded per contained
 * key; non-contained keys map to @c npos. Every write to @c heap_[i] funnels
 * through @c place_() to keep the map in sync.
 *
 * Complexity (Arity = d):
 *   - push     : O(log_d N)
 *   - pop      : O(d · log_d N)
 *   - decrease : O(log_d N)
 *   - top      : O(1)
 *   - contains : O(1) lookup in the position map
 *
 * d = 4 minimises the product (d / log d) on typical Dijkstra workloads;
 * see Boost.Graph's `d_ary_heap_indirect` and references therein.
 *
 * Concept-style requirements on @c PositionMap:
 *   std::size_t pm.position(Key) const;          // returns indexed_dary_heap::npos if not present
 *   void        pm.set_position(Key, std::size_t);
 *
 * Two adapters are provided in @c heap_position_map.hpp:
 *   - @c vector_position_map  for dense integral keys
 *   - @c assoc_position_map   for sparse / hashable keys
 */

#pragma once

#include "heap_position_map.hpp"

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace graph::detail {

// ---------------------------------------------------------------------------
// indexed_dary_heap
// ---------------------------------------------------------------------------

template <class Key,
          class DistanceFn,
          class Compare,
          class PositionMap,
          std::size_t Arity     = 4,
          class Allocator       = std::allocator<Key>>
class indexed_dary_heap {
  static_assert(Arity >= 2, "Arity must be at least 2");

public:
  using key_type        = Key;
  using size_type       = std::size_t;
  using distance_fn     = DistanceFn;
  using compare_type    = Compare;
  using position_map    = PositionMap;
  using allocator_type  = Allocator;

  static constexpr size_type arity = Arity;
  static constexpr size_type npos  = static_cast<size_type>(-1);

  indexed_dary_heap(DistanceFn dist, Compare comp, PositionMap pmap,
                    const Allocator& alloc = Allocator())
        : heap_(alloc), distance_(std::move(dist)), compare_(std::move(comp)),
          position_(std::move(pmap)) {}

  // ----- size / state ----------------------------------------------------

  [[nodiscard]] bool      empty() const noexcept { return heap_.empty(); }
  [[nodiscard]] size_type size()  const noexcept { return heap_.size(); }

  void reserve(size_type n) { heap_.reserve(n); }

  /// Remove all entries. Resets each contained key's position to npos.
  void clear() noexcept {
    for (const auto& k : heap_) {
      position_.set_position(k, npos);
    }
    heap_.clear();
  }

  // ----- queries ---------------------------------------------------------

  /// O(1). Returns the key with the smallest distance under @c Compare.
  /// Precondition: !empty().
  [[nodiscard]] const Key& top() const noexcept { return heap_.front(); }

  /// O(1). True iff @c k is currently in the heap.
  [[nodiscard]] bool contains(const Key& k) const noexcept {
    return position_.position(k) != npos;
  }

  // ----- modifiers -------------------------------------------------------

  /// O(log_d N). Insert @c k. Behaviour is undefined if @c k is already
  /// present — callers should use @c decrease() for re-insertions.
  void push(const Key& k) {
    const size_type i = heap_.size();
    heap_.push_back(k);
    position_.set_position(k, i);
    sift_up_(i);
  }

  /// O(d · log_d N). Remove the top element.
  /// Precondition: !empty().
  void pop() {
    const Key removed = heap_.front();
    position_.set_position(removed, npos);

    const size_type last = heap_.size() - 1;
    if (last == 0) {
      heap_.pop_back();
      return;
    }
    // Move last → root, then sift down.
    place_(0, heap_[last]);
    heap_.pop_back();
    sift_down_(0);
  }

  /// O(log_d N). Notify the heap that @c k's distance has decreased
  /// (under @c Compare). Sifts @c k up only.
  /// Precondition: contains(k).
  void decrease(const Key& k) {
    const size_type i = position_.position(k);
    sift_up_(i);
  }

  /// Equivalent to @c push(k) if !contains(k), else @c decrease(k).
  /// Convenience wrapper for the common Dijkstra relax pattern.
  void push_or_decrease(const Key& k) {
    const size_type i = position_.position(k);
    if (i == npos) {
      push(k);
    } else {
      sift_up_(i);
    }
  }

  // ----- accessors (mostly for testing / introspection) ------------------

  [[nodiscard]] const PositionMap& position_map_ref() const noexcept { return position_; }
  [[nodiscard]] PositionMap&       position_map_ref()       noexcept { return position_; }

private:
  // -----------------------------------------------------------------------
  // Heap topology helpers
  // -----------------------------------------------------------------------

  static constexpr size_type parent_of_(size_type i) noexcept {
    return (i - 1) / Arity;
  }
  static constexpr size_type first_child_of_(size_type i) noexcept {
    return Arity * i + 1;
  }

  /// Place @c k at index @c i and update the position map. Single point of
  /// truth for `heap_[i] = k` — guarantees position_ stays consistent.
  void place_(size_type i, const Key& k) {
    heap_[i] = k;
    position_.set_position(k, i);
  }

  /// Strict-less wrapper using the user's Compare on distances.
  [[nodiscard]] bool less_than_(const Key& a, const Key& b) const {
    return compare_(distance_(a), distance_(b));
  }

  // -----------------------------------------------------------------------
  // Sift operations
  //
  // Implemented "hole-style": instead of swap-walking, we pull the original
  // value out, walk the hole, then drop the value into its final slot. Saves
  // one write per level vs. a naive swap loop.
  // -----------------------------------------------------------------------

  void sift_up_(size_type i) {
    if (i == 0) return;
    const Key k = heap_[i];
    while (i > 0) {
      const size_type p = parent_of_(i);
      if (!compare_(distance_(k), distance_(heap_[p]))) {
        break;
      }
      place_(i, heap_[p]); // move parent down into the hole
      i = p;
    }
    place_(i, k);
  }

  void sift_down_(size_type i) {
    const size_type n = heap_.size();
    if (n == 0) return;
    const Key k = heap_[i];

    while (true) {
      const size_type first = first_child_of_(i);
      if (first >= n) break;

      // Find the smallest child in [first, first + Arity).
      const size_type last  = (first + Arity < n) ? first + Arity : n;
      size_type       best  = first;
      for (size_type c = first + 1; c < last; ++c) {
        if (compare_(distance_(heap_[c]), distance_(heap_[best]))) {
          best = c;
        }
      }

      if (!compare_(distance_(heap_[best]), distance_(k))) {
        break; // k is no greater than its smallest child → done
      }
      place_(i, heap_[best]); // promote the smallest child into the hole
      i = best;
    }
    place_(i, k);
  }

  std::vector<Key, Allocator> heap_;
  DistanceFn                  distance_;
  Compare                     compare_;
  PositionMap                 position_;
};

} // namespace graph::detail
