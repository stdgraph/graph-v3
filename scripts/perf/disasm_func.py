#!/usr/bin/env python3
"""
disasm_func.py — find a function in an exe by demangled name substring,
                 then dump only that function's disassembly via dumpbin.

Phase 2 (csr_edge_value_perf_plan.md) needs to disassemble the inlined
Dijkstra run-lambda. `dumpbin /disasm` on the full benchmark exe produces
hundreds of thousands of lines and most are unrelated; this script
demangles the symbol table, picks the matching symbol, and dumps only that
function's bytes (using `dumpbin /disasm:bytes /range:`).

Requires `dumpbin.exe` to be on PATH (run from a vcvars64 shell).

Example:
    python scripts/perf/disasm_func.py \
        --exe build/windows-msvc-profile/benchmark/algorithms/benchmark_dijkstra.exe \
        --pattern "dijkstra_shortest_paths" --pattern "Idx4" --pattern "Grid" \
        --out artifacts/relax_grid_idx4.asm
"""

from __future__ import annotations

import argparse
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


@dataclass
class Symbol:
    name: str
    rva: int                  # virtual address relative to image base
    section: str = ""

    def __repr__(self) -> str:
        return f"Symbol(rva=0x{self.rva:x}, name={self.name[:60]}…)"


# dumpbin /symbols line for a static symbol typically looks like:
#   00B 00000000 SECT3  notype ()    External     | ?run@dijkstra_shortest_paths@graph@@...
# but we don't get RVAs from /symbols. /headers + /disasm is what gives RVAs.
# Easiest path: parse `dumpbin /disasm:nobytes` and find the function entry
# lines, which look like:
#   ?run@... (graph::dijkstra_shortest_paths<...>):
#   0000000140002340: ...
_FUNC_HEADER_RE = re.compile(r"^(?P<name>[^\s].*?):\s*$")
_ADDR_RE        = re.compile(r"^\s*(?P<addr>[0-9A-Fa-f]{8,16}):\s")


def find_dumpbin() -> Path:
    p = shutil.which("dumpbin")
    if not p:
        raise SystemExit(
            "dumpbin not on PATH — run from a vcvars64 shell, "
            "or invoke via: cmd /c \"vcvars64.bat && python ...\""
        )
    return Path(p)


def index_functions(exe: Path) -> list[Symbol]:
    """Run `dumpbin /disasm:nobytes` and extract (name, RVA) for every function entry."""
    dumpbin = find_dumpbin()
    proc = subprocess.run(
        [str(dumpbin), "/disasm:nobytes", "/nologo", str(exe)],
        capture_output=True, text=True, errors="replace",
    )
    if proc.returncode != 0:
        sys.stderr.write(proc.stderr)
        raise SystemExit(f"dumpbin failed ({proc.returncode})")

    syms: list[Symbol] = []
    pending_name: str | None = None
    for line in proc.stdout.splitlines():
        if not line:
            pending_name = None
            continue
        if pending_name is None:
            m = _FUNC_HEADER_RE.match(line)
            # Heuristic: a function header is a non-indented line ending in ':' that is
            # NOT itself an address line. We accept anything that doesn't start with a hex
            # address followed by ':'.
            if m and not _ADDR_RE.match(line):
                candidate = m.group("name").strip()
                # Section dividers also end in ':'; ignore obvious ones.
                if not candidate.startswith("Dump of") and "0x" not in candidate.split()[0]:
                    pending_name = candidate
            continue
        # First address line after the header gives us the RVA.
        m = _ADDR_RE.match(line)
        if m:
            try:
                rva = int(m.group("addr"), 16)
                syms.append(Symbol(name=pending_name, rva=rva))
            except ValueError:
                pass
            pending_name = None
    return syms


def filter_symbols(syms: list[Symbol], patterns: list[str]) -> list[Symbol]:
    return [s for s in syms if all(p in s.name for p in patterns)]


def disasm_range(exe: Path, start: int, end: int) -> str:
    """Run `dumpbin /disasm /range:` for a single function."""
    dumpbin = find_dumpbin()
    proc = subprocess.run(
        [str(dumpbin), "/disasm", "/nologo",
         f"/range:0x{start:x},0x{end:x}", str(exe)],
        capture_output=True, text=True, errors="replace",
    )
    if proc.returncode != 0:
        sys.stderr.write(proc.stderr)
        raise SystemExit(f"dumpbin /range failed ({proc.returncode})")
    return proc.stdout


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", type=Path, required=True)
    ap.add_argument("--pattern", action="append", required=True,
                    help="substring that must appear in the demangled symbol "
                         "(may be given multiple times — all must match).")
    ap.add_argument("--list-only", action="store_true",
                    help="Print matching symbols and exit; do not disassemble.")
    ap.add_argument("--length", type=lambda s: int(s, 0), default=0x1000,
                    help="Bytes to dump from the symbol's RVA (default 0x1000).")
    ap.add_argument("--match-index", type=int, default=0,
                    help="0-based index into the matched-symbols list (default 0).")
    ap.add_argument("--out", type=Path, help="Write disassembly to this file (default stdout).")
    args = ap.parse_args()

    if not args.exe.exists():
        raise SystemExit(f"exe not found: {args.exe}")

    print(f"indexing functions in {args.exe.name} ...", file=sys.stderr)
    syms = index_functions(args.exe)
    print(f"  {len(syms)} function entries indexed", file=sys.stderr)

    matches = filter_symbols(syms, args.pattern)
    if not matches:
        sys.stderr.write(f"no symbols matched all of: {args.pattern}\n")
        return 1

    print(f"matches ({len(matches)}):", file=sys.stderr)
    for s in matches[:20]:
        snippet = (s.name[:120] + "…") if len(s.name) > 121 else s.name
        print(f"  0x{s.rva:x}  {snippet}", file=sys.stderr)
    if len(matches) > 20:
        print(f"  ... +{len(matches) - 20} more", file=sys.stderr)

    if args.list_only:
        return 0

    if args.match_index >= len(matches):
        raise SystemExit(
            f"--match-index {args.match_index} out of range (have {len(matches)} matches)"
        )
    if len(matches) > 1:
        print(
            f"info: {len(matches)} matches; disassembling index {args.match_index}. "
            "Refine --pattern or use --match-index to pick another.",
            file=sys.stderr,
        )

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
