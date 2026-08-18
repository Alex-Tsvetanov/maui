#!/usr/bin/env bash
# Pre-push verification gate for the C++26 MAUI port: configure + build + ctest across every backend /
# sanitizer / lint lane, the way STATUS.md's "gate" lines are produced. This is the SLOW, thorough run
# (correctness across the whole matrix) — for the fast inner dev loop use tools/dev.sh instead.
#
# Why a script: the lanes live in separate build/<preset> trees that share no objects, so the gate is
# really N independent builds. This runner makes them cheap and consistent:
#   * ccache (wired in CMakeLists.txt) is warmed by ordering `headless` first — `tidy` then reuses its
#     object cache (identical flags) and only pays for the clang-tidy analysis pass.
#   * INCREMENTAL by default — it does NOT wipe build dirs, so Ninja recompiles only what changed. Pass
#     --clean for the true from-scratch gate (e.g. right before a push / when proving a clean-build claim).
#   * ctest runs with -j (the ios lane on a simulator is the headline win: ~8 min serial -> ~2 min -j8).
#
# Lanes (canonical order): headless tidy asan-ubsan tsan apple ios   [android msan opt-in]
#   - tidy / msan are BUILD-ONLY (no testPreset in CMakePresets.json; tidy's job is the clang-tidy pass).
#   - msan is Linux/Clang-only and android needs a booted emulator — neither is in the default set.
#
# Usage:
#   tools/gate.sh                      # default gate: headless tidy asan-ubsan tsan apple ios
#   tools/gate.sh --fast               # quick pre-commit: headless tidy apple   (skip sanitizers + ios)
#   tools/gate.sh headless apple       # only the named lanes
#   tools/gate.sh --clean              # wipe each build dir first (from-scratch gate)
#   tools/gate.sh --build-only         # configure+build every lane, skip ctest
#   tools/gate.sh -j 8 --keep-going    # ctest parallelism / don't stop at the first failing lane
#
# Env: VCPKG_ROOT (default $HOME/vcpkg), MAUI_CCACHE_MAXSIZE (default 25G, applied for this run only),
#      MAUI_IOS_SIM_UDID / MAUI_ANDROID_AVD (passed through to the platform runners).
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"   # port/cpp
cd "${here}"

export VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"
# Bigger-than-default cache so six lanes' debug objects don't evict each other mid-gate (per-run override,
# does not touch the user's global ccache config). Harmless if ccache is absent.
export CCACHE_MAXSIZE="${MAUI_CCACHE_MAXSIZE:-25G}"

ALL_LANES=(headless tidy asan-ubsan tsan apple ios android msan)
BUILD_ONLY_LANES=" tidy msan "   # no testPreset -> build, never ctest

clean=0 build_only=0 keep_going=0 jobs=""
lanes=()

usage() { sed -n '2,28p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit "${1:-0}"; }

while [[ $# -gt 0 ]]; do
  case "$1" in
    --fast)        lanes=(headless tidy apple) ;;
    --clean)       clean=1 ;;
    --build-only)  build_only=1 ;;
    -k|--keep-going) keep_going=1 ;;
    -j|--jobs)     jobs="$2"; shift ;;
    -j*)           jobs="${1#-j}" ;;
    -h|--help)     usage 0 ;;
    -*)            echo "gate.sh: unknown option '$1'" >&2; usage 64 ;;
    *)             lanes+=("$1") ;;
  esac
  shift
done

[[ -z "${jobs}" ]] && jobs="$(sysctl -n hw.ncpu 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"
[[ ${#lanes[@]} -eq 0 ]] && lanes=(headless tidy asan-ubsan tsan apple ios)

# Re-order requested lanes into canonical order (headless first warms ccache for tidy) + validate.
ordered=()
for canon in "${ALL_LANES[@]}"; do
  for want in "${lanes[@]}"; do
    [[ "${canon}" == "${want}" ]] && ordered+=("${canon}")
  done
done
for want in "${lanes[@]}"; do
  [[ " ${ALL_LANES[*]} " == *" ${want} "* ]] || { echo "gate.sh: unknown lane '${want}' (valid: ${ALL_LANES[*]})" >&2; exit 64; }
done
lanes=("${ordered[@]}")

ccache_present=0; command -v ccache >/dev/null 2>&1 && ccache_present=1
[[ ${ccache_present} -eq 1 ]] && ccache -z >/dev/null 2>&1 || true   # zero stats so the summary is this-run only

echo "==> gate: lanes=[${lanes[*]}]  jobs=${jobs}  clean=${clean}  build_only=${build_only}  VCPKG_ROOT=${VCPKG_ROOT}"

declare -a results
overall_rc=0
gate_start=${SECONDS}

# e2e.py's bytes-mode generator runs at CMake CONFIGURE time on the windows + android lanes, and a
# FATAL_ERROR there only catches it CRASHING. The failure that matters is quieter: it silently drops a
# page's hand-written code-behind, the TU still compiles, and the page renders inert -- visible only as
# a board red, days later. Seconds to run, no build needed, so it goes before the lanes rather than
# inside one.
gate_start_e2e=${SECONDS}
if python3 "${here}/../tools/e2e/test_e2e.py" >/dev/null 2>&1; then
  results+=("$(printf '%-12s %-28s %4ds' "e2e-gen" "ok" "$(( SECONDS - gate_start_e2e ))")")
else
  echo "gate.sh: port/tools/e2e/test_e2e.py FAILED -- rerun it directly for the diff" >&2
  python3 "${here}/../tools/e2e/test_e2e.py" || true
  results+=("$(printf '%-12s %-28s %4ds' "e2e-gen" "pytest-FAILED" "$(( SECONDS - gate_start_e2e ))")")
  overall_rc=1
  [[ ${keep_going} -eq 0 ]] && { echo "gate.sh: stopping (pass --keep-going to run the build lanes anyway)" >&2; lanes=(); }
fi

for lane in "${lanes[@]}"; do
  lane_start=${SECONDS}
  status="ok"
  echo; echo "================ lane: ${lane} ================"

  if [[ ${clean} -eq 1 ]]; then rm -rf "build/${lane}"; fi

  # Configure (re-run is cheap + idempotent, and picks up CMakeLists/preset edits).
  if ! cmake --preset "${lane}"; then status="configure-FAILED"; fi

  if [[ "${status}" == "ok" ]]; then
    if ! cmake --build --preset "${lane}"; then status="build-FAILED"; fi
  fi

  if [[ "${status}" == "ok" && ${build_only} -eq 0 ]]; then
    if [[ "${BUILD_ONLY_LANES}" == *" ${lane} "* ]]; then
      status="ok (build-only: no testPreset)"
    elif ! ctest --preset "${lane}" -j "${jobs}"; then
      status="ctest-FAILED"
    fi
  fi

  elapsed=$(( SECONDS - lane_start ))
  results+=("$(printf '%-12s %-28s %4ds' "${lane}" "${status}" "${elapsed}")")
  if [[ "${status}" == *FAILED* ]]; then
    overall_rc=1
    [[ ${keep_going} -eq 0 ]] && { echo "gate.sh: ${lane} ${status} — stopping (pass --keep-going to run remaining lanes)" >&2; break; }
  fi
done

echo; echo "==================== gate summary ===================="
printf '  %s\n' "${results[@]}"
echo "  ----"
printf '  total %ds\n' "$(( SECONDS - gate_start ))"
if [[ ${ccache_present} -eq 1 ]]; then
  echo "  ---- ccache (this run) ----"
  ccache -s 2>/dev/null | sed 's/^/  /' || true
fi
[[ ${overall_rc} -eq 0 ]] && echo "  RESULT: PASS" || echo "  RESULT: FAIL"
exit "${overall_rc}"
