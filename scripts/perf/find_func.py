#!/usr/bin/env python3
"""find_func.py - search the cached symbol index by substring(s) and/or regex(es).

Light wrapper for sym_index. Use --regex when the pattern needs angle
brackets (cmd treats < and > as redirection).

Example:
    python scripts/perf/find_func.py \
        --exe build/windows-msvc-profile/benchmark/algorithms/benchmark_dijkstra.exe \
        --pattern compressed_graph --pattern lambda_2 \
        --regex 'use_indexed_dary_heap<4>'
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from sym_index import filter_symbols, index_functions  # noqa: E402


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", type=Path, required=True)
    ap.add_argument("--pattern", action="append", default=[])
    ap.add_argument("--regex", action="append", default=[])
    ap.add_argument("--limit", type=int, default=20)
    ap.add_argument("--no-truncate", action="store_true")
    ap.add_argument("--rebuild-cache", action="store_true")
    args = ap.parse_args()

    if not args.pattern and not args.regex:
        raise SystemExit("need at least one --pattern or --regex")

    syms = index_functions(args.exe, force_rebuild=args.rebuild_cache)
    matches = filter_symbols(syms, args.pattern, args.regex)
    print(f"{len(matches)} match(es) of {len(syms)} indexed:")
    for i, s in enumerate(matches[: args.limit]):
        name = s.name if args.no_truncate else (s.name if len(s.name) <= 200 else s.name[:200] + "\u2026")
        print(f"  [{i}] 0x{s.rva:x}  {name}")
    if len(matches) > args.limit:
        print(f"  ... +{len(matches) - args.limit} more")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
