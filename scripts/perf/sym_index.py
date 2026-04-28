#!/usr/bin/env python3
"""
sym_index.py — disk-cached symbol index for a Windows exe.

`dumpbin /disasm:nobytes` on a 1.4 MB benchmark takes ~30 s and returns
14k+ function entries. We need to query that table dozens of times during
a perf investigation; caching the parse result to a JSON file next to
the exe drops repeated lookups to <100 ms.

Cache invalidation: by exe mtime+size in the cache header.
"""

from __future__ import annotations

import json
import re
import shutil
import subprocess
import sys
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable


@dataclass
class Symbol:
    name: str
    rva: int  # virtual address relative to image base


_FUNC_HEADER_RE = re.compile(r"^(?P<name>[^\s].*?):\s*$")
_ADDR_RE        = re.compile(r"^\s*(?P<addr>[0-9A-Fa-f]{8,16}):\s")


def _find_dumpbin() -> Path:
    p = shutil.which("dumpbin")
    if not p:
        raise SystemExit(
            "dumpbin not on PATH \u2014 run from a vcvars64 shell."
        )
    return Path(p)


def _parse_dumpbin_output(text: str) -> list[Symbol]:
    syms: list[Symbol] = []
    pending: str | None = None
    for line in text.splitlines():
        if not line:
            pending = None
            continue
        if pending is None:
            if _ADDR_RE.match(line):
                continue
            m = _FUNC_HEADER_RE.match(line)
            if not m:
                continue
            cand = m.group("name").strip()
            if cand.startswith("Dump of"):
                continue
            head = cand.split()[0] if cand else ""
            if "0x" in head:
                continue
            pending = cand
            continue
        m = _ADDR_RE.match(line)
        if m:
            try:
                syms.append(Symbol(name=pending, rva=int(m.group("addr"), 16)))
            except ValueError:
                pass
            pending = None
    return syms


def _cache_path(exe: Path) -> Path:
    return exe.with_suffix(exe.suffix + ".symidx.json")


def _exe_fingerprint(exe: Path) -> dict:
    st = exe.stat()
    return {"path": str(exe), "size": st.st_size, "mtime_ns": st.st_mtime_ns}


def _read_cache(exe: Path) -> list[Symbol] | None:
    cache = _cache_path(exe)
    if not cache.exists():
        return None
    try:
        payload = json.loads(cache.read_text())
    except (OSError, json.JSONDecodeError):
        return None
    if payload.get("fingerprint") != _exe_fingerprint(exe):
        return None
    return [Symbol(**s) for s in payload.get("symbols", [])]


def _write_cache(exe: Path, syms: list[Symbol]) -> None:
    cache = _cache_path(exe)
    payload = {
        "fingerprint": _exe_fingerprint(exe),
        "symbols": [asdict(s) for s in syms],
    }
    try:
        cache.write_text(json.dumps(payload))
    except OSError as e:
        print(f"warning: failed to write {cache}: {e}", file=sys.stderr)


def index_functions(exe: Path, *, force_rebuild: bool = False) -> list[Symbol]:
    """Return the function-entry list for `exe`, caching the result on disk."""
    if not force_rebuild:
        cached = _read_cache(exe)
        if cached is not None:
            return cached

    dumpbin = _find_dumpbin()
    print(f"indexing functions in {exe.name} (one-time, ~30s) ...", file=sys.stderr)
    proc = subprocess.run(
        [str(dumpbin), "/disasm:nobytes", "/nologo", str(exe)],
        capture_output=True, text=True, errors="replace",
    )
    if proc.returncode != 0:
        sys.stderr.write(proc.stderr)
        raise SystemExit(f"dumpbin failed ({proc.returncode})")

    syms = _parse_dumpbin_output(proc.stdout)
    print(f"  indexed {len(syms)} functions; cached to {_cache_path(exe).name}", file=sys.stderr)
    _write_cache(exe, syms)
    return syms


def filter_symbols(
    syms: Iterable[Symbol],
    substrings: Iterable[str] = (),
    regexes: Iterable[str] = (),
    *,
    include_ilt_thunks: bool = False,
) -> list[Symbol]:
    """Return symbols matching ALL substrings AND ALL regexes.

    @ILT+... entries (incremental linker thunks - small forwarders, not real
    bodies) are skipped by default; pass include_ilt_thunks=True to keep them.
    """
    sub = list(substrings)
    rxs = [re.compile(r) for r in regexes]
    out: list[Symbol] = []
    for s in syms:
        if not include_ilt_thunks and s.name.startswith("@ILT"):
            continue
        if all(p in s.name for p in sub) and all(rx.search(s.name) for rx in rxs):
            out.append(s)
    return out


def disasm_range(exe: Path, start: int, end: int) -> str:
    dumpbin = _find_dumpbin()
    proc = subprocess.run(
        [str(dumpbin), "/disasm", "/nologo",
         f"/range:0x{start:x},0x{end:x}", str(exe)],
        capture_output=True, text=True, errors="replace",
    )
    if proc.returncode != 0:
        sys.stderr.write(proc.stderr)
        raise SystemExit(f"dumpbin /range failed ({proc.returncode})")
    return proc.stdout
