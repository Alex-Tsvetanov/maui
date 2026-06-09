#!/usr/bin/env bash
# CMAKE_CROSSCOMPILING_EMULATOR for the `ios` preset (PROFILE §4 — the iOS backend, M6): runs an
# iphonesimulator-built binary on a booted iOS simulator, so ctest and gtest test discovery execute
# the cross-compiled test executables transparently (`<this script> <binary> <args...>`).
#
# Device pick is deterministic: $MAUI_IOS_SIM_UDID when set, else the first available iPhone in
# `simctl list` order. The simulator is booted on demand (`bootstatus -b` is a no-op when already
# booted) and intentionally left running — booting per test process would be prohibitively slow.
#
# TCC DETOUR (macOS privacy mediation): a simulator-spawned process accessing files under a protected
# host folder (~/Documents, ~/Desktop, …) blocks inside open(2) until the user grants the simulator
# runtime Files-and-Folders access — and in a headless/CI session no prompt ever appears, so dyld
# hangs forever just mapping the test executable when the build tree lives in such a folder. The
# per-user temp dir is not TCC-protected, so this script (a) stages the binary there before spawning
# and (b) redirects a `--gtest_output=json:` argument (gtest discovery writes that file from inside
# the simulator) to the staging dir, moving the result back host-side afterwards.
set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "usage: ios-sim-run.sh <ios-simulator-binary> [args...]" >&2
  exit 64
fi

udid="${MAUI_IOS_SIM_UDID:-}"
if [[ -z "${udid}" ]]; then
  udid="$(xcrun simctl list devices available --json | /usr/bin/python3 -c '
import json
import sys

data = json.load(sys.stdin)
for runtime in sorted(data["devices"]):
    for device in data["devices"][runtime]:
        if device.get("isAvailable") and device["name"].startswith("iPhone"):
            print(device["udid"])
            raise SystemExit
sys.exit("ios-sim-run.sh: no available iPhone simulator (check: xcrun simctl list devices available)")
')"
fi

exe="$1"
shift

stage_dir="${TMPDIR:-/tmp}/maui-ios-sim-run"
mkdir -p "${stage_dir}"

# Stage the executable under a name keyed by (path, fractional mtime, size), so any rebuild gets a
# fresh staged copy (a whole-second `-nt` check could miss a rebuild landing in the same second) and
# concurrent ctest jobs never collide. The copy goes through a unique temp file + atomic rename so a
# parallel first run never spawns a torn binary. Old copies linger in the (purgeable) temp dir.
exe_stat="$(/usr/bin/stat -f '%Fm %z' "${exe}")"
exe_key="$(/usr/bin/shasum -a 256 <<< "${exe}|${exe_stat}" | cut -c1-16)"
staged_exe="${stage_dir}/${exe_key}-$(basename "${exe}")"
if [[ ! -e "${staged_exe}" ]]; then
  cp -f "${exe}" "${staged_exe}.tmp.$$"
  mv -f "${staged_exe}.tmp.$$" "${staged_exe}"
fi

# Redirect a --gtest_output=json:<host path> (gtest discovery) into the staging dir; the simulator
# process writes there freely and the file is moved to the requested host path afterwards.
args=()
json_host=""
json_staged=""
for arg in "$@"; do
  case "${arg}" in
  --gtest_output=json:*)
    json_host="${arg#--gtest_output=json:}"
    json_staged="${stage_dir}/${exe_key}-$(basename "${json_host}")"
    args+=("--gtest_output=json:${json_staged}")
    ;;
  *)
    args+=("${arg}")
    ;;
  esac
done

xcrun simctl bootstatus "${udid}" -b > /dev/null

rc=0
xcrun simctl spawn "${udid}" "${staged_exe}" ${args+"${args[@]}"} || rc=$?

if [[ -n "${json_host}" && -f "${json_staged}" ]]; then
  mv -f "${json_staged}" "${json_host}"
fi

exit "${rc}"
