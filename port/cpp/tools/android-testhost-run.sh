#!/usr/bin/env bash
# The Android WIDGET TEST-HOST lane (M-android milestone 3): runs a native gtest suite INSIDE an
# app_process-spawned ART runtime on the emulator, where the tests hold a usable themed
# android.content.Context and drive REAL android.widget views over JNI — the monkey/uiautomator
# pattern. No APK, no gradle, no installation.
#
# DECISION (proven on the emulator, API 34 google_apis arm64-v8a): strategy (i) — app_process +
# ActivityThread.systemMain() — WON. testhost/Bootstrap.java prepares the main Looper, preloads the
# system font map (the two steps Zygote/SystemServer normally perform; skipping either NPEs widget
# construction), mints the system Context, wraps it in a ContextThemeWrapper so widget default
# styles resolve, System.load()s the staged test library, and hands Context + argv across JNI to
# gtest (testhost/test_host.cpp). The fallback (ii) — a minimal aapt2/d8/apksigner gtest-runner APK
# — was NOT needed: android.widget.TextView/Button construct and round-trip text correctly here.
#
#   usage: android-testhost-run.sh <native test .so> <Bootstrap.java> [gtest args...]
#
# Pipeline: javac compiles Bootstrap.java against the SDK's android.jar -> d8 (build-tools) dexes it
# (both cached beside the .so, keyed on the source mtime) -> dex + .so are staged onto the device
# through the shared keyed staging -> `CLASSPATH=<dex> app_process /system/bin
# dev.mauicpp.testhost.Bootstrap <staged .so> <gtest args...>` -> RUN_ALL_TESTS()'s exit code flows
# back through adb shell-v2 (System.exit). A `--gtest_output=json:<host path>` argument is
# redirected to a device path and pulled back afterwards, exactly like android-emu-run.sh. The
# SDK-resolve/AVD-boot/keyed-staging machinery is shared via android-emu-lib.sh (see its header).
set -euo pipefail

if [[ $# -lt 2 ]]; then
  echo "usage: android-testhost-run.sh <native test .so> <Bootstrap.java> [gtest args...]" >&2
  exit 64
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=android-emu-lib.sh
source "${script_dir}/android-emu-lib.sh"

test_so="$1"
bootstrap_java="$2"
shift 2
[[ -f "${test_so}" ]] || maui_die "no such test library: ${test_so}"
[[ -f "${bootstrap_java}" ]] || maui_die "no such bootstrap source: ${bootstrap_java}"

maui_android_resolve_tools

# ---- Host-side dex build (cached): javac against android.jar, then d8 ----
# javac comes from the system JDK (the macOS /usr/bin/javac stub dispatches to the installed JDK —
# the same one avdmanager already requires); android.jar and d8 come from the pinned SDK, with the
# build-tools version picked as the highest installed.
javac_bin="/usr/bin/javac"
android_jar="${maui_sdk_root}/platforms/android-34/android.jar"
[[ -f "${android_jar}" ]] || maui_die "missing ${android_jar} (install platforms;android-34)"
d8_bin=""
for candidate in "${maui_sdk_root}"/build-tools/*/d8; do
  [[ -x "${candidate}" ]] && d8_bin="${candidate}"
done
[[ -n "${d8_bin}" ]] || maui_die "no d8 found under ${maui_sdk_root}/build-tools"

# The compiled sources: the bootstrap itself plus the backend's runtime Java support classes
# (src/platform/android/java — the dev.mauicpp.* trampolines the JNI handler partials construct,
# e.g. NativeOnClickListener; the port's twin of C#'s src/Core/AndroidNative maui Java library).
# The layout coupling (../java relative to the bootstrap) is deliberate: the test host must dex
# exactly what a real app host would have to ship.
runtime_java_dir="$(cd "$(dirname "${bootstrap_java}")/.." && pwd)/java"
java_sources=("${bootstrap_java}")
if [[ -d "${runtime_java_dir}" ]]; then
  for java_source in "${runtime_java_dir}"/*.java; do
    [[ -f "${java_source}" ]] && java_sources+=("${java_source}")
  done
fi

dex_dir="$(dirname "${test_so}")/android-testhost"
dex_file="${dex_dir}/classes.dex"
dex_stale=0
if [[ ! -f "${dex_file}" ]]; then
  dex_stale=1
else
  for java_source in "${java_sources[@]}"; do
    [[ "${java_source}" -nt "${dex_file}" ]] && dex_stale=1
  done
fi
if [[ "${dex_stale}" -eq 1 ]]; then
  rm -rf "${dex_dir}/classes"
  mkdir -p "${dex_dir}/classes"
  "${javac_bin}" --release 17 -classpath "${android_jar}" -d "${dex_dir}/classes" "${java_sources[@]}" >&2
  # Every emitted .class anywhere under classes/ (the runtime classes live outside the testhost
  # package, and a future lambda/inner class would otherwise silently miss the dex and
  # NoClassDefFoundError on the device).
  class_files=()
  while IFS= read -r -d '' class_file; do
    class_files+=("${class_file}")
  done < <(find "${dex_dir}/classes" -name '*.class' -print0)
  [[ "${#class_files[@]}" -gt 0 ]] || maui_die "javac emitted no classes under ${dex_dir}/classes"
  "${d8_bin}" --release --lib "${android_jar}" --min-api 34 --output "${dex_dir}" \
    "${class_files[@]}" >&2
  [[ -f "${dex_file}" ]] || maui_die "d8 produced no classes.dex in ${dex_dir}"
fi

# ---- Stage onto the device ----
maui_android_ensure_booted
maui_android_stage_file "${dex_file}"
staged_dex="${maui_staged_file}"
maui_android_stage_file "${test_so}"
staged_so="${maui_staged_file}"

# ---- Redirect a --gtest_output=json:<host path> to a device path (pulled back below) ----
args=()
json_host=""
json_device=""
for arg in ${@+"$@"}; do
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

# TMPDIR/HOME point at the staging dir like android-emu-run.sh (Android has no /tmp), so widget
# tests that touch std::filesystem::temp_directory_path keep working inside the host too.
cmd="cd '${maui_device_dir}' && TMPDIR='${maui_device_dir}/tmp' HOME='${maui_device_dir}' \
CLASSPATH='${staged_dex}' app_process /system/bin dev.mauicpp.testhost.Bootstrap \
$(maui_android_shell_quote "${staged_so}")"
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
