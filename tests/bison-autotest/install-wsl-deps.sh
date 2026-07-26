#!/usr/bin/env bash
# install-wsl-deps.sh — install everything the winflexbison test suites need on
# the WSL/Linux side of a fresh machine.
#
# What needs WSL:
#   * tests/bison-autotest/run.sh  — the full GNU Autotest bison suite
#       (autoconf/autom4te + m4 to build it; gcc/g++/make to run the C/C++
#        tiers; perl for error-message normalization).
#   * tests/bison/generate.sh      — regenerates the CTest golden files from the
#        REFERENCE bison (must match the vendored version, currently 3.8.2).
#
# The Windows CTest gate (runtests.bat, no --with-autotest) needs none of this.
#
# Usage (from WSL):
#   tests/bison-autotest/install-wsl-deps.sh              # required deps
#   tests/bison-autotest/install-wsl-deps.sh --with-java-d # also Java + D tiers
set -eu

WITH_JAVA_D=0
for arg in "$@"; do
    case "$arg" in
        --with-java-d) WITH_JAVA_D=1 ;;
        -h|--help)
            sed -n '2,20p' "$0"; exit 0 ;;
        *) echo "unknown option: $arg" >&2; exit 2 ;;
    esac
done

if ! grep -qiE 'microsoft|wsl' /proc/version 2>/dev/null; then
    echo "note: this does not look like WSL; continuing anyway (Linux is fine)." >&2
fi
if ! command -v apt-get >/dev/null 2>&1; then
    echo "error: apt-get not found — this script targets Debian/Ubuntu WSL." >&2
    exit 1
fi

# Required: autotest build (autoconf brings autom4te), m4, perl, C/C++ toolchain,
# and the reference bison used to regenerate the CTest golden.
REQUIRED="autoconf m4 perl build-essential bison"
# Optional: Java and D test tiers.
OPTIONAL="default-jdk gdc"

pkgs="$REQUIRED"
[ "$WITH_JAVA_D" -eq 1 ] && pkgs="$pkgs $OPTIONAL"

echo "=== installing: $pkgs ==="
sudo apt-get update
# shellcheck disable=SC2086
sudo apt-get install -y $pkgs

echo
echo "=== verification ==="
ok=0; miss=0
check() { # check <cmd> [note]
    if command -v "$1" >/dev/null 2>&1; then
        printf '  %-10s %s  %s\n' "$1" "$(command -v "$1")" "${2:-}"
        ok=$((ok + 1))
    else
        printf '  %-10s MISSING  %s\n' "$1" "${2:-}"
        miss=$((miss + 1))
    fi
}
check autom4te
check m4
check perl
check gcc
check g++
check make
check bison "$(bison --version 2>/dev/null | head -1)"
if [ "$WITH_JAVA_D" -eq 1 ]; then
    check javac
    check java
    check gdc
fi

# The CTest golden was generated with the vendored bison version; a mismatch
# means tests/bison/generate.sh would produce different golden.
ver="$(bison --version 2>/dev/null | head -1)"
case "$ver" in
    *"3.8.2"*) ;;
    *) echo; echo "WARNING: reference bison is '$ver', expected 3.8.2 —"
       echo "         regenerating CTest golden with it may not match the vendored bison." ;;
esac

echo
if [ "$miss" -eq 0 ]; then
    echo "All required tools present. Run the suite with:"
    echo "    runtests.bat --with-autotest        (from Windows)"
    echo "    tests/bison-autotest/run.sh         (from WSL)"
else
    echo "$miss tool(s) missing — see above." >&2
    exit 1
fi
