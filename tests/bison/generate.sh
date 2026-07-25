#!/usr/bin/env bash
# generate.sh — regenerate bison golden outputs (run under WSL/Linux).
#
# For every grammar in golden-cases/, runs the REFERENCE bison (must be 3.8.2,
# matching the vendored version) by bare filename and captures stdout, stderr
# and exit code into golden/. win_bison is then held to these on Windows by
# tests/bison/run_bison_test.cmake. Re-run this after a bison upgrade.
#
#   Usage (from WSL):  tests/bison/generate.sh
#   Override bison:    BISON=/path/to/bison tests/bison/generate.sh
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
CASES="$HERE/golden-cases"
GOLD="$HERE/golden"
BISON="${BISON:-bison}"

ver="$("$BISON" --version 2>/dev/null | head -1)"
echo "reference bison: ${ver:-<none>}"
case "$ver" in
    *"3.8.2"*) ;;
    *) echo "WARNING: expected bison 3.8.2 for golden fidelity" >&2 ;;
esac

mkdir -p "$GOLD"
cd "$CASES" || { echo "no golden-cases dir" >&2; exit 1; }

n=0
for y in *.y; do
    [ -e "$y" ] || continue
    name="${y%.y}"
    args=""
    [ -f "$name.args" ] && args="$(cat "$name.args")"
    # -fno-caret: compare the semantic diagnostic (message + location), not the
    # source-line/caret rendering, which is platform-fragile (win_bison omits
    # the echoed source line on Windows — a known port discrepancy).
    # shellcheck disable=SC2086
    "$BISON" -fno-caret $args "$y" >"$GOLD/$name.out" 2>"$GOLD/$name.err"
    rc=$?
    sed -i 's/\r$//' "$GOLD/$name.out" "$GOLD/$name.err"
    echo "$rc" >"$GOLD/$name.exit"
    rm -f "$name.tab.c" "$name.tab.h" "$name.output"
    echo "  $name: exit=$rc"
    n=$((n + 1))
done
echo "generated golden for $n grammar(s) into golden/"
