#!/usr/bin/env bash
# full_board_refresh.sh — ONE command: rebuild on all four lanes, recapture all 172 pages, re-measure,
# and regenerate comparison.json + README.md.
#
# WHY THIS EXISTS RATHER THAN JUST `recapture.py`: recapture.py does NOT build two things it depends
# on, and both silently produce a board that looks fine and is wrong.
#
#   1. THE MAUI REFERENCE APP. recapture.build()'s own docstring: "the MAUI reference app is built by
#      hand ... this covers the C++ galleries." So after ANY edit to port/maui-reference/pages/*.xaml
#      (which the maui column compiles AND the xaml column #embeds), a bare recapture leaves the
#      ground truth rendering the OLD markup. The diff then goes UP and reads as a port regression.
#   2. THE WINDOWS GUEST. For platform == "windows" recapture calls lane_vm directly with no build at
#      all. The guest builds its own artifacts from a LOCAL copy of the read-only Z:\ share. (Both
#      guest scripts dot-source sync_tree.ps1 and re-sync themselves, so no separate sync step.)
#
# EVERY LANE IS RELEASE. mobile.toml's `artifact` IS the release build for ios/android, the Windows
# guest builds Release, and run_comparison.column_artifact() now prefers `artifact_release` on the two
# macOS lanes — which previously deployed Debug while recapture built Release, so the binaries the
# harness had just compiled were never the ones photographed.
#
# USAGE:  bash port/cpp/tools/parity/full_board_refresh.sh [--skip-windows] [--clean]
#   --skip-windows  the Windows VM is not up; the other three lanes still refresh
#   --clean         wipe the release build dirs first for a true from-scratch compile (HOURS longer)
# Logged to port/cpp/docs/comparison/_recapture_logs/full-refresh-latest.log (gitignored).

set -euo pipefail

REPO="/Users/Alex.Tsvetanov/Documents/GitHub/maui"
export VCPKG_ROOT="${VCPKG_ROOT:-/Users/Alex.Tsvetanov/vcpkg}"
export ANDROID_HOME="${ANDROID_HOME:-/opt/homebrew/share/android-commandlinetools}"
export PATH="$ANDROID_HOME/platform-tools:$ANDROID_HOME/emulator:$PATH"

WIN_HOST="Testings-VM@WINDOWS-VM.local"
WIN_LIB='Z:\port\cpp\tools\parity\lib\windows'
SKIP_WINDOWS=0; CLEAN=0
for a in "$@"; do
  case "$a" in
    --skip-windows) SKIP_WINDOWS=1 ;;
    --clean)        CLEAN=1 ;;
    *) echo "unknown flag: $a" >&2; exit 2 ;;
  esac
done

LOGDIR="$REPO/port/cpp/docs/comparison/_recapture_logs"
mkdir -p "$LOGDIR"
LOG="$LOGDIR/full-refresh-$(date +%Y-%m-%d-%H%M%S).log"
ln -sf "$LOG" "$LOGDIR/full-refresh-latest.log"
exec > >(tee -a "$LOG") 2>&1

step() { echo; echo "############ [$(date +%H:%M:%S)] $* ############"; }
die()  { echo "!! ABORT: $*" >&2; exit 1; }

cd "$REPO"
step "PREFLIGHT"
echo "log:    $LOG"
echo "commit: $(git rev-parse --short HEAD)  branch: $(git rev-parse --abbrev-ref HEAD)"
[[ -d "$VCPKG_ROOT" ]]   || die "VCPKG_ROOT not a directory: $VCPKG_ROOT"
[[ -d "$ANDROID_HOME" ]] || die "ANDROID_HOME not a directory: $ANDROID_HOME"
command -v dotnet >/dev/null || die "dotnet not on PATH"
adb devices | grep -q emulator || die "no android emulator running (start it, then re-run)"
if (( ! SKIP_WINDOWS )); then
  ssh -o ConnectTimeout=10 -o BatchMode=yes "$WIN_HOST" "echo ok" >/dev/null \
    || die "cannot ssh $WIN_HOST — bring the VM up, or pass --skip-windows"
fi

if (( CLEAN )); then
  step "0/3 CLEAN — wiping the release build dirs (from-scratch compile)"
  rm -rf port/cpp/build/ios-release port/cpp/build/maccatalyst-release port/cpp/build/apple-release \
         port/cpp/examples/build-ios-release port/cpp/examples/build-maccatalyst-release \
         port/cpp/examples/build-apple-release
  # recapture.build() configures a missing preset dir on demand, so nothing else is needed here.
fi

# --- 1. WINDOWS GUEST ---------------------------------------------------------------------------
# First: it is the longest pole and the likeliest to need attention.
if (( ! SKIP_WINDOWS )); then
  step "1/3 WINDOWS GUEST — MauiReference + both galleries (self-syncing from Z:\\)"
  ssh "$WIN_HOST" powershell -NoProfile -ExecutionPolicy Bypass -File "$WIN_LIB\\build_maui_reference.ps1"
  ssh "$WIN_HOST" powershell -NoProfile -ExecutionPolicy Bypass -File "$WIN_LIB\\build_gallery_windows.ps1" \
      -Targets gallery,gallery_xaml -Jobs 3
else
  step "1/3 WINDOWS GUEST — SKIPPED (--skip-windows); the windows column will render STALE binaries"
fi

# --- 2. MAUI REFERENCE, Release on every lane ----------------------------------------------------
step "2/3 MAUI REFERENCE (host lanes) — recapture never builds this"
dotnet build port/maui-reference/app -f net10.0-ios         -c Release
dotnet build port/maui-reference/app -f net10.0-maccatalyst -c Release
# -p:AndroidSdkDirectory is REQUIRED on this host; without it the build fails XA5300.
dotnet build port/maui-reference/app -f net10.0-android     -c Release \
  -p:EmbedAssembliesIntoApk=true -p:AndroidFastDeploymentType= -p:AndroidSdkDirectory="$ANDROID_HOME"
adb install -r port/maui-reference/app/bin/Release/net10.0-android/dev.mauicpp.mauireference-Signed.apk

# FRESHNESS PROOF BY CONTENT, NOT BY MTIME. On android the APK's on-disk mtime is a normalized
# 1981-01-01, and a stale APK already contains the string "example.com" (it is
# "https://www.example.com", an unrelated framework literal in classes.dex) — so neither mtime nor a
# bare domain grep proves anything. The LINKED ASSEMBLY is the only honest witness. Note the iOS RID
# is iossimulator-arm64, NOT ios-arm64: the wrong-RID path exists and stat-ing it proves nothing.
step "2b/3 reference freshness — linked assemblies must carry the CURRENT WebView URL"
for dll in \
  "port/maui-reference/app/bin/Release/net10.0-ios/iossimulator-arm64/MauiReference.app/MauiReference.dll" \
  "port/maui-reference/app/bin/Release/net10.0-maccatalyst/maccatalyst-arm64/MauiReference.app/Contents/MonoBundle/MauiReference.dll" \
  "port/maui-reference/app/obj/Release/net10.0-android/android-arm64/linked/MauiReference.dll" ; do
  if [[ -f "$dll" ]]; then
    python3 - "$dll" <<'PY'
import sys, pathlib
b = pathlib.Path(sys.argv[1]).read_bytes()
hit = lambda s: s.encode("utf-16-le") in b or s.encode() in b
ok = hit("example.com") and not hit("bing.com")
print(f"  {'OK ' if ok else '!! '}example.com={hit('example.com')} bing.com={hit('bing.com')}  {sys.argv[1]}")
PY
  else
    echo "  !! MISSING $dll"
  fi
done

# --- 3. RECAPTURE + MEASURE ----------------------------------------------------------------------
# Defaults are the full board: --platforms android,ios,macos,windows (macos covers BOTH the catalyst
# and appkit lanes), --frameworks maui_xaml,cpp,cpp_xaml, --themes light,dark, --examples all 172.
# No --skip-build, so it builds the ios/catalyst/appkit RELEASE galleries — which are now also the
# ones deployed. No --no-measure, so it runs build_comparison_json.py -> pixel_score.py x4 ->
# measure_size.py -> measure_runtime.py (ttff) -> gen_readme.py. That chain is what updates
# comparison.json and README.md.
step "3/3 RECAPTURE ALL LANES + MEASURE + REGENERATE BOARD"
python3 port/cpp/tools/parity/recapture.py

step "DONE — $LOG"
echo "A run that exits 0 is NOT evidence the frames are correct. Check, in this order:"
echo "  1. the SUMMARY line above for failed steps, and any '! <col>: artifact_release declared but"
echo "     NOT BUILT' line — that skips a whole column rather than silently capturing Debug"
echo "  2. git status --short -- port/cpp/docs/comparison/captures   (expect many modified)"
echo "  3. context_flyout: the maui column's unique-colour count vs the two port columns. That is the"
echo "     settle fix under test; a BLANK WebView band means raise the settle, not a port cause."
echo "  4. the Catalyst selection cells (selection_synchronization / multiple_bound_selection /"
echo "     preselected_items) — they were green ONLY because Debug MAUI's interpreted loader selects"
echo "     where XamlC does not. On Release they should now agree with ios/android/windows."
