#!/usr/bin/env bash
# CMAKE_CROSSCOMPILING_EMULATOR for the `android` preset (PROFILE §4 — the Android backend): runs an
# NDK-built arm64-v8a binary on an Android emulator, so ctest and gtest test discovery execute the
# cross-compiled test executables transparently (`<this script> <binary> <args...>`) — the twin of
# tools/ios-sim-run.sh with adb in place of simctl.
#
# SDK tools are resolved to ABSOLUTE paths under a known SDK root (never the caller's PATH — adb and
# emulator are deliberately not on PATH on the dev machine, and a stray adb from another SDK would
# talk to the wrong server). Override the root with $MAUI_ANDROID_SDK_ROOT if the SDK moves.
#
# Device pick is deterministic: the AVD named $MAUI_ANDROID_AVD (default: the dedicated `maui-test`
# AVD, created on demand from the preinstalled android-34 google_apis arm64-v8a system image so the
# user's own AVDs are never touched; image preinstalled => no sdkmanager license round trip needed).
# The emulator is booted headless on demand and intentionally left running — booting per test
# process would be prohibitively slow. A running emulator is matched to the AVD by name via the
# console (`adb emu avd name`), so other emulators the user has running are left alone.
#
# Staging: the binary is pushed once to /data/local/tmp/maui under a name keyed by
# (path, fractional mtime, size) — same scheme as the ios runner — so rebuilds re-push, unchanged
# binaries don't, and concurrent ctest jobs never collide (push lands on a unique temp name, then an
# on-device mv publishes it atomically). ANDROID_STL=c++_static (preset) makes the executables
# self-contained, so the binary itself is the only thing staged.
#
# Execution: `adb shell` with each argument single-quoted for the device shell (gtest filters carry
# `*`/`:` which mksh would otherwise glob), cwd + $TMPDIR/$HOME pointed at the staging dir (Android
# has no /tmp — std::filesystem::temp_directory_path() honors $TMPDIR), and the remote exit code
# preserved (platform-tools >= 24 speak the shell-v2 protocol, which carries it; verified on the
# pinned platform-tools 37). A `--gtest_output=json:<host path>` argument (gtest discovery) is
# redirected to a device path and pulled back to the requested host path afterwards.
set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "usage: android-emu-run.sh <android-arm64-binary> [args...]" >&2
  exit 64
fi

die() {
  echo "android-emu-run.sh: $*" >&2
  exit 69 # EX_UNAVAILABLE
}

# ---- SDK root + absolute tool paths (never PATH) ----
sdk_root="${MAUI_ANDROID_SDK_ROOT:-}"
if [[ -z "${sdk_root}" ]]; then
  for candidate in /opt/homebrew/share/android-commandlinetools "${HOME}/Library/Android/sdk"; do
    if [[ -x "${candidate}/platform-tools/adb" ]]; then
      sdk_root="${candidate}"
      break
    fi
  done
fi
[[ -n "${sdk_root}" && -x "${sdk_root}/platform-tools/adb" ]] ||
  die "no Android SDK with platform-tools/adb found (set MAUI_ANDROID_SDK_ROOT)"
adb="${sdk_root}/platform-tools/adb"
emulator_bin="${sdk_root}/emulator/emulator"
avdmanager_bin="${sdk_root}/cmdline-tools/latest/bin/avdmanager"

exe="$1"
shift
[[ -f "${exe}" ]] || die "no such binary: ${exe}"

avd_name="${MAUI_ANDROID_AVD:-maui-test}"

# All adb chatter (server start, push progress) goes to stderr or /dev/null: gtest discovery PARSES
# this script's stdout for the test list, so stdout must carry the device process's output only.
"${adb}" start-server > /dev/null 2>&1

# ---- Find a running emulator hosting our AVD (match by console-reported AVD name) ----
find_serial() {
  local serial state name
  while read -r serial state; do
    [[ "${serial}" == emulator-* && "${state}" == device ]] || continue
    name="$("${adb}" -s "${serial}" emu avd name 2> /dev/null | head -n1 | tr -d '\r')"
    if [[ "${name}" == "${avd_name}" ]]; then
      printf '%s' "${serial}"
      return 0
    fi
  done < <("${adb}" devices | sed -n 's/^\(emulator-[0-9][0-9]*\)[[:space:]]\{1,\}\(.*\)$/\1 \2/p')
  return 1
}

serial="$(find_serial || true)"

# ---- Boot on demand (headless), creating the dedicated AVD first if it doesn't exist ----
if [[ -z "${serial}" ]]; then
  [[ -x "${emulator_bin}" ]] || die "emulator binary missing: ${emulator_bin}"
  if ! "${avdmanager_bin}" list avd -c 2> /dev/null | grep -qx "${avd_name}"; then
    # The hardware-profile prompt is answered "no" (stock profile); the system image ships with the
    # SDK, so no sdkmanager --licenses step is needed here.
    echo "no" | "${avdmanager_bin}" create avd --name "${avd_name}" \
      --package "system-images;android-34;google_apis;arm64-v8a" --device pixel_5 > /dev/null 2>&1 ||
      die "failed to create AVD '${avd_name}' (check cmdline-tools + java)"
  fi
  # Detached + fully redirected: a background child sharing this script's stdout would hold the
  # pipe open and stall gtest discovery long after this script exits.
  boot_log="${TMPDIR:-/tmp}/maui-android-emu-${avd_name}.log"
  nohup "${emulator_bin}" -avd "${avd_name}" -no-window -no-audio -no-boot-anim \
    < /dev/null > "${boot_log}" 2>&1 &
  disown
  deadline=$((SECONDS + 300))
  while [[ -z "${serial}" ]]; do
    ((SECONDS < deadline)) || die "emulator for AVD '${avd_name}' did not appear (log: ${boot_log})"
    sleep 2
    serial="$(find_serial || true)"
  done
fi

"${adb}" -s "${serial}" wait-for-device 2> /dev/null
boot_deadline=$((SECONDS + 300))
until [[ "$("${adb}" -s "${serial}" shell getprop sys.boot_completed 2> /dev/null | tr -d '[:space:]')" == "1" ]]; do
  ((SECONDS < boot_deadline)) || die "AVD '${avd_name}' (${serial}) did not finish booting"
  sleep 2
done

# ---- Keyed staging: push once per (path, fractional mtime, size); concurrent jobs never collide ----
device_dir="/data/local/tmp/maui"
exe_stat="$(/usr/bin/stat -f '%Fm %z' "${exe}")"
exe_key="$(/usr/bin/shasum -a 256 <<< "${exe}|${exe_stat}" | cut -c1-16)"
staged_exe="${device_dir}/${exe_key}-$(basename "${exe}")"
if ! "${adb}" -s "${serial}" shell "test -x '${staged_exe}'" > /dev/null 2>&1; then
  "${adb}" -s "${serial}" shell "mkdir -p '${device_dir}/tmp'" > /dev/null
  "${adb}" -s "${serial}" push "${exe}" "${staged_exe}.tmp.$$" > /dev/null 2>&1
  "${adb}" -s "${serial}" shell "chmod 755 '${staged_exe}.tmp.$$' && mv '${staged_exe}.tmp.$$' '${staged_exe}'" > /dev/null
fi

# ---- Redirect a --gtest_output=json:<host path> (gtest discovery) to a device path ----
args=()
json_host=""
json_device=""
for arg in "$@"; do
  case "${arg}" in
  --gtest_output=json:*)
    json_host="${arg#--gtest_output=json:}"
    json_device="${device_dir}/${exe_key}-$(basename "${json_host}")"
    args+=("--gtest_output=json:${json_device}")
    ;;
  *)
    args+=("${arg}")
    ;;
  esac
done

# Single-quote an argument for the device's mksh (close-quote, escaped quote, reopen for embedded ').
shell_quote() {
  local s="${1//\'/\'\\\'\'}"
  printf "'%s'" "${s}"
}

cmd="cd '${device_dir}' && TMPDIR='${device_dir}/tmp' HOME='${device_dir}' $(shell_quote "${staged_exe}")"
for arg in ${args+"${args[@]}"}; do
  cmd+=" $(shell_quote "${arg}")"
done

# shell-v2 (platform-tools 37 vs API 34) carries the remote exit code through adb shell.
rc=0
"${adb}" -s "${serial}" shell "${cmd}" || rc=$?

if [[ -n "${json_host}" ]] && "${adb}" -s "${serial}" shell "test -f '${json_device}'" > /dev/null 2>&1; then
  "${adb}" -s "${serial}" pull "${json_device}" "${json_host}" > /dev/null 2>&1
  "${adb}" -s "${serial}" shell "rm -f '${json_device}'" > /dev/null 2>&1 || true
fi

exit "${rc}"
