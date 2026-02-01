#include <catch2/catch_test_macros.hpp>
#include <graph/views/basic_views.hpp>
#include <vector>
#include <utility>

// Test that basic_views.hpp includes all basic view headers and compiles cleanly
TEST_CASE("basic_views.hpp header compilation", "[basic_views][header]") {
    // Verify the header compiles and provides access to edge_list views
    
    SECTION("edgelist view for edge_list is accessible") {
        std::vector<std::pair<int, int>> el = {{0, 1}, {1, 2}, {2, 3}};
        auto elist = graph::views::edgelist(el);
        REQUIRE(std::ranges::distance(elist) == 3);
    }

    SECTION("edgelist view with value function is accessible") {
        std::vector<std::tuple<int, int, double>> el = {{0, 1, 1.5}, {1, 2, 2.5}};
        // Note: EVF for edge_list receives (EL&, edge) by value
        auto elist = graph::views::edgelist(el, [](auto& /*el*/, auto e) { 
            return std::get<2>(e); 
        });
        REQUIRE(std::ranges::distance(elist) == 2);
    }
}

// Verify that including basic_views.hpp makes all individual view headers available
TEST_CASE("basic_views.hpp includes expected headers", "[basic_views][header]") {
    // These tests just verify the expected namespaces and functions exist
    // after including basic_views.hpp
    
    SECTION("graph::views namespace is accessible") {
        // Verify some expected functions are in scope
        std::vector<std::pair<int, int>> el = {{0, 1}};
        [[maybe_unused]] auto elist = graph::views::edgelist(el);
        SUCCEED("graph::views::edgelist accessible");
    }
}
