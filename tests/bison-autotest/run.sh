#!/usr/bin/env bash
# run.sh — run the faithful GNU Autotest bison suite against win_bison.exe.
#
# This is the "adopt all bison tests" path: the vendored .at sources (at/) are
# compiled by autom4te into the ~18 MB `testsuite` shell script, which is then
# run under MSYS2 driving the Windows win_bison.exe via a normalizing wrapper.
# Unlike the CTest golden suite, this needs a POSIX shell, so it is a separate,
# opt-in runner — not part of the Windows ctest gate.
#
# MSYS2 runs win_bison.exe as an ordinary child process: exported variables are
# inherited directly, no interop layer is involved, and the work dir is already
# a path Windows understands.
#
# Requires: autoconf2.71 (autom4te), m4, perl, diff, sed, and mingw-w64 gcc/g++
# for the compile tiers -- install with install-msys2-deps.sh. Compile/Java/D
# tiers auto-skip when their compilers are absent.
#
# Usage (from MSYS2):
#   tests/bison-autotest/run.sh [testsuite args...]
#   tests/bison-autotest/run.sh -k input        # only groups matching 'input'
#   tests/bison-autotest/run.sh 1 2 3            # specific group numbers
#   BISON=/path/to/win_bison.exe WORK=/tmp/x tests/bison-autotest/run.sh
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
AT="$HERE/at"
PORT_ROOT="$(cd "$HERE/../.." && pwd)"
BISON="${BISON:-$PORT_ROOT/bin/Release/win_bison.exe}"

# Autotest splits $PATH on ':' (_AS_PATH_WALK), so a Windows-style entry like
# C:\msys64\mingw64\bin is torn in half and every program in it becomes
# invisible -- the symptom is "cannot find bison" at startup, or "gcc: command
# not found" inside a compile group. Launching bash from cmd is fine (it
# converts $PATH on the way in); prepending a drive-letter path from *inside*
# the shell is what breaks. Warn rather than fail: only the affected entries
# are lost, and the run may still be useful.
case ":$PATH:" in
    *:[A-Za-z]:[\\/]*)
        echo "warning: \$PATH holds a Windows-style entry (X:\\...); autotest splits" >&2
        echo "         PATH on ':' and will not see programs there. Use /c/... form." >&2 ;;
esac
# Work dir. Kept under the port root (gitignored) so the tree stays self
# contained and the artifacts of a failed run are easy to find. Override WORK=
# to put it elsewhere; any path MSYS2 and Windows both resolve will do.
WORK="${WORK:-$(cd "$(dirname "$0")/../.." && pwd)/.autotest-work}"
# abs_top_srcdir points at the pristine bison baseline (some tests read fixture
# grammars from it). Default to the superproject's upstream/bison next to the
# port. Missing -> at_top_srcdir falls back to $WORK below and the tests that
# read from the baseline fail, so say so rather than degrading silently.
ORIG_BISON="${ORIG_BISON:-$(cd "$PORT_ROOT/../upstream/bison" 2>/dev/null && pwd)}"
[ -n "$ORIG_BISON" ] || echo "warning: bison baseline not found at $PORT_ROOT/../upstream/bison; set ORIG_BISON to override" >&2

# MSYS2 keeps the compilers in a subsystem prefix that only the MINGW64 login
# shell puts on PATH; pick them up when invoked from a plain msys shell so the
# compile tiers do not silently disappear.
if ! command -v gcc >/dev/null 2>&1 && [ -x /mingw64/bin/gcc.exe ]; then
    PATH="/mingw64/bin:$PATH"; export PATH
fi

# autom4te's version is part of the test *inputs*: it expands the .at sources,
# so 2.69 and 2.71 emit different testsuites with different group numbering,
# and an xfail list calibrated against one does not describe the other. Prefer
# the versioned binary when the distro ships one (MSYS2's autoconf2.71,
# Debian's autoconf2.69) so the pin is explicit rather than whatever won PATH.
AUTOM4TE="${AUTOM4TE:-$(command -v autom4te-2.71 2>/dev/null || command -v autom4te 2>/dev/null || true)}"

[ -x "$BISON" ] || { echo "win_bison not found/executable: $BISON" >&2; exit 1; }
[ -n "$AUTOM4TE" ] || { echo "missing tool: autom4te (pacman -S autoconf2.71)" >&2; exit 1; }
for t in m4 perl diff sed; do
    command -v "$t" >/dev/null || { echo "missing tool: $t (pacman -S $t)" >&2; exit 1; }
done

echo "win_bison : $BISON"
echo "baseline  : ${ORIG_BISON:-<unset>}"
echo "work dir  : $WORK"
# Record the environment that shapes the RESULTS, not just whether tools exist.
# autom4te generates the testsuite from the .at sources, so its version changes
# the script being run; and win_bison's quotearg picks quote styles from the
# locale forwarded below. A CI-vs-local mismatch in either shifts large numbers
# of diagnostic comparisons at once, which looks like a mass failure.
echo "autom4te  : $AUTOM4TE ($("$AUTOM4TE" --version 2>/dev/null | head -1))"
echo "m4        : $(m4 --version 2>/dev/null | head -1)"
echo "host      : $(uname -s) $(uname -r 2>/dev/null)"
echo "locale    : LANG=${LANG:-unset} LC_ALL=${LC_ALL:-unset} LC_CTYPE=${LC_CTYPE:-unset}"

rm -rf "$WORK"; mkdir -p "$WORK/bin"
cp "$AT"/*.at "$AT/testsuite.h" "$AT/package.m4" "$WORK/"

cd "$WORK"
echo "generating testsuite (autom4te)..."
"$AUTOM4TE" --language=autotest -I . testsuite.at -o testsuite || {
    echo "autom4te failed" >&2; exit 1; }
# The bison tests use the token @tb@ to mean a literal TAB (e.g. in %parse-param
# and api.namespace values). It is not a standard autotest quadrigraph, so our
# stand-alone autom4te does not convert it; substitute it in the generated
# script so grammars get a real tab (else it leaks as a stray '@' into output).
sed -i 's/@tb@/\t/g' testsuite

# Normalizing bison wrapper: rewrite the program name (win_bison[.exe] -> bison,
# GNU tools would strip .exe) and strip CR from win_bison's streams so the
# harness compares against upstream-style output.
#
# That is the whole job: MSYS2 runs win_bison.exe as an ordinary child, so the
# two variables below are simply inherited, as are the ones the tests set
# themselves (COLUMNS for caret width, YYFLAT, POSIXLY_CORRECT, TIME_LIMIT,
# BISON_USE_PUSH_FOR_PULL, and the locale -- diagnostics.at runs its multibyte
# cases as `LC_ALL="$locale" bison ...`). The wrapper is a plain exec: no temp
# files, no piping, no races.
cat > bin/bison <<WRAP
#!/usr/bin/env bash
#   BISON_PROGRAM_NAME=bison     -> diagnostics say "bison:", not "win_bison.exe:"
#   WINFLEXBISON_BINARY_OUTPUT=Y -> LF stdout/stderr instead of MSVC CRLF
#     (generated files are already LF via the xfopen binary-mode port fix)
export BISON_PROGRAM_NAME=bison
export WINFLEXBISON_BINARY_OUTPUT=Y
exec "$BISON" "\$@"
WRAP
chmod +x bin/bison

# Detect optional compilers to enable the compile tiers. These compile the
# parsers win_bison generates and run them (validating the generator output).
# Absent -> the C/C++ tiers auto-skip.
CC_BIN=$(command -v gcc 2>/dev/null || command -v cc 2>/dev/null || true)
CXX_BIN=$(command -v g++ 2>/dev/null || command -v c++ 2>/dev/null || true)
# mingw links to name.exe, and win_bison is a .exe regardless, so this is always
# the right suffix here.
EXEEXT='.exe'
echo "C compiler  : ${CC_BIN:-<none, C tier skipped>}"
echo "C++ compiler: ${CXX_BIN:-<none, C++ tier skipped>}"

# Probe which -std= flags the compiler actually accepts, rather than asserting
# them. Upstream's configure does this; hardcoding -std=c++20/-std=c++2b made
# older g++ fail ~90 groups with "unrecognized command line option" -- it spells
# them -std=c++2a/-std=c++2b. local.at expands these as ${CXX20_CXXFLAGS:+...},
# so an empty value simply drops that standard from the loop instead of failing.
#
# The probe links to a temp file, NOT to /dev/null: mingw's linker cannot write
# there, so a /dev/null probe reports EVERY standard as unsupported. That is
# silent -- local.at just drops the flags -- and it cost 59 glr2.cc groups,
# which are gated on C++11 being available, to a wholesale skip that looked like
# normal "no compiler" behaviour. Same /dev/null-is-not-a-file assumption that
# xfopen had to be taught about.
_probe_dir="$WORK/.cxx-probe"; mkdir -p "$_probe_dir"
cxx_std_flag () {   # $@ = candidate flags, echoes the first the compiler takes
    [ -n "$CXX_BIN" ] || return 0
    for _f in "$@"; do
        if printf 'int main(){}\n' \
             | "$CXX_BIN" "$_f" -x c++ - -o "$_probe_dir/probe.exe" >/dev/null 2>&1; then
            printf '%s' "$_f"; return 0
        fi
    done
}
CXX98_F=$(cxx_std_flag -std=c++98)
CXX03_F=$(cxx_std_flag -std=c++03)
CXX11_F=$(cxx_std_flag -std=c++11 -std=c++0x)
CXX14_F=$(cxx_std_flag -std=c++14 -std=c++1y)
CXX17_F=$(cxx_std_flag -std=c++17 -std=c++1z)
CXX20_F=$(cxx_std_flag -std=c++20 -std=c++2a)
CXX2B_F=$(cxx_std_flag -std=c++2b -std=c++23)
if [ -n "$CXX_BIN" ]; then
    echo "C++ standards: ${CXX98_F:-–} ${CXX03_F:-–} ${CXX11_F:-–} ${CXX14_F:-–} ${CXX17_F:-–} ${CXX20_F:-–} ${CXX2B_F:-–}"
fi

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

# mingw has no alarm(2). calc.at's driver arms a 200-second watchdog around the
# parse -- a hang guard sized for a 1995 DEC Alphastation, per its own comment --
# and nothing else in the suite uses it. Without a definition the entire calc.at
# compile tier (100 groups, 470-569) fails to *compile*, which tests nothing
# about bison. Supply a no-op through -include rather than patching the vendored
# .at sources, which stay byte-faithful to upstream.
COMPAT=''
if [ -n "$CC_BIN" ]; then
    cat > wfb-native-compat.h <<'H'
#ifndef WFB_NATIVE_COMPAT_H
#define WFB_NATIVE_COMPAT_H
/* mingw provides no alarm(2); calc.at uses it only as a watchdog. */
static unsigned int alarm (unsigned int seconds) { (void) seconds; return 0; }
#endif
H
    COMPAT=" -include $WORK/wfb-native-compat.h"
fi

# atlocal: enable the C/C++ tiers when compilers are present. -w silences
# warnings in generated parsers (some checks compile with -Werror otherwise).
c_works=false;   [ -n "$CC_BIN" ]  && c_works=true
cxx_works=false; [ -n "$CXX_BIN" ] && cxx_works=true
cat > atlocal <<LOC
: \${CC='$CC_BIN'} \${CXX='$CXX_BIN'} \${DC=''} \${CONF_JAVAC=''} \${CONF_JAVA=''}
: \${CPPFLAGS='-I$WORK$COMPAT'} \${CFLAGS='-w'} \${CXXFLAGS='-w'}
: \${NO_WERROR_CFLAGS='-w'} \${NO_WERROR_CXXFLAGS='-w'}
: \${CXX98_CXXFLAGS='$CXX98_F'} \${CXX03_CXXFLAGS='$CXX03_F'}
: \${CXX11_CXXFLAGS='$CXX11_F'} \${CXX14_CXXFLAGS='$CXX14_F'}
: \${CXX17_CXXFLAGS='$CXX17_F'} \${CXX20_CXXFLAGS='$CXX20_F'} \${CXX2B_CXXFLAGS='$CXX2B_F'}
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
# win_bison prints ASCII fallbacks where upstream prints Unicode glyphs (• for
# the item dot, ↳ in derivations, ε for empty). The port #if 0's bison's glyph
# machinery and drops <unicodeio.h> because gnulib's unicodeio module is not
# vendored, so the conversion cannot be linked:
#   159, 160  diagnostics.at  Counterexamples
# These only run where en_US.UTF-8 exists (see the AT_SKIP_IF in
# diagnostics.at), so they surface on Ubuntu 20.04/AppVeyor but silently skip
# on distros carrying only C.UTF-8. Vendoring unicodeio would fix them.
#
# Overridable so a candidate list can be tried without editing this file --
# useful when recalibrating for a different distro/autoconf, and it lets the
# failure-dump path below be exercised (BISON_XFAIL= makes a known xfail
# report as unexpected).
#
# Groups 129 and 283-287 (filenames with NTFS-illegal characters) are NOT here:
# the create fails up front and autotest skips them, which is the honest
# outcome. They were xfails under the previous WSL runner, where the Linux side
# could create a file Windows then could not open.
#
# 124 fails here and failed under WSL too, with no identified cause (see
# README). It was briefly delisted on the strength of a partial run that showed
# it passing; full runs contradict that.
#
# 764 (glr-regression.at, glr2.cc) trips a libstdc++ assertion inside the
# GENERATED parser: vector<bool>::operator[] with __n >= size(). That is a real
# out-of-bounds access in upstream bison 3.8.2's glr2.cc skeleton, not a port
# regression -- it happens under WSL too, silently, because Ubuntu's g++ builds
# without _GLIBCXX_ASSERTIONS while MSYS2's enables them by default. Do NOT
# "fix" this by disabling assertions: catching it is free coverage we did not
# have before. Worth reporting upstream.
BISON_XFAIL="${BISON_XFAIL-4 78 124 149 150 152 154 159 160 165 166 764}"

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
