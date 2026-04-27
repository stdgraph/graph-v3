#!/usr/bin/env python3
"""
bench_run.py — run a benchmark filter, capture median rows as JSON.

Wraps the manual core-pin / priority-High / 5-rep / median pattern used
throughout Phase 4.x perf work, and emits a structured result instead of
PowerShell `Select-String "median"` plumbing.

Example:
    python scripts/perf/bench_run.py \
        --exe build/windows-msvc-profile/benchmark/algorithms/benchmark_dijkstra.exe \
        --filter "BM_Dijkstra_(CSR|BGL_CSR)_Grid(_Idx4)?/(10000|100000)$" \
        --reps 5 --min-time 2s \
        --out artifacts/bench_grid_msvc_profile.json
"""

from __future__ import annotations

import argparse
import ctypes
import json
import re
import subprocess
import sys
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Optional


# Windows process-priority constants
PRIORITY_HIGH = 0x00000080


@dataclass
class BenchRow:
    name: str            # benchmark name, e.g. BM_Dijkstra_CSR_Grid_Idx4/100000
    aggregate: str       # mean / median / stddev / cv
    real_time_ns: float
    cpu_time_ns: float
    iterations: int


# Lines look like:
#   BM_Dijkstra_CSR_Grid_Idx4/100000_median   7440434 ns      7280759 ns         5
# stddev rows have ns; cv rows have %.
_ROW_RE = re.compile(
    r"^(?P<name>BM_\S+?)_(?P<agg>mean|median|stddev|cv)\s+"
    r"(?P<rt>\S+)\s+(?:ns|%)\s+(?P<cpu>\S+)\s+(?:ns|%)\s+(?P<iter>\d+)\s*$"
)


def parse_rows(text: str) -> list[BenchRow]:
    rows: list[BenchRow] = []
    for line in text.splitlines():
        m = _ROW_RE.match(line)
        if not m:
            continue
        try:
            rows.append(
                BenchRow(
                    name=m.group("name"),
                    aggregate=m.group("agg"),
                    real_time_ns=float(m.group("rt")),
                    cpu_time_ns=float(m.group("cpu")),
                    iterations=int(m.group("iter")),
                )
            )
        except ValueError:
            # silently skip malformed
            pass
    return rows


def _set_affinity_and_priority(pid: int, affinity_mask: int) -> None:
    """Pin process to cores in `affinity_mask` and set HIGH priority. Windows-only."""
    if sys.platform != "win32":
        return
    PROCESS_ALL_ACCESS = 0x1F0FFF
    h = ctypes.windll.kernel32.OpenProcess(PROCESS_ALL_ACCESS, False, pid)
    if not h:
        print(f"warning: OpenProcess({pid}) failed", file=sys.stderr)
        return
    try:
        if not ctypes.windll.kernel32.SetProcessAffinityMask(h, affinity_mask):
            print(f"warning: SetProcessAffinityMask failed", file=sys.stderr)
        if not ctypes.windll.kernel32.SetPriorityClass(h, PRIORITY_HIGH):
            print(f"warning: SetPriorityClass failed", file=sys.stderr)
    finally:
        ctypes.windll.kernel32.CloseHandle(h)


def run_benchmark(
    exe: Path,
    bench_filter: str,
    reps: int,
    min_time: str,
    affinity_mask: int = 0x1,
    aggregates_only: bool = True,
    extra_args: Optional[list[str]] = None,
) -> tuple[str, list[BenchRow]]:
    args = [
        str(exe),
        f"--benchmark_filter={bench_filter}",
        f"--benchmark_min_time={min_time}",
        f"--benchmark_repetitions={reps}",
    ]
    if aggregates_only:
        args.append("--benchmark_report_aggregates_only=true")
    if extra_args:
        args.extend(extra_args)

    # Start suspended so we can pin before the first iteration. Easiest cross-version
    # approach: start normally, immediately pin, then wait. The first ~ms of run loses
    # the pin, but Google Benchmark's per-rep median and our 5-rep aggregate absorb it.
    proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    _set_affinity_and_priority(proc.pid, affinity_mask)
    out, _ = proc.communicate()
    if proc.returncode != 0:
        print(out, file=sys.stderr)
        raise SystemExit(f"benchmark exited with {proc.returncode}")
    return out, parse_rows(out)


def main() -> int:
    ap = argparse.ArgumentParser(description="Run a benchmark filter and capture rows as JSON.")
    ap.add_argument("--exe", type=Path, required=True, help="Path to benchmark executable.")
    ap.add_argument("--filter", required=True, help="--benchmark_filter regex.")
    ap.add_argument("--reps", type=int, default=5, help="--benchmark_repetitions (default 5).")
    ap.add_argument("--min-time", default="2s", help="--benchmark_min_time (default 2s).")
    ap.add_argument("--affinity", type=lambda s: int(s, 0), default=0x1,
                    help="Process affinity mask (default 0x1 = core 0).")
    ap.add_argument("--out", type=Path, help="Write JSON to this path (default: stdout).")
    ap.add_argument("--label", default="", help="Free-form label stored in the JSON output.")
    ap.add_argument("--print-stdout", action="store_true",
                    help="Also print the raw benchmark stdout to this process's stderr.")
    args = ap.parse_args()

    if not args.exe.exists():
        raise SystemExit(f"executable not found: {args.exe}")

    raw, rows = run_benchmark(
        args.exe,
        args.filter,
        args.reps,
        args.min_time,
        affinity_mask=args.affinity,
    )
    if args.print_stdout:
        print(raw, file=sys.stderr)

    payload = {
        "label": args.label,
        "exe": str(args.exe),
        "filter": args.filter,
        "reps": args.reps,
        "min_time": args.min_time,
        "affinity_mask": hex(args.affinity),
        "rows": [asdict(r) for r in rows],
    }
    text = json.dumps(payload, indent=2)
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(text)
        print(f"wrote {len(rows)} rows to {args.out}")
    else:
        print(text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
