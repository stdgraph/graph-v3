/**
 * @file test_dynamic_graph_nonintegral_ids.cpp
 * @brief Tests for dynamic_graph with non-integral vertex ID types
 * 
 * Phase 5: Non-Integral Vertex IDs
 * Tests map/unordered_map-based traits with various VId types:
 * - std::string edge cases (empty, Unicode, long strings)
 * - double/float vertex IDs
 * - Custom compound types with operator<=> and std::hash
 * 
 * Key characteristics:
 * - Only map/unordered_map vertex containers support non-integral IDs
 * - std::map requires: operator< or custom comparator
 * - std::unordered_map requires: std::hash specialization + operator==
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/traits/mos_graph_traits.hpp>
#include <graph/container/traits/mous_graph_traits.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <functional>

using namespace graph::container;

//==================================================================================================
// Custom Compound Type for VId Testing
//==================================================================================================

// A compound vertex ID type representing a person
struct PersonId {
  std::string name;
  int         department;

  // Three-way comparison for std::map ordering
  auto operator<=>(const PersonId&) const = default;
  bool operator==(const PersonId&) const  = default;
};

// Hash specialization for std::unordered_map support
template <>
struct std::hash<PersonId> {
  size_t operator()(const PersonId& p) const noexcept {
    size_t h1 = std::hash<std::string>{}(p.name);
    size_t h2 = std::hash<int>{}(p.department);
    // Boost-style hash combine
    return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
  }
};

//==================================================================================================
// Type Aliases
//==================================================================================================

// String ID graphs (ordered and unordered)
using mos_string =
      dynamic_graph<void, void, void, std::string, false, false, mos_graph_traits<void, void, void, std::string, false>>;
using mous_string =
      dynamic_graph<void, void, void, std::string, false, false, mous_graph_traits<void, void, void, std::string, false>>;
using mos_string_ev =
      dynamic_graph<int, void, void, std::string, false, false, mos_graph_traits<int, void, void, std::string, false>>;
using mos_string_sourced =
      dynamic_graph<void, void, void, std::string, true, false, mos_graph_traits<void, void, void, std::string, true>>;

// Double ID graphs
using mos_double  = dynamic_graph<void, void, void, double, false, false, mos_graph_traits<void, void, void, double, false>>;
using mous_double = dynamic_graph<void, void, void, double, false, false, mous_graph_traits<void, void, void, double, false>>;
using mos_double_ev = dynamic_graph<int, void, void, double, false, false, mos_graph_traits<int, void, void, double, false>>;
using mos_double_sourced =
      dynamic_graph<void, void, void, double, true, false, mos_graph_traits<void, void, void, double, true>>;

// PersonId (compound type) graphs
using mos_person =
      dynamic_graph<void, void, void, PersonId, false, false, mos_graph_traits<void, void, void, PersonId, false>>;
using mous_person =
      dynamic_graph<void, void, void, PersonId, false, false, mous_graph_traits<void, void, void, PersonId, false>>;
using mos_person_ev =
      dynamic_graph<int, void, void, PersonId, false, false, mos_graph_traits<int, void, void, PersonId, false>>;

// Helper to count edges
template <typename G>
size_t count_all_edges(G& g) {
  size_t count = 0;
  for (auto& [vid, v] : g) {
    count += static_cast<size_t>(std::ranges::distance(v.edges()));
  }
  return count;
}

//==================================================================================================
// PART 1: String ID Edge Cases
//==================================================================================================

TEST_CASE("string ID edge cases - empty strings", "[nonintegral][string][edge_cases]") {
  SECTION("empty string as vertex ID") {
    mos_string g({{std::string(""), "target"}});
    REQUIRE(g.size() == 2);

    auto it = find_vertex(g, "");
    REQUIRE(it != vertices(g).end());
    REQUIRE(vertex_id(g, *it) == "");
  }

  SECTION("edge between empty string vertices") {
    mos_string g({{std::string(""), std::string("")}});
    REQUIRE(g.size() == 1); // Self-loop on empty string vertex
    REQUIRE(count_all_edges(g) == 1);
  }

  SECTION("empty string sorts before other strings") {
    mos_string g({{"", "b"}, {"a", "c"}});

    std::vector<std::string> ids;
    for (auto& [vid, v] : g) {
      ids.push_back(vid);
    }

    // Empty string should be first in lexicographic order
    REQUIRE(ids[0] == "");
    REQUIRE(ids[1] == "a");
  }

  SECTION("unordered_map with empty string") {
    mous_string g({{std::string(""), "target"}});
    REQUIRE(g.size() == 2);
    REQUIRE(find_vertex(g, "") != vertices(g).end());
  }
}

TEST_CASE("string ID edge cases - whitespace", "[nonintegral][string][edge_cases]") {
  SECTION("space-only vertex ID") {
    mos_string g({{" ", "target"}});
    REQUIRE(g.size() == 2);
    REQUIRE(find_vertex(g, " ") != vertices(g).end());
    REQUIRE(find_vertex(g, "") == vertices(g).end()); // Empty != space
  }

  SECTION("tab and newline in vertex ID") {
    mos_string g({{"\t", "\n"}});
    REQUIRE(g.size() == 2);
    REQUIRE(find_vertex(g, "\t") != vertices(g).end());
    REQUIRE(find_vertex(g, "\n") != vertices(g).end());
  }

  SECTION("mixed whitespace sorting") {
    mos_string g({{"\n", " "}, {"\t", "a"}});

    std::vector<std::string> ids;
    for (auto& [vid, v] : g) {
      ids.push_back(vid);
    }

    // Verify whitespace characters sort correctly (by ASCII value)
    // \t = 9, \n = 10, space = 32, a = 97
    REQUIRE(ids.size() == 4);
    REQUIRE(ids[0] == "\t");
    REQUIRE(ids[1] == "\n");
    REQUIRE(ids[2] == " ");
    REQUIRE(ids[3] == "a");
  }
}

TEST_CASE("string ID edge cases - Unicode", "[nonintegral][string][unicode]") {
  SECTION("basic Unicode vertex IDs") {
    mos_string g({{"日本", "中国"}, {"한국", "việt nam"}});
    REQUIRE(g.size() == 4);
  }

  SECTION("emoji vertex IDs") {
    mos_string g({{"🚀", "🌟"}, {"😀", "🎉"}});
    REQUIRE(g.size() == 4);
    REQUIRE(find_vertex(g, "🚀") != vertices(g).end());
  }

  SECTION("mixed ASCII and Unicode") {
    mos_string g({{"hello", "世界"}, {"foo", "バー"}});
    REQUIRE(g.size() == 4);
  }

  SECTION("Unicode with edge values") {
    mos_string_ev g({{"αλφα", "βήτα", 42}, {"γάμμα", "δέλτα", 100}});
    REQUIRE(g.size() == 4);
    REQUIRE(count_all_edges(g) == 2);
  }

  SECTION("Unicode CPO access") {
    mos_string g({{"北京", "上海"}});

    auto beijing = find_vertex(g, "北京");
    REQUIRE(beijing != vertices(g).end());
    REQUIRE(vertex_id(g, *beijing) == "北京");

    auto edge_rng = edges(g, *beijing);
    REQUIRE(std::ranges::distance(edge_rng) == 1);

    auto edge = *std::ranges::begin(edge_rng);
    REQUIRE(target_id(g, edge) == "上海");
  }
}

TEST_CASE("string ID edge cases - long strings", "[nonintegral][string][performance]") {
  SECTION("very long vertex IDs") {
    std::string long_id(10000, 'x');
    std::string long_id2(10000, 'y');

    mos_string g({{long_id, long_id2}});
    REQUIRE(g.size() == 2);
    REQUIRE(find_vertex(g, long_id) != vertices(g).end());
  }

  SECTION("long string with unique suffix") {
    std::string base(1000, 'a');
    std::string id1 = base + "1";
    std::string id2 = base + "2";
    std::string id3 = base + "3";

    mos_string g({{id1, id2}, {id2, id3}});
    REQUIRE(g.size() == 3);

    // Verify ordering based on suffix
    std::vector<std::string> ids;
    for (auto& [vid, v] : g) {
      ids.push_back(vid);
    }
    REQUIRE(ids[0] == id1);
    REQUIRE(ids[1] == id2);
    REQUIRE(ids[2] == id3);
  }

  SECTION("unordered_map with long strings") {
    std::string long_id(5000, 'z');
    mous_string g({{long_id, "short"}});
    REQUIRE(g.size() == 2);
    REQUIRE(find_vertex(g, long_id) != vertices(g).end());
  }
}

TEST_CASE("string ID - sourced edges", "[nonintegral][string][sourced]") {
  SECTION("sourced graph with string IDs") {
    mos_string_sourced g({{"alice", "bob"}, {"bob", "charlie"}});
    REQUIRE(g.size() == 3);

    auto alice = find_vertex(g, "alice");
    REQUIRE(alice != vertices(g).end());

    auto edge_rng = edges(g, *alice);
    auto edge     = *std::ranges::begin(edge_rng);

    // Verify source_id returns the correct string
    REQUIRE(source_id(g, edge) == "alice");
    REQUIRE(target_id(g, edge) == "bob");
  }
}

//==================================================================================================
// PART 2: Double/Floating-Point Vertex IDs
//==================================================================================================

TEST_CASE("double ID - basic construction", "[nonintegral][double][construction]") {
  SECTION("simple double IDs") {
    mos_double g({{1.0, 2.0}, {2.0, 3.0}});
    REQUIRE(g.size() == 3);
  }

  SECTION("negative double IDs") {
    mos_double g({{-1.5, 2.5}, {-100.0, 100.0}});
    REQUIRE(g.size() == 4);
  }

  SECTION("fractional double IDs") {
    mos_double g({{0.1, 0.2}, {0.3, 0.4}});
    REQUIRE(g.size() == 4);
  }

  SECTION("double IDs with edge values") {
    mos_double_ev g({{1.0, 2.0, 42}, {3.0, 4.0, 100}});
    REQUIRE(g.size() == 4);
    REQUIRE(count_all_edges(g) == 2);
  }
}

TEST_CASE("double ID - ordering", "[nonintegral][double][ordering]") {
  SECTION("negative before positive") {
    mos_double g({{-1.0, 1.0}, {0.0, 2.0}});

    std::vector<double> ids;
    for (auto& [vid, v] : g) {
      ids.push_back(vid);
    }

    REQUIRE(ids.size() == 4);
    REQUIRE(ids[0] == -1.0);
    REQUIRE(ids[1] == 0.0);
    REQUIRE(ids[2] == 1.0);
    REQUIRE(ids[3] == 2.0);
  }

  SECTION("very close values are distinct") {
    double a = 1.0;
    double b = 1.0 + std::numeric_limits<double>::epsilon();

    mos_double g({{a, b}});
    REQUIRE(g.size() == 2); // Different vertices

    auto it_a = find_vertex(g, a);
    auto it_b = find_vertex(g, b);
    REQUIRE(it_a != vertices(g).end());
    REQUIRE(it_b != vertices(g).end());
    REQUIRE(it_a != it_b);
  }
}

TEST_CASE("double ID - special values", "[nonintegral][double][special]") {
  SECTION("zero values") {
    mos_double g({{0.0, 1.0}});
    REQUIRE(g.size() == 2);
    REQUIRE(find_vertex(g, 0.0) != vertices(g).end());
  }

  SECTION("positive and negative zero") {
    // Note: 0.0 == -0.0 in IEEE 754, so they're the same vertex
    mos_double g({{0.0, 1.0}, {-0.0, 2.0}});

    // Both edges come from the same vertex (0.0 == -0.0)
    REQUIRE(find_vertex(g, 0.0) != vertices(g).end());
    REQUIRE(find_vertex(g, -0.0) != vertices(g).end());
    // They point to the same vertex
    REQUIRE(find_vertex(g, 0.0) == find_vertex(g, -0.0));
  }

  SECTION("large magnitude values") {
    double large = 1e308;
    double small = 1e-308;

    mos_double g({{large, small}, {-large, -small}});
    REQUIRE(g.size() == 4);
    REQUIRE(find_vertex(g, large) != vertices(g).end());
    REQUIRE(find_vertex(g, small) != vertices(g).end());
  }

  SECTION("infinity values") {
    double pos_inf = std::numeric_limits<double>::infinity();
    double neg_inf = -std::numeric_limits<double>::infinity();

    mos_double g({{neg_inf, 0.0}, {0.0, pos_inf}});
    REQUIRE(g.size() == 3);

    // Verify ordering: -inf < 0 < +inf
    std::vector<double> ids;
    for (auto& [vid, v] : g) {
      ids.push_back(vid);
    }
    REQUIRE(ids[0] == neg_inf);
    REQUIRE(ids[1] == 0.0);
    REQUIRE(ids[2] == pos_inf);
  }

  // Note: NaN is problematic for map keys because NaN != NaN
  // This test documents the expected behavior
  SECTION("NaN behavior warning") {
    // NaN as a map key is undefined behavior since NaN != NaN
    // We don't test it, but document that it should be avoided
    REQUIRE(std::isnan(std::numeric_limits<double>::quiet_NaN()));
    // Users should NOT use NaN as vertex IDs
  }
}

TEST_CASE("double ID - CPO access", "[nonintegral][double][cpo]") {
  SECTION("vertex_id returns double") {
    mos_double g({{1.5, 2.5}});

    auto v = find_vertex(g, 1.5);
    REQUIRE(v != vertices(g).end());

    double id = vertex_id(g, *v);
    REQUIRE(id == 1.5);
  }

  SECTION("target_id returns double") {
    mos_double g({{1.0, 2.0}});

    auto v        = find_vertex(g, 1.0);
    auto edge_rng = edges(g, *v);
    auto edge     = *std::ranges::begin(edge_rng);

    double tid = target_id(g, edge);
    REQUIRE(tid == 2.0);
  }

  SECTION("find_vertex with double") {
    mos_double g({{3.14159, 2.71828}});

    auto pi = find_vertex(g, 3.14159);
    REQUIRE(pi != vertices(g).end());

    auto e = find_vertex(g, 2.71828);
    REQUIRE(e != vertices(g).end());

    auto missing = find_vertex(g, 1.41421);
    REQUIRE(missing == vertices(g).end());
  }

  SECTION("contains_edge with double IDs") {
    mos_double g({{1.0, 2.0}, {2.0, 3.0}});

    REQUIRE(contains_edge(g, 1.0, 2.0));
    REQUIRE(contains_edge(g, 2.0, 3.0));
    REQUIRE_FALSE(contains_edge(g, 1.0, 3.0));
    REQUIRE_FALSE(contains_edge(g, 3.0, 1.0));
  }
}

TEST_CASE("double ID - unordered_map", "[nonintegral][double][unordered]") {
  SECTION("basic construction") {
    mous_double g({{1.0, 2.0}, {3.0, 4.0}});
    REQUIRE(g.size() == 4);
  }

  SECTION("hash-based lookup") {
    mous_double g({{3.14159, 2.71828}});
    REQUIRE(find_vertex(g, 3.14159) != vertices(g).end());
    REQUIRE(find_vertex(g, 2.71828) != vertices(g).end());
    REQUIRE(find_vertex(g, 1.41421) == vertices(g).end());
  }

  SECTION("special values in unordered_map") {
    double      pos_inf = std::numeric_limits<double>::infinity();
    mous_double g({{0.0, pos_inf}});
    REQUIRE(g.size() == 2);
    REQUIRE(find_vertex(g, pos_inf) != vertices(g).end());
  }
}

TEST_CASE("double ID - sourced edges", "[nonintegral][double][sourced]") {
  SECTION("source_id returns double") {
    mos_double_sourced g({{1.0, 2.0}, {2.0, 3.0}});

    auto v        = find_vertex(g, 1.0);
    auto edge_rng = edges(g, *v);
    auto edge     = *std::ranges::begin(edge_rng);

    REQUIRE(source_id(g, edge) == 1.0);
    REQUIRE(target_id(g, edge) == 2.0);
  }
}

//==================================================================================================
// PART 3: Compound/Custom Type Vertex IDs (PersonId)
//==================================================================================================

TEST_CASE("PersonId - basic construction", "[nonintegral][custom][construction]") {
  SECTION("initializer list with PersonId") {
    // Using initializer list syntax
    mos_person g({{PersonId{"Alice", 1}, PersonId{"Bob", 2}}});
    REQUIRE(g.size() == 2);
  }

  SECTION("single edge with PersonId") {
    PersonId   alice{"Alice", 1};
    PersonId   bob{"Bob", 2};
    mos_person g({{alice, bob}});
    REQUIRE(g.size() == 2);
    REQUIRE(count_all_edges(g) == 1);
  }

  SECTION("multiple edges") {
    PersonId alice{"Alice", 1};
    PersonId bob{"Bob", 2};
    PersonId charlie{"Charlie", 1};

    mos_person g({{alice, bob}, {bob, charlie}, {alice, charlie}});
    REQUIRE(g.size() == 3);
    REQUIRE(count_all_edges(g) == 3);
  }
}

TEST_CASE("PersonId - ordering", "[nonintegral][custom][ordering]") {
  SECTION("ordered by name first, then department") {
    PersonId a1{"Alice", 1};
    PersonId a2{"Alice", 2};
    PersonId b1{"Bob", 1};

    mos_person g({{a1, b1}, {a2, b1}});

    std::vector<PersonId> ids;
    for (auto& [vid, v] : g) {
      ids.push_back(vid);
    }

    // Ordering: Alice/1 < Alice/2 < Bob/1 (name first, then dept)
    REQUIRE(ids.size() == 3);
    REQUIRE(ids[0] == a1);
    REQUIRE(ids[1] == a2);
    REQUIRE(ids[2] == b1);
  }

  SECTION("same name different departments") {
    PersonId dept1{"Employee", 1};
    PersonId dept2{"Employee", 2};
    PersonId dept3{"Employee", 3};

    mos_person g({{dept3, dept1}, {dept2, dept3}});

    std::vector<PersonId> ids;
    for (auto& [vid, v] : g) {
      ids.push_back(vid);
    }

    // Ordered by department since names are equal
    REQUIRE(ids[0].department == 1);
    REQUIRE(ids[1].department == 2);
    REQUIRE(ids[2].department == 3);
  }
}

TEST_CASE("PersonId - CPO access", "[nonintegral][custom][cpo]") {
  PersonId alice{"Alice", 1};
  PersonId bob{"Bob", 2};

  mos_person g({{alice, bob}});

  SECTION("vertex_id returns PersonId") {
    auto v = find_vertex(g, alice);
    REQUIRE(v != vertices(g).end());

    PersonId id = vertex_id(g, *v);
    REQUIRE(id == alice);
    REQUIRE(id.name == "Alice");
    REQUIRE(id.department == 1);
  }

  SECTION("target_id returns PersonId") {
    auto v        = find_vertex(g, alice);
    auto edge_rng = edges(g, *v);
    auto edge     = *std::ranges::begin(edge_rng);

    PersonId tid = target_id(g, edge);
    REQUIRE(tid == bob);
  }

  SECTION("find_vertex with PersonId") {
    auto alice_v = find_vertex(g, alice);
    REQUIRE(alice_v != vertices(g).end());

    PersonId unknown{"Unknown", 99};
    auto     missing = find_vertex(g, unknown);
    REQUIRE(missing == vertices(g).end());
  }

  SECTION("contains_edge with PersonId") {
    REQUIRE(contains_edge(g, alice, bob));
    REQUIRE_FALSE(contains_edge(g, bob, alice));

    PersonId unknown{"Unknown", 99};
    REQUIRE_FALSE(contains_edge(g, alice, unknown));
  }
}

TEST_CASE("PersonId - with edge values", "[nonintegral][custom][values]") {
  PersonId alice{"Alice", 1};
  PersonId bob{"Bob", 2};
  PersonId charlie{"Charlie", 3};

  mos_person_ev g({{alice, bob, 100}, {bob, charlie, 200}});

  SECTION("edge values accessible") {
    auto v        = find_vertex(g, alice);
    auto edge_rng = edges(g, *v);
    auto edge     = *std::ranges::begin(edge_rng);

    REQUIRE(edge_value(g, edge) == 100);
  }

  SECTION("multiple edge values") {
    auto bob_v    = find_vertex(g, bob);
    auto edge_rng = edges(g, *bob_v);
    auto edge     = *std::ranges::begin(edge_rng);

    REQUIRE(edge_value(g, edge) == 200);
  }
}

TEST_CASE("PersonId - unordered_map (hash-based)", "[nonintegral][custom][unordered]") {
  PersonId alice{"Alice", 1};
  PersonId bob{"Bob", 2};

  SECTION("basic construction") {
    mous_person g({{alice, bob}});
    REQUIRE(g.size() == 2);
  }

  SECTION("hash-based lookup") {
    mous_person g({{alice, bob}});

    REQUIRE(find_vertex(g, alice) != vertices(g).end());
    REQUIRE(find_vertex(g, bob) != vertices(g).end());

    PersonId unknown{"Unknown", 99};
    REQUIRE(find_vertex(g, unknown) == vertices(g).end());
  }

  SECTION("hash function produces different values") {
    PersonId p1{"Test", 1};
    PersonId p2{"Test", 2};
    PersonId p3{"Other", 1};

    std::hash<PersonId> hasher;
    size_t              h1 = hasher(p1);
    size_t              h2 = hasher(p2);
    size_t              h3 = hasher(p3);

    // Different PersonIds should (usually) have different hashes
    // Note: This is probabilistic, not guaranteed
    REQUIRE((h1 != h2 || h1 != h3 || h2 != h3));
  }
}

TEST_CASE("PersonId - edge cases", "[nonintegral][custom][edge_cases]") {
  SECTION("empty name") {
    PersonId empty_name{"", 1};
    PersonId normal{"Bob", 2};

    mos_person g({{empty_name, normal}});
    REQUIRE(g.size() == 2);
    REQUIRE(find_vertex(g, empty_name) != vertices(g).end());
  }

  SECTION("negative department") {
    PersonId neg_dept{"Alice", -1};
    PersonId pos_dept{"Alice", 1};

    mos_person g({{neg_dept, pos_dept}});
    REQUIRE(g.size() == 2);

    // Negative department sorts before positive
    std::vector<PersonId> ids;
    for (auto& [vid, v] : g) {
      ids.push_back(vid);
    }
    REQUIRE(ids[0].department == -1);
    REQUIRE(ids[1].department == 1);
  }

  SECTION("self-loop with PersonId") {
    PersonId   self{"Self", 0};
    mos_person g({{self, self}});

    REQUIRE(g.size() == 1);
    REQUIRE(count_all_edges(g) == 1);
  }
}

//==================================================================================================
// PART 4: Cross-Type Verification
//==================================================================================================

TEST_CASE("non-integral IDs - type trait verification", "[nonintegral][traits]") {
  SECTION("string VId type") {
    using traits = mos_graph_traits<void, void, void, std::string, false>;
    static_assert(std::same_as<typename traits::vertex_id_type, std::string>);
    REQUIRE(true);
  }

  SECTION("double VId type") {
    using traits = mos_graph_traits<void, void, void, double, false>;
    static_assert(std::same_as<typename traits::vertex_id_type, double>);
    REQUIRE(true);
  }

  SECTION("PersonId VId type") {
    using traits = mos_graph_traits<void, void, void, PersonId, false>;
    static_assert(std::same_as<typename traits::vertex_id_type, PersonId>);
    REQUIRE(true);
  }

  SECTION("float VId type") {
    using traits = mos_graph_traits<void, void, void, float, false>;
    static_assert(std::same_as<typename traits::vertex_id_type, float>);
    REQUIRE(true);
  }
}

TEST_CASE("non-integral IDs - graph integration", "[nonintegral][integration]") {
  SECTION("string graph iteration") {
    mos_string g({{"a", "b"}, {"b", "c"}, {"c", "a"}});

    size_t vertex_count = 0;
    size_t edge_count   = 0;

    for (auto& [vid, v] : g) {
      ++vertex_count;
      for ([[maybe_unused]] auto& e : v.edges()) {
        ++edge_count;
      }
    }

    REQUIRE(vertex_count == 3);
    REQUIRE(edge_count == 3);
  }

  SECTION("double graph iteration") {
    mos_double g({{1.0, 2.0}, {2.0, 3.0}, {3.0, 1.0}});

    size_t vertex_count = 0;
    size_t edge_count   = 0;

    for (auto& [vid, v] : g) {
      ++vertex_count;
      for ([[maybe_unused]] auto& e : v.edges()) {
        ++edge_count;
      }
    }

    REQUIRE(vertex_count == 3);
    REQUIRE(edge_count == 3);
  }

  SECTION("PersonId graph iteration") {
    PersonId   a{"A", 1}, b{"B", 2}, c{"C", 3};
    mos_person g({{a, b}, {b, c}, {c, a}});

    size_t vertex_count = 0;
    size_t edge_count   = 0;

    for (auto& [vid, v] : g) {
      ++vertex_count;
      for ([[maybe_unused]] auto& e : v.edges()) {
        ++edge_count;
      }
    }

    REQUIRE(vertex_count == 3);
    REQUIRE(edge_count == 3);
  }
}

//==================================================================================================
// PART 5: load_vertices and load_edges with Non-Integral IDs
//==================================================================================================

TEST_CASE("load_vertices with non-integral VId (VV=void)", "[nonintegral][load_vertices]") {
  SECTION("string IDs - void vertex value") {
    mos_string g;

    std::vector<std::string> vertex_ids = {"alice", "bob", "charlie"};
    g.load_vertices(vertex_ids, [](const std::string& id) { return graph::copyable_vertex_t<std::string, void>{id}; });

    REQUIRE(g.size() == 3);
    REQUIRE(find_vertex(g, "alice") != vertices(g).end());
    REQUIRE(find_vertex(g, "bob") != vertices(g).end());
    REQUIRE(find_vertex(g, "charlie") != vertices(g).end());
  }

  SECTION("PersonId - void vertex value") {
    mos_person g;

    PersonId alice{"Alice", 1};
    PersonId bob{"Bob", 2};

    std::vector<PersonId> vertex_ids = {alice, bob};
    g.load_vertices(vertex_ids, [](const PersonId& id) { return graph::copyable_vertex_t<PersonId, void>{id}; });

    REQUIRE(g.size() == 2);
    REQUIRE(find_vertex(g, alice) != vertices(g).end());
    REQUIRE(find_vertex(g, bob) != vertices(g).end());
  }

  SECTION("double IDs - void vertex value") {
    mos_double g;

    std::vector<double> vertex_ids = {1.0, 2.5, 3.14159};
    g.load_vertices(vertex_ids, [](double id) { return graph::copyable_vertex_t<double, void>{id}; });

    REQUIRE(g.size() == 3);
    REQUIRE(find_vertex(g, 1.0) != vertices(g).end());
    REQUIRE(find_vertex(g, 2.5) != vertices(g).end());
    REQUIRE(find_vertex(g, 3.14159) != vertices(g).end());
  }
}

TEST_CASE("load_edges with non-integral VId", "[nonintegral][load_edges]") {
  SECTION("string IDs") {
    mos_string g;

    std::vector<std::tuple<std::string, std::string>> edge_data = {
          {"alice", "bob"}, {"bob", "charlie"}, {"charlie", "alice"}};

    g.load_edges(edge_data, [](const auto& t) {
      return graph::copyable_edge_t<std::string, void>{std::get<0>(t), std::get<1>(t)};
    });

    REQUIRE(g.size() == 3);
    REQUIRE(count_all_edges(g) == 3);
    REQUIRE(contains_edge(g, "alice", "bob"));
    REQUIRE(contains_edge(g, "bob", "charlie"));
    REQUIRE(contains_edge(g, "charlie", "alice"));
  }

  SECTION("PersonId - load_edges after load_vertices") {
    mos_person g;

    PersonId alice{"Alice", 1};
    PersonId bob{"Bob", 2};
    PersonId charlie{"Charlie", 3};

    // First load vertices
    std::vector<PersonId> vertex_ids = {alice, bob, charlie};
    g.load_vertices(vertex_ids, [](const PersonId& id) { return graph::copyable_vertex_t<PersonId, void>{id}; });

    // Then load edges
    std::vector<std::tuple<PersonId, PersonId>> edge_data = {{alice, bob}, {bob, charlie}};

    g.load_edges(edge_data,
                 [](const auto& t) { return graph::copyable_edge_t<PersonId, void>{std::get<0>(t), std::get<1>(t)}; });

    REQUIRE(g.size() == 3);
    REQUIRE(count_all_edges(g) == 2);
    REQUIRE(contains_edge(g, alice, bob));
    REQUIRE(contains_edge(g, bob, charlie));
  }

  SECTION("double IDs with edge values") {
    mos_double_ev g;

    std::vector<std::tuple<double, double, int>> edge_data = {{1.0, 2.0, 100}, {2.0, 3.0, 200}};

    g.load_edges(edge_data, [](const auto& t) {
      return graph::copyable_edge_t<double, int>{std::get<0>(t), std::get<1>(t), std::get<2>(t)};
    });

    REQUIRE(g.size() == 3);
    REQUIRE(count_all_edges(g) == 2);

    // Verify edge values
    auto v1       = find_vertex(g, 1.0);
    auto edge_rng = edges(g, *v1);
    auto edge     = *std::ranges::begin(edge_rng);
    REQUIRE(edge_value(g, edge) == 100);
  }
}
