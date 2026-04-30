# ── Graph generator tests ─────────────────────────────────────────────────────

add_executable(graph3_generator_tests
  test_generators.cpp
)

target_link_libraries(graph3_generator_tests
  PRIVATE graph3 Catch2::Catch2WithMain)

catch_discover_tests(graph3_generator_tests)
