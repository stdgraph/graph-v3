# ── Graph I/O tests ───────────────────────────────────────────────────────────

add_executable(graph3_io_tests
  test_io.cpp
)

target_link_libraries(graph3_io_tests
  PRIVATE graph3 Catch2::Catch2WithMain)

catch_discover_tests(graph3_io_tests)
