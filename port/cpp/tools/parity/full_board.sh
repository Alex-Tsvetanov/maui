#!/bin/bash
# Recapture the whole parity board — every lane, both themes — and tell you what is happening while it runs.
#
# Run it yourself:
#     port/cpp/tools/parity/full_board.sh                  # all four lanes
#     port/cpp/tools/parity/full_board.sh windows ios      # just these, in this order
#
# WHY THIS IS ONE SCRIPT AND NOT FOUR COMMANDS
# Lanes must run STRICTLY ONE AT A TIME. Two capture runs corrupt each other: a second emulator on the
# same AVD once produced 126 failed captures across a 4-hour pass, and two concurrent run_comparison.py
# runs kill each other over the single shared VM agent. So this waits for each lane to exit before
# starting the next, and refuses to start at all if something is already capturing.
#
# WHY IT PRINTS A HEARTBEAT
# A lane is a 3-5 hour silence otherwise. On 2026-08-13 a leaked app instance made the catalyst lane drop
# every port motion frame for 84 minutes and the run reported nothing — it kept exiting 0, page after
# page, producing holes. Silence is not health. The heartbeat below shows steps, drops and current page,
# and DROP_ALARM shouts when frames start disappearing, whatever the cause.
set -u

CPP=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)   # …/port/cpp — this script lives at tools/parity/
export VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"
export ANDROID_HOME="${ANDROID_HOME:-/opt/homebrew/share/android-commandlinetools}"
export PATH="$ANDROID_HOME/platform-tools:$ANDROID_HOME/emulator:$PATH"
cd "$CPP" || exit 1

LANES=("$@"); [ ${#LANES[@]} -eq 0 ] && LANES=(android macos windows ios)
HEARTBEAT=${HEARTBEAT:-60}     # seconds between status lines
DROP_ALARM=${DROP_ALARM:-10}   # drops within one heartbeat that mean something is wrong

# --- refuse to double-run -------------------------------------------------------------------------
# Checked BEFORE anything else: starting a second run is the single most expensive mistake here, and it
# looks like ill-health rather than contention, so it costs hours to diagnose after the fact.
if pgrep -f "recapture.py|run_comparison.py|build_android" > /dev/null; then
  echo "REFUSING TO START — a capture is already running:"
  pgrep -fl "recapture.py|run_comparison.py|build_android" | sed 's/^/    /'
  echo
  echo "Let it finish, or take over with:  pkill -f 'recapture.py|run_comparison.py'"
  exit 1
fi

status_line() {  # $1=lane $2=log $3=lane-start-epoch
  local lane=$1 log=$2 t0=$3 steps drops cur el
  steps=$(grep -c 'END ' "$log" 2>/dev/null || echo 0)
  drops=$(grep -c 'DROPPED' "$log" 2>/dev/null || echo 0)
  cur=$(grep 'BEGIN' "$log" 2>/dev/null | tail -1 | sed -E 's/.*framework=([a-z_]+) theme=([a-z]+) example=([a-z_0-9]+).*/\1\/\2\/\3/')
  el=$(( ($(date +%s) - t0) / 60 ))
  printf '%s  %-8s steps %-5s drops %-5s %-42s [%dh%02dm]\n' \
    "$(date '+%H:%M')" "$lane" "$steps" "$drops" "${cur:-starting}" $((el/60)) $((el%60))
}

OVERALL=0
for LANE in "${LANES[@]}"; do
  LOG="/tmp/lane_${LANE}.log"; : > "$LOG"
  T0=$(date +%s)
  echo "=================================================================="
  echo "  $LANE  starting $(date '+%H:%M')   log: $LOG"
  echo "=================================================================="
  python3 tools/parity/recapture.py --platforms "$LANE" --themes light,dark > "$LOG" 2>&1 &
  PID=$!

  LAST_DROPS=0
  while kill -0 $PID 2>/dev/null; do
    sleep "$HEARTBEAT"
    status_line "$LANE" "$LOG" "$T0"
    DROPS=$(grep -c 'DROPPED' "$LOG" 2>/dev/null || echo 0)
    if [ $((DROPS - LAST_DROPS)) -ge "$DROP_ALARM" ]; then
      echo "  !! $((DROPS - LAST_DROPS)) frames dropped in the last ${HEARTBEAT}s — frames are being LOST."
      echo "     Most recent reason: $(grep 'DROPPED' "$LOG" | tail -1 | sed 's/.*DROPPED — //')"
      echo "     If this is the macos/ios lane, check for leaked app instances:"
      echo "       ssh testinguser@Testings-Virtual-Machine.local 'ps ax -o pid,etime,comm | grep gallery'"
    fi
    LAST_DROPS=$DROPS
  done
  wait $PID; RC=$?

  # Exit 0 is NOT evidence. A pass that wrote nothing, or that raced a second instance, still exits 0 —
  # so judge the lane on what it produced, not on its return code.
  FAILS=$(grep -c 'could not resolve launcher activity' "$LOG" 2>/dev/null || echo 0)
  DROPS=$(grep -c 'DROPPED' "$LOG" 2>/dev/null || echo 0)
  STEPS=$(grep -c 'END ' "$LOG" 2>/dev/null || echo 0)
  echo "--- $LANE done $(date '+%H:%M')  rc=$RC steps=$STEPS launcher-failures=$FAILS dropped-frames=$DROPS"
  [ "$RC" -ne 0 ] || [ "$FAILS" -gt 0 ] || [ "$STEPS" -eq 0 ] && OVERALL=1
  [ "$DROPS" -gt 0 ] && echo "    NOTE: $DROPS dropped frames — those cells have HOLES and need re-driving."
  sleep 30   # let the lane's device settle before the next one claims it
done

echo
echo "=== all lanes done $(date '+%H:%M') ==="
# The real verdict: is each lane measured from ONE binary? A lane spanning many runs is an upper bound,
# not a count you can subtract from 172.
python3 tools/parity/provenance.py
exit $OVERALL
