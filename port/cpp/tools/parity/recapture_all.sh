#!/usr/bin/env bash
# Recapture every platform sequentially, rebuild the board, and (when the tool exists) re-measure
# time-to-first-frame. Designed to be run BY HAND and watched.
#
# WHY THIS EXISTS
# ---------------
# Individual capture steps on this project have hung for over an hour with nothing on stdout — a VM
# reboot that never returned, a guest agent waiting on a window that would never appear. This script
# makes that impossible to miss and impossible to sit through:
#   * every step is timestamped on entry AND exit, with its elapsed time
#   * every step has a HARD TIMEOUT and is killed past it (default 45m, per-step overrides below)
#   * a heartbeat prints every 60s while a step runs, so a silent terminal means the script died,
#     not that a step is quietly working
#   * one platform at a time, never in parallel — concurrent runs share one guest agent and one
#     scratch/shot.png on the macOS/Windows VMs and destroy each other's frames
#
# USAGE
#   tools/parity/recapture_all.sh                  # everything
#   tools/parity/recapture_all.sh ios android      # only these lanes
#   LANES="windows" tools/parity/recapture_all.sh  # same thing via env
#   DRY_RUN=1 tools/parity/recapture_all.sh        # print the plan, run nothing
#   SKIP_BUILD=1 tools/parity/recapture_all.sh     # capture with whatever is already built
#
# Lanes: ios, catalyst, appkit, android, windows   (board rebuild always runs last)
#
# EXIT CODE is the number of FAILED steps, so `echo $?` after it is a real verdict. Every failure is
# also repeated in the summary table at the end — the script does NOT stop on a failed lane, because
# a broken Windows VM should not cost you the Android numbers.
set -uo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
CPP="$REPO/port/cpp"
LOG_DIR="${LOG_DIR:-$CPP/docs/comparison/_recapture_logs}"
RUN_ID="$(date +%Y-%m-%d-%H%M%S)"
MAIN_LOG="$LOG_DIR/recapture-$RUN_ID.log"
SCENARIOS="${SCENARIOS:-$LOG_DIR/empty-scenarios}"   # empty dir => one idle screenshot per page, no taps
DEFAULT_TIMEOUT="${DEFAULT_TIMEOUT:-2700}"           # 45 minutes
mkdir -p "$LOG_DIR" "$SCENARIOS"

MAC_VM_HOST="${MAC_VM_HOST:-Testings-Virtual-Machine.local}"
MAC_VM_USER="${MAC_VM_USER:-testinguser}"
WIN_VM_HOST="${WIN_VM_HOST:-WINDOWS-VM.local}"
WIN_VM_USER="${WIN_VM_USER:-Testings-VM}"
IOS_UDID="${IOS_UDID:-C4926671-2FA7-428E-B4A4-480692EE742B}"
ANDROID_SERIAL="${ANDROID_SERIAL:-emulator-5554}"

export VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"
export ANDROID_HOME="${ANDROID_HOME:-/opt/homebrew/share/android-commandlinetools}"
export PATH="/opt/homebrew/bin:$PATH:$ANDROID_HOME/platform-tools"

FAILED=0; STEP_NO=0
declare -a SUMMARY

ts()  { date '+%Y-%m-%d %H:%M:%S'; }
log() { printf '%s | %s\n' "$(ts)" "$*" | tee -a "$MAIN_LOG"; }
hr()  { printf '%s | %s\n' "$(ts)" "────────────────────────────────────────────────────────────" | tee -a "$MAIN_LOG"; }

# Pull the page/example currently being captured out of a step's log. Each tool announces progress in
# its own shape, so this normalises all of them to a bare page key:
#
#   run_comparison.py            "  border_playground/cpp/light initial   -> border_playground/..."
#   capture_ios_clean.py         "[37] cpp light border_playground -> ..."
#   build_android_apphost.sh     "[apphost] launch border_playground (light)..."
#   capture_all_csharp_android.sh"[csharp-android] wrote /…/border_playground_light.png (12345B)"
#   measure_runtime.py           "  android/cpp rep 3/5: 1.482s"
#
# Without this the terminal shows one heartbeat a minute during a 172-page sweep and you cannot tell a
# working run from a wedged one — which is the whole reason this script exists.
example_lines() {
  # ALL progress lines so far, normalised to one page key each. run_step prints only the ones it has
  # not shown yet, so nothing is skipped when several pages finish between two polls — sampling the
  # LAST line (the first cut of this) silently dropped pages on fast steps.
  # NOTE the '#' delimiters: these patterns contain (light|dark), and using '|' as the sed delimiter
  # ends the pattern at the alternation. That mistake made this print nothing at all.
  local f="$1"
  grep -aE '^\[[0-9]+\] |^\[apphost\] launch |^\[csharp-android\] wrote |^  [a-z0-9_]+/[a-z_]+/(light|dark) |rep [0-9]+/[0-9]+:' "$f" 2>/dev/null | sed -E \
    -e 's#^\[apphost\] launch ([^ ]+) .*#\1#' \
    -e 's#^\[csharp-android\] wrote .*/([A-Za-z0-9_]+)_(light|dark)\.png.*#\1 (\2)#' \
    -e 's#^\[[0-9]+\] ([a-z_]+) (light|dark) ([A-Za-z0-9_]+) .*#\3 (\1/\2)#' \
    -e 's#^ *([a-z]+/[a-z_]+) rep ([0-9]+/[0-9]+).*#\1 rep \2#' \
    -e 's#^  ([^/]+)/([a-z_]+)/(light|dark) .*#\1 (\2/\3)#'
}

# run_step <timeout_seconds> <name> <command...>
# Runs the command with its own log file, a heartbeat, and a hard timeout. Never aborts the script.
run_step() {
  local timeout_s="$1"; shift
  local name="$1"; shift
  STEP_NO=$((STEP_NO + 1))
  local slug; slug="$(printf '%02d-%s' "$STEP_NO" "$(echo "$name" | tr ' /' '__' | tr -cd '[:alnum:]_-')")"
  local step_log="$LOG_DIR/$RUN_ID-$slug.log"
  local start_epoch; start_epoch=$(date +%s)

  hr
  log "STEP $STEP_NO START  $name"
  log "         timeout ${timeout_s}s   log $step_log"
  if [[ "${DRY_RUN:-0}" == "1" ]]; then log "         DRY_RUN — not executing: $*"; SUMMARY+=("SKIP(dry)  $name"); return 0; fi

  "$@" > "$step_log" 2>&1 &
  local pid=$!
  # Heartbeat + live example + timeout. Poll rather than `timeout(1)`: coreutils' timeout is not on a
  # stock macOS. The example line prints ONLY when the page changes, so a 172-page sweep produces 172
  # lines rather than one every poll — enough to watch, not enough to drown the heartbeat.
  local waited=0 seen=0 printed=0 cur="" prev=""
  while kill -0 "$pid" 2>/dev/null; do
    sleep 5; waited=$((waited + 5))
    while IFS= read -r cur; do
      [[ -z "$cur" || "$cur" == "$prev" ]] && continue
      seen=$((seen + 1)); prev="$cur"
      log "         ▸ [$seen] $cur"
    done < <(example_lines "$step_log" | tail -n "+$((printed + 1))")
    printed=$(example_lines "$step_log" | wc -l | tr -d ' ')
    if (( waited % 60 == 0 )); then
      log "         ... still running (${waited}s / ${timeout_s}s)$([[ -n "$prev" ]] && echo "  on: $prev")  last: $(tail -n 1 "$step_log" 2>/dev/null | cut -c1-80)"
    fi
    if (( waited >= timeout_s )); then
      log "         !! TIMEOUT after ${waited}s — killing PID $pid and its children"
      pkill -P "$pid" 2>/dev/null; kill -9 "$pid" 2>/dev/null; sleep 2
      local el=$(( $(date +%s) - start_epoch ))
      log "STEP $STEP_NO TIMEOUT $name  (${el}s)"
      SUMMARY+=("TIMEOUT    $name  (${el}s)  -> $step_log"); FAILED=$((FAILED + 1)); return 124
    fi
  done
  wait "$pid"; local rc=$?
  local el=$(( $(date +%s) - start_epoch ))
  if (( rc == 0 )); then
    log "STEP $STEP_NO OK      $name  (${el}s)"
    SUMMARY+=("OK         $name  (${el}s)")
  else
    log "STEP $STEP_NO FAILED  $name  (${el}s, rc=$rc)"
    log "         tail: $(tail -n 3 "$step_log" 2>/dev/null | tr '\n' ' ' | cut -c1-200)"
    SUMMARY+=("FAILED(rc=$rc) $name  (${el}s)  -> $step_log"); FAILED=$((FAILED + 1))
  fi
  return $rc
}

# ---------------------------------------------------------------- guest hygiene
# A leftover app from a killed run WILL be captured by the next one: it holds a window the agent
# happily photographs. Measured: a stale gallery_xaml survived a killed Catalyst run and was still
# alive an hour later. Always sweep before a VM lane.
mac_vm_clean() {
  ssh -o BatchMode=yes -o ConnectTimeout=10 "$MAC_VM_USER@$MAC_VM_HOST" \
    'pkill -x gallery; pkill -x gallery_xaml; pkill -f MauiReference; exit 0' 2>/dev/null || true
}

# The runner's own reboot_before_run has hung for 1h34m with 0 frames captured. But the reboot is
# LOAD-BEARING: without a clean WindowServer, app windows are not AX-enumerable and `present` drops
# every frame (measured: 58 of 62 dropped, evenly across all three columns). So reboot by hand with a
# bounded wait, then let the run proceed with reboot_before_run left alone in the config.
mac_vm_reboot_and_settle() {
  log "         rebooting $MAC_VM_HOST (bounded wait; the runner's own reboot-and-wait hangs)"
  ssh -o BatchMode=yes -o ConnectTimeout=10 "$MAC_VM_USER@$MAC_VM_HOST" 'sudo reboot' 2>/dev/null || true
  sleep 25
  local i
  for i in $(seq 1 60); do
    if ssh -o BatchMode=yes -o ConnectTimeout=5 "$MAC_VM_USER@$MAC_VM_HOST" 'echo up' 2>/dev/null | grep -q up; then
      log "         VM back after ~$((25 + i * 5))s"; break
    fi
    sleep 5
    if (( i == 60 )); then log "         !! VM did not return within 5min"; return 1; fi
  done
  # Wait for the login/WindowServer session, not just sshd: capturing before the GUI is up is how you
  # get a board of blank frames.
  for i in $(seq 1 30); do
    local who; who=$(ssh -o BatchMode=yes -o ConnectTimeout=5 "$MAC_VM_USER@$MAC_VM_HOST" 'stat -f%Su /dev/console' 2>/dev/null)
    [[ "$who" == "$MAC_VM_USER" ]] && { log "         console session up ($who) after ~$((i*10))s"; break; }
    sleep 10
  done
  log "         display: $(ssh -o BatchMode=yes "$MAC_VM_USER@$MAC_VM_HOST" '/opt/homebrew/bin/displayplacer list 2>/dev/null | grep -m1 "^Resolution"' 2>/dev/null)"
}

# ---------------------------------------------------------------- device bring-up
# The script starts its own devices. You should be able to run this on a freshly-booted Mac with
# nothing open and walk away — staging a simulator and an emulator by hand first is exactly the kind
# of undocumented prerequisite that makes a "just run it" script not one.
ensure_ios_sim() {
  if xcrun simctl list devices booted 2>/dev/null | grep -q "$IOS_UDID"; then
    log "         iOS simulator already booted"; return 0
  fi
  log "         booting iOS simulator $IOS_UDID"
  xcrun simctl boot "$IOS_UDID" 2>/dev/null || true
  local i
  for i in $(seq 1 60); do
    xcrun simctl list devices booted 2>/dev/null | grep -q "$IOS_UDID" && { log "         booted after ~$((i*3))s"; break; }
    sleep 3
    (( i == 60 )) && { log "         !! simulator did not boot within 3min"; return 1; }
  done
  # Booted != ready to serve screenshots. Poll until a screenshot actually succeeds.
  for i in $(seq 1 40); do
    xcrun simctl io "$IOS_UDID" screenshot --type=png /tmp/_iosready.png >/dev/null 2>&1 && {
      log "         screenshot path live after ~$((i*3))s"; rm -f /tmp/_iosready.png; return 0; }
    sleep 3
  done
  log "         !! simulator booted but screenshots never worked"; return 1
}

ensure_android_emulator() {
  if [[ "$(adb -s "$ANDROID_SERIAL" get-state 2>/dev/null)" == "device" ]]; then
    log "         Android emulator already running"; return 0
  fi
  local avd="${MAUI_AVD:-maui-test}"
  log "         starting Android emulator '$avd' (headless-safe, detached)"
  "$ANDROID_HOME/emulator/emulator" -avd "$avd" -no-snapshot-save -no-boot-anim > "$LOG_DIR/$RUN_ID-emulator.log" 2>&1 &
  local i
  for i in $(seq 1 60); do
    [[ "$(adb -s "$ANDROID_SERIAL" get-state 2>/dev/null)" == "device" ]] && break
    sleep 5
    (( i == 60 )) && { log "         !! emulator did not attach within 5min"; return 1; }
  done
  # `device` state precedes boot completion; capturing before sys.boot_completed gives blank frames.
  for i in $(seq 1 60); do
    [[ "$(adb -s "$ANDROID_SERIAL" shell getprop sys.boot_completed 2>/dev/null | tr -d '\r')" == "1" ]] && {
      log "         boot completed after ~$((i*5))s"; adb -s "$ANDROID_SERIAL" shell input keyevent KEYCODE_WAKEUP >/dev/null 2>&1; return 0; }
    sleep 5
  done
  log "         !! emulator attached but never reported boot_completed"; return 1
}

# ---------------------------------------------------------------- preflight
preflight() {
  hr; log "PREFLIGHT"
  log "  repo            $REPO"
  log "  run id          $RUN_ID"
  log "  main log        $MAIN_LOG"
  log "  lanes           ${LANES_TO_RUN[*]}"
  log "  scenarios dir   $SCENARIOS  ($(ls -1 "$SCENARIOS" | wc -l | tr -d ' ') files — empty means one idle shot per page)"
  local l ok
  for l in "${LANES_TO_RUN[@]}"; do
    case "$l" in
      ios)      ok=$(xcrun simctl list devices booted 2>/dev/null | grep -c "$IOS_UDID")
                log "  ios simulator   $([[ $ok -gt 0 ]] && echo "booted" || echo "NOT BOOTED — run: xcrun simctl boot $IOS_UDID")" ;;
      android)  ok=$(adb -s "$ANDROID_SERIAL" get-state 2>/dev/null)
                log "  android emu     ${ok:-NOT RUNNING — start the maui-test AVD}" ;;
      catalyst|appkit)
                ok=$(ssh -o BatchMode=yes -o ConnectTimeout=8 "$MAC_VM_USER@$MAC_VM_HOST" 'echo up' 2>/dev/null)
                log "  macOS VM        ${ok:-UNREACHABLE at $MAC_VM_HOST}" ;;
      windows)  ok=$(ssh -o BatchMode=yes -o ConnectTimeout=8 "$WIN_VM_USER@$WIN_VM_HOST" 'echo up' 2>/dev/null)
                log "  windows VM      ${ok:-UNREACHABLE at $WIN_VM_HOST}" ;;
    esac
  done
  log "  NOTE: a lane whose device is missing will FAIL fast rather than hang; other lanes still run."
}

# ---------------------------------------------------------------- lanes
lane_ios() {
  hr; log "LANE: iOS  (simulator $IOS_UDID)"
  run_step 600 "ios: ensure simulator booted + screenshot path live" ensure_ios_sim
  if [[ "${SKIP_BUILD:-0}" != "1" ]]; then
    run_step 3600 "ios: build framework + galleries" bash -c "cd '$CPP' && cmake --build --preset ios && cmake --install build/ios --prefix /tmp/maui-prefix-ios && cmake --build examples/build-ios --target gallery gallery_xaml"
    run_step  900 "ios: install apps on simulator" bash -c "
      xcrun simctl install '$IOS_UDID' '$REPO/port/maui-reference/app/bin/Debug/net10.0-ios/iossimulator-arm64/MauiReference.app' &&
      xcrun simctl install '$IOS_UDID' '$CPP/examples/build-ios/gallery/gallery.app' &&
      xcrun simctl install '$IOS_UDID' '$CPP/examples/build-ios/gallery_xaml/gallery_xaml.app'"
  fi
  # capture_ios_clean.py sets the SIMULATOR appearance per theme (system-wide, not an app env var)
  # and restores it, and drops any frame that is still the .NET splash.
  local app
  for app in maui cpp xaml; do
    run_step 3600 "ios: capture $app (light+dark)" python3 "$CPP/tools/parity/capture_ios_clean.py" --app "$app" --themes light,dark --settle 5
  done
  # The reference writes to port/maui-reference/captures/ios/; the BOARD reads
  # docs/comparison/captures/ios/maui/. Nothing else copies between those two roots.
  run_step 600 "ios: promote reference captures into the board" python3 "$CPP/tools/parity/promote_reference_captures.py" --platform ios
}

lane_catalyst() {
  hr; log "LANE: macOS Catalyst  (VM $MAC_VM_HOST)"
  if [[ "${SKIP_BUILD:-0}" != "1" ]]; then
    run_step 3600 "catalyst: build framework + galleries" bash -c "cd '$CPP' && cmake --build --preset maccatalyst && cmake --install build/maccatalyst --prefix /tmp/maui-prefix-maccatalyst && cmake --build examples/build-maccatalyst --target gallery gallery_xaml"
  fi
  mac_vm_clean
  run_step 900 "catalyst: reboot VM + settle" mac_vm_reboot_and_settle
  run_step 7200 "catalyst: capture all pages (light+dark)" python3 "$CPP/docs/comparison/tools/run_comparison.py" \
    --config "$CPP/docs/comparison/config/local.toml" --env macos-arm64 --themes light,dark --scenarios "$SCENARIOS"
  run_step 900 "catalyst: import run into canonical captures" bash -c "
    d=\$(ls -dt '$CPP'/docs/comparison/2026-* 2>/dev/null | head -1)
    [ -n \"\$d\" ] || { echo 'no run dir produced'; exit 1; }
    echo \"importing \$d\"; python3 '$CPP/docs/comparison/tools/import_run_captures.py' \"\$d\" maccatalyst"
}

lane_appkit() {
  hr; log "LANE: macOS AppKit  (VM $MAC_VM_HOST — NEVER concurrent with Catalyst)"
  if [[ "${SKIP_BUILD:-0}" != "1" ]]; then
    run_step 3600 "appkit: build framework + galleries" bash -c "cd '$CPP' && cmake --build --preset apple && cmake --install build/apple --prefix /tmp/maui-prefix-apple && cmake --build examples/build-apple --target gallery gallery_xaml"
  fi
  mac_vm_clean
  run_step 7200 "appkit: capture all pages (light+dark)" python3 "$CPP/docs/comparison/tools/run_comparison.py" \
    --config "$CPP/docs/comparison/config/local.toml" --env macos-appkit --themes light,dark --scenarios "$SCENARIOS"
  run_step 900 "appkit: import run into canonical captures" bash -c "
    d=\$(ls -dt '$CPP'/docs/comparison/2026-* 2>/dev/null | head -1)
    [ -n \"\$d\" ] || { echo 'no run dir produced'; exit 1; }
    echo \"importing \$d\"; python3 '$CPP/docs/comparison/tools/import_run_captures.py' \"\$d\" maccatalyst"
}

lane_android() {
  hr; log "LANE: Android  (emulator $ANDROID_SERIAL)"
  run_step 900 "android: ensure emulator running + boot_completed" ensure_android_emulator
  # These scripts BUILD + INSTALL + CAPTURE, and each sets device night mode for its theme and
  # restores what it found. MAUI_APPEARANCE selects the pass; it is no longer handed to the apps.
  local theme
  for theme in light dark; do
    run_step 5400 "android: MAUI reference ($theme)"  env MAUI_APPEARANCE="$theme" bash "$CPP/tools/parity/capture_all_csharp_android.sh"
    run_step 5400 "android: cpp column ($theme)"      env MAUI_APPEARANCE="$theme" bash "$CPP/tools/parity/build_android_apphost.sh"
    run_step 5400 "android: xaml column ($theme)"     env MAUI_APPEARANCE="$theme" bash "$CPP/tools/parity/build_android_apphost_xaml.sh"
  done
}

lane_windows() {
  hr; log "LANE: Windows  (VM $WIN_VM_HOST — builds on the guest)"
  run_step 7200 "windows: capture all pages (light+dark)" python3 "$CPP/docs/comparison/tools/run_comparison.py" \
    --config "$CPP/docs/comparison/config/windows.toml" --themes light,dark --scenarios "$SCENARIOS"
  run_step 900 "windows: import run into canonical captures" bash -c "
    d=\$(ls -dt '$CPP'/docs/comparison/2026-* 2>/dev/null | head -1)
    [ -n \"\$d\" ] || { echo 'no run dir produced'; exit 1; }
    echo \"importing \$d\"; python3 '$CPP/docs/comparison/tools/import_run_captures.py' \"\$d\" windows"
}

# ---------------------------------------------------------------- board + measurements
rebuild_board() {
  hr; log "BOARD REBUILD + SCORING"
  # ORDER MATTERS. build_comparison_json.py only CARRIES OVER pixel scores — it cannot compute them.
  # Skipping pixel_score.py leaves stale verdicts behind a cheerful success line.
  run_step 900 "board: refresh comparison.json" python3 "$CPP/docs/comparison/tools/build_comparison_json.py"
  local p
  for p in ios maccatalyst android windows; do
    run_step 2700 "board: pixel_score $p" python3 "$CPP/tools/parity/pixel_score.py" --platform "$p"
  done
  run_step 600 "board: measure artifact sizes" python3 "$CPP/docs/comparison/tools/measure_size.py"

  # TIME TO FIRST FRAME. PREDICTIONS.md defines it framework-agnostically as launch -> first captured
  # frame that is NOT the launch/blank screen, using the board's own capture path — deliberately NOT
  # "a window exists", which fires long before first paint on the managed side and would flatter it.
  # The tool that implements this is not written yet.
  # TTFF polls real devices. Bring them up here too: the measurement can be asked for on its own
  # (LANES=... skipping a device lane), and an absent device would otherwise be recorded as a miss.
  ensure_ios_sim >/dev/null 2>&1 || log "         (iOS simulator unavailable — its TTFF lane will report unmeasured)"
  ensure_android_emulator >/dev/null 2>&1 || log "         (Android emulator unavailable — its TTFF lane will report unmeasured)"
  if [[ -f "$CPP/docs/comparison/tools/measure_runtime.py" ]]; then
    run_step 5400 "measure: time-to-first-frame (all lanes)" python3 "$CPP/docs/comparison/tools/measure_runtime.py" --metric ttff --all-platforms
  else
    hr
    log "SKIPPED: time-to-first-frame — docs/comparison/tools/measure_runtime.py DOES NOT EXIST."
    log "         Nothing in the repo measures startup yet, so there is nothing to re-run. This step"
    log "         will pick the tool up automatically once it lands. Everything above still ran."
    SUMMARY+=("SKIP(absent) time-to-first-frame — measure_runtime.py not implemented")
  fi

  run_step 900 "board: regenerate README" python3 "$CPP/docs/comparison/tools/gen_readme.py"
}

# ---------------------------------------------------------------- main
# Sourcing this file defines the helpers WITHOUT running anything, so current_example / run_step can
# be exercised against real tool output in a test. `RECAPTURE_LIB=1 source recapture_all.sh`.
[[ "${RECAPTURE_LIB:-0}" == "1" ]] && return 0 2>/dev/null

ALL_LANES=(ios catalyst appkit android windows)
if (( $# > 0 )); then LANES_TO_RUN=("$@"); else read -r -a LANES_TO_RUN <<< "${LANES:-${ALL_LANES[*]}}"; fi

START_EPOCH=$(date +%s)
hr; log "RECAPTURE ALL — run $RUN_ID"; preflight

for lane in "${LANES_TO_RUN[@]}"; do
  case "$lane" in
    ios) lane_ios ;; catalyst) lane_catalyst ;; appkit) lane_appkit ;;
    android) lane_android ;; windows) lane_windows ;;
    *) log "!! unknown lane '$lane' (valid: ${ALL_LANES[*]})"; FAILED=$((FAILED+1)) ;;
  esac
done
rebuild_board

hr
TOTAL=$(( $(date +%s) - START_EPOCH ))
log "SUMMARY — $((TOTAL / 60))m $((TOTAL % 60))s total, $FAILED failed step(s)"
for s in "${SUMMARY[@]}"; do log "  $s"; done
log "full log: $MAIN_LOG"
log "per-step logs: $LOG_DIR/$RUN_ID-*.log"
[[ $FAILED -gt 0 ]] && log "NOTE: verify captures before trusting the board — a script exiting 0 is not evidence that frames are correct."
exit $FAILED
