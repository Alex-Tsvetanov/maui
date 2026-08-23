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
# USAGE:  bash port/cpp/tools/parity/full_board_refresh.sh [--skip-windows] [--clean] [--parallel]
#   --skip-windows  the Windows VM is not up; the other three lanes still refresh
#   --clean         wipe the release build dirs first for a true from-scratch compile (HOURS longer)
#   --parallel      capture the four lanes CONCURRENTLY, then measure ONCE (see below)
#   --smoke         implies --parallel, but captures ONE page (button, light) per lane. ~15 min.
#                   Run this FIRST. It exercises every part of a --parallel run that can fail early --
#                   the freshness gates, the in-lane builds being no-ops, ios_install, four guests
#                   reachable AT ONCE, the publish paths -- for 1/344th of the frames. This project's
#                   recurring failure mode is a multi-hour run that banked unusable frames; fifteen
#                   minutes buys that out. It writes real captures for `button`, which the full run
#                   then overwrites.
#
# --parallel: WHY IT IS SAFE, AND THE TWO THINGS IT MUST NOT DO.
#   recapture.py is strictly sequential BY DESIGN, and its docstring gives the reason: the macOS and
#   Windows VMs each have ONE guest agent and ONE scratch shot.png, so two runs against ONE guest
#   destroy each other's frames. That constraint is PER GUEST, not global -- ios (simctl), android
#   (emulator), the macOS VM and the Windows VM are four different machines. So four SEPARATE
#   recapture.py processes, one lane each, do not contend.
#
#   1. ONLY ONE MAY MEASURE. measure() rewrites comparison.json, measurements.json and README.md
#      wholesale; four concurrent writers would interleave. Every lane therefore runs --no-measure and
#      a single --measure-only pass follows the `wait`. recapture.py's own --measure-only help
#      documents exactly this split -- this is the tool being used as designed, not worked around.
#   2. macos MUST be --lanes catalyst. `--platforms macos` is TWO lanes on ONE guest (Catalyst then
#      AppKit) and AppKit is not one of the board's four. It also doubles what is otherwise the
#      critical path. AppKit's columns are then scored against an OLDER pass -- do not read them as
#      fresh after a --parallel run.
#
#   ALL THREE COLUMNS ARE CAPTURED, and "skip maui, its C# did not change" is a trap worth naming:
#   motion_score.find_frames requires BOTH columns' frames in the SAME run directory (it checks the
#   run's frames against the PUBLISHED stills, so an older run cannot supply the maui half once the
#   port half is republished). Drop maui and every motion-scored cell returns verdict=INVALID, which
#   pixel_score caps YELLOW. Measured: 41 motion-scored pages x 2 port columns = 82 cells PER LANE.
#   The maui column is not just content here, it is co-evidence.
# Logged to port/cpp/docs/comparison/_recapture_logs/full-refresh-latest.log (gitignored).

set -euo pipefail

REPO="/Users/Alex.Tsvetanov/Documents/GitHub/maui"

# RESOLVE THE TOOLCHAIN BY PROBING, NOT BY TRUSTING THE ENVIRONMENT. `${VAR:-default}` only fills in
# when the var is UNSET, so an exported-but-WRONG value wins and the default never fires -- which is
# exactly what happened: a shell exporting ANDROID_HOME=~/Library/Android/sdk (a path that does not
# exist on this host; the SDK is the homebrew one) aborted the run at preflight. A stale env var must
# not be able to defeat the script, and it must not be silently overridden either -- so probe each
# candidate for the tool that proves it, and SAY which one was chosen.
pick_dir() {   # pick_dir <label> <witness-relative-path> <candidate>...
  local label="$1" witness="$2"; shift 2
  local c
  for c in "$@"; do
    [[ -n "$c" && -e "$c/$witness" ]] && { printf '%s' "$c"; return 0; }
  done
  echo "!! ABORT: no usable $label — tried: $*" >&2
  echo "   (each must contain: $witness)" >&2
  return 1
}

ANDROID_HOME="$(pick_dir "Android SDK" "platform-tools/adb" \
  "${ANDROID_HOME:-}" "/opt/homebrew/share/android-commandlinetools" "$HOME/Library/Android/sdk")" || exit 1
VCPKG_ROOT="$(pick_dir "vcpkg" "scripts/buildsystems/vcpkg.cmake" \
  "${VCPKG_ROOT:-}" "/Users/Alex.Tsvetanov/vcpkg" "$HOME/vcpkg")" || exit 1
export ANDROID_HOME VCPKG_ROOT
export ANDROID_SDK_ROOT="$ANDROID_HOME"     # some tooling still reads the older name
export PATH="$ANDROID_HOME/platform-tools:$ANDROID_HOME/emulator:$PATH"

WIN_HOST="Testings-VM@WINDOWS-VM.local"
WIN_LIB='Z:\port\cpp\tools\parity\lib\windows'
SKIP_WINDOWS=0; CLEAN=0; PARALLEL=0; SMOKE=0
for a in "$@"; do
  case "$a" in
    --skip-windows) SKIP_WINDOWS=1 ;;
    --clean)        CLEAN=1 ;;
    --parallel)     PARALLEL=1 ;;
    --smoke)        SMOKE=1; PARALLEL=1 ;;
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
echo "ANDROID_HOME: $ANDROID_HOME"
echo "VCPKG_ROOT:   $VCPKG_ROOT"
command -v dotnet >/dev/null || die "dotnet not on PATH"
# `adb devices` prints its header even with nothing attached, so match a device LINE, not the word.
adb devices | awk 'NR>1 && /\tdevice$/ {n++} END {exit n?0:1}' \
  || die "no android device/emulator attached. Start one:
     \$ANDROID_HOME/emulator/emulator -avd \$($ANDROID_HOME/emulator/emulator -list-avds | head -1) -no-snapshot-load &
   then re-run. (adb devices currently shows: $(adb devices | tail -n +2 | tr '\n' ' '))"
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
WIN_BUILD_PID=""; WIN_BUILD_LOG=""
if (( ! SKIP_WINDOWS )); then
  step "1/3 WINDOWS GUEST — MauiReference + both galleries (self-syncing from Z:\\)"
  win_build() {
    ssh "$WIN_HOST" powershell -NoProfile -ExecutionPolicy Bypass -File "$WIN_LIB\\build_maui_reference.ps1"
    ssh "$WIN_HOST" powershell -NoProfile -ExecutionPolicy Bypass -File "$WIN_LIB\\build_gallery_windows.ps1" \
        -Targets gallery,gallery_xaml -Jobs 3
  }
  if (( PARALLEL )); then
    # DIFFERENT MACHINE, so this does not contend with stage 2's host builds -- and it is the longest
    # pole. Backgrounding it here is the one overlap that costs nothing: the guest compiles on the
    # guest's CPU while the host compiles the iOS/Catalyst galleries and the three dotnet references.
    # Joined BEFORE stage 3 so a guest build failure still stops the windows lane from capturing.
    WIN_BUILD_LOG="$LOGDIR/windows-guest-build-$(date +%Y-%m-%d-%H%M%S).log"
    ( win_build > "$WIN_BUILD_LOG" 2>&1; echo $? > "$WIN_BUILD_LOG.rc" ) &
    WIN_BUILD_PID=$!
    echo "  guest build backgrounded  pid=$WIN_BUILD_PID  log=$WIN_BUILD_LOG"
  else
    win_build
  fi
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
if [[ -n "$WIN_BUILD_PID" ]]; then
  step "2c/3 JOIN THE WINDOWS GUEST BUILD (backgrounded at stage 1)"
  wait "$WIN_BUILD_PID" || true
  WIN_RC="$(cat "$WIN_BUILD_LOG.rc" 2>/dev/null || echo '?')"
  echo "  guest build exit=$WIN_RC  log=$WIN_BUILD_LOG"
  if [[ "$WIN_RC" != "0" ]]; then
    # NOT fatal to the other three lanes, and NOT silently ignored either: the windows lane's own
    # freshness gate is what refuses to capture a stale guest binary, so the honest move is to say so
    # loudly and let that gate do its job rather than aborting a run three lanes could still complete.
    echo "  !! WINDOWS GUEST BUILD FAILED — the windows lane will capture whatever is already on the"
    echo "     guest, and its freshness gate should refuse. Read: $WIN_BUILD_LOG"
    tail -25 "$WIN_BUILD_LOG" || true
  fi
fi

step "3/3 RECAPTURE ALL LANES + MEASURE + REGENERATE BOARD"
if (( ! PARALLEL )); then
  python3 port/cpp/tools/parity/recapture.py
else
  # --- 3a. COLD-BOOT THE EMULATOR ----------------------------------------------------------------
  # MEASURED REMEDY, not hygiene. Android's three columns are three passes ~100 min apart, and a
  # long-lived emulator session drifts: the dark surface wanders from #121212 toward #2F2F2F, which
  # lands on the board as reds because the columns were photographed at different points of the
  # drift. Both apps hit it, so it is not a port defect -- and a cold boot plus one sitting took
  # column agreement from 1/28 to 27/28 with ZERO port code changed.
  # -no-snapshot-LOAD is the load-bearing flag. ensure_android_emulator() starts the AVD with
  # -no-snapshot-save, which still RESTORES the saved snapshot -- i.e. it inherits exactly the aged
  # session this is trying to discard. It also reuses an already-running emulator unconditionally, so
  # the kill has to happen out here, before recapture.py is invoked.
  step "3a/3 ANDROID COLD BOOT (discard the snapshot; the dark-wash remedy)"
  if adb devices | awk 'NR>1 && /\tdevice$/ {n++} END {exit n?0:1}'; then
    adb -s emulator-5554 emu kill || true
    for _ in $(seq 1 30); do
      adb devices | awk 'NR>1 && /\tdevice$/ {n++} END {exit n?0:1}' || break
      sleep 2
    done
  fi
  "$ANDROID_HOME/emulator/emulator" -avd "${MAUI_AVD:-maui-test}" \
      -no-snapshot-load -no-snapshot-save -no-boot-anim \
      > "$LOGDIR/emulator-cold-$(date +%H%M%S).log" 2>&1 &
  for _ in $(seq 1 90); do
    [[ "$(adb -s emulator-5554 shell getprop sys.boot_completed 2>/dev/null | tr -d '\r')" == "1" ]] && break
    sleep 5
  done
  [[ "$(adb -s emulator-5554 shell getprop sys.boot_completed 2>/dev/null | tr -d '\r')" == "1" ]] \
    || die "emulator did not finish cold-booting in 7.5 min"
  adb -s emulator-5554 shell input keyevent KEYCODE_WAKEUP || true
  echo "  cold boot OK ($(adb -s emulator-5554 shell getprop ro.build.version.sdk | tr -d '\r'))"

  # --- 3b. FOUR LANES, FOUR PROCESSES, FOUR LOGS -------------------------------------------------
  # Each lane gets its own log because four interleaved BEGIN/END streams on one terminal are
  # unreadable, and the per-lane log is what you tail to see which lane is the straggler.
  step "3b/3 CAPTURE — four lanes CONCURRENTLY (--no-measure); tail the per-lane logs"
  declare -a LANE_NAME=() LANE_PID=() LANE_LOG=()
  launch() {   # launch <name> <recapture-arg>...
    local name="$1"; shift
    local lg="$LOGDIR/lane-$name-$(date +%Y-%m-%d-%H%M%S).log"
    # `|| true` INSIDE the subshell: recapture.py exits with its failed-STEP COUNT, so a lane that
    # dropped one frame out of 1032 exits 3 -- and under `set -e` that would abort the whole refresh
    # at the `wait`. The exit code is recorded and reported instead of being allowed to kill the run.
    ( python3 port/cpp/tools/parity/recapture.py "$@" > "$lg" 2>&1; echo $? > "$lg.rc" ) &
    LANE_NAME+=("$name"); LANE_PID+=("$!"); LANE_LOG+=("$lg")
    echo "  launched $name  pid=$! log=$lg"
  }

  # THREE COLUMNS ON EVERY LANE — see the --parallel note in the header for why maui cannot be
  # dropped even when its C# is untouched.
  SLICE=()
  (( SMOKE )) && SLICE=(--examples button --themes light)
  (( SMOKE )) && echo "  SMOKE: one page (button, light) per lane — this is the de-risking pass"
  launch ios     --platforms ios     --no-measure "${SLICE[@]+"${SLICE[@]}"}"
  launch android --platforms android --no-measure --visible no "${SLICE[@]+"${SLICE[@]}"}"
  launch macos   --platforms macos   --no-measure --lanes catalyst "${SLICE[@]+"${SLICE[@]}"}"
  (( SKIP_WINDOWS )) || launch windows --platforms windows --no-measure "${SLICE[@]+"${SLICE[@]}"}"

  echo
  echo "  watch:  tail -f $LOGDIR/lane-*.log"
  for i in "${!LANE_PID[@]}"; do
    wait "${LANE_PID[$i]}" || true
    rc="$(cat "${LANE_LOG[$i]}.rc" 2>/dev/null || echo '?')"
    echo "  [$(date +%H:%M:%S)] lane ${LANE_NAME[$i]} finished, failed-step count = $rc"
  done

  if (( SMOKE )); then
    step "SMOKE DONE — measure DELIBERATELY SKIPPED"
    echo "  A smoke run captured ONE page. Running measure() here would rebuild comparison.json and"
    echo "  README.md for all 1376 cells off a board where only `button` moved — a full rescore"
    echo "  triggered by a 1-page capture. Check the four lane logs above, then re-run WITHOUT"
    echo "  --smoke for the real pass."
    for i in "${!LANE_NAME[@]}"; do
      echo "  --- ${LANE_NAME[$i]} (last 6 lines) ---"
      tail -6 "${LANE_LOG[$i]}" || true
    done
    exit 0
  fi

  # --- 3c. ONE MEASURE ---------------------------------------------------------------------------
  # AFTER the wait, never during: this rewrites comparison.json, measurements.json and README.md.
  step "3c/3 MEASURE — one pass over every lane the four processes just captured"
  python3 port/cpp/tools/parity/recapture.py --measure-only \
      --platforms "$( (( SKIP_WINDOWS )) && echo android,ios,macos || echo android,ios,macos,windows )"

  # --- 3d. READ THE INSTRUMENT -------------------------------------------------------------------
  # column_skew reports cells whose columns came from DIFFERENT passes. In a --parallel run that is
  # the one thing the parallelism can silently reintroduce, and it is printed by the lane gate as an
  # advisory -- i.e. into a log nobody re-opens. Surfaced here so it is read before the board is
  # committed. (`took=` once sat unread in a log for two hours.)
  step "3d/3 COLUMN SKEW — cells whose columns came from different passes"
  python3 - <<'PYSKEW' || true
import sys
sys.path.insert(0, "port/cpp/tools/parity/lib")
import freshness
for lane in ("ios", "android", "maccatalyst", "windows"):
    rows = freshness.column_skew(lane, limit=5)
    print(f"  {lane}: {'clean' if not rows else ''}")
    for r in rows:
        print(f"    {r}")
PYSKEW
fi

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
