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
#   2. aapt2 compile + link: the app manifest/res PLUS every AndroidX AppCompat + Google Material AAR
#      resource tree (tools/parity/android-aar-lib.sh, staged from the local NuGet cache) into one merged
#      table, emitting an R.java per library package. Runs BEFORE javac, which must compile those R classes.
#   3. javac + d8: dex MauiHostActivity.java + the runtime java support classes (src/platform/android/java —
#      MauiDialogBridge etc., the same set the testhost dexes) + the generated R classes + every
#      library jar into classes*.dex (native multidex — AppCompat+Material exceed the 64K method limit).
#   4. Assemble: add classes*.dex (root) + lib/<abi>/libmaui_android_apphost.so into the APK (zip).
#   5. zipalign 4-byte, then apksigner sign with a throwaway debug keystore (created on demand).
#   6. adb install -r, am start the launcher Activity with the MAUI_SAMPLE_PAGE extra, screencap to a PNG.
#
# Usage:
#   build_android_apphost.sh                       # build+install, then capture EVERY page in page_keys.txt
#   build_android_apphost.sh <page_key> [<key>...]  # capture just these keys (build+install once)
#   build_android_apphost.sh --no-capture           # build+install+launch the default page only (smoke)
#   MAUI_APPEARANCE=dark build_android_apphost.sh    # dark-theme shots (the env the app host reads)
#
# Output: docs/comparison/captures/android/cpp/<key>_<theme>.png — the canonical C++ Android column the
# build_comparison_json.py + gen_readme.py layout reads (theme is always suffixed; Android is captured
# single-theme=light on the board). The XAML column is build_android_apphost_xaml.sh's job.
#
# --dry-run: print the output path each requested key WOULD write and exit WITHOUT building/installing/
# capturing (device-free verification of the path layout).
#
# BEST-EFFORT — several steps are first-run on this env (no prior android APK build in-tree). Steps the
# integrator must verify are marked "VERIFY:" inline; the most load-bearing assumptions are also listed at
# the end of the agent's hand-off. The script is conservative: it fails loudly (set -euo pipefail) rather
# than producing a half-built APK.
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cpp_root="$(cd "${script_dir}/../../.." && pwd)" # port/cpp
# Reuse the SDK-resolve / AVD-boot / keyed-staging machinery the testhost lane already proved.
# shellcheck source=../android-emu-lib.sh
source "${cpp_root}/tools/android-emu-lib.sh"
# AndroidX AppCompat + Google Material linking (the AARs MAUI itself builds against, staged out of the
# local NuGet cache) — supplies maui_android_aar_prepare / maui_aar_jars / maui_aar_link.
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
# Pin the emulator's chrome BEFORE any capture: Android screencaps are full-screen, so the status bar
# clock/battery/signal and any notification icon land inside every frame and would score as a per-page
# diff the port never caused. Restored by the trap below. See tools/parity/device_state.py.
python3 "${script_dir}/device_state.py" --android >&2 || true
# DEVICE night mode drives the port's theme now. MauiHostActivity reads
# Configuration.uiMode (UI_MODE_NIGHT_MASK), not the MAUI_APPEARANCE intent extra — the extra painted the
# port's own surfaces dark while every theme-resolved native default stayed light, which is how the port
# ended up drawing near-black text (8,8,8) on a near-black page. One mechanism for both columns: the MAUI
# reference already takes its dark from the device, and now so does the port. Restored by the trap.
# Record the device's CURRENT night mode so the trap restores what we found rather than forcing light —
# an emulator that was already dark would otherwise be silently flipped by running a capture.
# Read-back-verify, not "set + sleep 2 + hope" — see capture_all_csharp_android.sh's twin block for the
# 2026-08-24 incident this closes (a not-yet-settled emulator silently captured mis-themed stills).
uimode_night_read() {
  "${maui_adb:-adb}" -s "${maui_serial:-emulator-5554}" shell cmd uimode night 2>/dev/null | grep -qi yes && echo yes || echo no
}
wait_uimode_night() {
  local target="$1"
  for _ in $(seq 1 40); do # ~10s ceiling
    [[ "$(uimode_night_read)" == "${target}" ]] && return 0
    sleep 0.25
  done
  return 1
}

maui_night_before="$(uimode_night_read)"
maui_night_target="$([[ "${MAUI_APPEARANCE:-light}" == "dark" ]] && echo yes || echo no)"
"${maui_adb:-adb}" -s "${maui_serial:-emulator-5554}" shell cmd uimode night "${maui_night_target}" > /dev/null 2>&1 || true
wait_uimode_night "${maui_night_target}" ||
  echo "[apphost] WARNING: uimode night did not settle to '${maui_night_target}' within 10s; captures may be mis-themed" >&2
trap '"${maui_adb:-adb}" -s "${maui_serial:-emulator-5554}" shell cmd uimode night "${maui_night_before}" > /dev/null 2>&1 || true;
      python3 "${script_dir}/device_state.py" --android --clear >&2 || true' EXIT
[[ "${appearance}" == "dark" || "${appearance}" == "light" ]] || maui_die "MAUI_APPEARANCE must be light|dark"
# Canonical layout ALWAYS suffixes the theme: captures/android/cpp/<key>_<theme>.png. Android is
# captured single-theme (light) on the board, but keep the theme in the name for the layout convention.
suffix="_${appearance}"

# --dry-run: resolve the key set and print each canonical output path WITHOUT building/installing/
# capturing (device-free verification of the new captures/android/cpp/<key>_<theme>.png layout).
if [[ "${dry_run}" -eq 1 ]]; then
  out_dir_rel="docs/comparison/captures/android/cpp"
  declare -a dry_keys=()
  if [[ "${#requested_keys[@]}" -gt 0 ]]; then
    dry_keys=("${requested_keys[@]}")
  else
    while IFS= read -r line; do [[ -n "${line}" ]] && dry_keys+=("${line}"); done < "${script_dir}/page_keys.txt"
  fi
  for key in "${dry_keys[@]}"; do
    echo "${out_dir_rel}/${key}${suffix}.png"
  done
  echo "DRY_RUN_DONE"
  exit 0
fi

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
# RELEASE, not the Debug `android` preset. The board compares this APK against real MAUI's, and MAUI's
# reference APK is built Release -- a Debug-vs-Release board measures build flags, not parity. It also
# dominated the size study: the Debug apphost measured 360.0 MB against MAUI's 28.9 MB, and 353.5 MB of
# that was unstripped DWARF in lib/*.so. The `headless` base preset stays Debug on purpose (dev.sh,
# ctest and the sanitizers inherit it); `android-release` only overrides CMAKE_BUILD_TYPE.
build_dir="${cpp_root}/build/android-release"
if [[ ! -f "${build_dir}/build.ninja" && ! -f "${build_dir}/Makefile" ]]; then
  echo "[apphost] configuring the android preset (first run)..." >&2
  ( cd "${cpp_root}" && cmake --preset android-release >&2 )
fi
echo "[apphost] building maui_android_apphost..." >&2
( cd "${cpp_root}" && cmake --build --preset android-release --target maui_android_apphost >&2 )
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

# ---- 2. stage the AndroidX/Material AARs, then aapt2 compile + link -> base APK + the R classes ----
# ORDER MATTERS: aapt2 link is what EMITS the R.java files (one per resource-owning library package),
# and javac must compile those, so linking happens BEFORE javac now. See tools/parity/android-aar-lib.sh
# for what the AAR staging replaces from AGP (classes.jar onto the classpath + into d8, res/ compiled and
# merged, an R.java per --extra-packages, all cached under build/android/aardeps).
apphost_dir="${cpp_root}/src/platform/android/apphost"
manifest="${apphost_dir}/AndroidManifest.xml"
activity_java="${apphost_dir}/MauiHostActivity.java"
runtime_java_dir="${cpp_root}/src/platform/android/java" # MauiDialogBridge.java etc.
[[ -f "${manifest}" && -f "${activity_java}" ]] || maui_die "missing apphost manifest/Activity under ${apphost_dir}"

work="${build_dir}/apphost-apk"
rm -rf "${work}"
mkdir -p "${work}/classes" "${work}/gen" "${work}/lib/${abi}"

# Staged + resource-compiled once into a shared cache; re-staged only when the dep list, the stager or
# the build-tools version change, so routine per-page capture iteration does not re-pay for it.
maui_android_aar_prepare "${build_dir}/aardeps"

base_apk="${work}/base.apk"
echo "[apphost] aapt2 compile (apphost res: strings + theme) + link (${#maui_aar_jars[@]} AAR jars)..." >&2
apphost_res_dir="${apphost_dir}/res"
[[ -d "${apphost_res_dir}/values" ]] || maui_die "missing apphost res dir ${apphost_res_dir}/values"
"${aapt2}" compile --dir "${apphost_res_dir}" -o "${work}/res-compiled.zip" >&2 || maui_die "aapt2 compile failed"
# The library units go first (maui_aar_link: the first as the positional unit, the rest as -R overlays,
# plus one --extra-packages per resource-owning library); the APP's own compiled res is the LAST -R so it
# wins any collision. --auto-add-overlay is required for overlays to introduce new resources.
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
echo "[apphost] javac (${#java_sources[@]} sources) + d8..." >&2
"${javac_bin}" --release 17 -classpath "${android_jar}:${aar_cp}" -d "${work}/classes" "${java_sources[@]}" >&2
class_files=()
while IFS= read -r -d '' f; do class_files+=("${f}"); done \
  < <(find "${work}/classes" -name '*.class' -print0)
[[ "${#class_files[@]}" -gt 0 ]] || maui_die "javac emitted no .class files"
# AppCompat + Material blow past the 64K method limit, so dex them as native MULTIDEX (classes.dex,
# classes2.dex, ...) — min-api 24 makes that a plain multi-file output the platform loads directly.
"${d8}" --release --lib "${android_jar}" --min-api 24 --output "${work}" \
  "${class_files[@]}" "${maui_aar_jars[@]}" >&2
[[ -f "${work}/classes.dex" ]] || maui_die "d8 produced no classes.dex"
# VERIFY: some aapt2 versions require at least one resource or an explicit --no-resource-deduping / a
# generated R.java target; if link errors on "no resources", add a minimal res/values/strings.xml (app
# label) under ${work}/res, aapt2 compile it, and pass the compiled .flat via -R.

# ---- 4. assemble: add classes.dex (root) + lib/<abi>/<so> + assets/* into the APK ----
cp "${app_so}" "${work}/lib/${abi}/libmaui_android_apphost.so"

# Package the gallery image/font resources into the APK under assets/. The gallery pages name them by bare
# filename (image_source::from_file("dotnet_bot.png")), and the android image_handler resolves a from_file()
# name against the Context's AssetManager (context.getAssets().open(name)) — the robust, SELinux-/uid-/
# reinstall-immune on-device mechanism (an installed app cannot read /data/local/tmp under enforcing
# SELinux: those files are labelled shell_data_file, denied to the untrusted_app domain). Assets are stored
# RAW in the zip (aapt2 does not process the assets/ tree), so we just add them alongside classes.dex.
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
echo "[apphost] staged ${asset_count} gallery asset(s) into assets/ (from ${gallery_res_dir})" >&2
[[ "${asset_count}" -gt 0 ]] || echo "[apphost] WARNING: no gallery assets found — image pages will render blank" >&2

echo "[apphost] adding classes.dex + lib/${abi}/*.so + assets/*..." >&2
# aapt2 has NO `add` subcommand (that was the old `aapt`); the APK is a zip, so add the dex (root), the
# .so (under lib/<abi>/), and the assets (under assets/) with `zip`. Store the .so UNCOMPRESSED (-0) so the
# next `zipalign -p 4` page-aligns it, letting the loader map it from the APK directly (no extractNativeLibs
# needed). Assets are stored compressed (default) — AssetManager.open() inflates them transparently.
unaligned_apk="${work}/app-unaligned.apk"
cp "${base_apk}" "${unaligned_apk}"
( cd "${work}" && zip -X "${unaligned_apk}" classes*.dex >&2 \
    && zip -X -0 "${unaligned_apk}" "lib/${abi}/libmaui_android_apphost.so" >&2 \
    && { [[ "${asset_count}" -eq 0 ]] || zip -X -r "${unaligned_apk}" assets >&2 ; } ) \
  || maui_die "zip-assembling the APK failed"

# ---- 5. zipalign + apksigner (throwaway debug keystore) ----
aligned_apk="${work}/app-aligned.apk"
"${zipalign}" -f -p 4 "${unaligned_apk}" "${aligned_apk}" >&2
# STABLE PATH, deliberately NOT inside $build_dir. This is a throwaway self-signed dev key, but Android
# refuses to update a package signed by a DIFFERENT key: moving the build dir (build/android ->
# build/android-release) minted a new keystore and every install then failed with
# INSTALL_FAILED_UPDATE_INCOMPATIBLE, taking all four android cpp/cpp_xaml capture passes with it.
# Keeping the key beside the build dirs rather than inside one means a new build dir is not a new
# identity.
keystore="${cpp_root}/build/apphost-debug.keystore"
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
# A signature mismatch is recoverable: uninstall and retry. That happens when the keystore is
# regenerated (see above) or when a differently-signed build of the same package is already present --
# there is no app STATE worth preserving here, the page is chosen per launch by an intent extra.
if ! "${maui_adb}" -s "${maui_serial}" install -r "${signed_apk}" >&2; then
  echo "[apphost] install failed - uninstalling dev.mauicpp.apphost and retrying once" >&2
  "${maui_adb}" -s "${maui_serial}" uninstall dev.mauicpp.apphost >&2 || true
  "${maui_adb}" -s "${maui_serial}" install -r "${signed_apk}" >&2
fi

# Post-install warm-up: launch + kill once before the real run so the first REAL capture is not the one
# that eats the install/dexopt churn (the first post-install launch is COLD + dexopts, which can outlast a
# readiness poll and leave the launcher on screen). This throwaway launch absorbs that churn; we discard
# its frame. (Cheap: one launch + force-stop.)
echo "[apphost] post-install warm-up launch..." >&2
"${maui_adb}" -s "${maui_serial}" shell am start -W -n "${activity}" \
  --es MAUI_SAMPLE_PAGE "label" > /dev/null 2>&1 || true
sleep 2
"${maui_adb}" -s "${maui_serial}" shell am force-stop "${pkg}" > /dev/null 2>&1 || true

# Launch one page key and screencap it to <out_dir>/<key><suffix>.png. The Activity reads the
# MAUI_SAMPLE_PAGE intent extra; MAUI_APPEARANCE is read by the native app host from the process env, which
# on android is NOT inherited from `am start`, so we pass it as a second extra the Activity can forward (or
# the app host reads it from a system property). VERIFY: getenv("MAUI_APPEARANCE") in app_host.cpp will be
# empty under a normal `am start` — to drive dark mode either (a) set it via `adb shell setprop` + read the
# prop in the app host, or (b) forward the extra in MauiHostActivity into the native call. For light-theme
# captures (the default) this does not matter. See the hand-off uncertainties.
out_dir="${cpp_root}/docs/comparison/captures/android/cpp"
mkdir -p "${out_dir}"
# Wait until our process is actually GONE (not just asked to stop). am force-stop is asynchronous: it
# returns before the process dies, and — crucially — before its Activity's window/frame is torn down. If
# we relaunch + screencap while the OLD frame is still composited, screencap grabs the PREVIOUS page (the
# dispatch-offset wave-15 bug: every <key>.png was the page captured one launch earlier, because the blind
# `sleep 5` could elapse while the prior frame still owned the surface under load/install churn). Polling
# `pidof` to zero before the next launch makes the old frame unreachable, so a stale grab is impossible.
wait_process_gone() {
  for _ in $(seq 1 40); do # ~10s ceiling (40 * 0.25s)
    local pid
    pid="$("${maui_adb}" -s "${maui_serial}" shell pidof "${pkg}" 2> /dev/null | tr -d '[:space:]')"
    [[ -z "${pid}" ]] && return 0
    sleep 0.25
  done
  return 1 # still alive after the ceiling — caller logs but proceeds (best-effort)
}

# Wait for THIS launch's Activity to have actually DRAWN, using the system's own first-frame signal:
# ActivityTaskManager logs `Displayed <pkg>/.MauiHostActivity for user 0: +Nms` exactly when the new
# instance's first frame is presented. We clear logcat immediately before the launch, so the ONLY such
# line that can appear is from the launch we just issued — a per-launch, race-free readiness barrier that
# replaces the blind sleep. (am start -W also blocks to first frame; the poll backstops it under load.)
#
# Backstop: right after `adb install -r`, dexopt + the install's own launch can either delay or pre-emit
# the Displayed line so the log-grep misses it (the launcher gets captured instead). So we ALSO accept a
# second, level-triggered signal — our Activity being the resumed/top activity in `dumpsys activity` —
# which is true for as long as our page is on screen regardless of when the one-shot log line fired. The
# barrier is satisfied when EITHER signal is seen, making the post-install first capture race-free too.
wait_displayed() {
  for _ in $(seq 1 60); do # ~15s ceiling (60 * 0.25s)
    if "${maui_adb}" -s "${maui_serial}" logcat -d 2> /dev/null \
      | grep -q "Displayed ${pkg}/.MauiHostActivity"; then
      return 0
    fi
    # Level-triggered fallback: our Activity is the currently resumed/top activity (survives a missed log).
    if "${maui_adb}" -s "${maui_serial}" shell dumpsys activity activities 2> /dev/null \
      | grep -qE "(topResumedActivity|ResumedActivity)[^\n]*${pkg}/.MauiHostActivity"; then
      return 0
    fi
    sleep 0.25
  done
  return 1 # neither signal seen after the ceiling — caller logs but proceeds (best-effort)
}

capture_one() {
  local key="$1"
  echo "[apphost] launch ${key} (${appearance})..." >&2
  local _parity_t0=${SECONDS}
  echo "@@PARITY BEGIN ${key} cpp ${appearance}"
  # (a) Kill the prior instance and WAIT for its process — and thus its on-screen frame — to be gone, so a
  #     screencap can never capture the previous page (the wave-15 dispatch-offset root cause).
  "${maui_adb}" -s "${maui_serial}" shell am force-stop "${pkg}" > /dev/null 2>&1 || true
  wait_process_gone || echo "[apphost] WARNING: ${pkg} still alive after force-stop (${key})" >&2
  # (b) Clear logcat so the next `Displayed` line is unambiguously from THIS launch (the readiness barrier).
  "${maui_adb}" -s "${maui_serial}" logcat -c > /dev/null 2>&1 || true
  # (c) Launch with -W (blocks to first frame) and then poll for this launch's Displayed marker.
  # Bounded: a hung launch drops ONE frame instead of killing the whole pass
  # (android-emu-lib.sh; the 2026-08-17 empty_view stall cost both dark passes).
  if ! maui_android_start_bounded "${activity}" "MAUI_SAMPLE_PAGE" "${key}"; then
    echo "@@PARITY END ${key} cpp ${appearance} $((SECONDS - _parity_t0))"
    return 0
  fi
  wait_displayed || echo "[apphost] WARNING: never saw Displayed for ${key}; capturing anyway" >&2
  # Dismiss the transient "System UI isn't responding" ANR dialog that can overlay the page during the
  # build/install/launch load burst. Use the CLOSE_SYSTEM_DIALOGS broadcast, NOT keyevent BACK — BACK would
  # be consumed by the ANR dialog the first time but close the Activity (-> launcher) when no dialog is up.
  "${maui_adb}" -s "${maui_serial}" shell am broadcast -a android.intent.action.CLOSE_SYSTEM_DIALOGS > /dev/null 2>&1 || true
  # A settle AFTER the first-frame barrier: the system's Displayed fires on the window's first frame,
  # but the maui tree's content draw (text/shapes via the Canvas bridge) can trail it by a frame or two.
  # 4s (not 1s) and IDENTICAL to the MAUI column's settle (capture_all_csharp_android.sh) so both columns
  # are photographed in the SAME state of Android's FADING SCROLLBARS. Android hides a fading scrollbar
  # until awakenScrollBars(); MAUI's ScrollViewExtensions.HandleScrollBarVisibilityChange awakens it during
  # layout, so at a short settle the MAUI shot caught a scrollbar mid-fade that the port's never showed —
  # a pure capture race that was contributing 43-79% of the pixel diff on clip / hit_testing / the pickers.
  # Measured on the emulator: the bar is still faintly present at 1.5s and completely gone by 4s.
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
  # ASSERT WHAT IS ACTUALLY ON SCREEN, after the settle and before the shutter. Twin of the guard in
  # build_android_apphost_xaml.sh; see that file for the full incident.
  #
  # Short version, because this column has not been bitten YET and that is the only difference:
  # `wait_displayed` above proves the app CAME UP and says nothing about 4 seconds later, and its
  # failure path literally says "capturing anyway". In the xaml column that combination banked 20
  # committed PNGs of CHROME — a WebView escaped to an ACTION_VIEW intent and a browser covered the
  # gallery — and a later rerun banked the LAUNCHER on the same pages, because the app was crashing.
  # Both were invisible: the lane wrote a foreign window as the port's render and said nothing.
  # Nothing about that mechanism is xaml-specific; only the page set was.
  #
  # Matched against this emulator's REAL dumpsys wording: API 34 prints `topResumedActivity=` /
  # `ResumedActivity:` and never `mResumedActivity`, the field name every guide greps for — a check
  # written from that name matches nothing and passes silently forever.
  _fg="$("${maui_adb}" -s "${maui_serial}" shell dumpsys activity activities 2>/dev/null \
        | sed -n 's/.*[Rr]esumedActivity[=:][^u]*u[0-9][0-9]* \([A-Za-z0-9_.]*\)\/.*/\1/p' | head -1)"
  if [[ -n "${_fg}" && "${_fg}" != "${pkg}" ]]; then
    # DROP the frame. A missing capture is a loud fixable gap; a wrong one scores as a port defect.
    echo "[apphost] !! ${key}${suffix}: foreground is ${_fg}, expected ${pkg} — frame DROPPED" >&2
    echo "@@PARITY END ${key} cpp ${appearance} $((SECONDS - _parity_t0))"
    return 0
  fi
  "${maui_adb}" -s "${maui_serial}" exec-out screencap -p > "${out_dir}/${key}${suffix}.png"
  # THE SOFT KEYBOARD IS A SECOND FOREIGN WINDOW, and the foreground check above cannot see it: the IME
  # belongs to a DIFFERENT PROCESS, so `am force-stop` on the gallery does not take it down and
  # dumpsys still names our package as resumed. A page whose scenario focused a field leaves Gboard
  # composited over the NEXT page's still, covering ~37% of the frame.
  #
  # This is not hypothetical either: the guard found 37 such frames already committed, across 14 pages
  # in alphabetically CONTIGUOUS runs (data_template_selector -> empty_view* -> filter_selection ->
  # focus) — one stretch of one run, the same shape as the splash incident. On 5 of those pages only the
  # PORT columns were contaminated while MAUI's was clean, which is most of what scored
  # border_playground / focus / shadow_playground RED.
  #
  # Shared with the xaml and MAUI columns (android-emu-lib.sh): all three leak, and the fix is identical.
  if ! reshoot_without_keyboard "${out_dir}/${key}${suffix}.png" "${activity}" "${pkg}" "${key}"; then
    echo "@@PARITY END ${key} cpp ${appearance} $((SECONDS - _parity_t0))"
    return 0
  fi
  echo "@@PARITY END ${key} cpp ${appearance} $((SECONDS - _parity_t0))"
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
