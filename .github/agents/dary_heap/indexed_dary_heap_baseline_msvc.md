# Dijkstra MSVC Baseline Benchmarks

Captured: 2026-04-26
Branch: `indexed-dary-heap` (HEAD `281fc7a`, working tree clean)
Binary: `build/windows-msvc-release/benchmark/algorithms/benchmark_dijkstra.exe`
Toolchain: MSVC 19.50.35729 (Visual Studio 18.5.1, host x64, target x64)
Build flags: default `windows-msvc-release` preset (`/O2 /Ob2 /DNDEBUG`)
Benchmark flags: `--benchmark_min_time=1s --benchmark_repetitions=5
--benchmark_report_aggregates_only=true`
Process pinning: single physical core (affinity mask `0x1`), priority class
`High`. Run was split into four per-topology batches to stay inside the
session's command-tool timeout; each batch is independent.

## Machine

| Property | Value |
|----------|-------|
| Host | Titania (same as Linux baseline / Phase 4.x results) |
| CPUs | 20 × 3610 MHz |
| L1-D | 48 KiB × 10 |
| L1-I | 32 KiB × 10 |
| L2 | 1280 KiB × 10 |
| L3 | 25600 KiB × 1 |
| OS | Windows |

The hardware exactly matches the Linux Phase 0 baseline and Phase 4.x
comparative runs, so any differences vs. those numbers reflect the
toolchain (MSVC vs. GCC) and the C++ standard library (MSVC STL vs.
libstdc++), not the machine.

## Methodology notes (vs. the Linux baseline)

| Concern | Linux baseline | MSVC baseline (this file) |
|---------|----------------|---------------------------|
| Frequency scaling | `cpupower frequency-set -g performance` | Process priority `High`; no governor knob on Windows. CV reported per row. |
| Single-core pin | `taskset -c 4` | `Process.ProcessorAffinity = 0x1` |
| min_time | `1s` (Phase 0 baseline) / `2s` (Phase 4.3a) | `1s` |
| Repetitions | 1 (Phase 0) / 3 (Phase 4.x averages) | 5 (median + CV reported) |
| Aggregation | mean of repetitions | median of 5 repetitions |

## Heap variants compared

| Tag | Description |
|-----|-------------|
| **Default** | `use_default_heap` — `std::priority_queue`, lazy deletion |
| **Idx2** | `use_indexed_dary_heap<2>` — binary heap, true decrease-key |
| **Idx4** | `use_indexed_dary_heap<4>` — 4-ary heap, true decrease-key |
| **Idx8** | `use_indexed_dary_heap<8>` — 8-ary heap, true decrease-key |

---

## Results — CSR (`compressed_graph`)

All times are wall-clock (real_time) nanoseconds per Dijkstra call,
**median of 5 repetitions**. CV is the coefficient of variation (real_time)
over those 5 repetitions; rows where CV exceeds 5 % are flagged with `†`.

### Erdős–Rényi sparse, E/V ≈ 8

| Heap | 1K ns | 10K ns | 100K ns | CV at 100K |
|------|------:|-------:|--------:|-----------:|
| Default | 69,406 | 1,288,481 | 26,655,249 | 1.58 % |
| Idx2 | 47,768 | 1,171,749 | 26,422,283 | 4.50 % |
| Idx4 | 54,294 | 1,063,524 | 21,124,883 | 4.63 % |
| Idx8 | 70,041 | 1,290,562 | 22,223,590 | 0.49 % |

### 2D Grid, E/V ≈ 4

| Heap | 1K ns | 10K ns | 100K ns | CV at 100K |
|------|------:|-------:|--------:|-----------:|
| Default | 24,939 | 510,742 | 6,190,532 | 1.16 % |
| Idx2 | 21,970 | 536,500 | 6,927,086 | 1.15 % |
| Idx4 | 27,455 | 544,960 | 6,873,101 | 0.52 % |
| Idx8 | 32,708 | 673,041 | 8,246,247 | 1.26 % |

### Barabási–Albert, m=4, E/V ≈ 8

| Heap | 1K ns | 10K ns | 100K ns | CV at 100K |
|------|------:|-------:|--------:|-----------:|
| Default | 70,715 | 1,279,933 | 25,268,386 | 0.82 % |
| Idx2 | 43,928 | 1,133,997 | 25,353,973 | 1.65 % |
| Idx4 | 50,625 | 1,024,064 | 19,603,770 | 0.80 % |
| Idx8 | 67,539 | 1,244,579 | 22,194,943 | 8.92 % † |

### Path, E/V = 1

| Heap | 1K ns | 10K ns | 100K ns | CV at 100K |
|------|------:|-------:|--------:|-----------:|
| Default | 13,241 | 132,771 | 1,331,743 | 0.27 % |
| Idx2 | 4,568 | 43,392 | 485,695 | 1.80 % |
| Idx4 | 4,510 | 43,019 | 498,438 | 1.77 % |
| Idx8 | 4,446 | 42,962 | 491,302 | 0.66 % |

`†` BA Idx8 at 100K had a single high-variance run (one outlier of the
five repetitions). The mean and median are within 4 % of each other so the
median number is robust; treat the absolute level as ±10 % until rerun.

---

## MSVC vs. Linux baseline — same machine, same hardware

CSR 100K, median (MSVC) vs. Phase 0 baseline / Phase 4.1 results (Linux GCC),
both wall-clock ns per call:

| Topology | Heap | MSVC ns | GCC ns (ref) | MSVC ÷ GCC | Source for GCC |
|----------|------|--------:|-------------:|-----------:|----------------|
| ER Sparse | Default | 26,655,249 | 27,049,885 | 0.99× | results §4.1 ER |
| ER Sparse | Idx4 | 21,124,883 | 25,756,981 | 0.82× | results §4.1 ER |
| ER Sparse | Idx8 | 22,223,590 | 20,216,860 | 1.10× | results §4.1 ER |
| Grid | Default | 6,190,532 | 6,026,301 | 1.03× | results §4.1 Grid |
| Grid | Idx4 | 6,873,101 | 8,165,088 | 0.84× | results §4.1 Grid |
| Grid | Idx8 | 8,246,247 | 8,400,126 | 0.98× | results §4.1 Grid |
| BA | Default | 25,268,386 | 22,904,717 | 1.10× | results §4.1 BA |
| BA | Idx4 | 19,603,770 | 19,998,964 | 0.98× | results §4.1 BA |
| BA | Idx8 | 22,194,943 | 19,038,871 | 1.17× † | results §4.1 BA |
| Path | Default | 1,331,743 | 268,708 | 4.96× ‼ | results §4.1 Path |
| Path | Idx4 | 498,438 | 326,018 | 1.53× | results §4.1 Path |
| Path | Idx8 | 491,302 | 327,820 | 1.50× | results §4.1 Path |

`†` BA Idx8 100K MSVC has CV 8.9 %; the 1.17× ratio may shift on rerun.
`‼` Path/Default shows the **largest divergence** between toolchains. With
the indexed heap the ratio drops to ~1.5×, suggesting MSVC's
`std::priority_queue` codegen is materially slower than libstdc++'s on this
no-decrease-key workload — the new heap path is much closer to GCC parity.

### Cross-topology relative ordering — does the Phase 4.x story hold under MSVC?

| Topology | Best heap (Linux GCC, Phase 4.1) | Best heap (MSVC, this run) | Same? |
|----------|----------------------------------|----------------------------|-------|
| ER Sparse | Idx8 (−25 %) | **Idx4** (−21 % vs Default), Idx8 close (−17 %) | Idx4 / Idx8 swap; both clearly beat Default |
| Grid | Default | **Default** (Idx2/Idx4 within +11 %; Idx8 +33 %) | ✅ |
| BA | Idx8 (−17 %) | **Idx4** (−22 % vs Default), Idx8 noisy | Mostly ✅ — both indexed variants win |
| Path | Default | **Default** for absolute time, indexed wins by 2.7× — see below | ⚠ swap |

The Path case is now the **opposite** of the Linux story: under MSVC the
indexed heap is **2.7× faster** than the default at 100K (491k ns vs
1.33M ns), whereas under GCC the default was 22 % faster than the indexed
heap. This is the headline MSVC-specific finding.

## Recommendation update

The Phase 4.2 default decision (`use_default_heap` is the public default)
holds under MSVC:

- It still wins or ties on Grid.
- It loses badly on Path under MSVC (where it lost slightly to indexed under
  GCC), but Path is a degenerate case (no decrease-key opportunity); the
  indexed-heap recommendation already covers it.
- On dense / scale-free workloads (ER, BA), Idx4 is now slightly better than
  Idx8 under MSVC at n = 100 K — opposite of GCC. The Phase 4.2
  recommendation of `use_indexed_dary_heap<8>` should be **softened to
  "Idx4 or Idx8"** for the MSVC documentation, with a note that the
  toolchain affects the optimum.

These observations are **not strong enough to change any defaults or
recommendations** — they are baseline numbers for the next phase
(Phase 4.3b on Windows: VTune Microarchitecture Exploration of the relax
loop). Their purpose is to anchor every later VTune number to a known-good
point so we can tell "this VTune sample reflects a representative run".

---

## `/Ob3` results — Phase 4.3e (2026-04-27)

Build: `windows-msvc-release` with `CMAKE_CXX_FLAGS_RELEASE=/O2 /Ob3 /DNDEBUG`  
`indexed_dary_heap.hpp`: `sift_down_` annotated `GRAPH_DETAIL_FORCE_INLINE`  
Same methodology: 5 reps, median, core 0, priority High.

### ER Sparse, E/V ≈ 8

| Heap | 1K ns | 10K ns | 100K ns | vs /Ob2 at 100K |
|------|------:|-------:|--------:|----------------:|
| Default | 77,657 | 1,336,662 | 26,533,738 | ≈0 % |
| Idx2 | 54,785 | 1,171,990 | 26,154,649 | −1.0 % |
| Idx4 | 50,190 | 1,020,621 | 20,572,087 | **−2.6 %** |
| Idx8 | 80,134 | 1,248,033 | 23,112,681 | +4.0 % |

### 2D Grid, E/V ≈ 4

| Heap | 1K ns | 10K ns | 100K ns | vs /Ob2 at 100K |
|------|------:|-------:|--------:|----------------:|
| Default | 25,203 | 537,223 | 6,323,796 | +2.2 % |
| Idx2 | 24,694 | 579,034 | 7,490,114 | +8.1 % |
| Idx4 | 28,244 | 606,708 | 7,440,434 | +8.2 % |
| Idx8 | 35,770 | 723,495 | 8,859,656 | +7.4 % |

### Barabási–Albert, m=4, E/V ≈ 8

| Heap | 1K ns | 10K ns | 100K ns | vs /Ob2 at 100K |
|------|------:|-------:|--------:|----------------:|
| Default | 91,214 | 1,422,637 | 27,633,036 | +9.3 % |
| Idx2 | 60,420 | 1,209,414 | 26,769,593 | +5.6 % |
| Idx4 | 54,289 | 1,068,178 | 20,839,074 | +6.3 % |
| Idx8 | 87,263 | 1,348,705 | 23,320,973 | +5.1 % |

### Path, E/V = 1

| Heap | 1K ns | 10K ns | 100K ns | vs /Ob2 at 100K |
|------|------:|-------:|--------:|----------------:|
| Default | 14,059 | 138,226 | 1,401,957 | +5.3 % |
| Idx2 | 4,555 | 44,408 | 463,958 | **−4.5 %** |
| Idx4 | 4,829 | 44,297 | 460,474 | **−7.6 %** |
| Idx8 | 4,700 | 44,029 | 461,246 | **−6.1 %** |

### Summary: /Ob3 vs /Ob2 at 100K

| Topology | Heap | /Ob2 ns | /Ob3 ns | Δ |
|----------|------|--------:|--------:|---|
| ER Sparse | Default | 26,655,249 | 26,533,738 | ≈0 % |
| ER Sparse | Idx4 | 21,124,883 | 20,572,087 | −2.6 % |
| Grid | Default | 6,190,532 | 6,323,796 | +2.2 % |
| Grid | Idx4 | 6,873,101 | 7,440,434 | +8.2 % ⚠ |
| BA | Default | 25,268,386 | 27,633,036 | +9.3 % ⚠ |
| BA | Idx4 | 19,603,770 | 20,839,074 | +6.3 % ⚠ |
| Path | Default | 1,331,743 | 1,401,957 | +5.3 % |
| Path | Idx4 | 498,438 | 460,474 | **−7.6 %** ✅ |

### Interpretation

- **Path indexed heap wins** (−4.5 % to −7.6 %): this is the workload where
  the VTune profile showed the most comparator-chain overhead — `/Ob3`
  collapses it and delivers a measurable wall-clock improvement.
- **Grid and BA show regressions (+6–9 %)**: the inlined `sift_down_` body
  expands the run-lambda significantly on these topologies (larger working
  set, more icache pressure, different branch predictor behaviour). The
  `/Ob2` code-layout was better for these cases.
- **ER Sparse is essentially neutral** (within noise).
- **Net verdict**: `/Ob3` + `GRAPH_DETAIL_FORCE_INLINE` on `sift_down_` is
  **not a universal win**. It helps Path (the inlining-bottlenecked case)
  but regresses Grid/BA (icache-sensitive cases). Reverting `sift_down_`
  annotation and keeping `/Ob3` only for the flag-level benefit (without
  force-inline on the loop body) is the next thing to try.
