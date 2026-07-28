#!/usr/bin/env bash
# install-msys2-deps.sh — install everything the winflexbison test suites need
# on the MSYS2 side of a fresh machine.
#
# What needs MSYS2:
#   * tests/bison-autotest/run.sh  — the full GNU Autotest bison suite
#       (autoconf2.71/autom4te + m4 to build it; mingw-w64 gcc/g++ to run the
#        C/C++ tiers; perl for error-message normalization).
#   * tests/bison/generate.sh      — regenerates the CTest golden files from the
#        REFERENCE bison (must match the vendored version, currently 3.8.2,
#        which is exactly what the msys 'bison' package ships).
#
# The Windows CTest gate (runtests.bat, no --with-autotest) needs none of this.
#
# autoconf2.71 is pinned deliberately: autom4te expands the .at sources, so its
# version is part of the test *inputs* -- 2.69 and 2.71 emit different
# testsuites with different group numbering, and an xfail list calibrated
# against one does not describe the other.
#
# Usage (from an MSYS2 shell):
#   tests/bison-autotest/install-msys2-deps.sh              # required deps
#   tests/bison-autotest/install-msys2-deps.sh --with-java-d # also Java + D tiers
set -eu

WITH_JAVA_D=0
for arg in "$@"; do
    case "$arg" in
        --with-java-d) WITH_JAVA_D=1 ;;
        -h|--help)
            sed -n '2,22p' "$0"; exit 0 ;;
        *) echo "unknown option: $arg" >&2; exit 2 ;;
    esac
done

if ! command -v pacman >/dev/null 2>&1; then
    echo "error: pacman not found -- run this from an MSYS2 shell" >&2
    echo "       (C:\\msys64\\usr\\bin\\bash -lc tests/bison-autotest/install-msys2-deps.sh)" >&2
    exit 1
fi

# Required: autotest build (autoconf2.71 brings autom4te-2.71), m4, perl, the
# text utilities the harness drives, the mingw C/C++ toolchain for the compile
# tiers, and the reference bison used to regenerate the CTest golden.
REQUIRED="autoconf2.71 m4 perl diffutils sed grep gawk mingw-w64-x86_64-gcc bison"
# Optional: Java and D test tiers.
OPTIONAL="mingw-w64-x86_64-openjdk mingw-w64-x86_64-gcc-libgfortran"

pkgs="$REQUIRED"
[ "$WITH_JAVA_D" -eq 1 ] && pkgs="$pkgs $OPTIONAL"

echo "=== installing: $pkgs ==="
# shellcheck disable=SC2086
pacman -Sy --noconfirm --needed $pkgs

echo
echo "=== verification ==="
# The compilers live under the MINGW64 prefix, which only a MINGW64 login shell
# puts on PATH; add it so this check works from a plain msys shell too.
[ -d /mingw64/bin ] && PATH="/mingw64/bin:$PATH"
ok=0; miss=0
check() { # check <cmd> [note]
    if command -v "$1" >/dev/null 2>&1; then
        printf '  %-14s %s  %s\n' "$1" "$(command -v "$1")" "${2:-}"
        ok=$((ok + 1))
    else
        printf '  %-14s MISSING  %s\n' "$1" "${2:-}"
        miss=$((miss + 1))
    fi
}
check autom4te-2.71
check m4
check perl
check gcc
check g++
check bison "$(bison --version 2>/dev/null | head -1)"
if [ "$WITH_JAVA_D" -eq 1 ]; then
    check javac
    check java
fi

# The CTest golden was generated with the vendored bison version; a mismatch
# means tests/bison/generate.sh would produce different golden.
ver="$(bison --version 2>/dev/null | head -1)"
case "$ver" in
    *"3.8.2"*) ;;
    *) echo; echo "WARNING: reference bison is '$ver', expected 3.8.2 --"
       echo "         regenerating CTest golden with it may not match the vendored bison." ;;
esac

echo
if [ "$miss" -eq 0 ]; then
    echo "All required tools present. Run the suite with:"
    echo "    runtests.bat --with-autotest        (from Windows)"
    echo "    tests/bison-autotest/run.sh         (from MSYS2)"
else
    echo "$miss tool(s) missing -- see above." >&2
    exit 1
fi
