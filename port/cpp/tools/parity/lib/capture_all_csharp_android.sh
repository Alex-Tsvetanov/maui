#!/usr/bin/env bash
# Capture every MauiReference (REAL .NET MAUI, the canonical shared-XAML ground truth per port/CLAUDE.md
# ruling 6) gallery page on the `maui-test` Android emulator into
# docs/comparison/captures/android/maui/<key>_light.png — the ground-truth MAUI reference column (twin
# of the iOS capture_all_csharp.py and the C++ column's build_android_apphost.sh; the canonical layout
# build_comparison_json.py + gen_readme.py read). Android has no dark capture, hence the fixed _light suffix.
#
# MIGRATED 2026-07-19 from the superseded ~/maui-compare app (com.companyname.mauicompare) to the in-repo
# port/maui-reference/app (MauiReference, dev.mauicpp.mauireference) — the same app iOS/maccatalyst already
# use, which renders the CANONICAL shared XAML (port/maui-reference/pages/*.xaml, the exact bytes gallery_xaml
# #embeds). The old maui-compare rendered its own hand-written C# pages, which had DIVERGED from the shared
# XAML after the P2 conversion (22e594be2e), so the Android maui column was stale-vs-the-other-platforms.
# MauiReference reads the SAME MAUI_COMPARE_PAGE intent extra (App.xaml.cs ResolveValue), so only the package
# name changed. On Android the page key is passed as an INTENT EXTRA (--es MAUI_COMPARE_PAGE <key>) because
# `am start` does NOT propagate process env vars; App.xaml.cs reads Platform.CurrentActivity.Intent on ANDROID
# and falls back to the env var on iOS/maccatalyst (unchanged there).
#
# BUILD + INSTALL (do this before running, from port/maui-reference/app — assemblies must be EMBEDDED in the
# APK, i.e. NOT Fast Deployment, so a plain `adb install` is self-contained):
#   export PATH=/opt/homebrew/bin:$PATH                       # the workload-bearing (Homebrew) dotnet
#   export ANDROID_HOME=/opt/homebrew/share/android-commandlinetools
#   export JAVA_HOME=/opt/homebrew/Cellar/openjdk@17/17.0.19/libexec/openjdk.jdk/Contents/Home
#   dotnet build -f net10.0-android -c Debug -p:EmbedAssembliesIntoApk=true -p:AndroidFastDeploymentType=
#   adb -s emulator-5554 install -r \
#     bin/Debug/net10.0-android/dev.mauicpp.mauireference-Signed.apk
# (This script only DRIVES an already-installed app; it does not build.)
#
# Deterministic per-page capture (mirrors build_android_apphost.sh's wave-15 fix, NOT a blind sleep):
# force-stop + WAIT for the process (and its on-screen frame) to be GONE, clear logcat, `am start -W`
# (blocks to first frame) then poll for THIS launch's `Displayed` marker OR our Activity being the
# resumed/top one, dismiss any transient ANR dialog, short settle, then `exec-out screencap`.
#
# Usage:  capture_all_csharp_android.sh [key ...]     # default: every key in page_keys.txt
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cpp_root="$(cd "${script_dir}/../../.." && pwd)"

# --- resolve adb + serial via the shared emu lib (same machinery the C++ android tools use) ---
# shellcheck source=/dev/null
source "${cpp_root}/tools/android-emu-lib.sh"
maui_android_resolve_tools
maui_android_ensure_booted   # reuse the running maui-test emulator, or boot it

pkg="dev.mauicpp.mauireference"
# MAUI mangles the Activity name; resolve it from the launcher intent rather than hard-coding the crc.
activity="$("${maui_adb}" -s "${maui_serial}" shell cmd package resolve-activity \
  -c android.intent.category.LAUNCHER "${pkg}" 2>/dev/null \
  | sed -n 's/^[[:space:]]*name=\(.*\)$/\1/p' | head -n1 | tr -d '\r')"
[[ -n "${activity}" ]] || maui_die "could not resolve ${pkg} launcher Activity (is the app installed?)"
component="${pkg}/${activity}"
echo "[csharp-android] component: ${component}" >&2

out_dir="${cpp_root}/docs/comparison/captures/android/maui"
mkdir -p "${out_dir}"

# Theme: the DEVICE's night mode drives both columns. MAUI_APPEARANCE here selects which system theme the
# pass runs under (and the output suffix) — it is no longer handed to the app. App.xaml.cs leaves
# UserAppTheme Unspecified unless a MAUI_THEME extra is present, so MauiReference follows the device.
appearance="${MAUI_APPEARANCE:-light}"
# Pin the emulator's chrome BEFORE any capture: Android screencaps are full-screen, so the status bar
# clock/battery/signal and any notification icon land inside every frame and would score as a per-page
# diff the port never caused. Restored by the trap below. See tools/parity/device_state.py.
python3 "${script_dir}/device_state.py" --android >&2 || true
# SYSTEM NIGHT MODE for the dark pass. MauiReference's UserAppTheme=Dark (set from the MAUI_THEME intent
# extra in App.xaml.cs CreateWindow) DOES NOT WORK on Android — measured, with the extra provably
# arriving: body mean 137.7 light vs 139.3 "dark", i.e. no theme change at all, while the C++ columns go
# 136.1 -> 81.1. The device uiMode does work: with `cmd uimode night yes` the same page renders 84.6.
# So Android dark for the MAUI column comes from the DEVICE, not the app.
#
# Safe to change for this column alone, verified rather than assumed: the port's status bar is
# BYTE-IDENTICAL under system-light and system-night (0 differing pixels over the strip), because the app
# renders that region itself. So flipping night mode does not invalidate the already-captured cpp/xaml
# columns, and only the MAUI column needs recapturing.
#
# Restored by the trap below alongside demo mode — an emulator left in night mode would silently darken
# the next LIGHT pass and read as a port regression.
# Record the device's CURRENT night mode so the trap restores what we found rather than forcing light —
# an emulator that was already dark would otherwise be silently flipped by running a capture.
maui_night_before="$("${maui_adb}" -s "${maui_serial}" shell cmd uimode night 2>/dev/null | grep -qi yes && echo yes || echo no)"
"${maui_adb}" -s "${maui_serial}" shell cmd uimode night \
  "$([[ "${MAUI_APPEARANCE:-light}" == "dark" ]] && echo yes || echo no)" > /dev/null 2>&1 || true
sleep 2
trap '"${maui_adb}" -s "${maui_serial}" shell cmd uimode night "${maui_night_before}" > /dev/null 2>&1 || true;
      python3 "${script_dir}/device_state.py" --android --clear >&2 || true' EXIT
[[ "${appearance}" == "dark" || "${appearance}" == "light" ]] || maui_die "MAUI_APPEARANCE must be light|dark"
suffix="_${appearance}"
# NO MAUI_THEME EXTRA. It used to pass `--es MAUI_THEME Dark` on the dark pass, which sets UserAppTheme
# and therefore OVERRIDES the device theme — so the reference column proved only that the override works,
# never that MAUI follows the system. App.xaml.cs now leaves UserAppTheme Unspecified when the extra is
# absent, so the device's `cmd uimode night` (set just above, restored by the trap) is the single source
# for BOTH columns. Set the extra by hand for a targeted one-off; the board does not.
theme_extra=()

wait_process_gone() {
  for _ in $(seq 1 40); do # ~10s ceiling
    local pid
    pid="$("${maui_adb}" -s "${maui_serial}" shell pidof "${pkg}" 2> /dev/null | tr -d '[:space:]')"
    [[ -z "${pid}" ]] && return 0
    sleep 0.25
  done
  return 1
}

wait_displayed() {
  for _ in $(seq 1 60); do # ~15s ceiling
    if "${maui_adb}" -s "${maui_serial}" logcat -d 2> /dev/null \
      | grep -q "Displayed ${pkg}/"; then
      return 0
    fi
    if "${maui_adb}" -s "${maui_serial}" shell dumpsys activity activities 2> /dev/null \
      | grep -qE "ResumedActivity[^\n]*${pkg}/"; then
      return 0
    fi
    sleep 0.25
  done
  return 1
}

capture_one() {
  local key="$1"
  local _parity_t0=${SECONDS}
  echo "@@PARITY BEGIN ${key} maui ${appearance}"
  # (a) Kill the prior instance and WAIT for its frame to be gone so screencap can't grab the previous page.
  "${maui_adb}" -s "${maui_serial}" shell am force-stop "${pkg}" > /dev/null 2>&1 || true
  wait_process_gone || echo "[csharp-android] WARNING: ${pkg} still alive after force-stop (${key})" >&2
  # (b) Clear logcat so the next Displayed line is unambiguously from THIS launch.
  "${maui_adb}" -s "${maui_serial}" logcat -c > /dev/null 2>&1 || true
  # (c) Launch this page (intent extra) with -W (blocks to first frame), then poll the readiness barrier.
  # Bounded: a hung launch drops ONE frame instead of killing the whole pass
  # (android-emu-lib.sh; the 2026-08-17 empty_view stall cost both dark passes).
  if ! maui_android_start_bounded "${component}" "MAUI_COMPARE_PAGE" "${key}"; then
    echo "@@PARITY END ${key} maui ${appearance} $((SECONDS - _parity_t0))"
    return 0
  fi
  wait_displayed || echo "[csharp-android] WARNING: never saw first-frame for ${key}; capturing anyway" >&2
  # Dismiss a transient "isn't responding" ANR dialog if the load burst raised one.
  "${maui_adb}" -s "${maui_serial}" shell am broadcast -a android.intent.action.CLOSE_SYSTEM_DIALOGS > /dev/null 2>&1 || true
  # Settle: the window's first frame can precede the maui tree's content draw by a frame or two.
  # 4s and IDENTICAL to the C++/XAML columns' settle so all three are photographed in the same state of
  # Android's FADING SCROLLBARS — see the long note in build_android_apphost.sh. (Measured here: MAUI's
  # scrollbar is still faintly visible at this step's old 1.5s and completely gone by 4s.)
  # WEB PAGES NEED LONGER, AND THE EXTRA IS NOT ABOUT THE NETWORK. MauiWebView.LoadUrl awaits
  # EnsureCoreWebView2Async() -- which SPAWNS THE BROWSER PROCESS -- before it assigns Source, so a
  # 4s settle races BROWSER INIT, not the fetch. MEASURED on the 2026-08-18 android recapture:
  # context_flyout's MAUI column came back with a BLANK WebView band (466 unique colours) while the
  # cpp and xaml columns rendered example.com in full (777 each), same page, same pass. As on the VM
  # lanes (scenarios/web_view.toml), it is MAUI'S OWN column that loses the race -- a ground truth
  # that alternates blank/rendered cannot be matched by any port change.
  # 9s = this lane's 4s base + the 5s scenarios/web_view.toml measured for WebView2 init. INHERITED,
  # not measured here: CONFIRM on the next pass by comparing the maui column's unique-colour count
  # against the two port columns, and raise it before looking for a port-side cause.
  # THE LIST AND THE VALUES MUST STAY IDENTICAL IN ALL THREE COLUMN SCRIPTS (this file,
  # build_android_apphost_xaml.sh, capture_all_csharp_android.sh) -- an asymmetric settle photographs
  # the columns in different states and silently invalidates every cell on these pages.
  case "${key}" in web_view|hybrid_web_view|context_flyout|image) sleep 9 ;; *) sleep 4 ;; esac
  "${maui_adb}" -s "${maui_serial}" exec-out screencap -p > "${out_dir}/${key}${suffix}.png"
  # The IME is a second foreign window no resumed-activity check can see (android-emu-lib.sh). THE
  # GROUND TRUTH LEAKS TOO — 9 committed frames in this column — and a keyboard in the reference is
  # worse than one in a port column: it scores as a port defect on a page the port rendered correctly.
  if ! reshoot_without_keyboard "${out_dir}/${key}${suffix}.png" "${component}" "${pkg}" "${key}" MAUI_COMPARE_PAGE; then
    echo "@@PARITY END ${key} maui ${appearance} $((SECONDS - _parity_t0))"
    return 0
  fi
  echo "@@PARITY END ${key} maui ${appearance} $((SECONDS - _parity_t0))"
  echo "[csharp-android] wrote ${out_dir}/${key}${suffix}.png ($(stat -f%z "${out_dir}/${key}${suffix}.png" 2>/dev/null || echo 0)B)" >&2
}

# Post-install / first-run warm-up: absorb the cold-start + JIT churn on a throwaway launch.
echo "[csharp-android] warm-up launch..." >&2
"${maui_adb}" -s "${maui_serial}" shell am start -W -n "${component}" --es MAUI_COMPARE_PAGE "button" > /dev/null 2>&1 || true
sleep 2
"${maui_adb}" -s "${maui_serial}" shell am force-stop "${pkg}" > /dev/null 2>&1 || true

# Key set: explicit args, else every key in page_keys.txt (the canonical 172).
declare -a keys=()
if [[ "$#" -gt 0 ]]; then
  keys=("$@")
else
  page_keys_file="${script_dir}/page_keys.txt"
  [[ -f "${page_keys_file}" ]] || maui_die "missing ${page_keys_file}"
  while IFS= read -r line; do
    [[ -n "${line}" ]] && keys+=("${line}")
  done < "${page_keys_file}"
fi

echo "[csharp-android] capturing ${#keys[@]} page(s) -> ${out_dir}" >&2
n=0
for key in "${keys[@]}"; do
  n=$((n + 1))
  echo "[csharp-android] [${n}/${#keys[@]}] ${key}" >&2
  capture_one "${key}"
done
echo "[csharp-android] CSHARP_ANDROID_CAPTURE_ALL_DONE (${#keys[@]} pages)" >&2
