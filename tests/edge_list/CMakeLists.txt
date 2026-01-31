cmake_minimum_required(VERSION 3.20)

# Test for edge_list CPO extensions (tiers 5-7)
add_executable(test_edge_list_cpo test_edge_list_cpo.cpp)
target_link_libraries(test_edge_list_cpo PRIVATE graph3 Catch2::Catch2WithMain)
add_test(NAME test_edge_list_cpo COMMAND test_edge_list_cpo)

# Test for edge_list::edge_descriptor type
add_executable(test_edge_list_descriptor test_edge_list_descriptor.cpp)
target_link_libraries(test_edge_list_descriptor PRIVATE graph3 Catch2::Catch2WithMain)
add_test(NAME test_edge_list_descriptor COMMAND test_edge_list_descriptor)

# Test for edge_list concepts with unified CPOs
add_executable(test_edge_list_concepts test_edge_list_concepts.cpp)
target_link_libraries(test_edge_list_concepts PRIVATE graph3 Catch2::Catch2WithMain)
add_test(NAME test_edge_list_concepts COMMAND test_edge_list_concepts)

# Integration tests for unified edge_list CPOs
add_executable(test_edge_list_integration test_edge_list_integration.cpp)
target_link_libraries(test_edge_list_integration PRIVATE graph3 Catch2::Catch2WithMain)
add_test(NAME test_edge_list_integration COMMAND test_edge_list_integration)
