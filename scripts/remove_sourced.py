#!/usr/bin/env python3
"""
Phase 5.2 — Remove Sourced references from test files.

Transformations applied to each file:
1. Remove `using xxx_sourced... = ...;` alias declarations (multi-line)
2. Remove TEST_CASE blocks tagged with [sourced]
3. Remove SECTION("sourced...") blocks (inside non-sourced TEST_CASEs)
4. Remove STATIC_REQUIRE(traits::sourced == ...) lines
5. Remove [[maybe_unused]] XXX_sourced variable declarations
6. Substitute `, false, false, ` → `, false, ` in remaining aliases (drop Sourced=false)
   and the corresponding `, false>` in trailing traits args (they stay — Bidirectionl=false)

Brace-counting logic handles nested {}.
"""

import re
import sys
from pathlib import Path


def remove_block(lines: list[str], start: int) -> int:
    """Remove a brace-delimited block starting at lines[start].
    Returns the index of the first line AFTER the block."""
    depth = 0
    i = start
    while i < len(lines):
        line = lines[i]
        depth += line.count('{') - line.count('}')
        i += 1
        if depth <= 0 and i > start + 1:
            break
    return i


def is_sourced_test_case(line: str) -> bool:
    """Return True if the line starts a TEST_CASE or TEMPLATE_TEST_CASE tagged [sourced].
    NOTE: only checks the single line; for multi-line, use is_sourced_test_case_multiline."""
    if not re.match(r'\s*(TEMPLATE_)?TEST_CASE\s*\(', line):
        return False
    return bool(re.search(r'\[sourced\]', line))


def is_sourced_test_case_multiline(lines: list[str], i: int) -> bool:
    """Return True if lines[i] starts a TEST_CASE/TEMPLATE_TEST_CASE and the [sourced]
    tag appears within the next few lines of the argument list (closing paren)."""
    line = lines[i]
    if not re.match(r'\s*(TEMPLATE_)?TEST_CASE\s*\(', line):
        return False
    # Check up to 5 lines for the closing ')' and [sourced] tag
    for j in range(i, min(i + 6, len(lines))):
        if re.search(r'\[sourced\]', lines[j]):
            return True
        if ')' in lines[j] and j > i:  # closing paren of TEST_CASE args
            break
    return False


def is_sourced_section(line: str) -> bool:
    """Return True if line is SECTION("sourced...)."""
    return bool(re.match(r'\s*SECTION\s*\(\s*"sourced', line, re.IGNORECASE))


def find_block_end(lines: list[str], start: int) -> int:
    """Find the end of the {} block that begins on or after lines[start].
    The block starts with the first '{' encountered.
    Returns the index after the closing '}'."""
    i = start
    depth = 0
    found_open = False
    while i < len(lines):
        line = lines[i]
        opens = line.count('{')
        closes = line.count('}')
        if not found_open and opens > 0:
            found_open = True
        if found_open:
            depth += opens - closes
            if depth <= 0:
                return i + 1
        i += 1
    return len(lines)


def remove_multiline_alias(lines: list[str], start: int) -> int:
    """Remove `using xxx_sourced = ...;` starting at lines[start].
    Returns index after the removed declaration."""
    i = start
    while i < len(lines):
        if lines[i].rstrip().endswith(';'):
            return i + 1
        i += 1
    return i + 1


def process_lines(lines: list[str]) -> list[str]:
    result = []
    i = 0
    while i < len(lines):
        line = lines[i]

        # 1. Sourced type alias declarations: `using xxx_sourced... =`
        if re.match(r'\s*using \w*[Ss]ourced\w*\s*=', line):
            i = remove_multiline_alias(lines, i)
            continue

        # 2. TEST_CASE / TEMPLATE_TEST_CASE blocks tagged [sourced]
        if is_sourced_test_case_multiline(lines, i):
            i = find_block_end(lines, i)
            continue

        # 3. SECTION("sourced ...") blocks
        if is_sourced_section(line):
            i = find_block_end(lines, i)
            continue

        # 4. STATIC_REQUIRE(traits::sourced == ...) and static_assert(G::sourced ...) lines
        if re.search(r'(STATIC_REQUIRE|static_assert)\s*\(\s*(traits|G)::sourced\s*==', line):
            i += 1
            continue

        # 5. [[maybe_unused]] XXX_sourced... variable declarations
        if re.match(r'\s*\[\[maybe_unused\]\]\s+\w*[Ss]ourced\w*\b', line):
            i += 1
            continue

        # 6. In template-arg lists, drop the Sourced=false position:
        #    `, false, false,` → `, false,`   (remove Sourced's slot; Bidir stays)
        #    Also handles continuation lines like `false, false, Traits<...>`
        if ', false, false,' in line:
            line = line.replace(', false, false,', ', false,', 1)
        elif re.match(r'^(\s*)false, false,', line):
            # Multi-line alias continuation: leading `false, false, ...` → `false, ...`
            line = re.sub(r'^(\s*)false, false,', r'\1false,', line, count=1)

        result.append(line)
        i += 1

    return result


def process_file(file_path: Path, dry_run: bool = False) -> bool:
    """Process a single file. Returns True if changed."""
    with open(file_path) as f:
        content = f.read()

    lines = content.splitlines(keepends=True)
    new_lines = process_lines(lines)
    new_content = ''.join(new_lines)

    if new_content == content:
        return False

    if not dry_run:
        with open(file_path, 'w') as f:
            f.write(new_content)

    return True


def main():
    dry_run = '--dry-run' in sys.argv
    paths = [p for p in sys.argv[1:] if not p.startswith('--')]

    if not paths:
        print("Usage: remove_sourced.py [--dry-run] <file.cpp>...")
        sys.exit(1)

    changed = 0
    for path_str in paths:
        p = Path(path_str)
        if not p.exists():
            print(f"  SKIP (not found): {p}")
            continue
        modified = process_file(p, dry_run=dry_run)
        if modified:
            print(f"  CHANGED: {p.name}")
            changed += 1
        else:
            print(f"  unchanged: {p.name}")

    print(f"\nTotal changed: {changed}/{len(paths)}")


if __name__ == '__main__':
    main()
