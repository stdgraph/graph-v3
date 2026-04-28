# scripts/perf - performance investigation tooling

Helpers for running benchmarks, parsing VTune output, indexing exe symbols,
and pulling targeted disassembly across both MSVC (`dumpbin`) and GCC
(`objdump`). Built to support `agents/csr_edge_value_perf_plan.md` and
`agents/thread_b_linux_runbook.md`.

All scripts are stdlib-only (Python 3.10+).

## Files

| Script | OS | Purpose |
|---|---|---|
| `bench_run.py`        | both | Run a benchmark filter with core-pinning + High priority; emit median rows as JSON. |
| `bench_compare.py`    | both | Diff two `bench_run.py` JSONs as a markdown delta table. |
| `vtune_top.py`        | both | Parse a VTune CSV hotspots report; emit a normalized top-N. |
| `sym_index.py`        | win  | Disk-cached `dumpbin /disasm:nobytes` parser. ~30s cold, ~0.5s warm. |
| `find_func.py`        | win  | Symbol search wrapper around `sym_index`; supports `--regex`. |
| `disasm_func.py`      | win  | Single-function disasm via `dumpbin /range:`. |
| `capture_asm.py`      | win  | Bulk-dump a manifest of functions in one cache-warm pass. |
| `objdump_capture.py`  | linux | Linux/GCC counterpart of `capture_asm.py` using `nm` + `objdump`. |
| `linux_gcc_capture.sh`| linux | One-shot runbook driver: bench + perf-stat + objdump. |

## Avoiding cmd-redirection of `<` and `>`

Use `--regex` instead of `--pattern` for any filter that needs angle
brackets. Even better, replace them with `.` wildcards so the arg is plain
text (e.g. `use_indexed_dary_heap.4.` instead of `use_indexed_dary_heap<4>`).
Both `disasm_func.py`, `find_func.py`, and the manifests in
`agents/perf_capture_manifest*.txt` follow this convention.

## Cache files

`sym_index.py` writes `<exe>.symidx.json` next to the exe. Cache is
invalidated automatically when the exe's size or mtime changes.
`objdump_capture.py` does the same on Linux.

## Workflow examples

### One-shot bulk capture (used to populate `artifacts/perf/msvc_profile/`)

```pwsh
# From a vcvars64 shell.
python scripts/perf/capture_asm.py `
  --exe build/windows-msvc-profile/benchmark/algorithms/benchmark_dijkstra.exe `
  --manifest agents/perf_capture_manifest.txt `
  --out-dir artifacts/perf/msvc_profile
```

### Hotspot table from a VTune collection

```pwsh
& "C:\Program Files (x86)\Intel\oneAPI\vtune\latest\bin64\vtune.exe" `
  -collect hotspots -knob sampling-mode=sw `
  -result-dir vtune/hot_001 -- `
  build/windows-msvc-profile/benchmark/algorithms/benchmark_dijkstra.exe `
  --benchmark_filter="BM_Dijkstra_CSR_Grid_Idx4/100000" --benchmark_min_time=15s

& "C:\Program Files (x86)\Intel\oneAPI\vtune\latest\bin64\vtune.exe" `
  -report hotspots -r vtune/hot_001 -format csv > artifacts/perf/hot_001.csv

python scripts/perf/vtune_top.py --csv artifacts/perf/hot_001.csv --top 15 --markdown
```

### Bench A vs bench B

```pwsh
python scripts/perf/bench_run.py --exe ... --filter ... `
  --label baseline --out artifacts/perf/baseline.json
# (apply change, rebuild)
python scripts/perf/bench_run.py --exe ... --filter ... `
  --label candidate --out artifacts/perf/candidate.json
python scripts/perf/bench_compare.py `
  --baseline artifacts/perf/baseline.json `
  --candidate artifacts/perf/candidate.json `
  --threshold 5
```

### Linux/WSL counterpart capture

See `agents/thread_b_linux_runbook.md`. The one-liner is:

```bash
bash scripts/perf/linux_gcc_capture.sh
```

## Output convention

All scripts write either to stdout or to `--out <path>`. The convention used
in `agents/`-side docs is `artifacts/perf/<phase-or-toolchain>/...`.
`artifacts/` is gitignored — these are working captures, not source-of-truth.
