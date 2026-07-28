#!/usr/bin/env bash
# run.sh — run the faithful GNU Autotest bison suite against win_bison.exe.
#
# This is the "adopt all bison tests" path: the vendored .at sources (at/) are
# compiled by autom4te into the ~18 MB `testsuite` shell script, which is then
# run under WSL/Linux driving the Windows win_bison.exe via a normalizing
# wrapper. Unlike the CTest golden suite, this needs a POSIX shell (WSL), so it
# is a separate, opt-in runner — not part of the Windows ctest gate.
#
# Requires (in WSL): autoconf/autom4te, m4, perl, diff, sed. Compile/Java/D
# test tiers auto-skip unless CC/CXX/DC/CONF_JAVAC are exported.
#
# Usage (from WSL):
#   tests/bison-autotest/run.sh [testsuite args...]
#   tests/bison-autotest/run.sh -k input        # only groups matching 'input'
#   tests/bison-autotest/run.sh 1 2 3            # specific group numbers
#   BISON=/path/to/win_bison.exe WORK=/tmp/x tests/bison-autotest/run.sh
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
AT="$HERE/at"
PORT_ROOT="$(cd "$HERE/../.." && pwd)"
BISON="${BISON:-$PORT_ROOT/bin/Release/win_bison.exe}"
WORK="${WORK:-/tmp/winflexbison-bison-autotest}"
# abs_top_srcdir points at the pristine bison baseline (some tests read fixture
# grammars from it). Default to the superproject's upstream/bison next to the
# port. Missing -> at_top_srcdir falls back to $WORK below and the tests that
# read from the baseline fail, so say so rather than degrading silently.
ORIG_BISON="${ORIG_BISON:-$(cd "$PORT_ROOT/../upstream/bison" 2>/dev/null && pwd)}"
[ -n "$ORIG_BISON" ] || echo "warning: bison baseline not found at $PORT_ROOT/../upstream/bison; set ORIG_BISON to override" >&2

[ -x "$BISON" ] || { echo "win_bison not found/executable: $BISON" >&2; exit 1; }
for t in autom4te m4 perl diff sed; do
    command -v "$t" >/dev/null || { echo "missing tool: $t (apt install autoconf m4)" >&2; exit 1; }
done

echo "win_bison : $BISON"
echo "baseline  : ${ORIG_BISON:-<unset>}"
echo "work dir  : $WORK"
# Record the environment that shapes the RESULTS, not just whether tools exist.
# autom4te generates the testsuite from the .at sources, so its version changes
# the script being run; and win_bison's quotearg picks quote styles from the
# locale forwarded below. A CI-vs-local mismatch in either shifts large numbers
# of diagnostic comparisons at once, which looks like a mass failure.
echo "autoconf  : $(autoconf --version 2>/dev/null | head -1)"
echo "m4        : $(m4 --version 2>/dev/null | head -1)"
echo "kernel    : $(uname -r 2>/dev/null)  (4.4.0-*-Microsoft = WSL1)"
echo "locale    : LANG=${LANG:-unset} LC_ALL=${LC_ALL:-unset} LC_CTYPE=${LC_CTYPE:-unset}"

rm -rf "$WORK"; mkdir -p "$WORK/bin"
cp "$AT"/*.at "$AT/testsuite.h" "$AT/package.m4" "$WORK/"

cd "$WORK"
echo "generating testsuite (autom4te)..."
autom4te --language=autotest -I . testsuite.at -o testsuite || {
    echo "autom4te failed" >&2; exit 1; }
# The bison tests use the token @tb@ to mean a literal TAB (e.g. in %parse-param
# and api.namespace values). It is not a standard autotest quadrigraph, so our
# stand-alone autom4te does not convert it; substitute it in the generated
# script so grammars get a real tab (else it leaks as a stray '@' into output).
sed -i 's/@tb@/\t/g' testsuite

# Normalizing bison wrapper: rewrite the program name (win_bison[.exe] -> bison,
# GNU tools would strip .exe) and strip CR from win_bison's streams so the
# harness compares against upstream-style output.
cat > bin/bison <<WRAP
#!/usr/bin/env bash
# Make win_bison behave like a native 'bison' with NO post-processing, so the
# wrapper is a plain exec (no temp files, no piping, no races):
#   BISON_PROGRAM_NAME=bison     -> diagnostics say "bison:", not "win_bison.exe:"
#   WINFLEXBISON_BINARY_OUTPUT=Y -> LF stdout/stderr instead of MSVC CRLF
#     (generated files are already LF via the xfopen binary-mode port fix)
export BISON_PROGRAM_NAME=bison
export WINFLEXBISON_BINARY_OUTPUT=Y
# WSL only forwards env vars to Windows processes listed in WSLENV. Forward the
# two above plus the vars the suite sets that win_bison reads (COLUMNS for caret
# width, YYFLAT for flat counterexamples, POSIXLY_CORRECT, TIME_LIMIT,
# BISON_USE_PUSH_FOR_PULL, LC_CTYPE).
_fwd="BISON_PROGRAM_NAME:WINFLEXBISON_BINARY_OUTPUT:COLUMNS:YYFLAT:POSIXLY_CORRECT:TIME_LIMIT:BISON_USE_PUSH_FOR_PULL:LC_CTYPE"
export WSLENV="\${_fwd}\${WSLENV:+:\$WSLENV}"
# WSL drops the WSLInterop binfmt_misc entry under sustained load. Once gone,
# every Windows exec fails with "cannot execute binary file: Exec format error"
# and the remainder of the suite fails wholesale -- 439 groups locally, ~700 on
# AppVeyor, with an arbitrary onset (group 346 vs group 4). It is session-scoped
# and re-registering restores it immediately, so heal it in place rather than
# losing the run. The test is a stat on the common path; the write only happens
# when the entry is actually missing.
if [ ! -e /proc/sys/fs/binfmt_misc/WSLInterop ]; then
    if [ "\$(id -u)" = 0 ]; then
        echo ':WSLInterop:M::MZ::/init:PF' > /proc/sys/fs/binfmt_misc/register 2>/dev/null
    else
        sudo -n sh -c "echo ':WSLInterop:M::MZ::/init:PF' > /proc/sys/fs/binfmt_misc/register" 2>/dev/null
    fi
fi
exec "$BISON" "\$@"
WRAP
chmod +x bin/bison

# Detect optional compilers to enable the compile tiers. These compile the
# parsers win_bison generates and run them (validating the generator output).
# Absent -> the C/C++ tiers auto-skip. Compiled programs are native Linux (no
# .exe) when a compiler is present.
CC_BIN=$(command -v gcc 2>/dev/null || command -v cc 2>/dev/null || true)
CXX_BIN=$(command -v g++ 2>/dev/null || command -v c++ 2>/dev/null || true)
if [ -n "$CC_BIN" ]; then EXEEXT=''; else EXEEXT='.exe'; fi
echo "C compiler  : ${CC_BIN:-<none, C tier skipped>}"
echo "C++ compiler: ${CXX_BIN:-<none, C++ tier skipped>}"

cat > atconfig <<CFG
at_testdir='.'
abs_builddir='$WORK'
at_srcdir='.'
abs_srcdir='$WORK'
at_top_srcdir='${ORIG_BISON:-$WORK}'
abs_top_srcdir='${ORIG_BISON:-$WORK}'
at_top_build_prefix=''
abs_top_builddir='$WORK'
AUTOTEST_PATH='$WORK/bin'
EXEEXT='$EXEEXT'
OBJEXT='o'
GREP='grep'
EGREP='grep -E'
FGREP='grep -F'
AWK='awk'
PERL='perl'
CFG

# atlocal: enable the C/C++ tiers when compilers are present. -w silences
# warnings in generated parsers (some checks compile with -Werror otherwise).
c_works=false;   [ -n "$CC_BIN" ]  && c_works=true
cxx_works=false; [ -n "$CXX_BIN" ] && cxx_works=true
cat > atlocal <<LOC
: \${CC='$CC_BIN'} \${CXX='$CXX_BIN'} \${DC=''} \${CONF_JAVAC=''} \${CONF_JAVA=''}
: \${CPPFLAGS='-I$WORK'} \${CFLAGS='-w'} \${CXXFLAGS='-w'}
: \${NO_WERROR_CFLAGS='-w'} \${NO_WERROR_CXXFLAGS='-w'}
: \${CXX98_CXXFLAGS='-std=c++98'} \${CXX03_CXXFLAGS='-std=c++03'}
: \${CXX11_CXXFLAGS='-std=c++11'} \${CXX14_CXXFLAGS='-std=c++14'}
: \${CXX17_CXXFLAGS='-std=c++17'} \${CXX20_CXXFLAGS='-std=c++20'} \${CXX2B_CXXFLAGS='-std=c++2b'}
: \${BISON_C_WORKS=$c_works} \${BISON_CXX_WORKS=$cxx_works} \${BISON_DC_WORKS=false}
: \${CC_IS_CXX=0}
POSIXLY_CORRECT_IS_EXPORTED=false
LOC

# Expected failures. See README.md. Three kinds:
#
# NTFS-illegal filenames (a Windows process cannot create/open these):
#   129          output.at    name with : < > | ...
#   283-287      synclines.at  name containing " quote chars
#
# win_bison in-process-m4 limitation (candidate for a future fix): skeleton
# complaints emitted via b4_cat/@complain during macro-argument expansion do
# not reach scan-skel, so these diagnostics are dropped:
#   165  skeletons.at  Complaining during macro argument expansion
#   166  skeletons.at  Fatal errors make M4 exit immediately
#
# win_bison diagnostic byte-escaping differs (locale/quotearg): high/invalid
# bytes (e.g. 0xFF) print as \xff or raw instead of upstream's octal \377,
# because win_bison's Windows locale treats 0x80-0xFF as printable:
#   4   input.at      Invalid inputs
#   78  named-refs.at Stray symbols in brackets
#
# Formerly listed here, now expected to pass:
#   314  actions.at  Invalid uses of %empty -- fixed by the caret_free() call
#        added before fixits_run() (bison/src/main.c): the test runs
#        `-fcaret -u`, and the cached caret FILE* used to block --update's
#        rename(). It was allowlisted before that fix landed and blamed on the
#        9p /tmp share, which was a misdiagnosis. A failure here now means the
#        handle-release regressed -- treat it as a real defect.
#   124  output.at   api.location.file="$at_dir/..." -- passes, but with no
#        identified fix; the perl in-place substitution plus heredoc it relies
#        on may still be environment-sensitive.
# Overridable so a candidate list can be tried without editing this file --
# useful when recalibrating for a different distro/autoconf, and it lets the
# failure-dump path below be exercised (BISON_XFAIL= makes a known xfail
# report as unexpected).
BISON_XFAIL="${BISON_XFAIL-129 165 166 283 284 285 286 287 4 78}"

echo "running testsuite $*..."
./testsuite "$@" 2>&1 | tee testsuite.out
rc=${PIPESTATUS[0]}

# Post-process: treat BISON_XFAIL groups as expected failures. Autotest's
# summary ends with an authoritative one-line list of every failed group:
#   Subject: [GNU Bison 3.8.2] testsuite: 129 165 166 ... failed
failed=$(sed -nE 's/.*testsuite:[[:space:]]*([0-9][0-9 ]*)[[:space:]]+failed.*/\1/p' \
         testsuite.out | tr ' ' '\n' | grep -E '^[0-9]+$' | sort -un)
unexpected=""; xfailed=""
for g in $failed; do
    case " $BISON_XFAIL " in
        *" $g "*) xfailed="$xfailed $g" ;;
        *)        unexpected="$unexpected $g" ;;
    esac
done

echo
echo "=== winflexbison adjusted results ==="
echo "expected failures (xfail):${xfailed:- none}"
echo "unexpected failures:${unexpected:- none}"
if [ -n "$unexpected" ]; then
    # Group numbers alone are not a diagnosis. autotest writes the actual
    # expected-vs-got diffs to testsuite.log, which nothing ever prints -- so a
    # CI failure used to cost a full build just to learn nothing. Dump the first
    # few so one run is enough to see what actually differs.
    n=0
    for g in $unexpected; do
        n=$((n + 1))
        [ "$n" -gt "${DUMP_FAILURES:-3}" ] && break
        echo
        echo "================= detail: group $g ================="
        awk -v g="$g" '
            $0 ~ "^ *"g"\\. "        { inblock = 1 }
            inblock && $0 ~ "^ *[0-9]+\\. " && $0 !~ "^ *"g"\\. " { exit }
            inblock                  { print }
        ' testsuite.log 2>/dev/null | head -"${DUMP_LINES:-50}"
    done
    echo
    echo "(showing ${DUMP_FAILURES:-3} of $(echo $unexpected | wc -w); raise with"
    echo " DUMP_FAILURES=N, lengthen each with DUMP_LINES=N; full log: $WORK/testsuite.log)"
    exit 1
fi
# Only expected failures (or none) remain: success.
exit 0
