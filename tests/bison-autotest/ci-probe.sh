#!/usr/bin/env bash
# ci-probe.sh — report what a machine's WSL side actually offers, so wiring
# tests/bison-autotest/run.sh into CI can be designed from data rather than
# assumptions. Answers, in order, the questions that decide the design:
#
#   1. Which distro is it, and is it EOL? (EOL archives 404 on apt-get update)
#   2. Are run.sh's required tools present? (autom4te m4 perl diff sed)
#   3. Are gcc/g++ present? The xfail list in run.sh was calibrated WITH them;
#      without them the compile tiers auto-skip and the list stops matching.
#   4. Can we install anything — are we root, or is sudo usable non-interactively?
#   5. Does WSL interop actually execute the Windows win_bison.exe? That is the
#      mechanism the whole harness depends on.
#
# Diagnostics only: this never fails, and always exits 0.
#
# No longer wired into .appveyor.yml -- the autotest now runs there for real,
# which proves the same things. Kept as a manual troubleshooting tool for when
# a worker image changes and the autotest suddenly breaks:
#     wsl -e bash ./tests/bison-autotest/ci-probe.sh
# Last measured on AppVeyor (Visual Studio 2022 image): WSL1, Ubuntu 20.04.3,
# perl/diff/sed present, autom4te/m4/gcc/g++/make absent, passwordless sudo and
# apt both working, and interop able to execute win_bison.exe.
echo "=== distro ==="
grep -E '^(NAME|VERSION)=' /etc/os-release 2>/dev/null || echo "  /etc/os-release unreadable"
echo "  kernel: $(uname -r 2>/dev/null)"

echo "=== tools run.sh requires ==="
for t in autom4te m4 perl diff sed; do
    echo "  $t: $(command -v "$t" 2>/dev/null || echo MISSING)"
done

echo "=== compile tiers (xfail list assumes these are present) ==="
for t in gcc g++ make; do
    echo "  $t: $(command -v "$t" 2>/dev/null || echo MISSING)"
done

echo "=== privilege ==="
echo "  whoami: $(whoami 2>/dev/null)"
echo "  sudo:   $(command -v sudo 2>/dev/null || echo MISSING)"

echo "=== apt reachable? ==="
if [ "$(id -u 2>/dev/null)" = "0" ]; then
    apt-get update >/dev/null 2>&1 && echo "  APT_OK (root)" || echo "  APT_FAILED (root) -- likely EOL archives"
elif command -v sudo >/dev/null 2>&1; then
    sudo -n apt-get update >/dev/null 2>&1 \
        && echo "  APT_OK (sudo -n)" \
        || echo "  APT_FAILED (sudo -n) -- needs a password, or EOL archives"
else
    echo "  SKIPPED (not root and no sudo)"
fi

echo "=== WSL interop: can bash run the Windows win_bison.exe? ==="
here="$(cd "$(dirname "$0")" && pwd)"
port_root="$(cd "$here/../.." && pwd)"
for cfg in Release Debug; do
    b="$port_root/bin/$cfg/win_bison.exe"
    if [ -x "$b" ]; then
        echo "  $cfg: $("$b" --version 2>&1 | head -1)"
    else
        echo "  $cfg: not found at $b"
    fi
done

echo "=== probe done ==="
exit 0
