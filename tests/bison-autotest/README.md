# Bison autotest suite (faithful, WSL-driven)

This adopts the **full upstream GNU Bison test suite** (the `.at` autotest files)
and runs it against the built `win_bison.exe`. It is the "adopt all bison tests"
path — 776 test groups — and complements the Windows-native CTest suites under
`tests/bison/` (compile-run + golden-diff) and `tests/flex/`.

Unlike those, autotest is a POSIX-shell harness, so this runner needs **WSL /
Linux**; it is deliberately *not* part of the Windows `ctest` gate.

## Layout

- `at/` — vendored `.at` sources + `testsuite.h`, version-matched to the
  vendored bison (currently 3.8.2), plus a hand-authored `package.m4`
  (upstream generates that from `configure`). Re-vendor from the new baseline
  on a bison upgrade and bump `package.m4`.
- `run.sh` — generates the `testsuite` script with `autom4te`, sets up a
  normalizing `bison` wrapper around `win_bison.exe`, writes `atconfig`/`atlocal`,
  and runs the suite. The generated `testsuite` (~18 MB) is **not committed** —
  it is produced on demand from `at/` (WSL has `autom4te`).

## Prerequisites (WSL, once)

```
sudo apt-get install -y autoconf m4        # autom4te + m4
# optional, to enable the compile tiers:
sudo apt-get install -y build-essential    # gcc/g++ (C, C++ tiers)
```

## Running

```
# from WSL, in the port dir:
tests/bison-autotest/run.sh                 # whole suite
tests/bison-autotest/run.sh -k input        # groups matching a keyword
tests/bison-autotest/run.sh 1 2 3           # specific group numbers
```

The compile / Java / D tiers auto-skip unless `CC` / `CXX` / `DC` / `CONF_JAVAC`
are exported before running.

## Normalization

`win_bison.exe` differs cosmetically from a native `bison`: it reports its
program name as `win_bison.exe` (GNU tools strip `.exe`) and may emit CR. The
`bison` wrapper rewrites `win_bison.exe -> bison` and strips CR before the
harness compares. Remaining failures are triaged case by case (path separators
and a few genuine port behaviors, e.g. win_bison omits the echoed source line in
caret diagnostics — the suite runs `-fno-caret`, as upstream's own harness does).

## Expected failures (xfail)

`run.sh` post-processes the results and treats a small allowlist (`BISON_XFAIL`)
as *expected* failures — these are WSL-environment limitations, not win_bison
defects, so the run exits 0 when only they (or nothing) fail:

- **129** (`output.at`) — output filename with NTFS-illegal characters
  (`: < > | …`); such a file cannot exist on Windows.
- **314** (`actions.at`) — `--fixit` backup rename; a Windows process cannot
  `rename` on the WSL 9p `/tmp` share (works on native NTFS).

The adjusted summary prints `expected failures (xfail): …` and
`unexpected failures: …`; only unexpected failures make the run fail.

## Status

Toolchain-free tier (no compilers): the harness drives win_bison over all 776
groups. Normalization plus several win_bison fixes (caret binary read, binary
output files, b4_cat `_m4eof`, `/utf-8` glyphs, fixit backup) took failures
from ~200 down to the low single digits, with 129/314 accepted as xfail. See
`docs/specs/03-test-adoption/spec.md` (BISON) for the phased plan; enabling the
C tier needs `build-essential` in WSL.
