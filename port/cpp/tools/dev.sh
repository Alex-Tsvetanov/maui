#!/usr/bin/env bash
# Fast inner dev loop for the C++26 MAUI port: incremental rebuild + targeted ctest on ONE preset.
# This is the answer to "just rebuild the changed files and run the tests" — Ninja already recompiles
# only the touched TUs (and ccache makes even those cheap), so a tight edit→test cycle is seconds, not
# the 20-30 min of the full multi-lane gate (tools/gate.sh).
#
# It never wipes the build dir; it configures only if the dir isn't configured yet (or with -c).
#
# Usage:
#   tools/dev.sh                       # incremental headless build + run ALL headless tests
#   tools/dev.sh layout                # build + run only tests whose name matches /layout/ (ctest -R)
#   tools/dev.sh -t maui_layouts_tests layout   # build ONLY that test target, then run /layout/
#   tools/dev.sh -p apple Button       # same loop on the apple (macOS) backend
#   tools/dev.sh -c                    # force a re-configure (after editing CMakeLists/presets)
#
# Env: VCPKG_ROOT (default $HOME/vcpkg), MAUI_IOS_SIM_UDID (passed through for -p ios).
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"   # port/cpp
cd "${here}"
export VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"

preset="headless" target="" jobs="" reconfigure=0 regex=""
while [[ $# -gt 0 ]]; do
  case "$1" in
    -p|--preset)  preset="$2"; shift ;;
    -t|--target)  target="$2"; shift ;;
    -j|--jobs)    jobs="$2"; shift ;;
    -c|--reconfigure) reconfigure=1 ;;
    -h|--help)    sed -n '2,20p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 0 ;;
    -*)           echo "dev.sh: unknown option '$1'" >&2; exit 64 ;;
    *)            regex="$1" ;;
  esac
  shift
done
[[ -z "${jobs}" ]] && jobs="$(sysctl -n hw.ncpu 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"

if [[ ${reconfigure} -eq 1 || ! -f "build/${preset}/CMakeCache.txt" ]]; then
  cmake --preset "${preset}"
fi

# `-j` CAPS THE BUILD TOO, NOT JUST CTEST. It used to reach only `ctest_args` below, so
# `dev.sh -j 6` still ran a bare `cmake --build`, which lets ninja default to core count.
# MEASURED 2026-08-22: a 16-way rebuild on this 14-core host took load 15 -> 52 while two
# unrepeatable timed captures were live, and the Windows lane's capture time went from a
# 12.7s mean / 32.6s worst to 18.6s / 109.6s. Nothing errored, but a starved capture is how
# this project has previously banked plausible-but-wrong frames, and re-running a 3h capture
# costs hours where a slower build costs minutes.
if [[ -n "${target}" ]]; then
  cmake --build --preset "${preset}" -j "${jobs}" --target "${target}"
else
  cmake --build --preset "${preset}" -j "${jobs}"
fi

ctest_args=(--preset "${preset}" -j "${jobs}" --output-on-failure)
[[ -n "${regex}" ]] && ctest_args+=(-R "${regex}")
ctest "${ctest_args[@]}"
