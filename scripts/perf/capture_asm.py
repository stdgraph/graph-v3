#!/usr/bin/env python3
"""capture_asm.py - bulk-dump a curated list of functions into one directory.

Manifest format (one capture per non-blank, non-# line; whitespace-separated):

    <basename>[:N]   <length_hex>   <regex>   [substring1 substring2 ...]

  - basename:   output filename stem
  - :N optional 0-based index (default 0) to disambiguate when the
                regex+substrings still match more than one symbol
  - length_hex: how many bytes to disassemble from the symbol's RVA
  - regex:      Python re matched against the demangled symbol name
                (use this for patterns containing < or >)
  - substrings: AND-filtered after the regex match
"""

from __future__ import annotations

import argparse
import shlex
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from sym_index import disasm_range, filter_symbols, index_functions  # noqa: E402


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", type=Path, required=True)
    ap.add_argument("--manifest", type=Path, required=True)
    ap.add_argument("--out-dir", type=Path, required=True)
    ap.add_argument("--rebuild-cache", action="store_true")
    args = ap.parse_args()

    syms = index_functions(args.exe, force_rebuild=args.rebuild_cache)
    args.out_dir.mkdir(parents=True, exist_ok=True)

    n_ok = 0
    n_skip = 0
    for raw in args.manifest.read_text().splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        parts = shlex.split(line)
        if len(parts) < 3:
            print(f"  skip (need basename, length, regex): {line}", file=sys.stderr)
            n_skip += 1
            continue

        basename, length_str, first_regex, *rest = parts

        pick = 0
        if ":" in basename:
            basename, pick_str = basename.rsplit(":", 1)
            try:
                pick = int(pick_str)
            except ValueError:
                print(f"  skip (bad :N suffix on {basename!r}): {line}", file=sys.stderr)
                n_skip += 1
                continue

        try:
            length = int(length_str, 0)
        except ValueError:
            print(f"  skip (bad length {length_str!r}): {line}", file=sys.stderr)
            n_skip += 1
            continue

        matches = filter_symbols(syms, substrings=rest, regexes=[first_regex])
        if not matches:
            print(f"  no match: {basename}  (regex={first_regex!r} subs={rest!r})", file=sys.stderr)
            n_skip += 1
            continue
        if pick >= len(matches):
            print(f"  skip ({basename}: pick={pick} but only {len(matches)} matches)", file=sys.stderr)
            n_skip += 1
            continue
        if len(matches) > 1 and pick == 0 and ":" not in raw.split()[0]:
            short = matches[0].name if len(matches[0].name) <= 140 else matches[0].name[:140] + "..."
            print(f"  note: {basename}: {len(matches)} matches; using [0] ({short})", file=sys.stderr)

        sym = matches[pick]
        asm = disasm_range(args.exe, sym.rva, sym.rva + length)
        out_path = args.out_dir / f"{basename}.asm"
        out_path.write_text(asm)
        print(f"  OK  {basename:<32}  0x{sym.rva:x}  pick={pick}  {len(asm.splitlines())} lines")
        n_ok += 1

    print(f"\ncaptured {n_ok}, skipped {n_skip}", file=sys.stderr)
    return 0 if n_skip == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
