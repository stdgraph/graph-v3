cmake_minimum_required(VERSION 3.20)

# Test for edge_list CPO extensions (tiers 5-7)
add_executable(test_edge_list_cpo test_edge_list_cpo.cpp)
target_link_libraries(test_edge_list_cpo PRIVATE graph3 Catch2::Catch2WithMain)
add_test(NAME test_edge_list_cpo COMMAND test_edge_list_cpo)
