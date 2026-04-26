/**
 * @file test_indexed_dary_heap.cpp
 * @brief Catch2 tests for graph::detail::indexed_dary_heap.
 *
 * Coverage:
 *   - Construction, empty, size
 *   - push / pop ordering (ascending and descending input)
 *   - decrease-key (single and repeated)
 *   - contains / clear
 *   - Both arity 2 and arity 4
 *   - Custom comparator (max-heap via std::greater)
 *   - Both position-map adapters: vector_position_map, assoc_position_map
 *   - Random stress (1 000 keys + 500 decrease-key ops)
 *   - push_or_decrease convenience
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/detail/indexed_dary_heap.hpp>
#include <graph/detail/heap_position_map.hpp>

#include <functional>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

using graph::detail::indexed_dary_heap;
using graph::detail::vector_position_map;
using graph::detail::assoc_position_map;

namespace {

// Helper: drain a heap into a vector of keys, preserving pop order.
template <class Heap>
std::vector<typename Heap::key_type> drain(Heap& h) {
  std::vector<typename Heap::key_type> out;
  while (!h.empty()) {
    out.push_back(h.top());
    h.pop();
  }
  return out;
}

// Build a min-heap with vector_position_map over [0, dist.size()).
template <std::size_t Arity = 4, class Compare = std::less<double>>
auto make_vec_heap(std::vector<double>&      dist,
                   std::vector<std::size_t>& pos,
                   Compare                   cmp = {}) {
  pos.assign(dist.size(), vector_position_map::npos);
  auto distfn = [&dist](unsigned k) -> const double& { return dist[k]; };
  return indexed_dary_heap<unsigned, decltype(distfn), Compare,
                           vector_position_map, Arity>(
        distfn, cmp, vector_position_map{pos});
}

} // namespace

// ---------------------------------------------------------------------------
// Basic construction / empty
// ---------------------------------------------------------------------------

TEST_CASE("indexed_dary_heap: empty after construction", "[heap][indexed_dary_heap]") {
  std::vector<double>      dist;
  std::vector<std::size_t> pos;
  auto                     h = make_vec_heap(dist, pos);

  CHECK(h.empty());
  CHECK(h.size() == 0u);
}

// ---------------------------------------------------------------------------
// push / pop ordering
// ---------------------------------------------------------------------------

TEST_CASE("indexed_dary_heap: pops in ascending distance order", "[heap][indexed_dary_heap]") {
  std::vector<double>      dist = {5.0, 2.0, 7.0, 1.0, 4.0};
  std::vector<std::size_t> pos;
  auto                     h = make_vec_heap(dist, pos);

  for (unsigned k = 0; k < dist.size(); ++k) {
    h.push(k);
  }
  REQUIRE(h.size() == 5u);

  // Distances: 0→5, 1→2, 2→7, 3→1, 4→4  ⇒  expected key order: 3,1,4,0,2
  CHECK(drain(h) == std::vector<unsigned>{3, 1, 4, 0, 2});
}

TEST_CASE("indexed_dary_heap: descending input still pops ascending", "[heap][indexed_dary_heap]") {
  std::vector<double>      dist = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  std::vector<std::size_t> pos;
  auto                     h = make_vec_heap(dist, pos);

  for (int k = 9; k >= 0; --k) {
    h.push(static_cast<unsigned>(k));
  }
  CHECK(drain(h) == std::vector<unsigned>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
}

TEST_CASE("indexed_dary_heap: single element", "[heap][indexed_dary_heap]") {
  std::vector<double>      dist = {42.0};
  std::vector<std::size_t> pos;
  auto                     h = make_vec_heap(dist, pos);

  h.push(0);
  REQUIRE(h.size() == 1u);
  CHECK(h.top() == 0u);
  h.pop();
  CHECK(h.empty());
}

// ---------------------------------------------------------------------------
// decrease-key
// ---------------------------------------------------------------------------

TEST_CASE("indexed_dary_heap: decrease-key reorders top", "[heap][indexed_dary_heap]") {
  std::vector<double>      dist = {10, 20, 30, 40};
  std::vector<std::size_t> pos;
  auto                     h = make_vec_heap(dist, pos);

  for (unsigned k = 0; k < 4; ++k) h.push(k);
  CHECK(h.top() == 0u);

  // Move key 3 to the front by lowering its distance.
  dist[3] = 1.0;
  h.decrease(3);
  CHECK(h.top() == 3u);

  h.pop();
  CHECK(h.top() == 0u);
}

TEST_CASE("indexed_dary_heap: repeated decrease-key on same key", "[heap][indexed_dary_heap]") {
  std::vector<double>      dist = {100, 100, 100, 100, 100};
  std::vector<std::size_t> pos;
  auto                     h = make_vec_heap(dist, pos);

  for (unsigned k = 0; k < 5; ++k) h.push(k);

  for (double d : {50.0, 25.0, 10.0, 1.0}) {
    dist[2] = d;
    h.decrease(2);
    CHECK(h.top() == 2u);
  }
}

// ---------------------------------------------------------------------------
// contains / clear
// ---------------------------------------------------------------------------

TEST_CASE("indexed_dary_heap: contains tracks membership", "[heap][indexed_dary_heap]") {
  std::vector<double>      dist = {1, 2, 3};
  std::vector<std::size_t> pos;
  auto                     h = make_vec_heap(dist, pos);

  CHECK_FALSE(h.contains(0));
  CHECK_FALSE(h.contains(1));

  h.push(0);
  h.push(2);
  CHECK(h.contains(0));
  CHECK_FALSE(h.contains(1));
  CHECK(h.contains(2));

  h.pop(); // removes 0
  CHECK_FALSE(h.contains(0));
  CHECK(h.contains(2));
}

TEST_CASE("indexed_dary_heap: clear empties and resets positions", "[heap][indexed_dary_heap]") {
  std::vector<double>      dist = {1, 2, 3, 4};
  std::vector<std::size_t> pos;
  auto                     h = make_vec_heap(dist, pos);

  for (unsigned k = 0; k < 4; ++k) h.push(k);
  REQUIRE(h.size() == 4u);

  h.clear();
  CHECK(h.empty());
  for (unsigned k = 0; k < 4; ++k) {
    CHECK_FALSE(h.contains(k));
    CHECK(pos[k] == vector_position_map::npos);
  }
}

// ---------------------------------------------------------------------------
// push_or_decrease
// ---------------------------------------------------------------------------

TEST_CASE("indexed_dary_heap: push_or_decrease inserts then decreases",
          "[heap][indexed_dary_heap]") {
  std::vector<double>      dist = {10, 20, 30};
  std::vector<std::size_t> pos;
  auto                     h = make_vec_heap(dist, pos);

  // First call inserts.
  h.push_or_decrease(1);
  CHECK(h.size() == 1u);
  CHECK(h.top() == 1u);

  h.push_or_decrease(2);
  CHECK(h.size() == 2u);
  CHECK(h.top() == 1u); // 20 < 30

  // Lower key 2 below key 1 — second call should decrease, not duplicate.
  dist[2] = 5.0;
  h.push_or_decrease(2);
  CHECK(h.size() == 2u);
  CHECK(h.top() == 2u);
}

// ---------------------------------------------------------------------------
// Arity 2
// ---------------------------------------------------------------------------

TEST_CASE("indexed_dary_heap: arity 2 produces sorted drain", "[heap][indexed_dary_heap]") {
  std::vector<double>      dist = {5, 2, 7, 1, 4, 9, 3, 8, 6, 0};
  std::vector<std::size_t> pos;
  auto                     h = make_vec_heap<2>(dist, pos);

  for (unsigned k = 0; k < dist.size(); ++k) h.push(k);
  auto out = drain(h);

  REQUIRE(out.size() == 10u);
  for (std::size_t i = 1; i < out.size(); ++i) {
    CHECK(dist[out[i - 1]] <= dist[out[i]]);
  }
}

// ---------------------------------------------------------------------------
// Custom comparator: max-heap via std::greater
// ---------------------------------------------------------------------------

TEST_CASE("indexed_dary_heap: std::greater yields max-heap", "[heap][indexed_dary_heap]") {
  std::vector<double>      dist = {5, 2, 7, 1, 4};
  std::vector<std::size_t> pos;
  auto                     h    = make_vec_heap<4, std::greater<double>>(dist, pos);

  for (unsigned k = 0; k < dist.size(); ++k) h.push(k);
  // Distances: 0→5, 1→2, 2→7, 3→1, 4→4  ⇒ max-heap order: 2,0,4,1,3
  CHECK(drain(h) == std::vector<unsigned>{2, 0, 4, 1, 3});
}

// ---------------------------------------------------------------------------
// assoc_position_map (string keys)
// ---------------------------------------------------------------------------

TEST_CASE("indexed_dary_heap: assoc_position_map supports string keys",
          "[heap][indexed_dary_heap][assoc_map]") {
  std::unordered_map<std::string, double> dist = {
        {"a", 5.0}, {"b", 2.0}, {"c", 7.0}, {"d", 1.0}};
  std::unordered_map<std::string, std::size_t> pos;
  auto distfn = [&dist](const std::string& k) -> const double& { return dist.at(k); };

  using PMap = assoc_position_map<std::string>;
  indexed_dary_heap<std::string, decltype(distfn), std::less<double>, PMap, 4> h(
        distfn, std::less<double>{}, PMap{pos});

  for (const auto& k : {"a", "b", "c", "d"}) h.push(k);
  CHECK(drain(h) == std::vector<std::string>{"d", "b", "a", "c"});
}

TEST_CASE("indexed_dary_heap: assoc_position_map decrease-key", "[heap][indexed_dary_heap][assoc_map]") {
  std::unordered_map<std::string, double> dist = {
        {"x", 100.0}, {"y", 50.0}, {"z", 25.0}};
  std::unordered_map<std::string, std::size_t> pos;
  auto distfn = [&dist](const std::string& k) -> const double& { return dist.at(k); };

  using PMap = assoc_position_map<std::string>;
  indexed_dary_heap<std::string, decltype(distfn), std::less<double>, PMap, 4> h(
        distfn, std::less<double>{}, PMap{pos});

  h.push("x"); h.push("y"); h.push("z");
  REQUIRE(h.top() == "z");

  dist["x"] = 1.0;
  h.decrease("x");
  CHECK(h.top() == "x");
  CHECK(h.contains("x"));
  CHECK(h.contains("y"));
  CHECK(h.contains("z"));

  h.pop();
  CHECK_FALSE(h.contains("x"));
}

// ---------------------------------------------------------------------------
// Random stress: cross-check monotone drain after mixed decrease-key
// ---------------------------------------------------------------------------

TEST_CASE("indexed_dary_heap: random stress with decrease-key",
          "[heap][indexed_dary_heap][stress]") {
  constexpr unsigned N = 1000;
  std::mt19937       rng(0xC0FFEE);

  std::vector<double>                    dist(N);
  std::uniform_real_distribution<double> dgen(0.0, 1000.0);
  for (auto& d : dist) d = dgen(rng);

  std::vector<std::size_t> pos;
  auto                     h = make_vec_heap<4>(dist, pos);

  for (unsigned k = 0; k < N; ++k) h.push(k);

  // 500 random decrease-key ops.
  std::uniform_int_distribution<unsigned> kpick(0, N - 1);
  for (int i = 0; i < 500; ++i) {
    const unsigned k = kpick(rng);
    dist[k] *= 0.5;
    h.decrease(k);
  }

  // Drain and assert monotone.
  double   prev  = -1.0;
  unsigned count = 0;
  while (!h.empty()) {
    const double cur = dist[h.top()];
    CHECK(cur >= prev);
    prev = cur;
    h.pop();
    ++count;
  }
  CHECK(count == N);
}
