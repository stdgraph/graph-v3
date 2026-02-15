#include <catch2/catch_test_macros.hpp>
#include <graph/graph_info.hpp>
#include <type_traits>

using namespace graph;

// Mock types for testing
struct mock_vertex_descriptor {
  int id;
};

struct mock_value {
  double data;
};

TEST_CASE("vertex_info: all 8 specializations compile", "[vertex_info]") {
  SECTION("VId, V, VV all present") {
    vertex_info<int, mock_vertex_descriptor, mock_value> vi{1, mock_vertex_descriptor{1}, mock_value{42.0}};
    REQUIRE(vi.id == 1);
    REQUIRE(vi.vertex.id == 1);
    REQUIRE(vi.value.data == 42.0);
  }

  SECTION("VId, V present; VV=void") {
    vertex_info<int, mock_vertex_descriptor, void> vi{2, mock_vertex_descriptor{2}};
    REQUIRE(vi.id == 2);
    REQUIRE(vi.vertex.id == 2);
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::value_type>);
  }

  SECTION("VId, VV present; V=void") {
    vertex_info<int, void, mock_value> vi{3, mock_value{99.9}};
    REQUIRE(vi.id == 3);
    REQUIRE(vi.value.data == 99.9);
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::vertex_type>);
  }

  SECTION("VId present; V=void, VV=void") {
    vertex_info<int, void, void> vi{4};
    REQUIRE(vi.id == 4);
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::vertex_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::value_type>);
  }

  SECTION("VId=void; V, VV present (descriptor-based pattern)") {
    vertex_info<void, mock_vertex_descriptor, mock_value> vi{mock_vertex_descriptor{5}, mock_value{123.4}};
    REQUIRE(vi.vertex.id == 5);
    REQUIRE(vi.value.data == 123.4);
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::id_type>);
  }

  SECTION("VId=void, VV=void; V present") {
    vertex_info<void, mock_vertex_descriptor, void> vi{mock_vertex_descriptor{6}};
    REQUIRE(vi.vertex.id == 6);
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::value_type>);
  }

  SECTION("VId=void, V=void; VV present") {
    vertex_info<void, void, mock_value> vi{mock_value{77.7}};
    REQUIRE(vi.value.data == 77.7);
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::vertex_type>);
  }

  SECTION("VId=void, V=void, VV=void (empty)") {
    vertex_info<void, void, void> vi{};
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::vertex_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::value_type>);
  }
}

TEST_CASE("vertex_info: structured bindings work correctly", "[vertex_info]") {
  SECTION("All three members") {
    vertex_info<int, mock_vertex_descriptor, mock_value> vi{1, mock_vertex_descriptor{1}, mock_value{42.0}};
    auto [id, v, val] = vi;
    REQUIRE(id == 1);
    REQUIRE(v.id == 1);
    REQUIRE(val.data == 42.0);
  }

  SECTION("Two members: id and vertex") {
    vertex_info<int, mock_vertex_descriptor, void> vi{2, mock_vertex_descriptor{2}};
    auto [id, v] = vi;
    REQUIRE(id == 2);
    REQUIRE(v.id == 2);
  }

  SECTION("Two members: id and value") {
    vertex_info<int, void, mock_value> vi{3, mock_value{99.9}};
    auto [id, val] = vi;
    REQUIRE(id == 3);
    REQUIRE(val.data == 99.9);
  }

  SECTION("One member: id only") {
    vertex_info<int, void, void> vi{4};
    auto [id] = vi;
    REQUIRE(id == 4);
  }

  SECTION("Two members: vertex and value (descriptor-based)") {
    vertex_info<void, mock_vertex_descriptor, mock_value> vi{mock_vertex_descriptor{5}, mock_value{123.4}};
    auto [v, val] = vi;
    REQUIRE(v.id == 5);
    REQUIRE(val.data == 123.4);
  }

  SECTION("One member: vertex only") {
    vertex_info<void, mock_vertex_descriptor, void> vi{mock_vertex_descriptor{6}};
    auto [v] = vi;
    REQUIRE(v.id == 6);
  }

  SECTION("One member: value only") {
    vertex_info<void, void, mock_value> vi{mock_value{77.7}};
    auto [val] = vi;
    REQUIRE(val.data == 77.7);
  }
}

TEST_CASE("vertex_info: sizeof verifies physical absence of void members", "[vertex_info]") {
  SECTION("Full struct vs VId=void reduces size") {
    using full_t  = vertex_info<int, mock_vertex_descriptor, mock_value>;
    using no_id_t = vertex_info<void, mock_vertex_descriptor, mock_value>;

    // No id member should be smaller or equal (padding may prevent strict reduction)
    REQUIRE(sizeof(no_id_t) <= sizeof(full_t));
    // Should be at most the size of the two members plus padding
    REQUIRE(sizeof(no_id_t) <= sizeof(mock_vertex_descriptor) + sizeof(mock_value) + sizeof(int));
  }

  SECTION("VId only struct") {
    using id_only_t = vertex_info<int, void, void>;
    REQUIRE(sizeof(id_only_t) == sizeof(int));
  }

  SECTION("Empty struct") {
    using empty_t = vertex_info<void, void, void>;
    // Empty struct has size 1 in C++ (must be distinct)
    REQUIRE(sizeof(empty_t) >= 1);
  }
}

TEST_CASE("vertex_info: copyable and movable", "[vertex_info]") {
  SECTION("Copy construction") {
    vertex_info<int, mock_vertex_descriptor, mock_value> vi1{1, mock_vertex_descriptor{1}, mock_value{42.0}};
    vertex_info<int, mock_vertex_descriptor, mock_value> vi2 = vi1;
    REQUIRE(vi2.id == vi1.id);
    REQUIRE(vi2.vertex.id == vi1.vertex.id);
    REQUIRE(vi2.value.data == vi1.value.data);
  }

  SECTION("Move construction") {
    vertex_info<int, mock_vertex_descriptor, mock_value> vi1{2, mock_vertex_descriptor{2}, mock_value{99.9}};
    vertex_info<int, mock_vertex_descriptor, mock_value> vi2 = std::move(vi1);
    REQUIRE(vi2.id == 2);
    REQUIRE(vi2.vertex.id == 2);
    REQUIRE(vi2.value.data == 99.9);
  }
}

TEST_CASE("vertex_info: descriptor-based pattern primary use case", "[vertex_info]") {
  // This is the primary pattern for views: vertex_info<void, vertex_descriptor, VV>
  SECTION("Descriptor with value function") {
    vertex_info<void, mock_vertex_descriptor, int> vi{mock_vertex_descriptor{10}, 42};

    auto [v, val] = vi;
    REQUIRE(v.id == 10);
    REQUIRE(val == 42);

    // Verify no id member present
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::id_type>);
    STATIC_REQUIRE(std::is_same_v<decltype(vi)::vertex_type, mock_vertex_descriptor>);
    STATIC_REQUIRE(std::is_same_v<decltype(vi)::value_type, int>);
  }

  SECTION("Descriptor without value function") {
    vertex_info<void, mock_vertex_descriptor, void> vi{mock_vertex_descriptor{20}};

    auto [v] = vi;
    REQUIRE(v.id == 20);

    // Verify only vertex member present
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::id_type>);
    STATIC_REQUIRE(std::is_same_v<decltype(vi)::vertex_type, mock_vertex_descriptor>);
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::value_type>);
  }
}

TEST_CASE("vertex_info: external data pattern use case", "[vertex_info]") {
  // This pattern is for external data: vertex_info<VId, void, VV>
  SECTION("ID and value for graph construction") {
    vertex_info<size_t, void, std::string> vi{42, std::string("vertex_data")};

    auto [id, val] = vi;
    REQUIRE(id == 42);
    REQUIRE(val == "vertex_data");

    // Verify no vertex member present
    STATIC_REQUIRE(std::is_same_v<decltype(vi)::id_type, size_t>);
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::vertex_type>);
    STATIC_REQUIRE(std::is_same_v<decltype(vi)::value_type, std::string>);
  }

  SECTION("ID only for lightweight iteration") {
    vertex_info<size_t, void, void> vi{123};

    auto [id] = vi;
    REQUIRE(id == 123);

    // Verify only id member present
    STATIC_REQUIRE(std::is_same_v<decltype(vi)::id_type, size_t>);
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::vertex_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(vi)::value_type>);
  }
}

TEST_CASE("vertex_info: type traits are correct", "[vertex_info]") {
  SECTION("All type aliases match template parameters") {
    using vi_t = vertex_info<int, mock_vertex_descriptor, mock_value>;
    STATIC_REQUIRE(std::is_same_v<vi_t::id_type, int>);
    STATIC_REQUIRE(std::is_same_v<vi_t::vertex_type, mock_vertex_descriptor>);
    STATIC_REQUIRE(std::is_same_v<vi_t::value_type, mock_value>);
  }

  SECTION("Void type aliases when void") {
    using vi_t = vertex_info<void, void, mock_value>;
    STATIC_REQUIRE(std::is_void_v<vi_t::id_type>);
    STATIC_REQUIRE(std::is_void_v<vi_t::vertex_type>);
    STATIC_REQUIRE(std::is_same_v<vi_t::value_type, mock_value>);
  }
}

TEST_CASE("vertex_info: copyable_vertex_t alias works", "[vertex_info]") {
  SECTION("Alias matches specialized form") {
    using alias_t    = copyable_vertex_t<int, double>;
    using explicit_t = vertex_info<int, void, double>;

    STATIC_REQUIRE(std::is_same_v<alias_t, explicit_t>);
  }

  SECTION("Alias used for external data") {
    copyable_vertex_t<size_t, std::string> cv{99, std::string("data")};
    auto [id, val] = cv;
    REQUIRE(id == 99);
    REQUIRE(val == "data");
  }
}
