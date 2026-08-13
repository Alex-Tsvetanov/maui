#!/bin/bash
# Device-free check of full_board.sh's scheduler.
#
# The scheduler is the part that can silently do the wrong thing: it can run two lanes that must never
# overlap (macos and windows are two full UTM guests on one host), or serialize everything and quietly
# waste the -j the caller asked for. Both look like "it ran fine" from the outside, so assert on the
# actual start/stop intervals rather than on the exit code.
#
# Run:  port/cpp/tools/parity/tests/full_board_test.sh
set -u
HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
SCRIPT="$HERE/../full_board.sh"
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

# Stub standing in for recapture.py: records when each lane starts and stops, then exits 0.
cat > "$WORK/stub.sh" <<'STUB'
#!/bin/bash
LANE=""; MEASURE=0
while [ $# -gt 0 ]; do
  case $1 in
    --platforms) LANE=$2; shift 2 ;;
    --measure-only) MEASURE=1; shift ;;
    *) shift ;;
  esac
done
if [ $MEASURE = 1 ]; then echo "MEASURE $(date +%s)" >> "$EVENTS"; exit 0; fi
echo "START $LANE $(date +%s)" >> "$EVENTS"
# Write the same progress markers a real lane writes, so the caller's "a lane that captured nothing is
# a failure" check sees a plausible lane rather than an empty one.
for i in 1 2 3; do
  echo "BEGIN platform=$LANE framework=cpp theme=light example=page_$i kind=png" >> "$LOGDIR/lane_$LANE.log"
  echo "END   platform=$LANE framework=cpp theme=light example=page_$i kind=png took=1.0s" >> "$LOGDIR/lane_$LANE.log"
done
sleep "${STUB_SECS:-4}"
echo "END $LANE $(date +%s)" >> "$EVENTS"
STUB
chmod +x "$WORK/stub.sh"

export EVENTS="$WORK/events.txt"; : > "$EVENTS"
export RECAPTURE="$WORK/stub.sh"
export STUB_SECS=4
export HEARTBEAT=1
export LOGDIR="$WORK"      # never the real /tmp paths — a live lane may be using them

echo "running scheduler with -j 2 over: android macos windows ios"
"$SCRIPT" -j 2 android macos windows ios > "$WORK/out.txt" 2>&1
RC=$?

python3 - "$EVENTS" "$RC" <<'CHECK'
import sys, collections
events, rc = sys.argv[1], int(sys.argv[2])
span, measure = collections.defaultdict(dict), []
for line in open(events):
    parts = line.split()
    if parts[0] == "MEASURE":
        measure.append(int(parts[1])); continue
    span[parts[1]][parts[0]] = int(parts[2])

fail = []
lanes = set(span)
if lanes != {"android", "macos", "windows", "ios"}:
    fail.append(f"not every lane ran: {sorted(lanes)}")

def overlaps(a, b):
    if a not in span or b not in span: return False
    return span[a]["START"] < span[b]["END"] and span[b]["START"] < span[a]["END"]

# THE constraint: two UTM guests never share the host.
if overlaps("macos", "windows"):
    fail.append("macos and windows overlapped — the two VM guests must never run together")

# …and -j 2 must actually buy concurrency, or the flag is a lie.
pairs = [(a, b) for a in lanes for b in lanes if a < b and overlaps(a, b)]
if not pairs:
    fail.append("-j 2 produced no overlap at all — lanes ran serially")

# The board is one file: it must be measured exactly once, after every lane finished capturing.
if len(measure) != 1:
    fail.append(f"expected exactly 1 measure phase, saw {len(measure)}")
elif any(measure[0] < s["END"] for s in span.values()):
    fail.append("measure phase started before a lane finished capturing")

if rc != 0:
    fail.append(f"scheduler exited {rc}")

print("  overlapping pairs:", pairs or "none")
print("  lane order:", " ".join(f"{k}[{v['START']}-{v['END']}]" for k, v in sorted(span.items(), key=lambda kv: kv[1]["START"])))
if fail:
    print("\nFAIL"); [print("  -", f) for f in fail]; sys.exit(1)
print("\nPASS — every lane ran, the two VM lanes never overlapped, -j 2 bought real concurrency,")
print("       and the board was measured exactly once after all captures landed.")
CHECK
