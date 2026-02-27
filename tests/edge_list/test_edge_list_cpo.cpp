#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/graph_data.hpp>
#include <vector>
#include <tuple>
#include <utility>

using namespace graph;
using namespace graph::adj_list::_cpo_instances;

// =============================================================================
// Tier 6 Tests: edge_data data member access
// =============================================================================

TEST_CASE("source_id with edge_data (bidirectional, no value)", "[cpo][source_id][tier6]") {
  using EI = edge_data<int, true, void, void>;
  EI              ei{1, 2};
  std::vector<EI> el{ei};

  auto uid = source_id(el, ei);
  REQUIRE(uid == 1);
}

TEST_CASE("source_id with edge_data (bidirectional, with value)", "[cpo][source_id][tier6]") {
  using EI = edge_data<int, true, void, double>;
  EI              ei{3, 4, 1.5};
  std::vector<EI> el{ei};

  auto uid = source_id(el, ei);
  REQUIRE(uid == 3);
}

TEST_CASE("target_id with edge_data (bidirectional, no value)", "[cpo][target_id][tier6]") {
  using EI = edge_data<int, true, void, void>;
  EI              ei{5, 6};
  std::vector<EI> el{ei};

  auto vid = target_id(el, ei);
  REQUIRE(vid == 6);
}

TEST_CASE("target_id with edge_data (bidirectional, with value)", "[cpo][target_id][tier6]") {
  using EI = edge_data<int, true, void, double>;
  EI              ei{7, 8, 2.5};
  std::vector<EI> el{ei};

  auto vid = target_id(el, ei);
  REQUIRE(vid == 8);
}

TEST_CASE("edge_value with edge_data (with value)", "[cpo][edge_value][tier6]") {
  using EI = edge_data<int, true, void, double>;
  EI              ei{9, 10, 3.5};
  std::vector<EI> el{ei};

  auto val = edge_value(el, ei);
  REQUIRE(val == 3.5);
}

TEST_CASE("edge_value with edge_data (unidirectional, with value)", "[cpo][edge_value][tier6]") {
  using EI = edge_data<int, false, void, double>;
  EI              ei{11, 4.5};
  std::vector<EI> el{ei};

  auto val = edge_value(el, ei);
  REQUIRE(val == 4.5);
}

// =============================================================================
// Tier 7 Tests: tuple-like edge access
// =============================================================================

TEST_CASE("source_id with pair", "[cpo][source_id][tier7]") {
  std::pair<int, int>              edge{12, 13};
  std::vector<std::pair<int, int>> el{edge};

  auto uid = source_id(el, edge);
  REQUIRE(uid == 12);
}

TEST_CASE("target_id with pair", "[cpo][target_id][tier7]") {
  std::pair<int, int>              edge{14, 15};
  std::vector<std::pair<int, int>> el{edge};

  auto vid = target_id(el, edge);
  REQUIRE(vid == 15);
}

TEST_CASE("source_id with tuple (3 elements)", "[cpo][source_id][tier7]") {
  std::tuple<int, int, double>              edge{16, 17, 5.5};
  std::vector<std::tuple<int, int, double>> el{edge};

  auto uid = source_id(el, edge);
  REQUIRE(uid == 16);
}

TEST_CASE("target_id with tuple (3 elements)", "[cpo][target_id][tier7]") {
  std::tuple<int, int, double>              edge{18, 19, 6.5};
  std::vector<std::tuple<int, int, double>> el{edge};

  auto vid = target_id(el, edge);
  REQUIRE(vid == 19);
}

TEST_CASE("edge_value with tuple (3 elements)", "[cpo][edge_value][tier7]") {
  std::tuple<int, int, double>              edge{20, 21, 7.5};
  std::vector<std::tuple<int, int, double>> el{edge};

  auto val = edge_value(el, edge);
  REQUIRE(val == 7.5);
}

TEST_CASE("source_id with tuple (4 elements)", "[cpo][source_id][tier7]") {
  std::tuple<int, int, double, std::string>              edge{22, 23, 8.5, "test"};
  std::vector<std::tuple<int, int, double, std::string>> el{edge};

  auto uid = source_id(el, edge);
  REQUIRE(uid == 22);
}

TEST_CASE("target_id with tuple (4 elements)", "[cpo][target_id][tier7]") {
  std::tuple<int, int, double, std::string>              edge{24, 25, 9.5, "test"};
  std::vector<std::tuple<int, int, double, std::string>> el{edge};

  auto vid = target_id(el, edge);
  REQUIRE(vid == 25);
}

TEST_CASE("edge_value with tuple (4 elements)", "[cpo][edge_value][tier7]") {
  std::tuple<int, int, double, std::string>              edge{26, 27, 10.5, "test"};
  std::vector<std::tuple<int, int, double, std::string>> el{edge};

  auto val = edge_value(el, edge);
  REQUIRE(val == 10.5);
}

// =============================================================================
// Ambiguity Tests: Verify tier precedence
// =============================================================================

// Helper types for ambiguity tests - must be at namespace scope for std::get specialization
struct EdgeWithSourceAndTarget {
  int source_id;
  int target_id;
};

// Make it tuple-like via std::tuple_size and std::get specializations
namespace std {
template <>
struct tuple_size<EdgeWithSourceAndTarget> : integral_constant<size_t, 2> {};

template <size_t I>
struct tuple_element<I, EdgeWithSourceAndTarget> {
  using type = int;
};
} // namespace std

template <size_t I>
auto get(const EdgeWithSourceAndTarget& e) {
  if constexpr (I == 0)
    return e.source_id + 100; // Different value to test precedence
  else
    return e.target_id + 100;
}

struct EdgeWithAllThree {
  int    source_id;
  int    target_id;
  double value;
};

namespace std {
template <>
struct tuple_size<EdgeWithAllThree> : integral_constant<size_t, 3> {};

template <size_t I>
struct tuple_element<I, EdgeWithAllThree> {
  using type = conditional_t < I<2, int, double>;
};
} // namespace std

template <size_t I>
auto get(const EdgeWithAllThree& e) {
  if constexpr (I == 0)
    return e.source_id;
  else if constexpr (I == 1)
    return e.target_id;
  else
    return e.value + 100.0; // Different value
}

TEST_CASE("source_id prefers data member over tuple", "[cpo][source_id][ambiguity]") {
  // Type with both source_id data member and tuple-like interface
  // Should pick data member (Tier 6) over tuple-like (Tier 7)
  EdgeWithSourceAndTarget              e{30, 31};
  std::vector<EdgeWithSourceAndTarget> el{e};

  // Should use data member (30), not tuple get<0> (130)
  auto uid = source_id(el, e);
  REQUIRE(uid == 30);
}

TEST_CASE("target_id prefers data member over tuple", "[cpo][target_id][ambiguity]") {
  EdgeWithSourceAndTarget              e{32, 33};
  std::vector<EdgeWithSourceAndTarget> el{e};

  // Should use data member (33), not tuple get<1> (133)
  auto vid = target_id(el, e);
  REQUIRE(vid == 33);
}

TEST_CASE("edge_value prefers data member over tuple", "[cpo][edge_value][ambiguity]") {
  EdgeWithAllThree              e{34, 35, 11.5};
  std::vector<EdgeWithAllThree> el{e};

  // Should use data member (11.5), not tuple get<2> (111.5)
  auto val = edge_value(el, e);
  REQUIRE(val == 11.5);
}

// =============================================================================
// Noexcept Tests: Verify noexcept propagation
// =============================================================================

TEST_CASE("source_id with edge_data is noexcept", "[cpo][source_id][noexcept]") {
  using EI = edge_data<int, true, void, void>;
  EI              ei{40, 41};
  std::vector<EI> el{ei};

  STATIC_REQUIRE(noexcept(source_id(el, ei)));
}

TEST_CASE("source_id with pair is noexcept", "[cpo][source_id][noexcept]") {
  std::pair<int, int>              edge{42, 43};
  std::vector<std::pair<int, int>> el{edge};

  STATIC_REQUIRE(noexcept(source_id(el, edge)));
}

TEST_CASE("target_id with edge_data is noexcept", "[cpo][target_id][noexcept]") {
  using EI = edge_data<int, true, void, void>;
  EI              ei{44, 45};
  std::vector<EI> el{ei};

  STATIC_REQUIRE(noexcept(target_id(el, ei)));
}

TEST_CASE("target_id with tuple is noexcept", "[cpo][target_id][noexcept]") {
  std::tuple<int, int, double>              edge{46, 47, 12.5};
  std::vector<std::tuple<int, int, double>> el{edge};

  STATIC_REQUIRE(noexcept(target_id(el, edge)));
}

TEST_CASE("edge_value with edge_data is noexcept", "[cpo][edge_value][noexcept]") {
  using EI = edge_data<int, true, void, double>;
  EI              ei{48, 49, 13.5};
  std::vector<EI> el{ei};

  STATIC_REQUIRE(noexcept(edge_value(el, ei)));
}

TEST_CASE("edge_value with tuple is noexcept", "[cpo][edge_value][noexcept]") {
  std::tuple<int, int, double>              edge{50, 51, 14.5};
  std::vector<std::tuple<int, int, double>> el{edge};

  STATIC_REQUIRE(noexcept(edge_value(el, edge)));
}
