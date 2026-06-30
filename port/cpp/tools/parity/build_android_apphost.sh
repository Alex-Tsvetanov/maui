#!/usr/bin/env bash
# build_android_apphost.sh — package the C++ MAUI gallery into a signed APK with the in-SDK build-tools
# (NO gradle), install it on the emulator, and `adb exec-out screencap` one (or every) gallery page for the
# 3-way MAUI ┃ C++ ┃ C++&XAML Android parity board. This is the Android twin of the macOS capture pipelines
# (tools/parity/capture_maccatalyst.py / capture_appkit.py); unlike them it grabs the EMULATOR framebuffer,
# so it does NOT hijack the host screen and can run while the Mac is in use.
#
# Pipeline (mirrors src/platform/android/apphost + the proven testhost javac→d8 lane in
# tools/android-testhost-run.sh, which this reuses android-emu-lib.sh from):
#   1. cmake-build libmaui_android_apphost.so (the SHARED app-host target) for the android preset.
#   2. javac + d8: dex MauiHostActivity.java + the runtime java support classes (src/platform/android/java —
#      NativeOnClickListener etc., the same set the testhost dexes) into classes.dex.
#   3. aapt2 link the manifest against android.jar → a base APK with the compiled manifest + resource table.
#   4. Assemble: add classes.dex (root) + lib/<abi>/libmaui_android_apphost.so into the APK (aapt2 add / zip).
#   5. zipalign 4-byte, then apksigner sign with a throwaway debug keystore (created on demand).
#   6. adb install -r, am start the launcher Activity with the MAUI_SAMPLE_PAGE extra, screencap to a PNG.
#
# Usage:
#   build_android_apphost.sh                       # build+install, then capture EVERY page in page_keys.txt
#   build_android_apphost.sh <page_key> [<key>...]  # capture just these keys (build+install once)
#   build_android_apphost.sh --no-capture           # build+install+launch the default page only (smoke)
#   MAUI_APPEARANCE=dark build_android_apphost.sh    # dark-theme shots (the env the app host reads)
#
# Output: docs/comparison/android/cpp/<key>.png (light) or <key>_dark.png (dark) — the C++ column. The XAML
# column would build the gallery_xaml twin the same way once it has an android app target; this script does
# the C++ builder column the user's 172-Android-captures goal needs first.
#
# BEST-EFFORT — several steps are first-run on this env (no prior android APK build in-tree). Steps the
# integrator must verify are marked "VERIFY:" inline; the most load-bearing assumptions are also listed at
# the end of the agent's hand-off. The script is conservative: it fails loudly (set -euo pipefail) rather
# than producing a half-built APK.
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cpp_root="$(cd "${script_dir}/../.." && pwd)" # port/cpp
# Reuse the SDK-resolve / AVD-boot / keyed-staging machinery the testhost lane already proved.
# shellcheck source=../android-emu-lib.sh
source "${cpp_root}/tools/android-emu-lib.sh"

# ---- args ----
do_capture=1
declare -a requested_keys=()
for arg in "$@"; do
  case "${arg}" in
  --no-capture) do_capture=0 ;;
  --*) maui_die "unknown flag: ${arg}" ;;
  *) requested_keys+=("${arg}") ;;
  esac
done

appearance="${MAUI_APPEARANCE:-light}"
[[ "${appearance}" == "dark" || "${appearance}" == "light" ]] || maui_die "MAUI_APPEARANCE must be light|dark"
suffix=""
[[ "${appearance}" == "dark" ]] && suffix="_dark"

# ---- 0. resolve the SDK + build-tools (highest installed; 34.0.0 confirmed present) ----
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

# The ABI the android preset cross-compiles for. The emulator AVD is arm64-v8a (android-emu-lib.sh creates
# the maui-test AVD from system-images;android-34;google_apis;arm64-v8a), so the .so must be arm64-v8a too.
# VERIFY: if the preset's ANDROID_ABI ever changes, update this (and lib/<abi> below) to match.
abi="arm64-v8a"

# ---- 1. cmake-build the app-host .so (android preset) ----
# The android preset writes to build/android (per STATUS.md "Build & test"); the .so lands at
# build/android/libmaui_android_apphost.so. Configure first only if the build dir is missing — routine
# iteration just re-builds the one target (Ninja incremental). VCPKG_ROOT + MAUI_ANDROID_SDK_ROOT must be
# set in the environment (the android preset needs them), same as the widget-test lane.
build_dir="${cpp_root}/build/android"
if [[ ! -f "${build_dir}/build.ninja" && ! -f "${build_dir}/Makefile" ]]; then
  echo "[apphost] configuring the android preset (first run)..." >&2
  ( cd "${cpp_root}" && cmake --preset android >&2 )
fi
echo "[apphost] building maui_android_apphost..." >&2
( cd "${cpp_root}" && cmake --build --preset android --target maui_android_apphost >&2 )
# VERIFY: confirm the actual output path/name the android preset emits for a SHARED lib (CMAKE_SHARED_
# LIBRARY_PREFIX is "lib", suffix ".so"; the preset may route artifacts to a subdir — adjust if so).
app_so=""
for candidate in \
  "${build_dir}/libmaui_android_apphost.so" \
  "${build_dir}"/**/libmaui_android_apphost.so; do
  [[ -f "${candidate}" ]] && app_so="${candidate}"
done
[[ -n "${app_so}" ]] || maui_die "could not find libmaui_android_apphost.so under ${build_dir} (check the preset's output dir)"
echo "[apphost] .so: ${app_so}" >&2

# ---- 2. javac + d8: dex the Activity + the runtime java support classes ----
apphost_dir="${cpp_root}/src/platform/android/apphost"
manifest="${apphost_dir}/AndroidManifest.xml"
activity_java="${apphost_dir}/MauiHostActivity.java"
runtime_java_dir="${cpp_root}/src/platform/android/java" # NativeOnClickListener.java etc.
[[ -f "${manifest}" && -f "${activity_java}" ]] || maui_die "missing apphost manifest/Activity under ${apphost_dir}"

work="${build_dir}/apphost-apk"
rm -rf "${work}"
mkdir -p "${work}/classes" "${work}/lib/${abi}"

java_sources=("${activity_java}")
if [[ -d "${runtime_java_dir}" ]]; then
  for j in "${runtime_java_dir}"/*.java; do
    [[ -f "${j}" ]] && java_sources+=("${j}")
  done
fi
echo "[apphost] javac (${#java_sources[@]} sources) + d8..." >&2
"${javac_bin}" --release 17 -classpath "${android_jar}" -d "${work}/classes" "${java_sources[@]}" >&2
class_files=()
while IFS= read -r -d '' f; do class_files+=("${f}"); done \
  < <(find "${work}/classes" -name '*.class' -print0)
[[ "${#class_files[@]}" -gt 0 ]] || maui_die "javac emitted no .class files"
"${d8}" --release --lib "${android_jar}" --min-api 24 --output "${work}" "${class_files[@]}" >&2
[[ -f "${work}/classes.dex" ]] || maui_die "d8 produced no classes.dex"

# ---- 3. aapt2 link the manifest -> a base APK (compiled manifest + resource table) ----
# No app resources (the manifest declares none), so we link with just the manifest + android.jar. aapt2
# link needs a -R/compiled resources flag only when there are resources; with none, --manifest alone is
# enough to produce a base APK carrying the binary manifest + an (empty) resource table.
base_apk="${work}/base.apk"
echo "[apphost] aapt2 compile (minimal res) + link..." >&2
# aapt2 link needs at least one compiled-resource input; with only --manifest it prints usage and makes
# nothing. Provide a minimal values resource (an app_name string) so the link has a compiled input.
mkdir -p "${work}/res/values"
cat > "${work}/res/values/strings.xml" <<'XML'
<?xml version="1.0" encoding="utf-8"?>
<resources>
    <string name="app_name">MAUI C++ Gallery</string>
</resources>
XML
"${aapt2}" compile --dir "${work}/res" -o "${work}/res-compiled.zip" >&2 || maui_die "aapt2 compile failed"
"${aapt2}" link \
  -o "${base_apk}" \
  -I "${android_jar}" \
  --manifest "${manifest}" \
  --min-sdk-version 24 --target-sdk-version 34 \
  "${work}/res-compiled.zip" >&2 || maui_die "aapt2 link failed"
[[ -f "${base_apk}" ]] || maui_die "aapt2 link produced no base APK"
# VERIFY: some aapt2 versions require at least one resource or an explicit --no-resource-deduping / a
# generated R.java target; if link errors on "no resources", add a minimal res/values/strings.xml (app
# label) under ${work}/res, aapt2 compile it, and pass the compiled .flat via -R.

# ---- 4. assemble: add classes.dex (root) + lib/<abi>/<so> into the APK ----
cp "${app_so}" "${work}/lib/${abi}/libmaui_android_apphost.so"
echo "[apphost] adding classes.dex + lib/${abi}/*.so..." >&2
# aapt2 add stores files into the APK uncompressed-as-needed; run from ${work} so the in-APK paths are
# relative (classes.dex at the root, lib/<abi>/<so> under lib/). VERIFY: .so must be stored with the
# correct path AND (on modern android) uncompressed + page-aligned for extractNativeLibs=false; the
# manifest does not set extractNativeLibs, so the default (extract on install) applies and plain `aapt2
# add` / zip is fine. If a "couldn't find libmaui_android_apphost.so" UnsatisfiedLinkError appears at
# runtime, set android:extractNativeLibs="true" in the manifest <application> or align the .so (zipalign
# -p 4) — see the hand-off uncertainties.
unaligned_apk="${work}/app-unaligned.apk"
cp "${base_apk}" "${unaligned_apk}"
# aapt2 has NO `add` subcommand (that was the old `aapt`); the APK is a zip, so add the dex (root) + the
# .so (under lib/<abi>/) with `zip`. Store the .so UNCOMPRESSED (-0) so the next `zipalign -p 4` page-aligns
# it, letting the loader map it from the APK directly (no extractNativeLibs needed).
( cd "${work}" && zip -X "${unaligned_apk}" classes.dex >&2 \
    && zip -X -0 "${unaligned_apk}" "lib/${abi}/libmaui_android_apphost.so" >&2 ) \
  || maui_die "zip-assembling the APK failed"

# ---- 5. zipalign + apksigner (throwaway debug keystore) ----
aligned_apk="${work}/app-aligned.apk"
"${zipalign}" -f -p 4 "${unaligned_apk}" "${aligned_apk}" >&2
keystore="${build_dir}/apphost-debug.keystore"
if [[ ! -f "${keystore}" ]]; then
  echo "[apphost] creating a throwaway debug keystore..." >&2
  "${keytool_bin}" -genkeypair -v -keystore "${keystore}" -storepass android -keypass android \
    -alias androiddebugkey -keyalg RSA -keysize 2048 -validity 10000 \
    -dname "CN=Maui CPP Debug,O=maui-cpp,C=US" >&2
fi
signed_apk="${build_dir}/maui_android_apphost.apk"
echo "[apphost] apksigner sign..." >&2
"${apksigner}" sign --ks "${keystore}" --ks-pass pass:android --key-pass pass:android \
  --out "${signed_apk}" "${aligned_apk}" >&2
"${apksigner}" verify "${signed_apk}" >&2 || maui_die "apksigner verify failed on ${signed_apk}"
echo "[apphost] APK: ${signed_apk}" >&2

# ---- 6. install on the emulator ----
maui_android_ensure_booted
pkg="dev.mauicpp.apphost"
activity="${pkg}/.MauiHostActivity"
echo "[apphost] adb install -r..." >&2
"${maui_adb}" -s "${maui_serial}" install -r "${signed_apk}" >&2

# Launch one page key and screencap it to <out_dir>/<key><suffix>.png. The Activity reads the
# MAUI_SAMPLE_PAGE intent extra; MAUI_APPEARANCE is read by the native app host from the process env, which
# on android is NOT inherited from `am start`, so we pass it as a second extra the Activity can forward (or
# the app host reads it from a system property). VERIFY: getenv("MAUI_APPEARANCE") in app_host.cpp will be
# empty under a normal `am start` — to drive dark mode either (a) set it via `adb shell setprop` + read the
# prop in the app host, or (b) forward the extra in MauiHostActivity into the native call. For light-theme
# captures (the default) this does not matter. See the hand-off uncertainties.
out_dir="${cpp_root}/docs/comparison/android/cpp"
mkdir -p "${out_dir}"
capture_one() {
  local key="$1"
  echo "[apphost] launch ${key} (${appearance})..." >&2
  "${maui_adb}" -s "${maui_serial}" shell am force-stop "${pkg}" > /dev/null 2>&1 || true
  "${maui_adb}" -s "${maui_serial}" shell am start -n "${activity}" \
    --es MAUI_SAMPLE_PAGE "${key}" --es MAUI_APPEARANCE "${appearance}" > /dev/null
  # Give the Activity time to create + mount + lay out before grabbing the framebuffer. VERIFY: bump this
  # if pages capture mid-layout; a poll on a known view would be more robust but needs a uiautomator hook.
  sleep 3
  "${maui_adb}" -s "${maui_serial}" exec-out screencap -p > "${out_dir}/${key}${suffix}.png"
  echo "[apphost] wrote ${out_dir}/${key}${suffix}.png" >&2
}

if [[ "${do_capture}" -eq 0 ]]; then
  echo "[apphost] --no-capture: launching the default page only (smoke test)" >&2
  "${maui_adb}" -s "${maui_serial}" shell am start -n "${activity}" --es MAUI_SAMPLE_PAGE "label" > /dev/null
  echo "[apphost] launched ${activity} with page=label" >&2
  exit 0
fi

# Pick the key set: the explicit args, else every key in page_keys.txt (the canonical 172).
declare -a keys=()
if [[ "${#requested_keys[@]}" -gt 0 ]]; then
  keys=("${requested_keys[@]}")
else
  page_keys_file="${script_dir}/page_keys.txt"
  [[ -f "${page_keys_file}" ]] || maui_die "missing ${page_keys_file}"
  while IFS= read -r line; do
    [[ -n "${line}" ]] && keys+=("${line}")
  done < "${page_keys_file}"
fi

echo "[apphost] capturing ${#keys[@]} page(s) -> ${out_dir}" >&2
for key in "${keys[@]}"; do
  capture_one "${key}"
done
echo "[apphost] done." >&2
