# Bison autotest suite (faithful, MSYS2-driven)

This adopts the **full upstream GNU Bison test suite** (the `.at` autotest files)
and runs it against the built `win_bison.exe`. It is the "adopt all bison tests"
path — 776 test groups — and complements the Windows-native CTest suites under
`tests/bison/` (compile-run + golden-diff) and `tests/flex/`.

Unlike those, autotest is a POSIX-shell harness, so this runner needs **MSYS2**;
it is deliberately *not* part of the Windows `ctest` gate.

## Layout

- `at/` — vendored `.at` sources + `testsuite.h`, version-matched to the
  vendored bison (currently 3.8.2), plus a hand-authored `package.m4`
  (upstream generates that from `configure`). Re-vendor from the new baseline
  on a bison upgrade and bump `package.m4`.
- `run.sh` — generates the `testsuite` script with `autom4te-2.71`, sets up a
  normalizing `bison` wrapper around `win_bison.exe`, writes `atconfig`/`atlocal`,
  and runs the suite. The generated `testsuite` (~18 MB) is **not committed** —
  it is produced on demand from `at/`.

## Prerequisites (MSYS2, once)

Install MSYS2 (`winget install --id MSYS2.MSYS2 -e` puts it at `C:\msys64`,
the same path AppVeyor's Visual Studio 2022 image uses), then run the installer:

```
C:\msys64\usr\bin\bash -lc tests/bison-autotest/install-msys2-deps.sh
```

It installs `autoconf2.71`/`m4` (build the testsuite), `mingw-w64-x86_64-gcc`
(C/C++ tiers), `perl` (error-message normalization — without it many tests
silently skip), and the reference `bison` (for `tests/bison/generate.sh` golden,
and the msys package is 3.8.2, matching the vendored version), then verifies
everything. Manual equivalent:

```
pacman -Sy --needed autoconf2.71 m4 perl diffutils sed grep gawk mingw-w64-x86_64-gcc bison
```

`autoconf2.71` is pinned on purpose. `autom4te` expands the `.at` sources, so
its version is part of the test *inputs*: 2.69 and 2.71 emit different
testsuites with different group numbering, and an xfail list calibrated against
one does not describe the other.

## Running

```
# from MSYS2, in the port dir:
tests/bison-autotest/run.sh                 # whole suite
tests/bison-autotest/run.sh -k input        # groups matching a keyword
tests/bison-autotest/run.sh 1 2 3           # specific group numbers
```

Or drive it from Windows:

```
runtests.bat --with-autotest                    # CTest gate, then this suite
cmake -B build -DWFB_MSYS2_AUTOTEST=ON ; ctest  # this suite as a ctest test
```

`run.sh` auto-detects `gcc`/`g++` to enable the C/C++ tiers; the Java / D tiers
skip unless `javac` / a D compiler are present.

**Runs in parallel by default** — `min(nproc, 16)` jobs. The suite is almost
pure process-spawn cost (~17,600 checks, and MSYS2 emulates `fork()` by copying
the process image), so this is where nearly all the time goes. On a 32-core box
`-j16` took **185s against ~25 min serial**, with all 776 verdicts identical to
the serial run. Tune or disable it:

```
WFB_JOBS=1 tests/bison-autotest/run.sh    # serial (what CI uses)
WFB_JOBS=8 tests/bison-autotest/run.sh    # pick a width
tests/bison-autotest/run.sh -j24          # your own -j is left alone
```

Always pass a *count* with `-j`: bare `-j` means "one job per test group" to
autotest, i.e. 776 of them.

**`$PATH` must be MSYS-style inside the shell** (`/c/msys64/mingw64/bin`, not
`C:\msys64\mingw64\bin`). Autotest splits `$PATH` on `:`, so a drive letter tears
the entry in half and every program in it goes invisible — the symptom is
`cannot find bison` at startup, or `gcc: command not found` inside a compile
group. Setting `PATH` in cmd *before* launching bash is fine; MSYS2 converts it
on the way in. Prepending a Windows-style path from inside the shell is not.
`run.sh` warns when it sees one.

## Normalization

`run.sh`'s `bison` wrapper makes win_bison behave like a native `bison` with no
post-processing: `BISON_PROGRAM_NAME=bison` (diagnostics say `bison:`) and
`WINFLEXBISON_BINARY_OUTPUT=Y` (LF, not CRLF). Generated files are already LF via
the `xfopen` binary-mode port fix. Because MSYS2 runs `win_bison.exe` as an
ordinary child process, exported variables are simply inherited — no forwarding
list is involved. `@tb@` (a test token meaning a literal TAB) is substituted in
the generated testsuite. The remaining differences are captured as documented
xfails below.

`calc.at`'s driver calls `alarm(200)` as a hang guard, which mingw does not
provide; `run.sh` supplies a no-op through `-include` rather than patching the
vendored `.at` sources, which stay byte-faithful to upstream. Without it all 100
`calc.at` groups fail to *compile*.

## Expected failures (xfail)

`run.sh` post-processes the results and treats a small allowlist (`BISON_XFAIL`)
as *expected* failures, so the run exits 0 when only they (or nothing) fail:

*win_bison diagnostic byte-escaping difference* (locale/`quotearg`): high or
invalid bytes (e.g. `0xFF`) are printed as `\xff` or raw rather than upstream's
octal `\377`, because win_bison's Windows locale treats `0x80`–`0xFF` as
printable:
- **4** (`input.at`) — Invalid inputs.
- **78** (`named-refs.at`) — Stray symbols in brackets.

*Caret geometry under multibyte text* — the `^~~~` span is measured in bytes
rather than display columns, so it overshoots:
- **149, 150** (`diagnostics.at`) — Tabulations and multibyte characters.
- **152** (`diagnostics.at`) — Complaints from M4 (one column too wide).

*CR surviving into the echoed source line*: `xfopen` reads text-mode and strips
CR, but `location.c`'s caret reader opens binary for true byte offsets, so a
CRLF grammar keeps the CR upstream has dropped:
- **154** (`diagnostics.at`) — CR NL.

*ASCII fallbacks where upstream prints Unicode glyphs* (• for the item dot, ↳ in
derivations, ε for empty). The port `#if 0`s bison's glyph machinery and drops
`<unicodeio.h>` because gnulib's unicodeio module is not vendored, so the
conversion cannot be linked. Vendoring unicodeio would fix them:
- **159, 160** (`diagnostics.at`) — Counterexamples, Deep Counterexamples.

*win_bison in-process-m4 limitation* (candidate for a future fix): skeleton
complaints emitted via `b4_cat`/`@complain` during macro-argument expansion do
not reach `scan-skel`, so the diagnostics are dropped and the exit code is not
set:
- **165** (`skeletons.at`) — Complaining during macro argument expansion.
- **166** (`skeletons.at`) — Fatal errors make M4 exit immediately.

*Upstream skeleton bug, newly detected here*:
- **764** (`glr-regression.at`, `glr2.cc`) — the **generated** parser trips a
  libstdc++ assertion, `vector<bool>::operator[]` with `__n >= size()`. That is a
  genuine out-of-bounds access in upstream bison 3.8.2's `glr2.cc` skeleton, not
  a port regression: it happens under WSL too, silently, because Ubuntu's g++
  builds without `_GLIBCXX_ASSERTIONS` while MSYS2's enables them by default.
  **Do not silence it by disabling assertions** — detecting this is coverage the
  WSL runner never had. Worth reporting upstream.

*Unexplained*:
- **124** (`output.at`, `api.location.file="$at_dir/…"`) — fails on MSYS2 and
  under WSL alike, with no identified cause. The perl in-place substitution plus
  shell heredoc it depends on is the suspect. It was briefly delisted on the
  strength of a partial run that showed it passing; full runs on either host
  contradict that.

Groups **129** and **283–287** (filenames containing NTFS-illegal characters:
`: < > | "`) are no longer listed. They *failed* under the previous WSL runner —
the Linux side created such a file on its own filesystem and win_bison then could
not open it — but under MSYS2 the create fails up front and autotest skips the
group, which is the honest outcome.

The adjusted summary prints `expected failures (xfail): …` and
`unexpected failures: …`; only unexpected failures make the run fail.

## Status

The harness drives win_bison over all 776 groups with the C/C++ compile tiers
live. Normalization plus a series of win_bison fixes (caret binary read, binary
output files, b4_cat `_m4eof`, `/utf-8` glyphs, fixit backup, `/dev/null` → `NUL`
in `xfopen`) took failures from ~200 down to the twelve documented xfails above.
See `docs/specs/03-test-adoption/spec.md` (BISON) for the phased plan.
