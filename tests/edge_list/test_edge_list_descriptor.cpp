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
    int src = 1, tgt = 2;
    edge_descriptor<int, void> e(src, tgt);
    
    REQUIRE(e.source_id() == 1);
    REQUIRE(e.target_id() == 2);
    REQUIRE(&e.source_id() == &src);  // Verify it's a reference
    REQUIRE(&e.target_id() == &tgt);  // Verify it's a reference
}

TEST_CASE("edge_list::edge_descriptor construction with value", "[edge_list][descriptor]") {
    int src = 3, tgt = 4;
    double val = 1.5;
    edge_descriptor<int, double> e(src, tgt, val);
    
    REQUIRE(e.source_id() == 3);
    REQUIRE(e.target_id() == 4);
    REQUIRE(e.value() == 1.5);
    REQUIRE(&e.source_id() == &src);  // Verify it's a reference
    REQUIRE(&e.target_id() == &tgt);  // Verify it's a reference
    REQUIRE(&e.value() == &val);      // Verify it's a reference
}

TEST_CASE("edge_list::edge_descriptor deduction guides", "[edge_list][descriptor]") {
    // Without value
    int src1 = 5, tgt1 = 6;
    auto e1 = edge_descriptor(src1, tgt1);
    static_assert(std::is_same_v<decltype(e1), edge_descriptor<int, void>>);
    REQUIRE(e1.source_id() == 5);
    REQUIRE(e1.target_id() == 6);
    
    // With value
    int src2 = 7, tgt2 = 8;
    double val2 = 2.5;
    auto e2 = edge_descriptor(src2, tgt2, val2);
    static_assert(std::is_same_v<decltype(e2), edge_descriptor<int, double>>);
    REQUIRE(e2.source_id() == 7);
    REQUIRE(e2.target_id() == 8);
    REQUIRE(e2.value() == 2.5);
}

TEST_CASE("edge_list::edge_descriptor with string value", "[edge_list][descriptor]") {
    int src = 9, tgt = 10;
    std::string val = "test";
    edge_descriptor<int, std::string> e(src, tgt, val);
    
    REQUIRE(e.source_id() == 9);
    REQUIRE(e.target_id() == 10);
    REQUIRE(e.value() == "test");
    REQUIRE(&e.value() == &val);  // Verify it's a reference
}

TEST_CASE("edge_list::edge_descriptor with string vertex IDs", "[edge_list][descriptor]") {
    std::string src = "vertex_a", tgt = "vertex_b";
    double val = 1.5;
    edge_descriptor<std::string, double> e(src, tgt, val);
    
    REQUIRE(e.source_id() == "vertex_a");
    REQUIRE(e.target_id() == "vertex_b");
    REQUIRE(e.value() == 1.5);
    
    // Verify accessors return const references (no copy)
    static_assert(std::is_same_v<decltype(e.source_id()), const std::string&>);
    static_assert(std::is_same_v<decltype(e.target_id()), const std::string&>);
    
    // Verify they're references to the original data
    REQUIRE(&e.source_id() == &src);
    REQUIRE(&e.target_id() == &tgt);
}

TEST_CASE("edge_list::edge_descriptor copy constructor", "[edge_list][descriptor]") {
    int src = 11, tgt = 12;
    double val = 3.5;
    edge_descriptor<int, double> e1(src, tgt, val);
    edge_descriptor<int, double> e2(e1);
    
    REQUIRE(e2.source_id() == 11);
    REQUIRE(e2.target_id() == 12);
    REQUIRE(e2.value() == 3.5);
    
    // Both should reference the same underlying data
    REQUIRE(&e1.source_id() == &e2.source_id());
    REQUIRE(&e1.target_id() == &e2.target_id());
    REQUIRE(&e1.value() == &e2.value());
}

TEST_CASE("edge_list::edge_descriptor move constructor", "[edge_list][descriptor]") {
    int src = 13, tgt = 14;
    std::string val = "moved";
    edge_descriptor<int, std::string> e1(src, tgt, val);
    edge_descriptor<int, std::string> e2(std::move(e1));
    
    REQUIRE(e2.source_id() == 13);
    REQUIRE(e2.target_id() == 14);
    REQUIRE(e2.value() == "moved");
    
    // Both should still reference the same underlying data
    REQUIRE(&e1.source_id() == &e2.source_id());
    REQUIRE(&e1.value() == &e2.value());
}

TEST_CASE("edge_list::edge_descriptor references underlying data", "[edge_list][descriptor]") {
    std::string src = "source_vertex";
    std::string tgt = "target_vertex";
    std::string val = "edge_data";
    
    edge_descriptor<std::string, std::string> e(src, tgt, val);
    
    REQUIRE(e.source_id() == "source_vertex");
    REQUIRE(e.target_id() == "target_vertex");
    REQUIRE(e.value() == "edge_data");
    
    // Modify the underlying data - descriptor should reflect the change
    src = "new_source";
    REQUIRE(e.source_id() == "new_source");
    
    val = "new_data";
    REQUIRE(e.value() == "new_data");
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
    int src = 15, tgt = 16;
    edge_descriptor<int, void> e(src, tgt);
    std::vector<edge_descriptor<int, void>> el{e};
    
    auto sid = source_id(el, e);
    REQUIRE(sid == 15);
}

TEST_CASE("target_id CPO with edge_list::edge_descriptor", "[cpo][edge_list][tier5]") {
    int src = 17, tgt = 18;
    edge_descriptor<int, void> e(src, tgt);
    std::vector<edge_descriptor<int, void>> el{e};
    
    auto tid = target_id(el, e);
    REQUIRE(tid == 18);
}

TEST_CASE("source_id and target_id with edge_descriptor (with value)", "[cpo][edge_list][tier5]") {
    int src = 19, tgt = 20;
    double val = 4.5;
    edge_descriptor<int, double> e(src, tgt, val);
    std::vector<edge_descriptor<int, double>> el{e};
    
    auto sid = source_id(el, e);
    auto tid = target_id(el, e);
    
    REQUIRE(sid == 19);
    REQUIRE(tid == 20);
}

TEST_CASE("edge_value CPO with edge_list::edge_descriptor", "[cpo][edge_list][tier5]") {
    int src = 21, tgt = 22;
    double val = 5.5;
    edge_descriptor<int, double> e(src, tgt, val);
    std::vector<edge_descriptor<int, double>> el{e};
    
    auto ev = edge_value(el, e);
    REQUIRE(ev == 5.5);
}

TEST_CASE("all CPOs with edge_list::edge_descriptor<string>", "[cpo][edge_list][tier5]") {
    int src = 23, tgt = 24;
    std::string val = "edge_value";
    edge_descriptor<int, std::string> e(src, tgt, val);
    std::vector<edge_descriptor<int, std::string>> el{e};
    
    auto sid = source_id(el, e);
    auto tid = target_id(el, e);
    auto ev = edge_value(el, e);
    
    REQUIRE(sid == 23);
    REQUIRE(tid == 24);
    REQUIRE(ev == "edge_value");
}

// =============================================================================
// Noexcept Tests
// =============================================================================

TEST_CASE("edge_list::edge_descriptor operations are noexcept", "[edge_list][descriptor][noexcept]") {
    int src = 25, tgt = 26;
    double val = 6.5;
    edge_descriptor<int, double> e(src, tgt, val);
    
    STATIC_REQUIRE(noexcept(e.source_id()));
    STATIC_REQUIRE(noexcept(e.target_id()));
    STATIC_REQUIRE(noexcept(e.value()));
}

TEST_CASE("CPOs with edge_list::edge_descriptor are noexcept", "[cpo][edge_list][noexcept]") {
    int src = 27, tgt = 28;
    double val = 7.5;
    edge_descriptor<int, double> e(src, tgt, val);
    std::vector<edge_descriptor<int, double>> el{e};
    
    STATIC_REQUIRE(noexcept(source_id(el, e)));
    STATIC_REQUIRE(noexcept(target_id(el, e)));
    STATIC_REQUIRE(noexcept(edge_value(el, e)));
}

// =============================================================================
// Comparison Tests
// =============================================================================

TEST_CASE("edge_list::edge_descriptor equality", "[edge_list][descriptor]") {
    int src1 = 29, tgt1 = 30;
    double val1 = 8.5;
    edge_descriptor<int, double> e1(src1, tgt1, val1);
    
    int src2 = 29, tgt2 = 30;
    double val2 = 8.5;
    edge_descriptor<int, double> e2(src2, tgt2, val2);
    
    int src3 = 31, tgt3 = 32;
    double val3 = 9.5;
    edge_descriptor<int, double> e3(src3, tgt3, val3);
    
    REQUIRE(e1 == e2);  // Same values
    REQUIRE(e1 != e3);  // Different values
}

TEST_CASE("edge_list::edge_descriptor ordering", "[edge_list][descriptor]") {
    int src1 = 33, tgt1 = 34;
    edge_descriptor<int, void> e1(src1, tgt1);
    
    int src2 = 33, tgt2 = 35;
    edge_descriptor<int, void> e2(src2, tgt2);
    
    int src3 = 34, tgt3 = 34;
    edge_descriptor<int, void> e3(src3, tgt3);
    
    REQUIRE(e1 < e2);
    REQUIRE(e1 < e3);
    REQUIRE(e2 < e3);
}
