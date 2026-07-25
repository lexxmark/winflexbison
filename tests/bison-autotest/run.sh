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
norm() { sed -e 's/win_bison\.exe/bison/g' -e 's/\r\$//'; }
"$BISON" "\$@" > >(norm) 2> >(norm >&2)
rc=\$?; wait; exit \$rc
WRAP
chmod +x bin/bison

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
EXEEXT='.exe'
CFG

# No compilers by default -> compile/Java/D tiers skip. Export CC=gcc etc.
# before running to enable them.
cat > atlocal <<'LOC'
: ${CC=''} ${CXX=''} ${DC=''} ${CONF_JAVAC=''} ${CONF_JAVA=''}
: ${CPPFLAGS=''} ${CFLAGS=''} ${CXXFLAGS=''}
: ${BISON_C_WORKS=false} ${BISON_CXX_WORKS=false} ${BISON_DC_WORKS=false}
POSIXLY_CORRECT_IS_EXPORTED=false
LOC

echo "running testsuite $*..."
./testsuite "$@"
