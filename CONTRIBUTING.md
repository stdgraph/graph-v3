# Contributing to graph-v3

Thank you for your interest in contributing! This document covers how to build, test, and
submit changes to the project.

---

## Requirements

- **C++20** compiler (GCC 13+, Clang 10+, MSVC 2022+)
- **CMake 3.20+**
- **Ninja** (recommended generator; installed by default on most Linux distros)

Dependencies (Catch2 v3, `tl::expected`) are fetched automatically via CMake FetchContent.

---

## Building

The project uses **CMake presets** for reproducible builds. Pick the preset that matches
your platform and toolchain:

### Configure + Build

```bash
# Configure
cmake --preset <preset-name>

# Build
cmake --build --preset <preset-name>
```

### Available Presets

| Preset | Platform | Compiler | Type |
|--------|----------|----------|------|
| `linux-gcc-debug` | Linux | GCC | Debug |
| `linux-gcc-release` | Linux | GCC | Release |
| `linux-clang-debug` | Linux | Clang | Debug |
| `linux-clang-release` | Linux | Clang | Release |
| `macos-clang-debug` | macOS | Clang | Debug |
| `macos-clang-release` | macOS | Clang | Release |
| `windows-msvc-debug` | Windows | MSVC | Debug |
| `windows-msvc-release` | Windows | MSVC | Release |
| `windows-msvc-relwithdebinfo` | Windows | MSVC | RelWithDebInfo |
| `windows-clang-debug` | Windows | Clang-cl | Debug |
| `windows-clang-release` | Windows | Clang-cl | Release |

### Quick Start (Linux / GCC)

```bash
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug
```

---

## Running Tests

Tests use [Catch2 v3](https://github.com/catchorg/Catch2) and are run via CTest:

```bash
# Run all tests
ctest --preset linux-gcc-debug

# Run a subset by regex
ctest --preset linux-gcc-debug -R dijkstra

# Verbose output
ctest --preset linux-gcc-debug -V
```

All 4261 tests must pass before submitting a PR.

---

## Sanitizers

Dedicated presets enable AddressSanitizer, ThreadSanitizer, and MemorySanitizer:

| Preset | Sanitizer |
|--------|-----------|
| `linux-gcc-asan` | AddressSanitizer + UndefinedBehaviorSanitizer |
| `linux-gcc-tsan` | ThreadSanitizer |
| `linux-clang-msan` | MemorySanitizer |

```bash
cmake --preset linux-gcc-asan
cmake --build --preset linux-gcc-asan
ctest --preset linux-gcc-asan
```

---

## Code Coverage

```bash
cmake --preset linux-gcc-coverage
cmake --build --preset linux-gcc-coverage
ctest --preset linux-gcc-coverage
```

Coverage reports are generated using the `CodeCoverage.cmake` module. See
[cmake/CodeCoverage.cmake](cmake/CodeCoverage.cmake) for details.

---

## Code Style

- **Formatting:** clang-format (run `scripts/format.sh` to format all source files)
- **Naming conventions:** follow [docs/common_graph_guidelines.md](docs/common_graph_guidelines.md)
- **CPO implementation patterns:** follow [docs/graph_cpo_implementation.md](docs/graph_cpo_implementation.md)
- Header-only; all code lives under `include/graph/`
- Tests live under `tests/` mirroring the source layout

---

## Adding New Functionality

### New Algorithm

1. Create `include/graph/algorithm/<name>.hpp`. Create documentation using `docs/algorithm_template.md`.
2. Add the include to `include/graph/algorithms.hpp`
3. Create `tests/algorithms/test_<name>.cpp` and register it in `tests/CMakeLists.txt`
4. Update `docs/status/implementation_matrix.md`

### New View

1. Create `include/graph/views/<name>.hpp`. Create documentation using `docs/view_template.md`.
2. Add the include to `include/graph/views.hpp` (if an umbrella header exists)
3. Create `tests/views/test_<name>.cpp` and register it in `tests/CMakeLists.txt`
4. Update `docs/status/implementation_matrix.md`

### New Container Trait Combination

1. Create `include/graph/container/traits/<combo>_graph_traits.hpp`
2. Create corresponding test under `tests/container/`
3. Update `docs/status/implementation_matrix.md`

---

## Pull Request Process

1. Fork the repository and create a feature branch from `main`
2. Make your changes, ensuring:
   - All existing tests still pass
   - New functionality has tests
   - Code is formatted (`scripts/format.sh`)
   - Documentation is updated (metrics, implementation matrix)
3. Open a pull request with a clear description of the change

---

## Project Layout

For architecture details, namespace structure, and design decisions, see:

- [docs/index.md](docs/index.md) — documentation hub
- [docs/common_graph_guidelines.md](docs/common_graph_guidelines.md) — conventions and project structure
- [docs/container_interface.md](docs/container_interface.md) — Graph Container Interface specification

---

## License

By contributing, you agree that your contributions will be licensed under the
[Boost Software License 1.0](LICENSE).
