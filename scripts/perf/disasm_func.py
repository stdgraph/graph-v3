#!/usr/bin/env python3
"""
disasm_func.py - disassemble one function from a Windows exe.

Selects a function by --pattern (substring) and/or --regex (full Python re),
then dumps just that function's bytes via dumpbin /range.

The symbol index is cached on disk by sym_index.py, so repeated invocations
on the same exe are near-instant after the first call.

Avoid putting raw '<' or '>' on a Windows command line: cmd treats them as
redirection. Use --regex with escaped angle brackets instead, e.g.:
    --regex 'use_indexed_dary_heap<4>'

Example:
    python scripts/perf/disasm_func.py \
        --exe build/windows-msvc-profile/benchmark/algorithms/benchmark_dijkstra.exe \
        --regex 'use_indexed_dary_heap<4>' --pattern sift_down_ \
        --out artifacts/perf/sift_down_idx4.asm
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from sym_index import (  # noqa: E402
    disasm_range,
    filter_symbols,
    index_functions,
)


def _truncate(name: str, n: int) -> str:
    return name if len(name) <= n else name[:n] + "\u2026"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", type=Path, required=True)
    ap.add_argument("--pattern", action="append", default=[],
                    help="substring filter (multiple = AND).")
    ap.add_argument("--regex", action="append", default=[],
                    help="Python regex filter (multiple = AND). Use this for patterns with angle brackets.")
    ap.add_argument("--list-only", action="store_true")
    ap.add_argument("--length", type=lambda s: int(s, 0), default=0x1000)
    ap.add_argument("--match-index", type=int, default=0)
    ap.add_argument("--rebuild-cache", action="store_true")
    ap.add_argument("--out", type=Path)
    ap.add_argument("--no-truncate", action="store_true")
    args = ap.parse_args()

    if not args.exe.exists():
        raise SystemExit(f"exe not found: {args.exe}")
    if not args.pattern and not args.regex:
        raise SystemExit("need at least one --pattern or --regex")

    syms = index_functions(args.exe, force_rebuild=args.rebuild_cache)
    matches = filter_symbols(syms, args.pattern, args.regex)
    if not matches:
        sys.stderr.write(f"no symbols matched: patterns={args.pattern} regexes={args.regex}\n")
        return 1

    print(f"matches ({len(matches)} of {len(syms)} indexed):", file=sys.stderr)
    for i, s in enumerate(matches[:30]):
        name = s.name if args.no_truncate else _truncate(s.name, 200)
        print(f"  [{i}] 0x{s.rva:x}  {name}", file=sys.stderr)
    if len(matches) > 30:
        print(f"  ... +{len(matches) - 30} more", file=sys.stderr)

    if args.list_only:
        return 0

    if args.match_index >= len(matches):
        raise SystemExit(f"--match-index {args.match_index} out of range (have {len(matches)})")
    sym = matches[args.match_index]
    asm = disasm_range(args.exe, sym.rva, sym.rva + args.length)
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(asm)
        print(f"wrote {len(asm.splitlines())} lines to {args.out}", file=sys.stderr)
    else:
        print(asm)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
