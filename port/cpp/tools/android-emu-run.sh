#!/usr/bin/env bash
# CMAKE_CROSSCOMPILING_EMULATOR for the `android` preset (PROFILE §4 — the Android backend): runs an
# NDK-built arm64-v8a binary on an Android emulator, so ctest and gtest test discovery execute the
# cross-compiled test executables transparently (`<this script> <binary> <args...>`) — the twin of
# tools/ios-sim-run.sh with adb in place of simctl. The SDK/AVD/boot/staging machinery is shared with
# the app_process widget test host (android-testhost-run.sh) via android-emu-lib.sh; see the lib
# header for the SDK-root, AVD-pick, and keyed-staging contracts.
#
# Execution: `adb shell` with each argument single-quoted for the device shell (gtest filters carry
# `*`/`:` which mksh would otherwise glob), cwd + $TMPDIR/$HOME pointed at the staging dir (Android
# has no /tmp — std::filesystem::temp_directory_path() honors $TMPDIR; ANDROID_STL=c++_static keeps
# the staged binary self-contained), and the remote exit code preserved (platform-tools >= 24 speak
# the shell-v2 protocol, which carries it; verified on the pinned platform-tools 37). A
# `--gtest_output=json:<host path>` argument (gtest discovery) is redirected to a device path and
# pulled back to the requested host path afterwards.
set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "usage: android-emu-run.sh <android-arm64-binary> [args...]" >&2
  exit 64
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=android-emu-lib.sh
source "${script_dir}/android-emu-lib.sh"

exe="$1"
shift
[[ -f "${exe}" ]] || maui_die "no such binary: ${exe}"

maui_android_resolve_tools
maui_android_ensure_booted

maui_android_stage_file "${exe}"
staged_exe="${maui_staged_file}"

# ---- Redirect a --gtest_output=json:<host path> (gtest discovery) to a device path ----
args=()
json_host=""
json_device=""
for arg in "$@"; do
  case "${arg}" in
  --gtest_output=json:*)
    json_host="${arg#--gtest_output=json:}"
    json_device="${maui_device_dir}/${maui_staged_key}-$(basename "${json_host}")"
    args+=("--gtest_output=json:${json_device}")
    ;;
  *)
    args+=("${arg}")
    ;;
  esac
done

cmd="cd '${maui_device_dir}' && TMPDIR='${maui_device_dir}/tmp' HOME='${maui_device_dir}' $(maui_android_shell_quote "${staged_exe}")"
for arg in ${args+"${args[@]}"}; do
  cmd+=" $(maui_android_shell_quote "${arg}")"
done

rc=0
"${maui_adb}" -s "${maui_serial}" shell "${cmd}" || rc=$?

if [[ -n "${json_host}" ]] && "${maui_adb}" -s "${maui_serial}" shell "test -f '${json_device}'" > /dev/null 2>&1; then
  "${maui_adb}" -s "${maui_serial}" pull "${json_device}" "${json_host}" > /dev/null 2>&1
  "${maui_adb}" -s "${maui_serial}" shell "rm -f '${json_device}'" > /dev/null 2>&1 || true
fi

exit "${rc}"
