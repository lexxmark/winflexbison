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

Run the installer (Debian/Ubuntu WSL):

```
tests/bison-autotest/install-wsl-deps.sh              # required deps
tests/bison-autotest/install-wsl-deps.sh --with-java-d # also the Java + D tiers
```

It installs `autoconf`/`m4` (build the testsuite), `build-essential` (C/C++
tiers), `perl` (error-message normalization — without it many tests silently
skip), and the reference `bison` (for `tests/bison/generate.sh` golden), then
verifies everything. Manual equivalent:

```
sudo apt-get install -y autoconf m4 perl build-essential bison
sudo apt-get install -y default-jdk gdc     # optional: Java + D tiers
```

## Running

```
# from WSL, in the port dir:
tests/bison-autotest/run.sh                 # whole suite
tests/bison-autotest/run.sh -k input        # groups matching a keyword
tests/bison-autotest/run.sh 1 2 3           # specific group numbers
```

Or drive it from Windows:

```
runtests.bat --with-autotest                 # CTest gate, then this suite
cmake -B build -DWFB_WSL_AUTOTEST=ON ; ctest # this suite as a ctest test
```

`run.sh` auto-detects `gcc`/`g++` to enable the C/C++ tiers; the Java / D tiers
skip unless `javac` / a D compiler are present.

## Normalization

`run.sh`'s `bison` wrapper makes win_bison behave like a native `bison` with no
post-processing, via env vars forwarded through `WSLENV`: `BISON_PROGRAM_NAME=bison`
(diagnostics say `bison:`) and `WINFLEXBISON_BINARY_OUTPUT=Y` (LF, not CRLF).
Generated files are already LF via the `xfopen` binary-mode port fix. Env vars
the tests set (`COLUMNS`, `YYFLAT`, `POSIXLY_CORRECT`, …) are also forwarded, and
`@tb@` (a test token meaning a literal TAB) is substituted in the generated
testsuite. The remaining differences are captured as documented xfails below.

## Expected failures (xfail)

`run.sh` post-processes the results and treats a small allowlist (`BISON_XFAIL`)
as *expected* failures — these are WSL-environment limitations, not win_bison
defects, so the run exits 0 when only they (or nothing) fail:

*WSL-environment limits* (not win_bison defects):
- **129** (`output.at`) — output filename with NTFS-illegal characters
  (`: < > | …`); such a file cannot exist on Windows.
- **283–287** (`synclines.at`) — filenames containing `"` quote characters,
  likewise impossible on NTFS.

*win_bison in-process-m4 limitation* (candidate for a future fix): skeleton
complaints emitted via `b4_cat`/`@complain` during macro-argument expansion do
not reach `scan-skel`, so the diagnostics are dropped and the exit code is not
set:
- **165** (`skeletons.at`) — Complaining during macro argument expansion.
- **166** (`skeletons.at`) — Fatal errors make M4 exit immediately.

*win_bison diagnostic byte-escaping difference* (locale/`quotearg`): high or
invalid bytes (e.g. `0xFF`) are printed as `\xff` or raw rather than upstream's
octal `\377`, because win_bison's Windows locale treats `0x80`–`0xFF` as
printable:
- **4** (`input.at`) — Invalid inputs.
- **78** (`named-refs.at`) — Stray symbols in brackets.

Two groups were previously listed here and now pass, so they are no longer
allowlisted — a failure in either is reported as unexpected:
- **314** (`actions.at`, *Invalid uses of %empty*) — **fixed in win_bison.** The
  group runs `-fcaret -u`, and the caret diagnostic's cached `FILE*` on the
  grammar blocked `--update`'s `rename()` under MSVCRT. Releasing it via
  `caret_free()` before `fixits_run()` (`bison/src/main.c`) resolved it. This
  group was allowlisted *before* that fix landed, and the entry attributed the
  failure to the WSL 9p `/tmp` share — a misdiagnosis. A failure here now means
  the handle release regressed and should be treated as a real defect.
- **124** (`output.at`, `api.location.file="$at_dir/…"`) — passes, but no fix is
  known to explain it. The perl in-place substitution plus shell heredoc it
  depends on may still be environment-sensitive, so a failure on a different WSL
  setup is plausible without implying a win_bison regression.

The adjusted summary prints `expected failures (xfail): …` and
`unexpected failures: …`; only unexpected failures make the run fail.

## Status

Toolchain-free tier (no compilers): the harness drives win_bison over all 776
groups. Normalization plus several win_bison fixes (caret binary read, binary
output files, b4_cat `_m4eof`, `/utf-8` glyphs, fixit backup) took failures
from ~200 down to the low single digits, with 129 accepted as xfail. See
`docs/specs/03-test-adoption/spec.md` (BISON) for the phased plan; enabling the
C tier needs `build-essential` in WSL.
