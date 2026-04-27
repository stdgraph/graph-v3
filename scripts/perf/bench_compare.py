#!/usr/bin/env python3
"""
bench_compare.py — diff two bench_run.py JSON outputs as a markdown table.

Joins on (benchmark name, aggregate). Default aggregate is `median`.
Emits a markdown table with absolute times and Δ%, plus regression / win flags.

Example:
    python scripts/perf/bench_compare.py \
        --baseline artifacts/grid_ob2.json \
        --candidate artifacts/grid_ob3.json \
        --threshold 5
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Optional


def _load(path: Path) -> dict:
    return json.loads(path.read_text())


def _index_rows(payload: dict, agg: str) -> dict[str, float]:
    return {
        r["name"]: r["real_time_ns"]
        for r in payload["rows"]
        if r["aggregate"] == agg
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--baseline", type=Path, required=True)
    ap.add_argument("--candidate", type=Path, required=True)
    ap.add_argument("--aggregate", default="median",
                    choices=["mean", "median", "stddev", "cv"],
                    help="Aggregate to compare (default median).")
    ap.add_argument("--threshold", type=float, default=5.0,
                    help="Δ%% above which to flag regression (⚠) or win (✅).")
    ap.add_argument("--label-baseline", default=None,
                    help="Override column header for baseline (default: from JSON).")
    ap.add_argument("--label-candidate", default=None,
                    help="Override column header for candidate.")
    ap.add_argument("--out", type=Path, help="Write markdown to file (default stdout).")
    args = ap.parse_args()

    base = _load(args.baseline)
    cand = _load(args.candidate)
    base_rows = _index_rows(base, args.aggregate)
    cand_rows = _index_rows(cand, args.aggregate)

    label_base = args.label_baseline or base.get("label") or args.baseline.stem
    label_cand = args.label_candidate or cand.get("label") or args.candidate.stem

    # union of keys, sorted
    keys = sorted(set(base_rows) | set(cand_rows))
    if not keys:
        raise SystemExit(f"no rows with aggregate={args.aggregate!r} in either file")

    lines = [
        f"| Benchmark | {label_base} (ns) | {label_cand} (ns) | Δ % |",
        "|---|---:|---:|---:|",
    ]
    for k in keys:
        b = base_rows.get(k)
        c = cand_rows.get(k)
        if b is None:
            lines.append(f"| {k} | — | {c:,.0f} | new |")
            continue
        if c is None:
            lines.append(f"| {k} | {b:,.0f} | — | dropped |")
            continue
        delta = (c - b) / b * 100.0
        flag = ""
        if delta >= args.threshold:
            flag = " ⚠"
        elif delta <= -args.threshold:
            flag = " ✅"
        lines.append(f"| {k} | {b:,.0f} | {c:,.0f} | {delta:+.1f} %{flag} |")

    text = "\n".join(lines) + "\n"
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(text)
        print(f"wrote table to {args.out}")
    else:
        print(text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
