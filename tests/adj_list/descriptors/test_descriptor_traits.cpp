/**
 * @file test_descriptor_traits.cpp
 * @brief Comprehensive unit tests for descriptor traits and type utilities
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include <graph/adj_list/descriptor_traits.hpp>
#include <graph/adj_list/vertex_descriptor.hpp>
#include <graph/adj_list/edge_descriptor.hpp>
#include <graph/adj_list/vertex_descriptor_view.hpp>
#include <graph/adj_list/edge_descriptor_view.hpp>

#include <vector>
#include <list>
#include <string>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Type Identification Traits Tests
// =============================================================================

TEST_CASE("is_vertex_descriptor trait identifies vertex descriptors", "[traits][vertex_descriptor]") {
    using VectorIter = std::vector<int>::iterator;
    using ListIter = std::list<std::pair<int, double>>::iterator;
    using VD_Vector = vertex_descriptor<VectorIter>;
    using VD_List = vertex_descriptor<ListIter>;
    
    SECTION("Identifies vertex_descriptor types") {
        STATIC_REQUIRE(is_vertex_descriptor_v<VD_Vector>);
        STATIC_REQUIRE(is_vertex_descriptor_v<VD_List>);
    }
    
    SECTION("Rejects non-vertex_descriptor types") {
        STATIC_REQUIRE_FALSE(is_vertex_descriptor_v<int>);
        STATIC_REQUIRE_FALSE(is_vertex_descriptor_v<std::vector<int>>);
        STATIC_REQUIRE_FALSE(is_vertex_descriptor_v<VectorIter>);
    }
    
    SECTION("Works with cv-qualified types") {
        STATIC_REQUIRE(is_vertex_descriptor_v<const VD_Vector>);
        STATIC_REQUIRE(is_vertex_descriptor_v<volatile VD_Vector>);
        STATIC_REQUIRE(is_vertex_descriptor_v<const volatile VD_Vector>);
    }
}

TEST_CASE("is_edge_descriptor trait identifies edge descriptors", "[traits][edge_descriptor]") {
    using VectorIter = std::vector<int>::iterator;
    using EdgeIter = std::vector<int>::iterator;
    using ListIter = std::list<std::pair<int, double>>::iterator;
    using ED_Vector = edge_descriptor<EdgeIter, VectorIter>;
    using ED_List = edge_descriptor<ListIter, VectorIter>;
    
    SECTION("Identifies edge_descriptor types") {
        STATIC_REQUIRE(is_edge_descriptor_v<ED_Vector>);
        STATIC_REQUIRE(is_edge_descriptor_v<ED_List>);
    }
    
    SECTION("Rejects non-edge_descriptor types") {
        STATIC_REQUIRE_FALSE(is_edge_descriptor_v<int>);
        STATIC_REQUIRE_FALSE(is_edge_descriptor_v<std::vector<int>>);
        STATIC_REQUIRE_FALSE(is_edge_descriptor_v<vertex_descriptor<VectorIter>>);
    }
}

TEST_CASE("is_descriptor trait identifies any descriptor", "[traits][descriptor]") {
    using VectorIter = std::vector<int>::iterator;
    using VD = vertex_descriptor<VectorIter>;
    using ED = edge_descriptor<VectorIter, VectorIter>;
    
    SECTION("Identifies both vertex and edge descriptors") {
        STATIC_REQUIRE(is_descriptor_v<VD>);
        STATIC_REQUIRE(is_descriptor_v<ED>);
    }
    
    SECTION("Rejects non-descriptor types") {
        STATIC_REQUIRE_FALSE(is_descriptor_v<int>);
        STATIC_REQUIRE_FALSE(is_descriptor_v<std::string>);
        STATIC_REQUIRE_FALSE(is_descriptor_v<VectorIter>);
    }
}

// =============================================================================
// View Traits Tests
// =============================================================================

TEST_CASE("is_vertex_descriptor_view trait identifies vertex views", "[traits][vertex_view]") {
    using VectorIter = std::vector<int>::iterator;
    using VDView = vertex_descriptor_view<VectorIter>;
    
    SECTION("Identifies vertex_descriptor_view types") {
        STATIC_REQUIRE(is_vertex_descriptor_view_v<VDView>);
    }
    
    SECTION("Rejects non-view types") {
        STATIC_REQUIRE_FALSE(is_vertex_descriptor_view_v<int>);
        STATIC_REQUIRE_FALSE(is_vertex_descriptor_view_v<vertex_descriptor<VectorIter>>);
    }
}

TEST_CASE("is_edge_descriptor_view trait identifies edge views", "[traits][edge_view]") {
    using VectorIter = std::vector<int>::iterator;
    using EDView = edge_descriptor_view<VectorIter, VectorIter>;
    
    SECTION("Identifies edge_descriptor_view types") {
        STATIC_REQUIRE(is_edge_descriptor_view_v<EDView>);
    }
    
    SECTION("Rejects non-view types") {
        STATIC_REQUIRE_FALSE(is_edge_descriptor_view_v<int>);
        STATIC_REQUIRE_FALSE(is_edge_descriptor_view_v<edge_descriptor<VectorIter, VectorIter>>);
    }
}

TEST_CASE("is_descriptor_view trait identifies any view", "[traits][view]") {
    using VectorIter = std::vector<int>::iterator;
    using VDView = vertex_descriptor_view<VectorIter>;
    using EDView = edge_descriptor_view<VectorIter, VectorIter>;
    
    SECTION("Identifies both vertex and edge views") {
        STATIC_REQUIRE(is_descriptor_view_v<VDView>);
        STATIC_REQUIRE(is_descriptor_view_v<EDView>);
    }
    
    SECTION("Rejects non-view types") {
        STATIC_REQUIRE_FALSE(is_descriptor_view_v<int>);
        STATIC_REQUIRE_FALSE(is_descriptor_view_v<vertex_descriptor<VectorIter>>);
    }
}

// =============================================================================
// Type Extraction Traits Tests
// =============================================================================

TEST_CASE("descriptor_iterator_type extracts iterator type", "[traits][type_extraction]") {
    using VectorIter = std::vector<int>::iterator;
    using ListIter = std::list<std::pair<int, double>>::iterator;
    using VD_Vector = vertex_descriptor<VectorIter>;
    using VD_List = vertex_descriptor<ListIter>;
    
    SECTION("Extracts correct iterator type from vertex_descriptor") {
        STATIC_REQUIRE(std::same_as<descriptor_iterator_type_t<VD_Vector>, VectorIter>);
        STATIC_REQUIRE(std::same_as<descriptor_iterator_type_t<VD_List>, ListIter>);
    }
    
    SECTION("Extracts correct iterator type from vertex_descriptor_view") {
        using VDView = vertex_descriptor_view<VectorIter>;
        STATIC_REQUIRE(std::same_as<descriptor_iterator_type_t<VDView>, VectorIter>);
    }
}

TEST_CASE("edge_descriptor iterator type extraction", "[traits][type_extraction]") {
    using VectorIter = std::vector<int>::iterator;
    using ListIter = std::list<int>::iterator;
    using ED = edge_descriptor<ListIter, VectorIter>;
    
    SECTION("Extracts edge iterator type") {
        STATIC_REQUIRE(std::same_as<edge_descriptor_edge_iterator_type_t<ED>, ListIter>);
    }
    
    SECTION("Extracts vertex iterator type") {
        STATIC_REQUIRE(std::same_as<edge_descriptor_vertex_iterator_type_t<ED>, VectorIter>);
    }
}

TEST_CASE("descriptor_storage_type extracts storage type", "[traits][type_extraction]") {
    using VectorIter = std::vector<int>::iterator;
    using ListIter = std::list<std::pair<int, double>>::iterator;
    using VD_Vector = vertex_descriptor<VectorIter>;
    using VD_List = vertex_descriptor<ListIter>;
    
    SECTION("Random access iterator uses size_t storage") {
        STATIC_REQUIRE(std::same_as<descriptor_storage_type_t<VD_Vector>, std::size_t>);
    }
    
    SECTION("Bidirectional iterator uses iterator storage") {
        STATIC_REQUIRE(std::same_as<descriptor_storage_type_t<VD_List>, ListIter>);
    }
}

TEST_CASE("edge_descriptor_storage_type extracts edge storage type", "[traits][type_extraction]") {
    using VectorIter = std::vector<int>::iterator;
    using ListIter = std::list<int>::iterator;
    using ED_Vector = edge_descriptor<VectorIter, VectorIter>;
    using ED_List = edge_descriptor<ListIter, VectorIter>;
    
    SECTION("Random access edge iterator uses size_t storage") {
        STATIC_REQUIRE(std::same_as<edge_descriptor_storage_type_t<ED_Vector>, std::size_t>);
    }
    
    SECTION("Forward edge iterator uses iterator storage") {
        STATIC_REQUIRE(std::same_as<edge_descriptor_storage_type_t<ED_List>, ListIter>);
    }
}

// =============================================================================
// Storage Category Traits Tests
// =============================================================================

TEST_CASE("is_random_access_descriptor identifies random access descriptors", "[traits][storage_category]") {
    using VectorIter = std::vector<int>::iterator;
    using ListIter = std::list<std::pair<int, double>>::iterator;
    using VD_Vector = vertex_descriptor<VectorIter>;
    using VD_List = vertex_descriptor<ListIter>;
    
    SECTION("Random access iterators produce random access descriptors") {
        STATIC_REQUIRE(is_random_access_descriptor_v<VD_Vector>);
    }
    
    SECTION("Non-random access iterators don't produce random access descriptors") {
        STATIC_REQUIRE_FALSE(is_random_access_descriptor_v<VD_List>);
    }
}

TEST_CASE("is_iterator_based_descriptor identifies iterator-based descriptors", "[traits][storage_category]") {
    using VectorIter = std::vector<int>::iterator;
    using ListIter = std::list<std::pair<int, double>>::iterator;
    using VD_Vector = vertex_descriptor<VectorIter>;
    using VD_List = vertex_descriptor<ListIter>;
    
    SECTION("Non-random access iterators produce iterator-based descriptors") {
        STATIC_REQUIRE(is_iterator_based_descriptor_v<VD_List>);
    }
    
    SECTION("Random access iterators don't produce iterator-based descriptors") {
        STATIC_REQUIRE_FALSE(is_iterator_based_descriptor_v<VD_Vector>);
    }
}

TEST_CASE("Storage categories are mutually exclusive", "[traits][storage_category]") {
    using VectorIter = std::vector<int>::iterator;
    using ListIter = std::list<std::pair<int, double>>::iterator;
    using VD_Vector = vertex_descriptor<VectorIter>;
    using VD_List = vertex_descriptor<ListIter>;
    
    SECTION("Random access and iterator-based are mutually exclusive") {
        STATIC_REQUIRE(is_random_access_descriptor_v<VD_Vector> != 
                      is_iterator_based_descriptor_v<VD_Vector>);
        STATIC_REQUIRE(is_random_access_descriptor_v<VD_List> != 
                      is_iterator_based_descriptor_v<VD_List>);
    }
}

// =============================================================================
// Concept Tests
// =============================================================================

TEST_CASE("vertex_descriptor_type concept", "[traits][concepts]") {
    using VectorIter = std::vector<int>::iterator;
    using VD = vertex_descriptor<VectorIter>;
    using ED = edge_descriptor<VectorIter, VectorIter>;
    
    SECTION("Accepts vertex descriptors") {
        STATIC_REQUIRE(vertex_descriptor_type<VD>);
        STATIC_REQUIRE(vertex_descriptor_type<const VD>);
        STATIC_REQUIRE(vertex_descriptor_type<VD&>);
        STATIC_REQUIRE(vertex_descriptor_type<const VD&>);
    }
    
    SECTION("Rejects non-vertex descriptors") {
        STATIC_REQUIRE_FALSE(vertex_descriptor_type<ED>);
        STATIC_REQUIRE_FALSE(vertex_descriptor_type<int>);
    }
}

TEST_CASE("edge_descriptor_type concept", "[traits][concepts]") {
    using VectorIter = std::vector<int>::iterator;
    using VD = vertex_descriptor<VectorIter>;
    using ED = edge_descriptor<VectorIter, VectorIter>;
    
    SECTION("Accepts edge descriptors") {
        STATIC_REQUIRE(edge_descriptor_type<ED>);
        STATIC_REQUIRE(edge_descriptor_type<const ED>);
        STATIC_REQUIRE(edge_descriptor_type<ED&>);
    }
    
    SECTION("Rejects non-edge descriptors") {
        STATIC_REQUIRE_FALSE(edge_descriptor_type<VD>);
        STATIC_REQUIRE_FALSE(edge_descriptor_type<int>);
    }
}

TEST_CASE("descriptor_type concept", "[traits][concepts]") {
    using VectorIter = std::vector<int>::iterator;
    using VD = vertex_descriptor<VectorIter>;
    using ED = edge_descriptor<VectorIter, VectorIter>;
    
    SECTION("Accepts any descriptor") {
        STATIC_REQUIRE(descriptor_type<VD>);
        STATIC_REQUIRE(descriptor_type<ED>);
    }
    
    SECTION("Rejects non-descriptors") {
        STATIC_REQUIRE_FALSE(descriptor_type<int>);
        STATIC_REQUIRE_FALSE(descriptor_type<std::string>);
    }
}

TEST_CASE("random_access_descriptor concept", "[traits][concepts]") {
    using VectorIter = std::vector<int>::iterator;
    using ListIter = std::list<std::pair<int, double>>::iterator;
    using VD_Vector = vertex_descriptor<VectorIter>;
    using VD_List = vertex_descriptor<ListIter>;
    
    SECTION("Accepts random access descriptors") {
        STATIC_REQUIRE(random_access_descriptor<VD_Vector>);
    }
    
    SECTION("Rejects non-random access descriptors") {
        STATIC_REQUIRE_FALSE(random_access_descriptor<VD_List>);
        STATIC_REQUIRE_FALSE(random_access_descriptor<int>);
    }
}

TEST_CASE("iterator_based_descriptor concept", "[traits][concepts]") {
    using VectorIter = std::vector<int>::iterator;
    using ListIter = std::list<std::pair<int, double>>::iterator;
    using VD_Vector = vertex_descriptor<VectorIter>;
    using VD_List = vertex_descriptor<ListIter>;
    
    SECTION("Accepts iterator-based descriptors") {
        STATIC_REQUIRE(iterator_based_descriptor<VD_List>);
    }
    
    SECTION("Rejects non-iterator-based descriptors") {
        STATIC_REQUIRE_FALSE(iterator_based_descriptor<VD_Vector>);
        STATIC_REQUIRE_FALSE(iterator_based_descriptor<int>);
    }
}

// =============================================================================
// Utility Function Tests
// =============================================================================

TEST_CASE("descriptor_category returns correct string", "[traits][utilities]") {
    using VectorIter = std::vector<int>::iterator;
    using VD = vertex_descriptor<VectorIter>;
    using ED = edge_descriptor<VectorIter, VectorIter>;
    using VDView = vertex_descriptor_view<VectorIter>;
    using EDView = edge_descriptor_view<VectorIter, VectorIter>;
    
    SECTION("Returns correct category for vertex_descriptor") {
        REQUIRE(std::string(descriptor_category<VD>()) == "vertex_descriptor");
    }
    
    SECTION("Returns correct category for edge_descriptor") {
        REQUIRE(std::string(descriptor_category<ED>()) == "edge_descriptor");
    }
    
    SECTION("Returns correct category for vertex_descriptor_view") {
        REQUIRE(std::string(descriptor_category<VDView>()) == "vertex_descriptor_view");
    }
    
    SECTION("Returns correct category for edge_descriptor_view") {
        REQUIRE(std::string(descriptor_category<EDView>()) == "edge_descriptor_view");
    }
    
    SECTION("Returns 'not_a_descriptor' for non-descriptor types") {
        REQUIRE(std::string(descriptor_category<int>()) == "not_a_descriptor");
    }
}

TEST_CASE("storage_category returns correct string", "[traits][utilities]") {
    using VectorIter = std::vector<int>::iterator;
    using ListIter = std::list<std::pair<int, double>>::iterator;
    using VD_Vector = vertex_descriptor<VectorIter>;
    using VD_List = vertex_descriptor<ListIter>;
    
    SECTION("Returns 'random_access' for random access descriptors") {
        REQUIRE(std::string(storage_category<VD_Vector>()) == "random_access");
    }
    
    SECTION("Returns 'iterator_based' for iterator-based descriptors") {
        REQUIRE(std::string(storage_category<VD_List>()) == "iterator_based");
    }
    
    SECTION("Returns 'unknown' for non-descriptor types") {
        REQUIRE(std::string(storage_category<int>()) == "unknown");
    }
}

// =============================================================================
// Integration Tests with Generic Code
// =============================================================================

TEST_CASE("Traits enable generic programming with descriptors", "[traits][integration]") {
    using VectorIter = std::vector<int>::iterator;
    using VD = vertex_descriptor<VectorIter>;
    
    SECTION("Can write generic code using concepts") {
        auto process_vertex = []<vertex_descriptor_type T>(const T& vd) {
            return vd.vertex_id();
        };
        
        VD vd{5};
        REQUIRE(process_vertex(vd) == 5);
    }
    
    SECTION("Can use SFINAE with type traits") {
        auto get_storage_info = []<typename T>(const T&) -> std::string {
            if constexpr (is_random_access_descriptor_v<T>) {
                return "uses index storage";
            } else if constexpr (is_iterator_based_descriptor_v<T>) {
                return "uses iterator storage";
            } else {
                return "not a descriptor";
            }
        };
        
        VD vd{5};
        REQUIRE(get_storage_info(vd) == "uses index storage");
        REQUIRE(get_storage_info(42) == "not a descriptor");
    }
}
