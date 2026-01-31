cmake_minimum_required(VERSION 3.20)

# Consolidated edge_list test executable
add_executable(test_edge_list
    test_edge_list_cpo.cpp
    test_edge_list_descriptor.cpp
    test_edge_list_concepts.cpp
    test_edge_list_integration.cpp
)
target_link_libraries(test_edge_list PRIVATE graph3 Catch2::Catch2WithMain)
add_test(NAME test_edge_list COMMAND test_edge_list)
