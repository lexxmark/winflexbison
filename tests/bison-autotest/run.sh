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
# grammars from it). Default to the superproject's orig/bison next to the port.
ORIG_BISON="${ORIG_BISON:-$(cd "$PORT_ROOT/../orig/bison" 2>/dev/null && pwd)}"

[ -x "$BISON" ] || { echo "win_bison not found/executable: $BISON" >&2; exit 1; }
for t in autom4te m4 perl diff sed; do
    command -v "$t" >/dev/null || { echo "missing tool: $t (apt install autoconf m4)" >&2; exit 1; }
done

echo "win_bison : $BISON"
echo "baseline  : ${ORIG_BISON:-<unset>}"
echo "work dir  : $WORK"

rm -rf "$WORK"; mkdir -p "$WORK/bin"
cp "$AT"/*.at "$AT/testsuite.h" "$AT/package.m4" "$WORK/"

cd "$WORK"
echo "generating testsuite (autom4te)..."
autom4te --language=autotest -I . testsuite.at -o testsuite || {
    echo "autom4te failed" >&2; exit 1; }

# Normalizing bison wrapper: rewrite the program name (win_bison[.exe] -> bison,
# GNU tools would strip .exe) and strip CR from win_bison's streams so the
# harness compares against upstream-style output.
cat > bin/bison <<WRAP
#!/usr/bin/env bash
# win_bison.exe writes CRLF (MSVC text-mode stdio) and names itself
# "win_bison.exe"; strip CR and rewrite the program name so the harness sees
# upstream-style output.
norm() { tr -d '\r' | sed 's/win_bison\.exe/bison/g'; }
# WSL does not forward env vars to Windows processes unless listed in WSLENV.
# The suite sets several env vars that win_bison reads via getenv (COLUMNS for
# caret width, YYFLAT for flat counterexamples, POSIXLY_CORRECT, TIME_LIMIT,
# BISON_USE_PUSH_FOR_PULL, BISON_PROGRAM_NAME, LC_CTYPE); forward them all (no
# flag = shared as-is) so win_bison behaves like the reference.
_fwd="COLUMNS:YYFLAT:POSIXLY_CORRECT:TIME_LIMIT:BISON_USE_PUSH_FOR_PULL:BISON_PROGRAM_NAME:LC_CTYPE"
export WSLENV="\${_fwd}\${WSLENV:+:\$WSLENV}"
# Capture to temp files and normalize synchronously. (Process substitution with
# a trailing 'wait' does NOT reliably flush the substitution processes before
# the parent exits, which intermittently truncated output under load.)
_o=\$(mktemp); _e=\$(mktemp)
"$BISON" "\$@" >"\$_o" 2>"\$_e"; rc=\$?
norm <"\$_o"
norm <"\$_e" >&2
rm -f "\$_o" "\$_e"
exit \$rc
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
POSIXLY_CORRECT_IS_EXPORTED=false
LOC

# Expected failures. See README.md. Three kinds:
#
# WSL-environment limits (a Windows process can't do this via the WSL FS bridge):
#   129  output.at  filename with NTFS-illegal chars (: < > | ...)
#   314  actions.at --fixit backup rename on the 9p /tmp share
#
# win_bison in-process-m4 limitation (candidate for a future fix): skeleton
# complaints emitted via b4_cat/@complain during macro-argument expansion do
# not reach scan-skel, so these diagnostics are dropped:
#   54   input.at      C++ namespace reference errors
#   165  skeletons.at  Complaining during macro argument expansion
#   166  skeletons.at  Fatal errors make M4 exit immediately
#
# Harness edge case (perl in-place $at_dir substitution + heredoc on Windows):
#   124  output.at     Output files: ... api.location.file="$at_dir/..."
BISON_XFAIL="129 314 54 124 165 166"

echo "running testsuite $*..."
./testsuite "$@" 2>&1 | tee testsuite.out
rc=${PIPESTATUS[0]}

# Post-process: treat BISON_XFAIL groups as expected failures.
failed=$(grep -aE '^[[:space:]]*[0-9]+: .* FAILED' testsuite.out \
         | sed -E 's/^[[:space:]]*([0-9]+):.*/\1/' | sort -un)
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
    exit 1
fi
# Only expected failures (or none) remain: success.
exit 0
