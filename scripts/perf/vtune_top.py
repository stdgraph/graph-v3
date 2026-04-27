#!/usr/bin/env python3
"""
vtune_top.py — read a VTune `-format csv` hotspots report, emit clean top-N.

Replaces the brittle PowerShell parsing used in earlier Phase 4.x runs:
    $vtune -report hotspots -r <dir> -format csv | <regex juggling>

Symbol normalization rules:
  graph::detail::indexed_dary_heap<...>::F   → heap::F
  graph::container_value_fn<...>::F          → cfn::F
  graph::detail::vector_position_map::F      → pm::F
  std::less<...>::F                          → less::F
  std::vector<T,Alloc>::F                    → vector<T>::F   (drops the Alloc)
  std::_Vector_iterator<...>::F              → _Vector_iter::F
  graph::views::incidence_view<...>::F       → incidence_view::F

Example:
    vtune.exe -report hotspots -r vtune/hot_001 -format csv > hot.csv
    python scripts/perf/vtune_top.py --csv hot.csv --top 15
"""

from __future__ import annotations

import argparse
import csv
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Optional


@dataclass
class Hotspot:
    function: str
    cpu_time: float
    module: str = ""
    source: str = ""


def _strip_template(name: str) -> str:
    """Remove balanced angle-brackets from `name`."""
    out: list[str] = []
    depth = 0
    for ch in name:
        if ch == "<":
            depth += 1
            continue
        if ch == ">":
            depth = max(0, depth - 1)
            continue
        if depth == 0:
            out.append(ch)
    return "".join(out)


_NORMALIZERS: list[tuple[re.Pattern[str], str]] = [
    (re.compile(r"graph::detail::indexed_dary_heap<.*?>::"),       "heap::"),
    (re.compile(r"graph::container_value_fn<.*?>::"),              "cfn::"),
    (re.compile(r"graph::detail::vector_position_map::"),          "pm::"),
    (re.compile(r"std::less<.*?>::"),                              "less::"),
    (re.compile(r"std::_Vector_iterator<.*?>::"),                  "_Vector_iter::"),
    (re.compile(r"graph::views::incidence_view<.*?>::"),           "incidence_view::"),
    (re.compile(r"std::vector<([^,>]+),.*?>::"),                   r"vector<\1>::"),
]


def normalize(name: str) -> str:
    s = name
    for rx, repl in _NORMALIZERS:
        s = rx.sub(repl, s)
    # Trim crazy-long template instantiations on the bare end ("X<...>") that no rule matched.
    if "<" in s and len(s) > 120:
        s = _strip_template(s) + "<...>"
    return s.strip()


def load_csv(path: Path) -> list[Hotspot]:
    spots: list[Hotspot] = []
    # VTune CSV is tab-delimited despite the name.
    with path.open(newline="", encoding="utf-8", errors="replace") as f:
        reader = csv.reader(f, delimiter="\t")
        header: Optional[list[str]] = None
        for row in reader:
            if not row or row[0].lower().startswith(("function", "vtune")):
                if row and row[0].lower() == "function":
                    header = row
                continue
            if len(row) < 2:
                continue
            try:
                cpu = float(row[1])
            except ValueError:
                continue
            module = row[5] if len(row) > 5 else ""
            source = row[7] if len(row) > 7 else ""
            spots.append(Hotspot(function=row[0], cpu_time=cpu, module=module, source=source))
    return spots


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", type=Path, required=True, help="VTune hotspots CSV (tab-separated).")
    ap.add_argument("--top", type=int, default=15, help="Rows to show (default 15).")
    ap.add_argument("--no-normalize", action="store_true",
                    help="Skip the heap::/cfn::/etc. normalization rules.")
    ap.add_argument("--markdown", action="store_true",
                    help="Emit a markdown table instead of plain text.")
    args = ap.parse_args()

    spots = load_csv(args.csv)
    if not spots:
        print(f"no rows parsed from {args.csv}", file=sys.stderr)
        return 1

    total = sum(s.cpu_time for s in spots)
    spots.sort(key=lambda s: s.cpu_time, reverse=True)
    top = spots[: args.top]

    if args.markdown:
        print(f"Total CPU collected: **{total:.2f} s** across {len(spots)} symbols\n")
        print("| Rank | Function | CPU (s) | % |")
        print("|---:|---|---:|---:|")
        for i, s in enumerate(top, 1):
            name = s.function if args.no_normalize else normalize(s.function)
            pct = s.cpu_time / total * 100 if total else 0
            print(f"| {i} | `{name}` | {s.cpu_time:.3f} | {pct:.1f} |")
    else:
        print(f"Total CPU: {total:.2f} s across {len(spots)} symbols")
        for i, s in enumerate(top, 1):
            name = s.function if args.no_normalize else normalize(s.function)
            pct = s.cpu_time / total * 100 if total else 0
            short = (name[:75] + "…") if len(name) > 76 else name
            print(f"  {i:2d}. {pct:5.1f} %  {s.cpu_time:7.3f}s  {short}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
