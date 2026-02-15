#include <catch2/catch_test_macros.hpp>
#include <graph/views/view_concepts.hpp>
#include <string>

using namespace graph::views;

// Mock graph type for testing
struct mock_graph {
  int data;
};

// Mock descriptor types for testing
struct mock_vertex_descriptor {
  std::size_t id;
};

struct mock_edge_descriptor {
  std::size_t source;
  std::size_t target;
};

// Mock value functions (now take graph + descriptor)
auto valid_vertex_value_fn = [](const mock_graph&, mock_vertex_descriptor) { return 42; };
auto valid_edge_value_fn   = [](const mock_graph&, mock_edge_descriptor) { return 3.14; };

auto void_vertex_fn = [](const mock_graph&, mock_vertex_descriptor) {};
auto void_edge_fn   = [](const mock_graph&, mock_edge_descriptor) {};

struct not_invocable {
  int x;
};

// Mock search view for testing
struct mock_search_view {
  cancel_search cancel() { return cancel_search::continue_search; }
  std::size_t   depth() const { return 5; }
  std::size_t   num_visited() const { return 10; }
};

struct incomplete_search_view {
  std::size_t depth() const { return 5; }
  std::size_t num_visited() const { return 10; };
  // Missing cancel()
};

TEST_CASE("vertex_value_function concept", "[views][concepts]") {
  SECTION("valid value functions") {
    // Lambda returning int
    STATIC_REQUIRE(vertex_value_function<decltype(valid_vertex_value_fn), mock_graph, mock_vertex_descriptor>);

    // Lambda returning string
    auto string_fn = [](const mock_graph&, mock_vertex_descriptor) { return std::string("test"); };
    STATIC_REQUIRE(vertex_value_function<decltype(string_fn), mock_graph, mock_vertex_descriptor>);

    // Function pointer
    using fn_ptr = int (*)(const mock_graph&, mock_vertex_descriptor);
    STATIC_REQUIRE(vertex_value_function<fn_ptr, mock_graph, mock_vertex_descriptor>);

    // Generic lambda
    auto generic_fn = [](const auto&, auto vdesc) { return vdesc.id; };
    STATIC_REQUIRE(vertex_value_function<decltype(generic_fn), mock_graph, mock_vertex_descriptor>);
  }

  SECTION("invalid value functions") {
    // Returns void
    STATIC_REQUIRE_FALSE(vertex_value_function<decltype(void_vertex_fn), mock_graph, mock_vertex_descriptor>);

    // Not invocable
    STATIC_REQUIRE_FALSE(vertex_value_function<not_invocable, mock_graph, mock_vertex_descriptor>);

    // Wrong parameter type (edge fn used as vertex fn)
    STATIC_REQUIRE_FALSE(vertex_value_function<decltype(valid_edge_value_fn), mock_graph, mock_vertex_descriptor>);
  }
}

TEST_CASE("edge_value_function concept", "[views][concepts]") {
  SECTION("valid value functions") {
    // Lambda returning double
    STATIC_REQUIRE(edge_value_function<decltype(valid_edge_value_fn), mock_graph, mock_edge_descriptor>);

    // Lambda returning string
    auto string_fn = [](const mock_graph&, mock_edge_descriptor) { return std::string("edge"); };
    STATIC_REQUIRE(edge_value_function<decltype(string_fn), mock_graph, mock_edge_descriptor>);

    // Function pointer
    using fn_ptr = double (*)(const mock_graph&, mock_edge_descriptor);
    STATIC_REQUIRE(edge_value_function<fn_ptr, mock_graph, mock_edge_descriptor>);

    // Generic lambda
    auto generic_fn = [](const auto&, auto edesc) { return edesc.source + edesc.target; };
    STATIC_REQUIRE(edge_value_function<decltype(generic_fn), mock_graph, mock_edge_descriptor>);
  }

  SECTION("invalid value functions") {
    // Returns void
    STATIC_REQUIRE_FALSE(edge_value_function<decltype(void_edge_fn), mock_graph, mock_edge_descriptor>);

    // Not invocable
    STATIC_REQUIRE_FALSE(edge_value_function<not_invocable, mock_graph, mock_edge_descriptor>);

    // Wrong parameter type (vertex fn used as edge fn)
    STATIC_REQUIRE_FALSE(edge_value_function<decltype(valid_vertex_value_fn), mock_graph, mock_edge_descriptor>);
  }
}

TEST_CASE("search_view concept", "[views][concepts]") {
  SECTION("valid search view") {
    STATIC_REQUIRE(search_view<mock_search_view>);

    // Test at runtime that the mock actually works
    mock_search_view view;
    REQUIRE(view.cancel() == cancel_search::continue_search);
    REQUIRE(view.depth() == 5);
    REQUIRE(view.num_visited() == 10);
  }

  SECTION("invalid search view - missing cancel()") { STATIC_REQUIRE_FALSE(search_view<incomplete_search_view>); }

  SECTION("invalid search view - not a type with required methods") {
    STATIC_REQUIRE_FALSE(search_view<int>);
    STATIC_REQUIRE_FALSE(search_view<not_invocable>);
  }
}

TEST_CASE("concept interaction with actual types", "[views][concepts]") {
  SECTION("value functions with different return types") {
    // int return
    auto int_fn = [](const mock_graph&, mock_vertex_descriptor) { return 42; };
    STATIC_REQUIRE(vertex_value_function<decltype(int_fn), mock_graph, mock_vertex_descriptor>);

    // double return
    auto double_fn = [](const mock_graph&, mock_vertex_descriptor) { return 3.14; };
    STATIC_REQUIRE(vertex_value_function<decltype(double_fn), mock_graph, mock_vertex_descriptor>);

    // string return
    auto string_fn = [](const mock_graph&, mock_vertex_descriptor) { return std::string("test"); };
    STATIC_REQUIRE(vertex_value_function<decltype(string_fn), mock_graph, mock_vertex_descriptor>);

    // struct return
    struct custom_value {
      int x;
      int y;
    };
    auto struct_fn = [](const mock_graph&, mock_vertex_descriptor) { return custom_value{1, 2}; };
    STATIC_REQUIRE(vertex_value_function<decltype(struct_fn), mock_graph, mock_vertex_descriptor>);
  }

  SECTION("mutable lambdas") {
    // Mutable lambda (captures by value and modifies)
    int  counter    = 0;
    auto mutable_fn = [counter](const mock_graph&, mock_vertex_descriptor) mutable {
      ++counter;
      return counter;
    };
    STATIC_REQUIRE(vertex_value_function<decltype(mutable_fn), mock_graph, mock_vertex_descriptor>);
  }

  SECTION("capturing lambdas") {
    int  multiplier   = 10;
    auto capturing_fn = [&multiplier](const mock_graph&, mock_vertex_descriptor vdesc) {
      return static_cast<int>(vdesc.id) * multiplier;
    };
    STATIC_REQUIRE(vertex_value_function<decltype(capturing_fn), mock_graph, mock_vertex_descriptor>);
  }
}
