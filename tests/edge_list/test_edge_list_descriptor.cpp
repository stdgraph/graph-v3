#include <catch2/catch_test_macros.hpp>
#include <graph/edge_list/edge_list_descriptor.hpp>
#include <graph/edge_list/edge_list_traits.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <string>
#include <vector>

using namespace graph;
using namespace graph::edge_list;
using namespace graph::adj_list::_cpo_instances;

// =============================================================================
// Construction Tests
// =============================================================================

TEST_CASE("edge_list::edge_descriptor construction without value", "[edge_list][descriptor]") {
    edge_descriptor<int, void> e(1, 2);
    
    REQUIRE(e.source_id() == 1);
    REQUIRE(e.target_id() == 2);
}

TEST_CASE("edge_list::edge_descriptor construction with value", "[edge_list][descriptor]") {
    edge_descriptor<int, double> e(3, 4, 1.5);
    
    REQUIRE(e.source_id() == 3);
    REQUIRE(e.target_id() == 4);
    REQUIRE(e.value() == 1.5);
}

TEST_CASE("edge_list::edge_descriptor deduction guides", "[edge_list][descriptor]") {
    // Without value
    auto e1 = edge_descriptor(5, 6);
    static_assert(std::is_same_v<decltype(e1), edge_descriptor<int, void>>);
    REQUIRE(e1.source_id() == 5);
    REQUIRE(e1.target_id() == 6);
    
    // With value
    auto e2 = edge_descriptor(7, 8, 2.5);
    static_assert(std::is_same_v<decltype(e2), edge_descriptor<int, double>>);
    REQUIRE(e2.source_id() == 7);
    REQUIRE(e2.target_id() == 8);
    REQUIRE(e2.value() == 2.5);
}

TEST_CASE("edge_list::edge_descriptor with string value", "[edge_list][descriptor]") {
    edge_descriptor<int, std::string> e(9, 10, "test");
    
    REQUIRE(e.source_id() == 9);
    REQUIRE(e.target_id() == 10);
    REQUIRE(e.value() == "test");
}

TEST_CASE("edge_list::edge_descriptor copy constructor", "[edge_list][descriptor]") {
    edge_descriptor<int, double> e1(11, 12, 3.5);
    edge_descriptor<int, double> e2(e1);
    
    REQUIRE(e2.source_id() == 11);
    REQUIRE(e2.target_id() == 12);
    REQUIRE(e2.value() == 3.5);
}

TEST_CASE("edge_list::edge_descriptor move constructor", "[edge_list][descriptor]") {
    edge_descriptor<int, std::string> e1(13, 14, "moved");
    edge_descriptor<int, std::string> e2(std::move(e1));
    
    REQUIRE(e2.source_id() == 13);
    REQUIRE(e2.target_id() == 14);
    REQUIRE(e2.value() == "moved");
}

// =============================================================================
// Trait Tests
// =============================================================================

TEST_CASE("is_edge_list_descriptor_v trait", "[edge_list][traits]") {
    // Should be true for edge_list::edge_descriptor
    static_assert(is_edge_list_descriptor_v<edge_descriptor<int, void>>);
    static_assert(is_edge_list_descriptor_v<edge_descriptor<int, double>>);
    static_assert(is_edge_list_descriptor_v<edge_descriptor<size_t, std::string>>);
    
    // Should be false for other types
    static_assert(!is_edge_list_descriptor_v<int>);
    static_assert(!is_edge_list_descriptor_v<std::pair<int, int>>);
    static_assert(!is_edge_list_descriptor_v<std::tuple<int, int, double>>);
    
    SUCCEED("All trait checks passed at compile time");
}

// =============================================================================
// CPO Integration Tests (Tier 5)
// =============================================================================

TEST_CASE("source_id CPO with edge_list::edge_descriptor", "[cpo][edge_list][tier5]") {
    edge_descriptor<int, void> e(15, 16);
    std::vector<edge_descriptor<int, void>> el{e};
    
    auto sid = source_id(el, e);
    REQUIRE(sid == 15);
}

TEST_CASE("target_id CPO with edge_list::edge_descriptor", "[cpo][edge_list][tier5]") {
    edge_descriptor<int, void> e(17, 18);
    std::vector<edge_descriptor<int, void>> el{e};
    
    auto tid = target_id(el, e);
    REQUIRE(tid == 18);
}

TEST_CASE("source_id and target_id with edge_descriptor (with value)", "[cpo][edge_list][tier5]") {
    edge_descriptor<int, double> e(19, 20, 4.5);
    std::vector<edge_descriptor<int, double>> el{e};
    
    auto sid = source_id(el, e);
    auto tid = target_id(el, e);
    
    REQUIRE(sid == 19);
    REQUIRE(tid == 20);
}

TEST_CASE("edge_value CPO with edge_list::edge_descriptor", "[cpo][edge_list][tier5]") {
    edge_descriptor<int, double> e(21, 22, 5.5);
    std::vector<edge_descriptor<int, double>> el{e};
    
    auto val = edge_value(el, e);
    REQUIRE(val == 5.5);
}

TEST_CASE("all CPOs with edge_list::edge_descriptor<string>", "[cpo][edge_list][tier5]") {
    edge_descriptor<int, std::string> e(23, 24, "edge_value");
    std::vector<edge_descriptor<int, std::string>> el{e};
    
    auto sid = source_id(el, e);
    auto tid = target_id(el, e);
    auto val = edge_value(el, e);
    
    REQUIRE(sid == 23);
    REQUIRE(tid == 24);
    REQUIRE(val == "edge_value");
}

// =============================================================================
// Noexcept Tests
// =============================================================================

TEST_CASE("edge_list::edge_descriptor operations are noexcept", "[edge_list][descriptor][noexcept]") {
    edge_descriptor<int, double> e(25, 26, 6.5);
    
    STATIC_REQUIRE(noexcept(e.source_id()));
    STATIC_REQUIRE(noexcept(e.target_id()));
    STATIC_REQUIRE(noexcept(e.value()));
}

TEST_CASE("CPOs with edge_list::edge_descriptor are noexcept", "[cpo][edge_list][noexcept]") {
    edge_descriptor<int, double> e(27, 28, 7.5);
    std::vector<edge_descriptor<int, double>> el{e};
    
    STATIC_REQUIRE(noexcept(source_id(el, e)));
    STATIC_REQUIRE(noexcept(target_id(el, e)));
    STATIC_REQUIRE(noexcept(edge_value(el, e)));
}

// =============================================================================
// Comparison Tests
// =============================================================================

TEST_CASE("edge_list::edge_descriptor equality", "[edge_list][descriptor]") {
    edge_descriptor<int, double> e1(29, 30, 8.5);
    edge_descriptor<int, double> e2(29, 30, 8.5);
    edge_descriptor<int, double> e3(31, 32, 9.5);
    
    REQUIRE(e1 == e2);
    REQUIRE(e1 != e3);
}

TEST_CASE("edge_list::edge_descriptor ordering", "[edge_list][descriptor]") {
    edge_descriptor<int, void> e1(33, 34);
    edge_descriptor<int, void> e2(33, 35);
    edge_descriptor<int, void> e3(34, 34);
    
    REQUIRE(e1 < e2);
    REQUIRE(e1 < e3);
    REQUIRE(e2 < e3);
}
