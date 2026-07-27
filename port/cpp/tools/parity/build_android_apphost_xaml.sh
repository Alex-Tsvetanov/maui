#!/usr/bin/env bash
# build_android_apphost_xaml.sh — the C++&XAML twin of build_android_apphost.sh. Packages the gallery_xaml
# twin (every page authored as Views/<name>.xaml markup, built at COMPILE TIME) into a signed APK with the
# in-SDK build-tools (NO gradle), installs it on the emulator, and `adb exec-out screencap`s one (or every)
# gallery page for the Android C++&XAML parity column.
#
# It is byte-for-byte the C++ apphost pipeline (javac→d8, aapt2 link, zip-assemble, zipalign, apksigner,
# adb install, deterministic per-key capture) with FOUR swaps:
#   1. build target / .so     : maui_android_apphost_xaml → libmaui_android_apphost_xaml.so
#   2. apphost dir            : examples/gallery_xaml/apphost (its own Activity + manifest)
#   3. package id / activity  : dev.mauicpp.apphost.xaml/.MauiHostActivity (coexists with the C++ host)
#   4. out dir                : docs/comparison/captures/android/xaml/<key>_<theme>.png (the C++&XAML column)
#
# The bytes-mode .xaml.cpp TUs the .so compiles are generated at CMake CONFIGURE time (port/tools/e2e/
# e2e.py gen --embed-mode=bytes) — see the maui_android_apphost_xaml target in CMakeLists.txt. The NDK's
# Clang 18 has no #embed, so the committed #embed TUs can't build there; the byte-literal form can.
#
# Usage:
#   build_android_apphost_xaml.sh                       # build+install, then capture EVERY page in page_keys.txt
#   build_android_apphost_xaml.sh <page_key> [<key>...]  # capture just these keys (build+install once)
#   build_android_apphost_xaml.sh --no-capture           # build+install+launch the default page only (smoke)
#   MAUI_APPEARANCE=dark build_android_apphost_xaml.sh    # dark-theme shots
#
# Output: docs/comparison/captures/android/xaml/<key>_<theme>.png — the canonical C++&XAML Android
# column the build_comparison_json.py + gen_readme.py layout reads (theme always suffixed; Android is
# captured single-theme=light on the board). --dry-run prints the paths without building/capturing.
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cpp_root="$(cd "${script_dir}/../.." && pwd)" # port/cpp
# shellcheck source=../android-emu-lib.sh
source "${cpp_root}/tools/android-emu-lib.sh"
# AndroidX AppCompat + Google Material linking, shared with build_android_apphost.sh.
# shellcheck source=android-aar-lib.sh
source "${script_dir}/android-aar-lib.sh"

# ---- args ----
do_capture=1
dry_run=0
declare -a requested_keys=()
for arg in "$@"; do
  case "${arg}" in
  --no-capture) do_capture=0 ;;
  --dry-run) dry_run=1 ;;
  --*) maui_die "unknown flag: ${arg}" ;;
  *) requested_keys+=("${arg}") ;;
  esac
done

appearance="${MAUI_APPEARANCE:-light}"
[[ "${appearance}" == "dark" || "${appearance}" == "light" ]] || maui_die "MAUI_APPEARANCE must be light|dark"
# Canonical layout ALWAYS suffixes the theme: captures/android/xaml/<key>_<theme>.png.
suffix="_${appearance}"

# --dry-run: resolve the key set and print each canonical output path WITHOUT building/installing/
# capturing. The XAML column mirrors the gallery_xaml twin's OWN page set (Views/<name>.xaml), so the
# default key list is derived from the checked-in twins (matching the real run below), not page_keys.txt.
if [[ "${dry_run}" -eq 1 ]]; then
  out_dir_rel="docs/comparison/captures/android/xaml"
  declare -a dry_keys=()
  if [[ "${#requested_keys[@]}" -gt 0 ]]; then
    dry_keys=("${requested_keys[@]}")
  else
    views_dir="${cpp_root}/examples/gallery_xaml/Views"
    [[ -d "${views_dir}" ]] || maui_die "missing ${views_dir}"
    while IFS= read -r f; do dry_keys+=("$(basename "${f}" .xaml)"); done \
      < <(find "${views_dir}" -maxdepth 1 -name '*.xaml' ! -name '*.xaml.*' | sort)
  fi
  for key in "${dry_keys[@]}"; do
    echo "${out_dir_rel}/${key}${suffix}.png"
  done
  echo "DRY_RUN_DONE"
  exit 0
fi

# ---- 0. resolve the SDK + build-tools (highest installed) ----
maui_android_resolve_tools
android_jar="${maui_sdk_root}/platforms/android-34/android.jar"
[[ -f "${android_jar}" ]] || maui_die "missing ${android_jar} (install platforms;android-34)"
build_tools=""
for candidate in "${maui_sdk_root}"/build-tools/*; do
  [[ -x "${candidate}/aapt2" && -x "${candidate}/d8" ]] && build_tools="${candidate}"
done
[[ -n "${build_tools}" ]] || maui_die "no build-tools with aapt2+d8 under ${maui_sdk_root}/build-tools"
aapt2="${build_tools}/aapt2"
d8="${build_tools}/d8"
zipalign="${build_tools}/zipalign"
apksigner="${build_tools}/apksigner"
javac_bin="/usr/bin/javac"
keytool_bin="/usr/bin/keytool"
for tool in "${aapt2}" "${d8}" "${zipalign}" "${apksigner}"; do
  [[ -x "${tool}" ]] || maui_die "missing build-tool: ${tool}"
done

# The emulator AVD is arm64-v8a, so the .so must be arm64-v8a too.
abi="arm64-v8a"

# ---- 1. cmake-build the XAML app-host .so (android preset) ----
build_dir="${cpp_root}/build/android"
if [[ ! -f "${build_dir}/build.ninja" && ! -f "${build_dir}/Makefile" ]]; then
  echo "[apphost-xaml] configuring the android preset (first run)..." >&2
  ( cd "${cpp_root}" && cmake --preset android >&2 )
fi
echo "[apphost-xaml] building maui_android_apphost_xaml..." >&2
( cd "${cpp_root}" && cmake --build --preset android --target maui_android_apphost_xaml >&2 )
app_so=""
for candidate in \
  "${build_dir}/libmaui_android_apphost_xaml.so" \
  "${build_dir}"/**/libmaui_android_apphost_xaml.so; do
  [[ -f "${candidate}" ]] && app_so="${candidate}"
done
[[ -n "${app_so}" ]] || maui_die "could not find libmaui_android_apphost_xaml.so under ${build_dir}"
echo "[apphost-xaml] .so: ${app_so}" >&2

# Strip debug info from the .so before packaging. A -g build of the 181 embedded-XAML byte-array TUs
# produces a ~922 MB .so; its uncompressed native lib then fails to install on the emulator
# (INSTALL_FAILED_INSUFFICIENT_STORAGE / CONTAINER_ERROR res=-18). --strip-debug keeps the exported JNI
# symbols and shrinks it to ~130 MB, which installs cleanly. (cmake --build above relinks the full .so
# each run, so this strip must run every time, after the build and before the APK is assembled.)
strip_bin="$(find "${ANDROID_HOME:-/opt/homebrew/share/android-commandlinetools}/ndk" -path '*/bin/llvm-strip' 2>/dev/null | sort | tail -1)"
if [[ -n "${strip_bin}" ]]; then
  echo "[apphost-xaml] stripping .so (was $(du -h "${app_so}" | cut -f1))..." >&2
  "${strip_bin}" --strip-debug "${app_so}" >&2 || echo "[apphost-xaml] WARNING: strip failed; continuing unstripped" >&2
  echo "[apphost-xaml]   stripped .so: $(du -h "${app_so}" | cut -f1)" >&2
fi

# ---- 2. stage the AndroidX/Material AARs, then aapt2 compile + link -> base APK + the R classes ----
# ORDER MATTERS: aapt2 link EMITS the R.java per library package and javac must compile them, so linking
# runs BEFORE javac. Mirrors build_android_apphost.sh exactly; see tools/parity/android-aar-lib.sh.
apphost_dir="${cpp_root}/examples/gallery_xaml/apphost"
manifest="${apphost_dir}/AndroidManifest.xml"
activity_java="${apphost_dir}/MauiHostActivity.java"
runtime_java_dir="${cpp_root}/src/platform/android/java" # NativeOnClickListener.java etc. (shared)
[[ -f "${manifest}" && -f "${activity_java}" ]] || maui_die "missing XAML apphost manifest/Activity under ${apphost_dir}"

work="${build_dir}/apphost-xaml-apk"
rm -rf "${work}"
mkdir -p "${work}/classes" "${work}/gen" "${work}/lib/${abi}" "${work}/res/values"

maui_android_aar_prepare "${build_dir}/aardeps"

base_apk="${work}/base.apk"
echo "[apphost-xaml] aapt2 compile + link (${#maui_aar_jars[@]} AAR jars)..." >&2
cat > "${work}/res/values/strings.xml" <<'XML'
<?xml version="1.0" encoding="utf-8"?>
<resources>
    <string name="app_name">MAUI C++ Gallery (XAML)</string>
</resources>
XML
# The THEME is the shared, checked-in one — src/platform/android/apphost/res/values/styles.xml (MAUI's own
# Maui.MainTheme reproduced on the real Theme.MaterialComponents.DayNight). It is COPIED rather than
# re-declared inline so the two hosts can never drift apart: the XAML manifest references
# @style/MauiAppHost.Theme and both columns must resolve the SAME widget defaults, or every field-bearing
# page shifts between them.
cp "${cpp_root}/src/platform/android/apphost/res/values/styles.xml" "${work}/res/values/styles.xml"
"${aapt2}" compile --dir "${work}/res" -o "${work}/res-compiled.zip" >&2 || maui_die "aapt2 compile failed"
"${aapt2}" link \
  -o "${base_apk}" \
  -I "${android_jar}" \
  --manifest "${manifest}" \
  --min-sdk-version 24 --target-sdk-version 34 \
  --auto-add-overlay \
  --java "${work}/gen" \
  "${maui_aar_link[@]}" \
  -R "${work}/res-compiled.zip" >&2 || maui_die "aapt2 link failed"
[[ -f "${base_apk}" ]] || maui_die "aapt2 link produced no base APK"

# ---- 3. javac + d8: the Activity + runtime java + the generated R classes + every library jar ----
java_sources=("${activity_java}")
if [[ -d "${runtime_java_dir}" ]]; then
  for j in "${runtime_java_dir}"/*.java; do
    [[ -f "${j}" ]] && java_sources+=("${j}")
  done
fi
while IFS= read -r -d '' f; do java_sources+=("${f}"); done \
  < <(find "${work}/gen" -name 'R.java' -print0)
aar_cp="$(IFS=:; echo "${maui_aar_jars[*]}")"
echo "[apphost-xaml] javac (${#java_sources[@]} sources) + d8..." >&2
"${javac_bin}" --release 17 -classpath "${android_jar}:${aar_cp}" -d "${work}/classes" "${java_sources[@]}" >&2
class_files=()
while IFS= read -r -d '' f; do class_files+=("${f}"); done \
  < <(find "${work}/classes" -name '*.class' -print0)
[[ "${#class_files[@]}" -gt 0 ]] || maui_die "javac emitted no .class files"
"${d8}" --release --lib "${android_jar}" --min-api 24 --output "${work}" \
  "${class_files[@]}" "${maui_aar_jars[@]}" >&2
[[ -f "${work}/classes.dex" ]] || maui_die "d8 produced no classes.dex"

# ---- 4. assemble: add classes.dex (root) + lib/<abi>/<so> + assets/* into the APK ----
cp "${app_so}" "${work}/lib/${abi}/libmaui_android_apphost_xaml.so"

# Package the gallery image/font resources into assets/ (the XAML twins name them by bare filename, resolved
# against the Context AssetManager — the SELinux-/uid-/reinstall-immune mechanism; same as the C++ host).
gallery_res_dir="${cpp_root}/examples/gallery/resources"
mkdir -p "${work}/assets"
asset_count=0
if [[ -d "${gallery_res_dir}" ]]; then
  for f in "${gallery_res_dir}"/*; do
    name="$(basename "${f}")"
    [[ -f "${f}" && "${name}" != "README.md" ]] || continue
    cp "${f}" "${work}/assets/${name}"
    asset_count=$((asset_count + 1))
  done
fi
echo "[apphost-xaml] staged ${asset_count} gallery asset(s) into assets/ (from ${gallery_res_dir})" >&2
[[ "${asset_count}" -gt 0 ]] || echo "[apphost-xaml] WARNING: no gallery assets found — image pages will render blank" >&2

# Also stage the WebView welcome page. The shared web_view.xaml uses Source="welcome.html", a relative
# source the android WebView handler rebases to file:///android_asset/welcome.html — so the canonical
# Resources/Raw asset must land at assets/welcome.html (MAUI's own build packages Resources/Raw the same
# way; this mirrors it for the gallery_xaml apphost). Single source of truth: the maui-reference Raw file.
welcome_html="${cpp_root}/../maui-reference/app/Resources/Raw/welcome.html"
if [[ -f "${welcome_html}" ]]; then
  cp "${welcome_html}" "${work}/assets/welcome.html"
  asset_count=$((asset_count + 1))
  echo "[apphost-xaml] staged welcome.html into assets/ (from ${welcome_html})" >&2
else
  echo "[apphost-xaml] WARNING: ${welcome_html} missing — web_view.xaml Source=welcome.html will 404" >&2
fi

echo "[apphost-xaml] adding classes.dex + lib/${abi}/*.so + assets/*..." >&2
unaligned_apk="${work}/app-unaligned.apk"
cp "${base_apk}" "${unaligned_apk}"
( cd "${work}" && zip -X "${unaligned_apk}" classes*.dex >&2 \
    && zip -X -0 "${unaligned_apk}" "lib/${abi}/libmaui_android_apphost_xaml.so" >&2 \
    && { [[ "${asset_count}" -eq 0 ]] || zip -X -r "${unaligned_apk}" assets >&2 ; } ) \
  || maui_die "zip-assembling the APK failed"

# ---- 5. zipalign + apksigner (throwaway debug keystore) ----
aligned_apk="${work}/app-aligned.apk"
"${zipalign}" -f -p 4 "${unaligned_apk}" "${aligned_apk}" >&2
keystore="${build_dir}/apphost-debug.keystore" # reuse the C++ host's debug keystore (same throwaway key)
if [[ ! -f "${keystore}" ]]; then
  echo "[apphost-xaml] creating a throwaway debug keystore..." >&2
  "${keytool_bin}" -genkeypair -v -keystore "${keystore}" -storepass android -keypass android \
    -alias androiddebugkey -keyalg RSA -keysize 2048 -validity 10000 \
    -dname "CN=Maui CPP Debug,O=maui-cpp,C=US" >&2
fi
signed_apk="${build_dir}/maui_android_apphost_xaml.apk"
echo "[apphost-xaml] apksigner sign..." >&2
"${apksigner}" sign --ks "${keystore}" --ks-pass pass:android --key-pass pass:android \
  --out "${signed_apk}" "${aligned_apk}" >&2
"${apksigner}" verify "${signed_apk}" >&2 || maui_die "apksigner verify failed on ${signed_apk}"
echo "[apphost-xaml] APK: ${signed_apk}" >&2

# ---- 6. install on the emulator ----
maui_android_ensure_booted
pkg="dev.mauicpp.apphost.xaml"
activity="${pkg}/.MauiHostActivity"
echo "[apphost-xaml] adb install -r..." >&2
"${maui_adb}" -s "${maui_serial}" install -r "${signed_apk}" >&2

# Post-install warm-up launch: absorb the first-COLD dexopt churn so the first REAL capture isn't the one
# that eats it (the wave-15 deterministic-capture discipline).
echo "[apphost-xaml] post-install warm-up launch..." >&2
"${maui_adb}" -s "${maui_serial}" shell am start -W -n "${activity}" \
  --es MAUI_SAMPLE_PAGE "value_controls" --es MAUI_APPEARANCE "${appearance}" > /dev/null 2>&1 || true
sleep 2
"${maui_adb}" -s "${maui_serial}" shell am force-stop "${pkg}" > /dev/null 2>&1 || true

out_dir="${cpp_root}/docs/comparison/captures/android/xaml"
mkdir -p "${out_dir}"

# Wait until our process is actually GONE (not just asked to stop) — am force-stop is async; a screencap
# before the old frame tears down grabs the PREVIOUS page (the wave-15 stale-frame race).
wait_process_gone() {
  for _ in $(seq 1 40); do # ~10s ceiling (40 * 0.25s)
    local pid
    pid="$("${maui_adb}" -s "${maui_serial}" shell pidof "${pkg}" 2> /dev/null | tr -d '[:space:]')"
    [[ -z "${pid}" ]] && return 0
    sleep 0.25
  done
  return 1
}

# Wait for THIS launch's Activity to have DRAWN, via the system's own first-frame signal (Displayed log
# line) with a level-triggered resumed-activity fallback. Race-free per-launch readiness barrier.
wait_displayed() {
  for _ in $(seq 1 60); do # ~15s ceiling (60 * 0.25s)
    if "${maui_adb}" -s "${maui_serial}" logcat -d 2> /dev/null \
      | grep -q "Displayed ${pkg}/.MauiHostActivity"; then
      return 0
    fi
    if "${maui_adb}" -s "${maui_serial}" shell dumpsys activity activities 2> /dev/null \
      | grep -qE "(topResumedActivity|ResumedActivity)[^\n]*${pkg}/.MauiHostActivity"; then
      return 0
    fi
    sleep 0.25
  done
  return 1
}

capture_one() {
  local key="$1"
  echo "[apphost-xaml] launch ${key} (${appearance})..." >&2
  # (a) Kill the prior instance and WAIT for its process/frame to be gone.
  "${maui_adb}" -s "${maui_serial}" shell am force-stop "${pkg}" > /dev/null 2>&1 || true
  wait_process_gone || echo "[apphost-xaml] WARNING: ${pkg} still alive after force-stop (${key})" >&2
  # (b) Clear logcat so the next Displayed line is unambiguously from THIS launch.
  "${maui_adb}" -s "${maui_serial}" logcat -c > /dev/null 2>&1 || true
  # (c) Launch with -W and poll for this launch's Displayed marker.
  "${maui_adb}" -s "${maui_serial}" shell am start -W -n "${activity}" \
    --es MAUI_SAMPLE_PAGE "${key}" --es MAUI_APPEARANCE "${appearance}" > /dev/null
  wait_displayed || echo "[apphost-xaml] WARNING: never saw Displayed for ${key}; capturing anyway" >&2
  # Dismiss any transient ANR dialog via CLOSE_SYSTEM_DIALOGS (NOT keyevent BACK — BACK would close us).
  "${maui_adb}" -s "${maui_serial}" shell am broadcast -a android.intent.action.CLOSE_SYSTEM_DIALOGS > /dev/null 2>&1 || true
  # A settle after the first-frame barrier (the maui tree's content draw can trail it a frame or two).
  # 4s and IDENTICAL to the other two columns' settle so all three are photographed in the same state of
  # Android's FADING SCROLLBARS — see the long note in build_android_apphost.sh.
  sleep 4
  "${maui_adb}" -s "${maui_serial}" exec-out screencap -p > "${out_dir}/${key}${suffix}.png"
  echo "[apphost-xaml] wrote ${out_dir}/${key}${suffix}.png" >&2
}

if [[ "${do_capture}" -eq 0 ]]; then
  echo "[apphost-xaml] --no-capture: launching the default page only (smoke test)" >&2
  "${maui_adb}" -s "${maui_serial}" shell am start -n "${activity}" --es MAUI_SAMPLE_PAGE "value_controls" > /dev/null
  echo "[apphost-xaml] launched ${activity} with page=value_controls" >&2
  exit 0
fi

# Pick the key set: the explicit args, else every key the gallery_xaml twin covers (its Views/*.xaml stems).
declare -a keys=()
if [[ "${#requested_keys[@]}" -gt 0 ]]; then
  keys=("${requested_keys[@]}")
else
  # The gallery_xaml column mirrors the XAML page set: the canonical shared pages
  # (port/maui-reference/pages/*.xaml) plus any not-yet-migrated legacy Views twins. The unified E2E
  # tool owns that union (the same source its bytes-mode codegen reads), so ask it instead of globbing.
  e2e_tool="${cpp_root}/../tools/e2e/e2e.py"
  [[ -f "${e2e_tool}" ]] || maui_die "missing ${e2e_tool}"
  while IFS= read -r k; do
    [[ -n "${k}" ]] && keys+=("${k}")
  done < <(python3 "${e2e_tool}" keys)
fi

echo "[apphost-xaml] capturing ${#keys[@]} page(s) -> ${out_dir}" >&2
for key in "${keys[@]}"; do
  capture_one "${key}"
done
echo "[apphost-xaml] done." >&2
