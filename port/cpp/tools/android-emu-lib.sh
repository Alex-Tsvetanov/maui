# Shared machinery for the Android tool scripts — SOURCED by android-emu-run.sh (the ctest
# cross-compiling emulator) and android-testhost-run.sh (the app_process widget test host).
# Not a standalone executable. Callers run under `set -euo pipefail`.
#
# Everything resolves to ABSOLUTE paths under a known SDK root (never the caller's PATH — adb and
# emulator are deliberately not on PATH on the dev machine, and a stray adb from another SDK would
# talk to the wrong server). Override the root with $MAUI_ANDROID_SDK_ROOT if the SDK moves.
#
# Device pick is deterministic: the AVD named $MAUI_ANDROID_AVD (default: the dedicated `maui-test`
# AVD, created on demand from the preinstalled android-34 google_apis arm64-v8a system image so the
# user's own AVDs are never touched; image preinstalled => no sdkmanager license round trip needed).
# The emulator is booted headless on demand and intentionally left running — booting per test
# process would be prohibitively slow. A running emulator is matched to the AVD by name via the
# console (`adb emu avd name`), so other emulators the user has running are left alone.

maui_device_dir="/data/local/tmp/maui"

maui_die() {
  echo "${0##*/}: $*" >&2
  exit 69 # EX_UNAVAILABLE
}

# Resolves maui_sdk_root / maui_adb / maui_emulator_bin / maui_avdmanager_bin / maui_avd_name and
# starts the adb server. All adb chatter goes to stderr or /dev/null: gtest discovery PARSES the
# runner's stdout for the test list, so stdout must carry the device process's output only.
maui_android_resolve_tools() {
  maui_sdk_root="${MAUI_ANDROID_SDK_ROOT:-}"
  if [[ -z "${maui_sdk_root}" ]]; then
    local candidate
    for candidate in /opt/homebrew/share/android-commandlinetools "${HOME}/Library/Android/sdk"; do
      if [[ -x "${candidate}/platform-tools/adb" ]]; then
        maui_sdk_root="${candidate}"
        break
      fi
    done
  fi
  [[ -n "${maui_sdk_root}" && -x "${maui_sdk_root}/platform-tools/adb" ]] ||
    maui_die "no Android SDK with platform-tools/adb found (set MAUI_ANDROID_SDK_ROOT)"
  maui_adb="${maui_sdk_root}/platform-tools/adb"
  maui_emulator_bin="${maui_sdk_root}/emulator/emulator"
  maui_avdmanager_bin="${maui_sdk_root}/cmdline-tools/latest/bin/avdmanager"
  maui_avd_name="${MAUI_ANDROID_AVD:-maui-test}"
  "${maui_adb}" start-server > /dev/null 2>&1
}

# Finds a RUNNING emulator hosting our AVD (matched by console-reported AVD name); sets maui_serial.
maui_android_find_serial() {
  local serial state name
  maui_serial=""
  while read -r serial state; do
    [[ "${serial}" == emulator-* && "${state}" == device ]] || continue
    name="$("${maui_adb}" -s "${serial}" emu avd name 2> /dev/null | head -n1 | tr -d '\r')"
    if [[ "${name}" == "${maui_avd_name}" ]]; then
      maui_serial="${serial}"
      return 0
    fi
  done < <("${maui_adb}" devices | sed -n 's/^\(emulator-[0-9][0-9]*\)[[:space:]]\{1,\}\(.*\)$/\1 \2/p')
  return 1
}

# Boot-on-demand: reuse a running emulator for our AVD, else create the AVD (if missing) and boot it
# headless, detached, polling through sys.boot_completed. Sets maui_serial.
maui_android_ensure_booted() {
  maui_android_find_serial || true
  if [[ -z "${maui_serial}" ]]; then
    [[ -x "${maui_emulator_bin}" ]] || maui_die "emulator binary missing: ${maui_emulator_bin}"
    if ! "${maui_avdmanager_bin}" list avd -c 2> /dev/null | grep -qx "${maui_avd_name}"; then
      # The hardware-profile prompt is answered "no" (stock profile); the system image ships with
      # the SDK, so no sdkmanager --licenses step is needed here.
      echo "no" | "${maui_avdmanager_bin}" create avd --name "${maui_avd_name}" \
        --package "system-images;android-34;google_apis;arm64-v8a" --device pixel_5 > /dev/null 2>&1 ||
        maui_die "failed to create AVD '${maui_avd_name}' (check cmdline-tools + java)"
    fi
    # Detached + fully redirected: a background child sharing the caller's stdout would hold the
    # pipe open and stall gtest discovery long after the script exits.
    local boot_log="${TMPDIR:-/tmp}/maui-android-emu-${maui_avd_name}.log"
    nohup "${maui_emulator_bin}" -avd "${maui_avd_name}" -no-window -no-audio -no-boot-anim \
      < /dev/null > "${boot_log}" 2>&1 &
    disown
    local deadline=$((SECONDS + 300))
    while [[ -z "${maui_serial}" ]]; do
      ((SECONDS < deadline)) || maui_die "emulator for AVD '${maui_avd_name}' did not appear (log: ${boot_log})"
      sleep 2
      maui_android_find_serial || true
    done
  fi
  "${maui_adb}" -s "${maui_serial}" wait-for-device 2> /dev/null
  local boot_deadline=$((SECONDS + 300))
  until [[ "$("${maui_adb}" -s "${maui_serial}" shell getprop sys.boot_completed 2> /dev/null | tr -d '[:space:]')" == "1" ]]; do
    ((SECONDS < boot_deadline)) || maui_die "AVD '${maui_avd_name}' (${maui_serial}) did not finish booting"
    sleep 2
  done
}

# Keyed staging: push <host file> once per (path, fractional mtime, size) — rebuilds re-push,
# unchanged files don't, and concurrent ctest jobs never collide (push lands on a unique temp name,
# then an on-device mv publishes it atomically). Sets maui_staged_file (the device path) and
# maui_staged_key (the 16-hex key, reusable for sibling artifacts like the gtest json redirect).
maui_android_stage_file() {
  local host_file="$1"
  local file_stat
  file_stat="$(/usr/bin/stat -f '%Fm %z' "${host_file}")"
  maui_staged_key="$(/usr/bin/shasum -a 256 <<< "${host_file}|${file_stat}" | cut -c1-16)"
  maui_staged_file="${maui_device_dir}/${maui_staged_key}-$(basename "${host_file}")"
  if ! "${maui_adb}" -s "${maui_serial}" shell "test -e '${maui_staged_file}'" > /dev/null 2>&1; then
    "${maui_adb}" -s "${maui_serial}" shell "mkdir -p '${maui_device_dir}/tmp'" > /dev/null
    # Prune STALE builds of this same artifact before pushing the new one. Every rebuild mints a
    # fresh key (path|mtime|size), so without this each `maui_xaml_tests` etc. accumulates a full
    # copy on the device and /data fills up (observed: 4.3 GB of stale binaries → push fails with
    # no space, killing test discovery). The glob is exactly the 16-hex key + "-<basename>", so it
    # only ever matches prior builds of THIS file, never a different artifact (the "-" separator and
    # fixed key width prevent a basename-suffix collision). The current key does not exist yet (the
    # test above just failed), so nothing live is removed. `rm -f ... || true` keeps the no-match
    # case (mksh leaves the unexpanded glob) from failing under `set -e`.
    local base
    base="$(basename "${host_file}")"
    "${maui_adb}" -s "${maui_serial}" shell \
      "rm -f ${maui_device_dir}/????????????????-${base} 2>/dev/null || true" > /dev/null 2>&1
    "${maui_adb}" -s "${maui_serial}" push "${host_file}" "${maui_staged_file}.tmp.$$" > /dev/null 2>&1
    "${maui_adb}" -s "${maui_serial}" shell "chmod 755 '${maui_staged_file}.tmp.$$' && mv '${maui_staged_file}.tmp.$$' '${maui_staged_file}'" > /dev/null
  fi
}

# Single-quote an argument for the device's mksh (close-quote, escaped quote, reopen for embedded ').
maui_android_shell_quote() {
  local s="${1//\'/\'\\\'\'}"
  printf "'%s'" "${s}"
}

# ---- the soft keyboard is a SECOND foreign window --------------------------------------------------
# The Android IME is a DIFFERENT PROCESS, so `am force-stop` on the gallery does not take its window
# down and a foreground check still names our package as resumed. A page whose scenario focused a field
# leaves Gboard composited over the NEXT page's still, covering ~37% of the frame.
#
# Not hypothetical: capture_guard.py found 37 such frames already committed, across 14 pages, in
# alphabetically CONTIGUOUS runs (data_template_selector -> empty_view* -> filter_selection -> focus) —
# one stretch of one run, the same shape as the splash incident that guard was written for. On 5 of
# those pages only the PORT columns were contaminated while MAUI's was clean, which is most of what
# scored border_playground / focus / shadow_playground RED. All three columns leak, so this lives here
# rather than in any one capture script.
#
# Recovery is a full IME reset, not a keystroke: ESCAPE and CLOSE_SYSTEM_DIALOGS both leave it up, and
# keyevent BACK is banned in these lanes (it closes the Activity when no dialog is up). Force-stopping
# the IME package is the one action nothing can swallow; it restarts on demand at the next field tap.
#
# Returns 0 when the frame at $1 is clean (possibly after one re-shoot), 1 when it was DROPPED — a
# missing capture is a loud fixable gap, a keyboard banked as a render is a 37%-of-frame "defect".
maui_android_reshoot_without_keyboard() {
  local out="$1" activity="$2" pkg="$3" key="$4"
  local guard="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/parity/lib/capture_guard.py"
  [[ -f "${guard}" && -f "${out}" ]] || return 0
  python3 "${guard}" --keyboard --quiet "${out}" && return 0
  echo "[android] ~ ${key}: soft keyboard over the page — resetting the IME and re-shooting" >&2
  local ime
  ime="$("${maui_adb}" -s "${maui_serial}" shell settings get secure default_input_method 2>/dev/null \
        | cut -d/ -f1 | tr -d '\r')"
  [[ -n "${ime}" ]] && "${maui_adb}" -s "${maui_serial}" shell am force-stop "${ime}" > /dev/null 2>&1
  "${maui_adb}" -s "${maui_serial}" shell am force-stop "${pkg}" > /dev/null 2>&1 || true
  sleep 1
  "${maui_adb}" -s "${maui_serial}" shell am start -W -n "${activity}" \
    --es MAUI_SAMPLE_PAGE "${key}" > /dev/null 2>&1 || true
  sleep 4
  "${maui_adb}" -s "${maui_serial}" exec-out screencap -p > "${out}"
  python3 "${guard}" --keyboard --quiet "${out}" && return 0
  rm -f "${out}"
  echo "[android] !! ${key}: keyboard survived the IME reset — frame DROPPED" >&2
  return 1
}

# Short alias the capture scripts call (they already source this file).
reshoot_without_keyboard() { maui_android_reshoot_without_keyboard "$@"; }

# ---- a hung launch must cost ONE frame, not the whole pass -------------------------------------------
# `am start -W` blocks until the app's first frame. When the emulator degrades during a long run it can
# block FOREVER, and recapture.py's per-step watchdog then kills the entire lane pass. Measured
# 2026-08-17: the android cpp and cpp_xaml DARK passes both died this way at `empty_view` (900s, no
# output), taking every page after it with them; the partial result scored the board 123g/25r -> 106g/36r
# and had to be reverted. Relaunching that same page by hand right afterwards returned in 2.0s, so this
# is emulator flakiness, not a page that cannot start.
#
# macOS has no coreutils `timeout`, so the bound is a background wait loop. On expiry the launch is
# killed and 1 is returned; the caller drops the frame and moves to the next key, which is the same
# degradation the foreground and keyboard guards already use.
maui_android_start_bounded() {
  local activity="$1" extra_key="$2" key="$3" ceiling="${4:-90}"
  "${maui_adb}" -s "${maui_serial}" shell am start -W -n "${activity}" \
    --es "${extra_key}" "${key}" > /dev/null 2>&1 &
  local pid=$! waited=0
  while kill -0 "${pid}" 2>/dev/null; do
    sleep 1
    waited=$((waited + 1))
    if [[ "${waited}" -ge "${ceiling}" ]]; then
      kill "${pid}" 2>/dev/null || true
      wait "${pid}" 2>/dev/null || true
      echo "[android] !! ${key}: am start -W did not return in ${ceiling}s — frame DROPPED, pass continues" >&2
      return 1
    fi
  done
  wait "${pid}" 2>/dev/null || true
  return 0
}
