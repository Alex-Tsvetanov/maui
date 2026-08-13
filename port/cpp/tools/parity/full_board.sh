#!/bin/bash
# Recapture the whole parity board — every lane, both themes — and tell you what is happening while it runs.
#
#     port/cpp/tools/parity/full_board.sh                  # all four lanes, one at a time
#     port/cpp/tools/parity/full_board.sh -j 2             # two lanes at once
#     port/cpp/tools/parity/full_board.sh -j 2 windows ios # just these two
#
# PARALLELISM, AND WHY IT IS CAPPED
# Lanes drive four different devices (android emulator, mac VM, windows VM, ios simulator), so they can
# overlap. Two things stop it being free:
#
#   1. THE BOARD IS ONE FILE. The measure phase rewrites comparison.json, measurements.json and README.md,
#      so two concurrent runs clobber each other's scores. Every lane here captures with --no-measure and
#      the board is measured ONCE at the end (recapture.py's own documented recipe for parallel lanes).
#   2. THE HOST IS ONE MACHINE. Captures are timing-sensitive — a 4s settle, motion bursts 0.3s apart,
#      present/self-heal on timeouts. Starve the host and settles are missed, which shows up as dropped
#      frames and motion phase jitter: noise in exactly the number this board exists to make trustworthy.
#      Measured on a 14-core/24GB host: ONE VM lane alone already runs at load ~9. So macos and windows —
#      two full UTM guests — are never scheduled together, whatever -j says. Pair a VM lane with a mobile
#      lane instead.
#
# Wall-clock is not the goal; a trustworthy number is. A pass degraded by contention has to be redone,
# which costs more than it saved. Watch the drop counter: if it climbs under -j 2, drop back to -j 1.
#
# WHY IT PRINTS A HEARTBEAT
# A lane is a 3-5 hour silence otherwise. On 2026-08-13 a leaked app instance made the catalyst lane drop
# every port motion frame for 84 minutes while still exiting 0, page after page, producing holes. Silence
# is not health.
set -u

CPP=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)   # …/port/cpp — this script lives at tools/parity/
export VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"
export ANDROID_HOME="${ANDROID_HOME:-/opt/homebrew/share/android-commandlinetools}"
export PATH="$ANDROID_HOME/platform-tools:$ANDROID_HOME/emulator:$PATH"
cd "$CPP" || exit 1

JOBS=1
while getopts "j:" opt; do
  case $opt in
    j) JOBS=$OPTARG ;;
    *) echo "usage: full_board.sh [-j N] [lane ...]"; exit 2 ;;
  esac
done
shift $((OPTIND - 1))

LANES=("$@"); [ ${#LANES[@]} -eq 0 ] && LANES=(android macos windows ios)
HEARTBEAT=${HEARTBEAT:-60}     # seconds between status lines
DROP_ALARM=${DROP_ALARM:-10}   # drops within one heartbeat that mean something is wrong
CORES=$(sysctl -n hw.ncpu 2>/dev/null || echo 8)
# Where lane logs go. Overridable so the test never writes to the paths a LIVE run is using — on
# 2026-08-13 the test truncated /tmp/lane_macos.log out from under a running 4-hour catalyst lane.
LOGDIR=${LOGDIR:-/tmp}

# Test seam: full_board_test.sh substitutes a stub here to exercise the scheduler without devices. When
# it is overridden we are not driving real captures, so the real-capture guard below does not apply.
RECAPTURE=${RECAPTURE:-}
TESTING=1; [ -z "$RECAPTURE" ] && { RECAPTURE="python3 tools/parity/recapture.py"; TESTING=0; }

# --- refuse to double-run -------------------------------------------------------------------------
# Checked BEFORE anything else: starting a second run is the single most expensive mistake here, and it
# looks like device ill-health rather than contention, so it costs hours to diagnose after the fact.
if [ "$TESTING" = 0 ] && pgrep -f "recapture.py|run_comparison.py|build_android" > /dev/null; then
  echo "REFUSING TO START — a capture is already running:"
  pgrep -fl "recapture.py|run_comparison.py|build_android" | cut -c1-110 | sed 's/^/    /'
  echo
  echo "Let it finish, or take over with:  pkill -f 'recapture.py|run_comparison.py'"
  exit 1
fi

# `grep -c` prints "0" AND exits 1 when nothing matches, so the obvious `grep -c ... || echo 0` yields
# the two-line string "0\n0" — which then breaks every arithmetic comparison downstream and silently
# collapsed the whole scheduler loop. Count in one place instead.
count() { local n; n=$(grep -c "$1" "$2" 2>/dev/null); echo "${n:-0}"; }

is_vm() { case $1 in macos|windows) return 0 ;; *) return 1 ;; esac; }
vm_busy() { local l; for l in ${R_LANE[@]+"${R_LANE[@]}"}; do is_vm "$l" && return 0; done; return 1; }

lane_status() {  # $1=lane $2=start-epoch
  local lane=$1 t0=$2 log=$LOGDIR/lane_$1.log steps drops cur el
  steps=$(count 'END ' "$log")
  drops=$(count 'DROPPED' "$log")
  cur=$(grep 'BEGIN' "$log" 2>/dev/null | tail -1 \
        | sed -E 's/.*framework=([a-z_]+) theme=([a-z]+) example=([a-z_0-9]+).*/\1\/\2\/\3/')
  el=$(( ($(date +%s) - t0) / 60 ))
  printf '  %-8s steps %-5s drops %-5s %-40s [%dh%02dm]\n' \
    "$lane" "$steps" "$drops" "${cur:-starting}" $((el/60)) $((el%60))
}

PENDING=("${LANES[@]}")
R_LANE=(); R_PID=(); R_T0=(); R_DROPS=()
OVERALL=0
echo "board recapture: lanes=${LANES[*]}  -j $JOBS  (macos+windows never paired)  $(date '+%H:%M')"

while [ ${#PENDING[@]} -gt 0 ] || [ ${#R_LANE[@]} -gt 0 ]; do
  # --- start whatever the caps allow ---
  STILL=()
  for lane in ${PENDING[@]+"${PENDING[@]}"}; do
    if [ ${#R_LANE[@]} -lt "$JOBS" ] && { ! is_vm "$lane" || ! vm_busy; }; then
      : > "$LOGDIR/lane_$lane.log"
      # --no-measure: the board is measured ONCE, after every lane has landed its captures.
      $RECAPTURE --platforms "$lane" --themes light,dark --no-measure \
        > "$LOGDIR/lane_$lane.log" 2>&1 &
      R_LANE+=("$lane"); R_PID+=($!); R_T0+=($(date +%s)); R_DROPS+=(0)
      echo "--- $lane started $(date '+%H:%M')  log: $LOGDIR/lane_$lane.log"
    else
      STILL+=("$lane")
    fi
  done
  PENDING=(${STILL[@]+"${STILL[@]}"})

  sleep "$HEARTBEAT"

  # --- report and reap ---
  LOAD=$(sysctl -n vm.loadavg | awk '{print $2}')
  echo "$(date '+%H:%M')  load $LOAD/$CORES"
  N_LANE=(); N_PID=(); N_T0=(); N_DROPS=()
  for i in $(seq 0 $(( ${#R_LANE[@]} - 1 )) ); do
    lane=${R_LANE[$i]}; pid=${R_PID[$i]}; t0=${R_T0[$i]}; log=$LOGDIR/lane_$lane.log
    drops=$(count 'DROPPED' "$log")
    if kill -0 "$pid" 2>/dev/null; then
      lane_status "$lane" "$t0"
      if [ $((drops - ${R_DROPS[$i]})) -ge "$DROP_ALARM" ]; then
        echo "    !! $((drops - ${R_DROPS[$i]})) frames dropped in ${HEARTBEAT}s — frames are being LOST."
        echo "       reason: $(grep 'DROPPED' "$log" | tail -1 | sed 's/.*DROPPED — //')"
        echo "       leaked app instances are the usual cause on a VM lane:"
        echo "         ssh testinguser@Testings-Virtual-Machine.local 'ps ax -o pid,etime,comm | grep gallery'"
      fi
      N_LANE+=("$lane"); N_PID+=("$pid"); N_T0+=("$t0"); N_DROPS+=("$drops")
    else
      wait "$pid"; rc=$?
      # Exit 0 is NOT evidence — a pass that wrote nothing, or that raced another instance, still exits 0.
      steps=$(count 'END ' "$log")
      fails=$(count 'could not resolve launcher activity' "$log")
      echo "--- $lane DONE $(date '+%H:%M')  rc=$rc steps=$steps launcher-failures=$fails dropped=$drops"
      [ "$rc" -ne 0 ] || [ "$fails" -gt 0 ] || [ "$steps" -eq 0 ] && OVERALL=1
      [ "$drops" -gt 0 ] && echo "    NOTE: $drops dropped frames — those cells have HOLES and need re-driving."
    fi
  done
  R_LANE=(${N_LANE[@]+"${N_LANE[@]}"}); R_PID=(${N_PID[@]+"${N_PID[@]}"})
  R_T0=(${N_T0[@]+"${N_T0[@]}"});      R_DROPS=(${N_DROPS[@]+"${N_DROPS[@]}"})
done

echo
echo "=== all lanes captured $(date '+%H:%M') — measuring the board once ==="
$RECAPTURE --measure-only || OVERALL=1

echo
# The real verdict: is each lane measured from ONE binary? A lane spanning many runs is an upper bound,
# not a count you can subtract from 172.
python3 tools/parity/provenance.py
exit $OVERALL
