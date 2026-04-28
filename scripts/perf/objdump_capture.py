#!/usr/bin/env python3
"""
objdump_capture.py - Linux/GCC counterpart of capture_asm.py.

Same manifest format as capture_asm.py, but uses `objdump` instead of
`dumpbin` and operates on demangled C++ symbol names from `nm --demangle`.

Required tools (Linux/WSL):
  - objdump  (binutils)
  - nm       (binutils)

Manifest format (one capture per non-blank, non-# line):

    <basename>[:N]   <length_hex>   <regex>   [substring1 substring2 ...]

The Linux manifest (agents/perf_capture_manifest_linux.txt) parallels the
MSVC one but accounts for:
  - Itanium mangling vs MSVC mangling (e.g. `_Z...` vs `??...`)
  - GCC's tendency to emit `.cold` partitions (fold them into the main body
    by extending the --length)
  - BGL's d_ary_heap_indirect inlines preserve_heap_property_down on GCC,
    so its capture often returns `<inlined>` only.
"""

from __future__ import annotations

import argparse
import json
import re
import shlex
import shutil
import subprocess
import sys
from dataclasses import asdict, dataclass
from pathlib import Path


@dataclass
class Symbol:
    name: str
    addr: int
    size: int = 0


def _which(tool: str) -> str:
    p = shutil.which(tool)
    if not p:
        raise SystemExit(f"{tool} not on PATH (need binutils on Linux/WSL)")
    return p


def _cache_path(exe: Path) -> Path:
    return exe.with_suffix(exe.suffix + ".symidx.json")


def _exe_fingerprint(exe: Path) -> dict:
    st = exe.stat()
    return {"path": str(exe), "size": st.st_size, "mtime_ns": st.st_mtime_ns}


# nm output line format with --demangle and -S (sizes):
#   0000000000003fa0 0000000000000123 T graph::dijkstra_shortest_paths<...>(...)
_NM_RE = re.compile(
    r"^(?P<addr>[0-9a-f]+)\s+(?P<size>[0-9a-f]+)\s+(?P<type>[A-Za-z])\s+(?P<name>.+)$"
)


def index_symbols(exe: Path, *, force_rebuild: bool = False) -> list[Symbol]:
    cache = _cache_path(exe)
    if not force_rebuild and cache.exists():
        try:
            payload = json.loads(cache.read_text())
            if payload.get("fingerprint") == _exe_fingerprint(exe):
                return [Symbol(**s) for s in payload["symbols"]]
        except (OSError, json.JSONDecodeError, KeyError):
            pass

    nm = _which("nm")
    print(f"indexing functions in {exe.name} via nm --demangle ...", file=sys.stderr)
    proc = subprocess.run(
        [nm, "--demangle", "--print-size", "--defined-only",
         "--no-sort", str(exe)],
        capture_output=True, text=True, errors="replace",
    )
    if proc.returncode != 0:
        sys.stderr.write(proc.stderr)
        raise SystemExit(f"nm failed ({proc.returncode})")

    syms: list[Symbol] = []
    for line in proc.stdout.splitlines():
        m = _NM_RE.match(line)
        if not m:
            continue
        if m.group("type").lower() not in ("t", "w"):  # text or weak text
            continue
        try:
            syms.append(Symbol(
                name=m.group("name"),
                addr=int(m.group("addr"), 16),
                size=int(m.group("size"), 16),
            ))
        except ValueError:
            pass

    print(f"  indexed {len(syms)} text symbols", file=sys.stderr)
    try:
        cache.write_text(json.dumps({
            "fingerprint": _exe_fingerprint(exe),
            "symbols": [asdict(s) for s in syms],
        }))
    except OSError as e:
        print(f"  warning: failed to write cache: {e}", file=sys.stderr)
    return syms


def filter_symbols(
    syms,
    substrings=(),
    regexes=(),
):
    sub = list(substrings)
    rxs = [re.compile(r) for r in regexes]
    out = []
    for s in syms:
        if all(p in s.name for p in sub) and all(rx.search(s.name) for rx in rxs):
            out.append(s)
    return out


def disasm_range(exe: Path, start: int, end: int) -> str:
    objdump = _which("objdump")
    proc = subprocess.run(
        [objdump, "-d", "--demangle", "--no-show-raw-insn",
         f"--start-address=0x{start:x}", f"--stop-address=0x{end:x}",
         str(exe)],
        capture_output=True, text=True, errors="replace",
    )
    if proc.returncode != 0:
        sys.stderr.write(proc.stderr)
        raise SystemExit(f"objdump failed ({proc.returncode})")
    return proc.stdout


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", type=Path, required=True)
    ap.add_argument("--manifest", type=Path, required=True)
    ap.add_argument("--out-dir", type=Path, required=True)
    ap.add_argument("--rebuild-cache", action="store_true")
    args = ap.parse_args()

    syms = index_symbols(args.exe, force_rebuild=args.rebuild_cache)
    args.out_dir.mkdir(parents=True, exist_ok=True)

    n_ok = n_skip = 0
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
                print(f"  skip (bad :N suffix on {basename!r})", file=sys.stderr)
                n_skip += 1
                continue
        try:
            length = int(length_str, 0)
        except ValueError:
            print(f"  skip (bad length {length_str!r})", file=sys.stderr)
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

        sym = matches[pick]
        # Use nm-reported size when available; else fall back to manifest length.
        end = sym.addr + (sym.size if sym.size else length)
        asm = disasm_range(args.exe, sym.addr, end)
        out_path = args.out_dir / f"{basename}.asm"
        out_path.write_text(asm)
        print(f"  OK  {basename:<32}  0x{sym.addr:x}  pick={pick}  "
              f"size={sym.size}  {len(asm.splitlines())} lines")
        n_ok += 1

    print(f"\ncaptured {n_ok}, skipped {n_skip}", file=sys.stderr)
    return 0 if n_skip == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
