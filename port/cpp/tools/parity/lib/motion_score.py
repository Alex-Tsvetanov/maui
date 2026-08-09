#!/usr/bin/env python3
"""Frame-by-frame, FULL-RESOLUTION scoring for the board's animated pages — the GIF parity number.

pixel_score.full_res() deliberately scores an animated cell from its sibling PNG, because the board's
GIF is 400px wide (gif.py `_SCALE`) while the still is the lane's real capture size: scoring the
downscale throws away most of the pixels and inflates SSIM. The consequence was that an animated page
was judged on ONE resting frame and its MOTION was never compared at all — a port that arrives at the
right end state through the wrong intermediate frames scored green. This module closes that: it scores
every frame of the sequence at full resolution and reports worst-frame and mean SSIM. The 400px GIF
stays a human-viewable artifact; no number is ever computed from it.

WHERE THE FRAMES COME FROM, AND THE TRADE-OFF THAT PICKS THAT SOURCE
--------------------------------------------------------------------
The full-res frames of a VM lane live in the RUN DIRECTORY the E2E runner wrote —
`docs/comparison/<YYYY-MM-DD-HH_MM_SS>/<key>/<platform>/<column>/NNNN.png` plus an `NNNN.json`
sidecar carrying `theme`, `step`, `commit`, `captured_at`. `captures/` holds only the published
at-rest still and the 400px GIF. So there are exactly two places the score could come from:

  (a) the run dir — full res, available the moment a capture finishes, but per-run, gitignored
      (docs/comparison/.gitignore), and eventually deleted: the score cannot be RECOMPUTED later.
  (b) publish the full-res frames into captures/ — reproducible forever, at the cost of roughly
      14 animated pages x 3 columns x 2 themes x 13 frames x 4 platforms ~= 4000 more PNGs in the tree.

**This module takes (a).** (b) buys reproducibility of a number that is already durable: the score is
written into comparison.json, which IS committed, together with the run id, the run's commit and the
capture date — so the number survives the frames and stays auditable. Thousands of large binaries in
git to re-derive a value already recorded is a bad trade. The cost of (a) is real and is stated
rather than hidden: a cell can only be motion-scored while its run directory still exists.

The failure mode is therefore explicit everywhere. When the frames are not available this module NEVER
returns a bare still score — it returns the still score carrying a `detail` string that SAYS "NOT
motion-scored" and why, AND a `verdict` of INVALID with an EXPIRED why-code. A single-still number
wearing a motion label is the one outcome this file exists to prevent, and prose alone did not prevent
it: the sentence was there for months while six cells scored green off one frame, because nothing
downstream aggregates English. The verdict is the machine-readable half of that same claim.

WHAT A CELL'S MOTION RESULT IS: one of PASS / FAIL / INVALID / INCONCLUSIVE, defined in the lattice
block below the constants. Read that before changing any threshold here — the distinction it draws
between "the port did not move" and "nothing was ever aimed at this page" is the reason 80 board cells
stopped reading as port findings.

WHICH FRAMES: exactly the ones the GIF was built from. `recapture.burst_frames` is imported rather
than reimplemented, so the number always describes the sequence a human can actually look at.

WHICH RUN: the newest run directory that (1) holds both columns' frames for this cell AND (2) whose
frames include the byte-identical twin of the still currently published in captures/. Rule (2) is the
staleness guard, and it is deliberately mtime-free — a git checkout rewrites every mtime, so a
timestamp test would refuse everything on a fresh clone. import_run_captures.py publishes with
shutil.copyfile, so the run that produced the board's capture is the run whose bytes match it. A newer
run that did not produce the board's still (e.g. the AppKit-only lane, or one whose import was refused)
is skipped rather than silently paired against a still it never made.

PAIRING: BY STEP NAME, never by index. Both columns of one run are driven from the SAME scenario file
(recapture.seed_scenarios + write_gif_scenarios write one dir per lane, shared by every column), so
the step names are a real join key — including the runner's generated `step{n}` names, which are a
function of the theme and step index only and so agree across columns of the same run. Index pairing
would silently re-align the whole tail of a sequence when one column drops a frame, which is exactly
the "wrong score that looks right" this tool must not produce. Frames with no partner are NOT scored
and their count is reported. If NOTHING pairs, the cell is refused, not scored on an empty set.

ALL FOUR LANES NOW KEEP FRAMES. This paragraph used to say only the VM lanes (maccatalyst, windows)
banked full-res per-step frames, because `capture_ios.capture_gif` deleted its mp4 after the ffmpeg
conversion and `capture_android.capture_gif` burst into a `tempfile.TemporaryDirectory`. Both were
changed to write run units in the same shape the VM lanes leave, so iOS and Android animated cells are
motion-scored like everything else. What still varies is the FRAME LABELLING — the VM and iOS lanes name
each frame after the scenario step that produced it, Android's burst is time-labelled (`gifNN`) — and
that difference is load-bearing in several places below; do not assume a step name means the same thing
on every lane.

Modes (all read-only — no device, no board writes):
  python3 tools/parity/lib/motion_score.py                the selftest
  python3 tools/parity/lib/motion_score.py --stability    do verdicts depend on WHICH RUN they read?
"""
from __future__ import annotations

import glob
import json
import os
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))          # tools/parity, for recapture.py

# recapture.py is a script, but its module level is only constants + path math (verified: no mkdir, no
# device call), so importing it is free and keeps ONE definition of "which frames make the GIF".
from recapture import ANIMATED, burst_frames  # noqa: E402

COMP = HERE.parents[2] / "docs" / "comparison"   # lib -> parity -> tools -> port/cpp

# board framework directory (captures/<platform>/<fw>/) -> the runner column that fed it. The inverse
# of recapture.COL_TO_DIR; kept as its own literal because pixel_score speaks in framework dirs.
FW_TO_COL = {"maui": "maui_xaml", "cpp": "cpp", "xaml": "cpp_xaml",
             "appkit_cpp": "appkit_cpp", "appkit_xaml": "appkit_xaml"}

# A column "moved" if at least this FRACTION OF ITS OWN FRAME changed between its first frame and some
# later one, and is "frozen" below the lower bound. The gap between the two is deliberate: it keeps a
# page whose animation is marginal out of BOTH buckets rather than forcing a verdict on it. Both the
# fraction and the raw pixel count are printed in the review, so a false positive is arguable from the
# text alone instead of from a constant nobody can see.
#
# FRACTIONS, NOT ABSOLUTE COUNTS, and that correction is load-bearing — it REVERSES the previous one.
# The bounds here were once percentages, were changed to absolute pixel counts (MOVED_PX/FROZEN_PX =
# 2000/800), and are now percentages again, because the measurement that justified the counts was
# MISLABELLED. That comment read:
#
#     maccatalyst, genuinely frozen (animations pinned):  0.03% of 819k = 246 px
#
# maccatalyst was NOT frozen. 246 px is the rounded reconstruction of a REAL 231-px signal: the five
# UIActivityIndicator rings on activity_indicator, spinning. Measured on run 2026-08-04-15_21_26, the
# changed pixels are five compact 15x15 boxes at x 504-518 — the rings themselves, nothing else. (The
# "animations pinned" attribute belongs to the ANDROID still pass, which does pin the animation scales;
# see recapture's module docstring. It was never true of the mac VM.) A bound of 800 px was therefore
# set ABOVE the entire real signal of a 1x lane, and every maccatalyst animated cell — including a
# perfectly matching activity_indicator — was reported "!! NOTHING MOVED" and capped yellow.
#
# WHY A FRACTION IS THE RIGHT UNIT. A widget is a fixed size in POINTS. At scale s it covers ~k*s^2
# pixels, and the frame covers A_pt*s^2 pixels, so changed/frame = k/A_pt — INDEPENDENT of s. An
# absolute count is not: the same spinner is 15x15 px on the 1x mac window and 60x60 px on the 3x
# phone, a 16x spread with no single line through it. Measured, burst frames only, >25/channel:
#
#                                         worst frozen page      activity_indicator (real motion)
#   maccatalyst  1024x800   (1x)              0 px  0.0000%          231 px   0.0282%
#   windows      1024x800   (1x)              0 px  0.0000%         3753 px   0.4581%
#   ios          1206x2622  (3x)            281 px  0.0089%         4376 px   0.1384%
#   android      1080x2340  (2.75x)           0 px  0.0000%        40095 px   1.5865%
#
# In counts, iOS's frozen 281 px OVERLAPS maccatalyst's moving 231 px — no absolute bound can separate
# them. As fractions the worst frozen reading (0.0089%) and the weakest real signal (0.0282%) are 3.2x
# apart, and the bounds below sit inside that gap with ~35% headroom under and ~41% over.
#
# The 281 px is the iOS MAUI column's H.264 floor: `simctl io recordVideo` -> ffmpeg re-quantises
# anti-aliased glyph edges, so a genuinely still page reads as isolated speckle scattered over every
# text run. It is diffuse where real motion is compact, but NOT separable by eroding or by requiring
# changed neighbours (both were measured: 3x3 erosion zeroes the 1-px-wide mac ring as well, and at
# ">=2 changed neighbours" the noise still reads 141 px against the ring's 186). Normalising by frame
# area is what works, because the noise scales with the frame just as the widget does.
MOVED_PCT = 0.020
FROZEN_PCT = 0.012
# ---- the un-decidable case: both columns moved the same distance, from the same resting frame ----
# An inertial fling is NOT reproducible on a device lane, in either column. `adb shell input swipe`
# releases at full velocity and Android's VelocityTracker/OverScroller coast a distance that varies
# run to run, so a frame captured during or after the fling is a sample of a random offset. Measured on
# emulator-5554, MAUI's own column against ITSELF across two runs of the same page:
#
#                        at rest, run-to-run     after a scroll, run-to-run (5s settle)
#   clip                       0.00%                          2.90%
#   clip_gallery               0.00%                         11.57%
#
# At rest MAUI is byte-stable, so this is the fling and nothing else — and clip_gallery's 11.57% is the
# SAME number the scored worst frame reported for that cell. The port, by contrast, measured 0.00%
# against itself: its scroll is deterministic. So a low worst-frame SSIM on such a page is a reading of
# MAUI's velocity tracker, not of the port.
#
# What IS decidable is that both columns moved, by the same amount, from the same starting frame. That
# is reported as its own verdict rather than folded into the SSIM: pixel_score caps such a cell at
# YELLOW — never green, because frame parity genuinely was not established — and the review carries
# every number so the claim can be argued from the text alone.
#
# The gate is deliberately three-part, and the third part is what keeps it honest: the RESTING frame
# must already match. A port that animates the right distance from a WRONG starting layout (selftest
# case 3, the 8px-shifted box) fails that clause and stays red, which is the whole difference between
# "we could not measure this" and "we measured nothing".
PHASE_SELF_MOTION_TOL = 0.10  # relative spread between the two columns' own motion
PHASE_AT_REST_PCT = 1.0       # how tightly the first paired (pre-gesture) frame must already agree
# THE BOARD'S GREEN BAR, defined ONCE, here. It lives in this module rather than in pixel_score purely
# because of the import direction — pixel_score imports motion_score at top level, so the reverse can
# only ever be a deferred in-function import, and a threshold nobody can reference at module scope gets
# copied instead. It was copied: these two were `PHASE_ONLY_SSIM`/`PHASE_ONLY_DIFF_PCT` carrying the
# comment "these mirror pixel_score.classify's green test", a duplicate with nothing holding it in sync.
#
# THREE readers now share this one pair, and they must agree by construction:
#   * pixel_score.classify   — is this cell green?
#   * the phase gate below   — only call a cell "not decidable" when it ISN'T already green anyway
#   * the conjunctive PASS   — motion PASS means the frames would be called green
# That last one is what makes "static green AND motion PASS" a statement about one bar rather than two.
GREEN_SSIM = 0.98
GREEN_DIFF = 1.0
# AND THE CLAUSE THE FIRST CUT OF THIS GATE WAS MISSING — the lane's motion must actually be
# irreproducible. The three clauses above describe a SIGNATURE (both moved, same distance, matching
# resting frame), and a reproducible end-state difference produces the very same signature. Measured,
# same probe, MAUI's column against itself across two runs of `clip`:
#
#   android   maui 2.90%-11.57%   port 0.00%   -> undecidable, the coast is a sample
#   ios       maui 0.00%          port 0.00%   -> fully reproducible; the maui-vs-port settled
#                                                 difference repeated at 30.03% in BOTH pairs
#
# On iOS the port scrolls to a visibly different place (its frame reveals the page's trailing "Toggle
# clip on/off" that MAUI has not reached) and does so identically every run. That is a finding, not a
# phase artifact, and without this clause the gate would have quietly forgiven three iOS cells.
# Structurally the two lanes also sample differently: the iOS driven run banks TWO frames (initial +
# settled, per-frame diff reads "0.00/29.76"), so it never sees the transient at all, while android
# banks a 12-frame burst straight through it.
#
# Membership is a MEASURED property of the lane's injector, not a preference. Re-measure before editing:
# launch one column twice, drive it, screenshot after settle, diff below the status bar.
NON_REPRODUCIBLE_DRIVE = {"android"}
# WHEN THE TWO COLUMNS' OWN MOTION DIFFERS BY THIS MUCH, SAY SO — even though the cell may still be a
# legitimate PASS. Measured across the whole board (302 PASS theme-readings carrying a motion number),
# 8 exceed 2x. The largest is gestures/android at 247x (MAUI 3311 px vs the port's 818452 in dark),
# which is NOT a false green: its light theme reads 2985 vs 2883, and the dark review already says
# "2 frame(s) had no partner and were NOT scored". The port's large change lives in frames that could
# not be PAIRED, while every step that did pair agrees to 0.00%.
#
# That combination is the point. Self-motion is measured over each column's FULL sequence (deliberately
# — a frozen column that dropped frames must not be able to hide), but the VERDICT is taken only on the
# paired intersection. So a cell can carry a 247x asymmetry and pass every clause, and nothing in the
# review said so. This does not change the verdict — the analysis above shows it would be wrong to —
# it makes the cell SELF-REPORTING, so the board screen that found these does not have to be re-run by
# hand to find the next one.
ASYMMETRY_FLAG = 2.0
# How far back to look for the run that produced the board's capture. Run dirs accumulate for weeks;
# without a bound, a cell whose run was deleted would read every surviving run's frames to prove it.
MAX_RUNS_SCANNED = 20

# --------------------------------------------------------------------------- the verdict lattice
# WHAT A MOTION VERDICT IS, AND THE CONFLATION IT EXISTS TO END.
#
# Until now the outcome of motion scoring was a bag of booleans, and one of them — `both_frozen`,
# rendered "!! NOTHING MOVED" — was carrying three unrelated claims at once:
#
#   * the port is frozen where MAUI animates          (a PORT DEFECT)
#   * the harness injected an action and NOTHING in either column reacted   (a HARNESS/AIM failure)
#   * the page has no scenario at all, so nothing was ever driven           (NO EVIDENCE AT ALL)
#
# MEASURED on this board, 2026-08-08, over the 139 cells then reading "!! NOTHING MOVED":
#
#     59  pages WITH an action scenario   -> something was injected and nothing reacted
#     80  pages with NO action scenario   -> never driven. These are 10 pages x 8 cells: animation,
#                                            chrome, empty_view_load_simulate, ios_blur_effect,
#                                            ios_pan_gesture, ios_swipe_transition, pan_gesture_events,
#                                            pointer_gesture, swipe_gesture, swipe_item_position.
#
# ELEVEN of the 14 hard-coded ANIMATED pages have no scenario file at all; the eleventh is
# activity_indicator, which is absent from the frozen bucket only because its spinners animate
# unprompted. Just THREE of the 14 — carousel_page, gestures, swipe_refresh — are actually driven.
# The other ten pages' GIFs are N copies of one frame. See PARITY_REVIEW.md for the work list.
#
# 80 cells were reading like a port finding when the true statement is "nobody wrote a scenario". A
# reader cannot act on that, and worse, cannot tell it apart from the 59 that ARE actionable. So the
# outcome becomes an explicit four-way verdict, and the distinction is IN THE VERDICT, not in prose:
#
#   PASS          valid evidence, and the two columns' motion agreed
#   FAIL          valid evidence, and it demonstrates a real difference (one animates, one is frozen)
#   INVALID       the evidence is unusable: absent, stale, unpairable, or never driven at all
#   INCONCLUSIVE  the evidence is valid but the model cannot decide equivalence from it (an
#                 irreproducible fling; a twin that structurally cannot react)
#
# PASS IS CONJUNCTIVE. It is tempting to define it as "the comparison was validly performed" and let
# the board colour carry pixel agreement — but then a cell with perfect provenance and a worst-frame
# SSIM of 0.60 reads `motion_verdict=PASS`, and every reader of that field concludes the motion
# matched. It requires BOTH valid evidence AND frames that agree to the board's own green bar, which
# is imported from pixel_score rather than restated so the two can never drift.
PASS, FAIL, INVALID, INCONCLUSIVE = "PASS", "FAIL", "INVALID", "INCONCLUSIVE"
# Precedence for collapsing several themes (or several columns) into one. FAIL first because a
# demonstrated difference outranks a missing measurement; INVALID above INCONCLUSIVE because "I have
# no evidence" is a worse position than "I have evidence I cannot decide on".
VERDICT_RANK = {FAIL: 3, INVALID: 2, INCONCLUSIVE: 1, PASS: 0}
# Why-codes, so a caller can branch on the REASON without parsing the prose. The two marked EXPIRED
# are the ones pixel_score may carry a previously recorded verdict through: they say the frames are
# gone, never that the frames disagreed. Run directories are gitignored and pruned, so without that
# distinction the board would decay with the calendar — on a fresh clone every motion cell would be
# INVALID and capped yellow purely because nobody had the run dirs, which is a statement about the
# machine and not about the port.
WHY_NO_FRAMES = "no-frames"        # EXPIRED: no run directory holds this cell's frames any more
WHY_PROVENANCE = "provenance"      # EXPIRED: frames exist but none match the published stills
WHY_UNPAIRABLE = "unpairable"      # frames exist, but no step name occurs in both columns
WHY_NOT_DRIVEN = "not-driven"      # an action WAS injected and neither column reacted
WHY_NO_SCENARIO = "no-scenario"    # no action scenario exists; this page was never driven
EXPIRED_WHY = (WHY_NO_FRAMES, WHY_PROVENANCE)


def worst_verdict(verdicts):
    """Collapse several verdicts into the one that governs, by VERDICT_RANK. None if none were given."""
    seen = [v for v in verdicts if v]
    return max(seen, key=lambda v: VERDICT_RANK.get(v, 0)) if seen else None


def _compare(path_a: str, path_b: str, crop_top: int) -> dict:
    """One frame pair -> {"ssim", "diff_pct"}. Deferred import: pixel_score imports THIS module at
    top level, so importing it back at ours would be a cycle."""
    import pixel_score  # noqa: PLC0415  see docstring

    ia, ib = pixel_score.load_pair(path_a, path_b)
    return pixel_score.score_images(ia, ib, crop_top)


def _shots(unit_dir: Path, theme: str) -> list[tuple[str, str, dict]]:
    """(step, png, sidecar) for EVERY frame this unit captured in `theme`, in capture order.

    Everything, not just the GIF's frames: the published-still match below has to be able to see the
    at-rest frame, which burst_frames drops on an undriven page."""
    out = []
    for sidecar in sorted(unit_dir.glob("*.json")):     # NNNN.json: sorted IS capture order
        try:
            meta = json.loads(sidecar.read_text())
        except (OSError, json.JSONDecodeError):
            continue
        png = sidecar.with_suffix(".png")
        if meta.get("theme") == theme and png.exists():
            out.append((str(meta.get("step") or ""), str(png), meta))
    return out


def _is_published_run(published: str, shots) -> bool:
    """Did THIS run produce the still currently published for this cell? (byte identity)

    import_run_captures.py copies with shutil.copyfile, so the run behind the board is the run holding
    a byte-identical frame. mtimes are not used on purpose — `git checkout` rewrites them all."""
    if not published or not os.path.isfile(published):
        return False
    size = os.path.getsize(published)
    want = None
    for _step, png, _meta in shots:
        if os.path.getsize(png) != size:
            continue                                    # cheap reject before reading megabytes
        if want is None:
            want = Path(published).read_bytes()
        if Path(png).read_bytes() == want:
            return True
    return False


def _pair(shots_a, shots_b) -> list[tuple[str, str, str]]:
    """Join two columns' frames by step name -> [(step, png_a, png_b)] in column A's capture order.

    Keyed by (name, Nth occurrence of that name) so a scenario that REUSES a step name — which
    import_run_captures.at_rest_steps documents as legal — pairs its repeats in order instead of
    collapsing them onto one frame. A frame whose sidecar carries NO step name is dropped rather than
    keyed on "": every such frame would share one key and pair by ordinal, which is index pairing
    wearing a step name's clothes. Dropped frames surface in the caller's unpaired count."""
    def keyed(shots):
        seen, out = {}, {}
        for step, png in shots:
            if not step:
                continue
            n = seen.get(step, 0)
            seen[step] = n + 1
            out[(step, n)] = png
        return out

    ka, kb = keyed(shots_a), keyed(shots_b)
    return [(k[0], ka[k], kb[k]) for k in ka if k in kb]   # dict order == capture order


# How far the two columns' sampling may be out of step before a shift stops being a shift. The burst
# schedule is 12 frames over 4s (~0.33s apart) and a screencap costs ~0.13s, so a drift of one or two
# samples is ordinary; beyond three the sequences are not the same motion seen late, they are different
# motion.
MAX_PHASE_SHIFT = 3


def _align(pairs, crop_top: int):
    """[(step, png_a, png_b)] -> (aligned_pairs, offset): column B shifted by the offset that makes the
    two sequences agree BEST, and the offset it chose.

    WHY THIS EXISTS. Pairing frame k to frame k asserts that both columns are at the same point of the
    same animation at sample k. Nothing enforces that: the burst names encode the REQUESTED schedule
    (`gif01@4s/12f`), each screencap costs ~0.13s of its own, and that cost does not accumulate
    identically in MAUI's app and in the port. capture_android's own docstring has said so from the
    start — "frames nominally the same moment can be tens of milliseconds apart in wall clock ... a
    frame-by-frame parity signal, not a timing measurement" — and this scorer did not honour it.

    The cost was real and measured, not theoretical:
      * gestures/android scored RED at worst SSIM 0.8820 with 34.42% of pixels differing on a frame BOTH
        columns label `at-rest`, on a page where a direct `adb shell input tap` proves the port reacts
        exactly as MAUI does (readout "(none)" -> "Pointer released", self-motion 2985 vs 2883 px).
      * 13 cells moved in one rescore with NO change to scoring logic; 8 of the 13 landed on PHASE ONLY,
        the verdict that exists for this artifact and that its own comment says cannot be widened
        because "a reproducible end-state difference produces the very same signature".
      * clip_views, entry and scroll_view flipped between two runs of byte-identical code.

    A SINGLE GLOBAL OFFSET, not per-frame free choice. A timing skew is one offset for the whole
    sequence; letting every frame pick its own partner would let a port that visits the right states in
    the WRONG ORDER score as well as one that matches, which is the property this board exists to catch.
    The offset is chosen on the cheap changed-pixel metric (no SSIM in the search) and reported, so a
    reader can see how far the columns drifted rather than infer it.
    """
    if len(pairs) < 2:
        return pairs, 0
    best, best_off = None, 0
    for off in range(-MAX_PHASE_SHIFT, MAX_PHASE_SHIFT + 1):
        overlap = [(pairs[i][0], pairs[i][1], pairs[i + off][2])
                   for i in range(len(pairs)) if 0 <= i + off < len(pairs)]
        if len(overlap) < max(2, len(pairs) - MAX_PHASE_SHIFT):
            continue                     # too little left to judge: a shift that discards the sequence
        cost = sum(_changed(a, b, crop_top)[0] for _s, a, b in overlap) / len(overlap)
        if best is None or cost < best[0]:
            best, best_off = (cost, overlap), off
    return (best[1], best_off) if best else (pairs, 0)


def _has_action_scenario(key: str, comp: Path) -> bool:
    """Does this page have an authored scenario that INJECTS something?

    True only for a scenario carrying at least one step with an `action` — a settle-only file
    (web_view's 5s wait) drives nothing and must keep the undriven frame selection. Missing file or
    unreadable TOML is False: the frames' own step names then decide, exactly as before."""
    import tomllib
    f = comp / "scenarios" / f"{key}.toml"
    try:
        return any(s.get("action") for s in tomllib.loads(f.read_text()).get("steps", []))
    except (OSError, tomllib.TOMLDecodeError):
        return False


def _step_rois(key: str, comp: Path, plat_dir: str = "") -> dict:
    """{step name: (x0, y0, x1, y1)} for every step declaring a `roi`, as 0..1 fractions.

    WHY THIS EXISTS — a green this scorer earned honestly and that was still wrong. On
    button/maccatalyst (run 2026-08-09-18_48_05) MAUI's column left its readout at "Taps: 0" while both
    port columns advanced to "Taps: 1". The columns ended in DIFFERENT LOGICAL STATES and the cell
    scored PASS, because the whole-frame numbers could not see it: one digit is ~41 px of 819,200,
    about 0.005% of the frame, and worst-frame SSIM read 0.9897 with 0.40% differing — comfortably
    inside the green bar.

    No threshold tightening fixes that. The signal is not small NOISE, it is a small SIGNAL, and any
    bar low enough to catch a changed digit would red every page with an antialiasing difference. What
    is missing is not sensitivity but LOCATION: the scenario knows where its reaction is supposed to
    appear, and nothing was asking it.

    So a step may declare `roi = [x0, y0, x1, y1]` — the region that MUST react. Fractions of the frame,
    the same space as `at`, so one declaration serves every lane. Deliberately GENEROUS rather than
    tight: it is a "the readout lives around here" box, not a bounding box, and it only has to separate
    the reaction area from the rest of the page.

    PER-LANE, via `roi_<board platform>` (roi_ios / roi_android / roi_maccatalyst / roi_windows), and
    that is FORCED rather than symmetric-with-`at` for tidiness. Measured on the `button` page, the
    readout and the first button below it:

        ios          readout y 0.088..0.103    first button starts 0.123
        maccatalyst  readout y 0.055..0.066    first button starts 0.084
        windows      readout y ~0.068          first button starts 0.088
        android      readout y ~0.073..0.085   first button starts 0.102

    iOS's readout sits BELOW maccatalyst's first button, so no single portable y-range can contain every
    lane's readout while excluding every lane's buttons — and an ROI that swallows the button is worse
    than none, because the button reacting would satisfy it and the gate would pass the very case it
    exists to catch. Unlike `at`, this is resolved HERE rather than by run_comparison.for_lane: that
    helper promotes only `at`/`to` and is keyed by ENV name, while this needs the BOARD platform, which
    is what score_cell already has.

    Missing file, unreadable TOML or no `roi` key all mean "no ROI declared", and the cell scores
    exactly as it did before — this gate can only ever ADD a finding, never remove one."""
    import tomllib
    f = comp / "scenarios" / f"{key}.toml"
    out = {}
    try:
        doc = tomllib.loads(f.read_text())
    except (OSError, tomllib.TOMLDecodeError):
        return out
    for step in doc.get("steps", []):
        name = step.get("name")
        roi = step.get(f"roi_{plat_dir}") if plat_dir else None
        if roi is None:
            roi = step.get("roi")
        if name and isinstance(roi, list) and len(roi) == 4:
            out[str(name)] = tuple(float(v) for v in roi)
    return out


# THE SENSITIVITY FLOOR INSIDE A DECLARED REGION, and why it is not DIFF_THRESHOLD.
#
# pixel_score.DIFF_THRESHOLD = 25 answers "would a human see a difference between these two COLUMNS" —
# a visibility question, for which 25 is right. Inside a declared `roi` the question is different: "did
# the thing the scenario author pointed at change at all". A control can answer that with an amplitude
# no human would notice.
#
# MEASURED on picker/windows (run 2026-08-09-20_13_27), the WinUI ComboBox's focus state:
#
#   threshold  >0     >1     >2     >3     >5     >8    >25
#   changed    29456  29454  29446  29434  29410     0      0     max per-pixel delta: 6
#
# 29,434 pixels — the full width of the control — change by at most SIX values. At 25 that reads as a
# dead page, and all three columns scored INVALID/`not-driven` on a cell where they agree TO THE PIXEL
# (29434 each, identical box). The usable range is 1..5; 3 sits in the middle with margin either side.
#
# CHECKED IN THE OTHER DIRECTION on button/maccatalyst's readout region, where the answer is also known:
# MAUI's column is max=0 there — byte-identical, the handler genuinely did not fire — while both port
# columns read 53 px at >3. So 3 separates "did not react" from "reacted faintly" on both cases with a
# known answer, which is the entire population available today.
#
# WHY NO GLOBAL FLOOR WAS DERIVED INSTEAD: it could not be. Repeat captures of the same frame differ for
# reasons that are not capture noise (clocks, dates, rebuilt apps), so the stored runs yield no clean
# measurement — see PARITY_REVIEW's "The step-paired noise floor CANNOT be derived". A declared region
# needs none, because the region itself bounds where noise could come from.
ROI_DIFF_THRESHOLD = 3


def _roi_changed(first_png: str, later_png: str, roi, crop_top: int) -> int:
    """Changed pixels INSIDE `roi` between two frames of the SAME column. 0 when nothing moved there.

    Uses ROI_DIFF_THRESHOLD, NOT the whole-frame visibility threshold — see the block above."""
    import numpy as np  # noqa: PLC0415
    import pixel_score  # noqa: PLC0415

    ia, ib = pixel_score.load_pair(first_png, later_png)
    w, h = ia.size
    if crop_top > 0 and h > crop_top:
        ia, ib = ia.crop((0, crop_top, w, h)), ib.crop((0, crop_top, w, h))
        w, h = ia.size
    x0, y0, x1, y1 = roi
    box = (max(0, int(x0 * w)), max(0, int(y0 * h)), min(w, int(x1 * w)), min(h, int(y1 * h)))
    if box[2] <= box[0] or box[3] <= box[1]:
        return 0
    a = np.asarray(ia.crop(box).convert("RGB"), dtype=np.int16)
    b = np.asarray(ib.crop(box).convert("RGB"), dtype=np.int16)
    return int((np.max(np.abs(a - b), axis=-1) > ROI_DIFF_THRESHOLD).sum())


def _twin_cannot_react(key: str, comp: Path) -> bool:
    """Does this page's scenario declare that the MAUI ground-truth column is static by construction?

    True only for a scenario carrying `twin_cannot_react = true`. Missing file, unreadable TOML or a
    missing key all mean False — the exemption has to be asked for explicitly, never inferred."""
    import tomllib
    f = comp / "scenarios" / f"{key}.toml"
    try:
        return bool(tomllib.loads(f.read_text()).get("twin_cannot_react", False))
    except (OSError, tomllib.TOMLDecodeError):
        return False


def _self_motion(pngs: list[str], crop_top: int) -> tuple[float, int]:
    """How much a column moved ON ITS OWN -> (percent, PIXEL COUNT) of the largest change between its
    first frame and any later one.

    This is the ONLY measurement that separates "both columns are static and identical" (fine) from
    "MAUI animates and the port is frozen" (the finding this tool exists for) — a frame-vs-frame SSIM
    between the two columns cannot tell those apart when the motion is small.

    The PERCENT is what the verdict is taken on (see MOVED_PCT/FROZEN_PCT); the count is carried only
    so the review text can quote both, since a raw pixel count is what makes a small percentage real.

    NOT routed through _compare(): score_images rounds diff_pct to 2 decimals and the verdict reads
    fractions near 0.01%, where that rounding IS the signal — the previous code recovered a count by
    multiplying the rounded percent back out by the frame area, which turned a measured 231 px into
    246 and put the wrong number in the comment that set the bounds. It also skips the SSIM, which
    self-motion never looks at."""
    if len(pngs) < 2:
        return 0.0, 0
    best_pct, best_px = 0.0, 0
    for p in pngs[1:]:
        pct, px = _changed(pngs[0], p, crop_top)
        if px > best_px:
            best_pct, best_px = pct, px
    return best_pct, best_px


def _changed(path_a: str, path_b: str, crop_top: int) -> tuple[float, int]:
    """One frame pair -> (percent of comparable pixels that differ, that same count), UNROUNDED.

    Same mask score_images builds — pixel_score.load_pair and pixel_score.DIFF_THRESHOLD are imported
    rather than restated so "visibly different" can never mean two things — and the percent is the
    mask's own mean, so area and count agree by construction with crop_top excluded from both."""
    import numpy as np  # noqa: PLC0415  keeps module import free of numpy/PIL for --plan/--selftest
    import pixel_score  # noqa: PLC0415  see _compare on the import cycle

    ia, ib = pixel_score.load_pair(path_a, path_b)
    if crop_top > 0:
        w, h = ia.size
        if h > crop_top:
            ia, ib = ia.crop((0, crop_top, w, h)), ib.crop((0, crop_top, w, h))
    a = np.asarray(ia.convert("RGB"), dtype=np.int16)
    b = np.asarray(ib.convert("RGB"), dtype=np.int16)
    mask = np.max(np.abs(a - b), axis=-1) > pixel_score.DIFF_THRESHOLD
    return float(mask.mean() * 100.0), int(mask.sum())


def _run_dirs(comp: Path, plat_dir: str, key: str) -> list[Path]:
    """Run directories that captured this page on this platform, NEWEST FIRST."""
    runs = sorted(comp.glob("20??-??-??-*"), key=lambda p: p.name, reverse=True)
    return [r for r in runs if (r / key / plat_dir).is_dir()][:MAX_RUNS_SCANNED]


def find_frames(key, plat_dir, fw_dir, theme, published_maui, published_other, comp=COMP,
                only_run=None):
    """The newest run holding BOTH columns' frames for this cell behind the published stills.

    -> (run_dir, shots_maui, shots_other, why_not). Exactly one of run_dir / why_not is set. `why_not`
    is a (WHY_* code, prose) pair: the code so a caller can tell EXPIRED evidence from contradicted
    evidence without parsing English, the prose because the code alone never explains itself."""
    col_m, col_o = FW_TO_COL["maui"], FW_TO_COL.get(fw_dir, fw_dir)
    saw_frames = False
    scanned = [only_run] if only_run is not None else _run_dirs(comp, plat_dir, key)
    for run in scanned:
        dm, do = run / key / plat_dir / col_m, run / key / plat_dir / col_o
        if not (dm.is_dir() and do.is_dir()):
            continue
        sm, so = _shots(dm, theme), _shots(do, theme)
        if not (sm and so):
            continue
        saw_frames = True
        # BOTH columns must be the ones behind the board, not just one: a mixed pairing would compare
        # two different runs' builds and report the difference as a port bug. `only_run` deliberately
        # BYPASSES that tie — it exists solely for --stability, which asks "would a DIFFERENT run have
        # produced a different verdict?" and therefore has to look at runs that are not the board's.
        # It must never be reachable from the scoring path, and it isn't: score_cell's own parameter is
        # keyword-only and pixel_score never passes it.
        if only_run is not None or (_is_published_run(published_maui, sm)
                                    and _is_published_run(published_other, so)):
            return run, sm, so, None
    if saw_frames:
        # The scan window is named, because "I did not find it" and "I stopped looking" are different
        # claims and this file's whole point is that no failure reads as something it is not.
        capped = " (the newest %d were scanned)" % MAX_RUNS_SCANNED if len(scanned) == MAX_RUNS_SCANNED else ""
        return None, None, None, (WHY_PROVENANCE,
                                  f"no run directory holds the frames behind the CURRENTLY PUBLISHED "
                                  f"stills for both columns{capped} — their frames do not match "
                                  f"captures/ byte-for-byte, so re-capture this page")
    return None, None, None, (WHY_NO_FRAMES,
                              f"no run directory under docs/comparison/ has {theme} frames for both "
                              f"columns of this cell (run dirs are per-run and gitignored, so this "
                              f"says the evidence EXPIRED — never that it disagreed)")


def score_cell(key, plat_dir, fw_dir, theme, crop_top, still, comp=COMP, fw_label=None, *,
               only_run=None):
    """The motion score for one (page, platform, framework, theme), shaped for pixel_score.classify.

    Returns the same {"ssim", "diff_pct"} contract classify() already reads — with `ssim`/`diff_pct`
    set to the WORST frame, so the existing thresholds judge the worst moment rather than an average
    that a long static tail can hide — plus:
      detail    the review sentence (mean/worst/per-frame diffs/frame counts/provenance)
      mismatch  True when one column moved and the other did not; pixel_score forces RED on it
      verdict   PASS / FAIL / INVALID / INCONCLUSIVE — see the lattice block above
      why       the WHY_* code behind a non-PASS verdict, or "" — branchable without parsing prose
      evidence  {run, commit, captured_at} of the frames the verdict was taken on, or None

    `still` is the single-frame score pixel_score already computed. It is returned UNCHANGED except
    for a `detail` that says the page was NOT motion-scored whenever the frames are unavailable, and
    None stays None (A CELL WITH NO COMPARABLE PAIR STAYS BLANK AND CARRIES NO VERDICT — an absent
    screenshot is not invalid motion evidence, it is no cell at all, and stamping INVALID on it would
    turn every page missing a dark capture into a motion finding)."""
    label = fw_label or fw_dir

    def not_scored(why_pair):
        why_code, why = why_pair
        if still is None:
            return None
        # "single frame" rather than "still": on a cell whose PNG is missing, full_res() leaves
        # pixel_score scoring the 400px GIF, and calling that a still would misstate it twice over.
        return dict(still, verdict=INVALID, why=why_code, evidence=None,
                    detail=f"SSIM {still['ssim']:.4f}, {still['diff_pct']:.2f}% pixels differ "
                           f"(single frame only) — NOT motion-scored: {why}")

    pub_m = str(comp / "captures" / plat_dir / "maui" / f"{key}_{theme}.png")
    pub_o = str(comp / "captures" / plat_dir / fw_dir / f"{key}_{theme}.png")
    run, shots_m, shots_o, why = find_frames(key, plat_dir, fw_dir, theme, pub_m, pub_o, comp,
                                             only_run=only_run)
    if run is None:
        return not_scored(why)

    # The GIF's own frames, in the GIF's own order, via the GIF's own selector.
    dm = run / key / plat_dir / FW_TO_COL["maui"]
    do = run / key / plat_dir / FW_TO_COL.get(fw_dir, fw_dir)
    burst_m = {p: s for s, p, _ in shots_m}
    burst_o = {p: s for s, p, _ in shots_o}
    # A page with an AUTHORED SCENARIO was driven, whatever this lane happened to call the frames — see
    # burst_frames' `driven` note. Passing it keeps the at-rest BEFORE on Android, where the burst is
    # labelled by time (gifNN) rather than by step and the inference silently reads "undriven".
    # OFF, and this is the second time it was switched on and had to come back off. Both failures are
    # recorded because the mechanism is right and the next person will want to try it again.
    #
    #   attempt 1 (ccb057fcca): forced driven=True while capture_android still spliced the MAIN pass's
    #     still in as `initial`. That still is shot under different device state than the burst, so
    #     hit_testing/dark paired MAUI's at 66.4 mean luma against the port's at 41.6 — "89.63% differ"
    #     on a page whose motion is identical in both columns. 269g/58y/17r -> 259g/56y/29r.
    #   attempt 2 (4c3444010a + this): fixed the frame — capture_gif now shoots the at-rest frame INSIDE
    #     the burst, verified matching luma (227.3 vs 227.2) and identical 2985 px self-motion in both
    #     columns. Board went 269g -> 255g anyway. CAUSE: `initial` was doing DOUBLE DUTY. find_frames
    #     (:346) requires the run's frames to match the PUBLISHED STILL byte-for-byte — `initial` is the
    #     provenance witness that ties a run unit to the board. Replacing it with a fresh shot breaks
    #     that tie, so find_frames REJECTS the new run and silently falls back to an older one whose
    #     `initial` still matches — scoring the very frames the fix removed.
    #
    # The real fix needs BOTH: the published still kept as the provenance witness AND the burst at-rest
    # frame carried under its own step name, with find_frames matching the former and the motion
    # selector using the latter. Until that exists, the name-based inference stands and `gestures` stays
    # mis-scored on Android — the smallest of the three wrongs.
    was_driven = None
    sel_m = [(burst_m.get(p, ""), p) for p in burst_frames(dm, theme, driven=was_driven)]
    sel_o = [(burst_o.get(p, ""), p) for p in burst_frames(do, theme, driven=was_driven)]
    pairs = _pair(sel_m, sel_o)
    if not pairs:
        return not_scored((WHY_UNPAIRABLE,
                           f"run {run.name} has {len(sel_m)} MAUI and {len(sel_o)} {label} frames but "
                           f"NO step name occurs in both, so nothing can be paired — a comparison by "
                           f"frame index would be a guess. Re-capture this page"))

    # PHASE-INVARIANT: shift column B by the offset that makes the sequences agree best before scoring.
    # A sampling drift is a shift, not a defect; see _align for the measurements that forced this.
    pairs, phase_shift = _align(pairs, crop_top)
    scores = [_compare(a, b, crop_top) for _step, a, b in pairs]
    ssims = [s["ssim"] for s in scores]
    worst_i = min(range(len(ssims)), key=lambda i: ssims[i])
    mean_ssim = sum(ssims) / len(ssims)
    # Self-motion is measured over each column's OWN FULL sequence, never over the paired intersection.
    # Pairing drops any frame whose step has no partner, so measuring motion there goes blind in exactly
    # the case this detector exists for: if the port froze and dropped frames, the survivors are the
    # ones that matched, and the intersection can look calm on both sides while the raw sequences do
    # not. The pairing is for COMPARING the columns; motion is a property of one column alone.
    move_m, px_m = _self_motion([p for _s, p in sel_m], crop_top)
    move_o, px_o = _self_motion([p for _s, p in sel_o], crop_top)
    # WHICH FLOOR: the percentage bounds above were calibrated against BURST frames, whose noise is a
    # VIDEO artifact — `simctl io recordVideo` -> ffmpeg re-quantises anti-aliased glyph edges, so a
    # still iOS page reads 281 px of speckle. A STEP-PAIRED sequence has no encoder in it at all: it is
    # two PNG screenshots of the same window. Applying the burst floor to it rejects real signal.
    #
    # MEASURED over every step pair in this repo's 2026-08-06/07 runs (1510 pairs, gif frames excluded):
    #
    #                pairs   exactly 0 px   smallest NON-zero
    #   maccatalyst    749         249         22 px (0.0027%)   <- stepper's "-" glyph re-enabling
    #   windows        394         146         35 px (0.0043%)   <- the same glyph
    #   ios            367          18        245 px (0.0078%)   <- the same glyph at 3x
    #
    # The population is EXACTLY ZERO or it is a real reaction; there is no small-noise band on any lane,
    # so the step-paired floor needs no threshold and gets none — moved iff any pixel changed. That is
    # strictly sharper than a percentage in both directions: stepper's 22/35/245 px stop reading as
    # "!! NOTHING MOVED" when all three columns agree perfectly, AND a 22-px reaction present in MAUI
    # and absent in the port now raises a MISMATCH that 0.012% silently swallowed.
    # ---- the DECLARED REACTION REGION, measured BEFORE the frozen determination below because its
    # answer overrides that one. See _step_rois for the green this exists to stop, and
    # ROI_DIFF_THRESHOLD for why it uses its own floor.
    roi_by_step = _step_rois(key, comp, plat_dir)
    roi_split = []                       # (step, maui px, other px) where only ONE column reacted
    roi_moved_m = roi_moved_o = False    # did EITHER column change inside any declared region at all
    if roi_by_step and pairs:
        first_m, first_o = pairs[0][1], pairs[0][2]
        for step_name, path_m, path_o in pairs:
            roi = roi_by_step.get(step_name)
            if roi is None:
                continue
            px_roi_m = _roi_changed(first_m, path_m, roi, crop_top)
            px_roi_o = _roi_changed(first_o, path_o, roi, crop_top)
            if (px_roi_m > 0) != (px_roi_o > 0):
                roi_split.append((step_name, px_roi_m, px_roi_o))
            roi_moved_m = roi_moved_m or px_roi_m > 0
            roi_moved_o = roi_moved_o or px_roi_o > 0

    step_paired = not any(s.startswith("gif") for s, _p in sel_m + sel_o)
    # A page whose GROUND TRUTH cannot react. The shared XAML twins deliberately omit every Clicked /
    # GestureRecognizer, so on those pages the maui_xaml column is static BY CONSTRUCTION while the
    # port's code-first builder does wire the handler. `button` is the case: the twin carries a literal
    # <Label Text="Taps: 0"/> and the comment "<!-- Clicked (handler omitted) -->", and the port's
    # readout goes "Taps: 0" -> "Taps: 1" on the same click. That asymmetry is AUTHORED, not a defect —
    # it can only ever produce a mismatch, so a red there accuses the port of the twin's omission.
    # Flagged in the scenario rather than hard-coded here, so the batched twin-markup change that adds
    # those handlers retires the exemption by deleting one line.
    asymmetric = _twin_cannot_react(key, comp)
    # Was anything ever AIMED at this page? Splits the "neither column moved" verdict below into its
    # actionable and non-actionable halves — see the lattice block for the 59/80 measurement.
    driven_page = _has_action_scenario(key, comp)
    if step_paired:
        m_moved, m_frozen = px_m > 0, px_m == 0
        o_moved, o_frozen = px_o > 0, px_o == 0
    else:
        m_moved, m_frozen = move_m >= MOVED_PCT, move_m <= FROZEN_PCT
        o_moved, o_frozen = move_o >= MOVED_PCT, move_o <= FROZEN_PCT
    # A DECLARED REGION ALSO ANSWERS "DID THIS COLUMN MOVE", independently of the whole-frame floor.
    # picker/windows is why: all three columns change 29,434 px inside the control at an amplitude of 6,
    # so _self_motion — which asks the VISIBILITY question at 25/channel — reads 0 for every column and
    # the cell scores `both_frozen` -> INVALID/`not-driven`, on a cell where the three agree to the
    # pixel. The author pointed at that region and said the reaction belongs there; a change inside it
    # IS the page reacting, whatever its amplitude. It can only ever UN-freeze: with no roi declared
    # both flags stay False and every verdict below is exactly what it was.
    if roi_moved_m:
        m_moved, m_frozen = True, False
    if roi_moved_o:
        o_moved, o_frozen = True, False
    mismatch = (m_moved and o_frozen) or (o_moved and m_frozen)
    # NEITHER column moved on a page the board calls animated. That is not parity — it is the original
    # bug: a page nobody managed to drive, whose GIF is N copies of one frame. Two frozen columns match
    # each other perfectly and would otherwise score a confident green, which is precisely the
    # "did nothing, reported success" outcome this whole pass exists to make impossible. It cannot be
    # graded from SSIM, because the SSIM is 1.0 — it has to be its own verdict.
    both_frozen = m_frozen and o_frozen
    # See the PHASE_* block: both moved, by the same amount, from an already-matching resting frame.
    at_rest_diff = scores[0]["diff_pct"] if scores else 100.0
    widest = max(move_m, move_o)
    spread = abs(move_m - move_o) / widest if widest > 0 else 1.0
    # The board's OWN green bar, imported rather than restated. This used to be two module constants
    # (PHASE_ONLY_SSIM/PHASE_ONLY_DIFF_PCT) whose comment already said "these mirror
    # pixel_score.classify's green test" — a copy that nothing stopped from drifting. It now feeds both
    # the phase gate and the conjunctive PASS clause, so "motion PASS" and "the board would call these
    # frames green" cannot mean two different things.
    frames_agree = ssims[worst_i] >= GREEN_SSIM and scores[worst_i]["diff_pct"] <= GREEN_DIFF
    frames_disagree = not frames_agree
    phase_only = (plat_dir in NON_REPRODUCIBLE_DRIVE and frames_disagree
                  and not mismatch and m_moved and o_moved
                  and spread <= PHASE_SELF_MOTION_TOL and at_rest_diff <= PHASE_AT_REST_PCT)

    meta = shots_m[0][2]
    prov = (f"run {run.name}, commit {meta.get('commit', '?')}, "
            f"{str(meta.get('captured_at', '?'))[:10]}")
    dropped = (len(sel_m) - len(pairs)) + (len(sel_o) - len(pairs))
    unpaired = (f"; {dropped} frame(s) had no partner and were NOT scored" if dropped else "")
    per_frame = "/".join(f"{s['diff_pct']:.2f}" for s in scores)
    shifted = (f"; column frames realigned by {phase_shift:+d} sample(s) — a sampling drift, not a "
               f"defect (see _align)" if phase_shift else "")
    detail = (f"MOTION {len(pairs)} frames paired by step ({prov}){unpaired}{shifted} — "
              f"worst SSIM {ssims[worst_i]:.4f} at frame {worst_i + 1} '{pairs[worst_i][0]}' "
              f"({scores[worst_i]['diff_pct']:.2f}% pixels differ), mean SSIM {mean_ssim:.4f}; "
              f"per-frame diff% {per_frame}; self-motion MAUI {move_m:.4f}% ({px_m} px) vs "
              f"{label} {move_o:.4f}% ({px_o} px)")
    # See ASYMMETRY_FLAG. Appended to the detail rather than folded into the verdict: a large ratio is a
    # reason to LOOK, not a finding, and on the one cell where it is largest the pixels are innocent.
    asym_hi, asym_lo = max(px_m, px_o), min(px_m, px_o)
    if asym_lo > 0 and asym_hi / asym_lo >= ASYMMETRY_FLAG:
        louder = "MAUI" if px_m > px_o else label
        unpaired_note = (" Its own frames did not all pair (see the frame count above), so part of that "
                         "motion was never compared at all." if dropped else "")
        detail += (f"; !! SELF-MOTION ASYMMETRY {asym_hi / asym_lo:.0f}x — {louder} moved far more over "
                   f"its OWN sequence than the other column did. The verdict is taken on the PAIRED "
                   f"frames only, so this does not by itself contradict it, and it is not treated as a "
                   f"defect.{unpaired_note} It is flagged because a ratio this size means the two "
                   f"columns did visibly different amounts of work and the paired numbers cannot say "
                   f"why")
    if roi_split:
        step_name, px_roi_m, px_roi_o = roi_split[0]
        reacted, silent = ("MAUI", label) if px_roi_m else (label, "MAUI")
        detail = (f"!! DECLARED REACTION REGION SPLIT at step '{step_name}': {reacted} changed "
                  f"{max(px_roi_m, px_roi_o)} px inside the scenario's `roi` and {silent} changed NONE. "
                  f"The columns end in different logical states, and the whole-frame numbers below "
                  f"cannot show it — a readout is a rounding error against a full page (measured: one "
                  f"digit is ~41 px of 819,200, 0.005%). This is why the region is declared. {detail}")
    # The exemption applies ONLY in the authored direction — the port moved and MAUI did not. If MAUI
    # reacts and the PORT is frozen on such a page, the twin's omission cannot explain it and the
    # mismatch is exactly as damning as anywhere else, so it is left alone.
    authored_asymmetry = bool(mismatch and asymmetric and o_moved and m_frozen)
    if authored_asymmetry:
        detail = (f"AUTHORED ASYMMETRY, not a port defect: {label} reacted ({px_o} px) and MAUI did not "
                  f"({px_m} px), on a page whose shared XAML twin deliberately OMITS the handler — the "
                  f"ground-truth column has nothing to react WITH, so no motion parity can be "
                  f"established here either way. Retire this by adding the handler to the twin, not by "
                  f"changing the port. {detail}")
    elif mismatch:
        still_side, moved_side = ("MAUI", label) if m_frozen else (label, "MAUI")
        # FIRST in the string and in caps, because this is the finding the whole pass exists to make.
        # A page where one column animates and the other is frozen can still score a high per-frame
        # SSIM (a spinner is a few hundred pixels), so it must not be left to the number to reveal.
        detail = (f"!! MOTION MISMATCH: {moved_side} ANIMATES and {still_side} IS FROZEN "
                  f"({max(move_m, move_o):.4f}% vs {min(move_m, move_o):.4f}% of its own frame changed "
                  f"across the sequence) — the end state may match while the animation does not. "
                  f"{detail}")
    elif phase_only:
        detail = (f"!! PHASE ONLY, NOT DECIDABLE ON THIS LANE: MAUI and {label} both moved and moved the "
                  f"SAME distance ({move_m:.4f}% vs {move_o:.4f}% of their own frame, {spread * 100:.1f}% "
                  f"apart) from a resting frame that already agreed to {at_rest_diff:.2f}%. What differs "
                  f"is WHEN, not whether or how far. An `input swipe` releases at full velocity and the "
                  f"fling coasts a random distance — measured on THIS lane, MAUI's own column differs "
                  f"from ITSELF by up to 11.57% across two runs of the same page while it is byte-stable "
                  f"at rest — so the per-frame SSIM below samples two different moments of the same "
                  f"motion. Capped YELLOW: frame parity was NOT established, and no port defect is "
                  f"evidenced either. "
                  f"{detail}")
    elif both_frozen:
        bound = "a single pixel" if step_paired else f"{FROZEN_PCT}% of its own frame"
        # THE SPLIT THAT 80 CELLS WERE MISSING (see the verdict lattice block). "Neither column moved"
        # means two completely different things depending on whether anything was ever AIMED at this
        # page, and only one of them is worth a human's time.
        if driven_page:
            cause = (f"An action WAS injected here — {key}.toml declares one — and NEITHER column "
                     f"reacted to it. So either the coordinate misses its target on this lane, or the "
                     f"interaction is not reachable here. This is the actionable half of the old "
                     f"'NOTHING MOVED' bucket: 59 of the 139 cells that carried it.")
        else:
            cause = (f"NO ACTION SCENARIO EXISTS for this page — docs/comparison/scenarios/{key}.toml "
                     f"is absent or declares no `action` — so nothing was ever aimed at it and both "
                     f"columns are at rest by construction. This is NOT a port finding and nothing "
                     f"about the port can be concluded from it; the missing artifact is the scenario. "
                     f"80 of the 139 cells that carried the old 'NOTHING MOVED' banner were this, "
                     f"including 10 of the 14 hard-coded ANIMATED pages.")
        detail = (f"!! NO MOTION EVIDENCE: neither MAUI nor {label} changed by more than {bound} "
                  f"across the sequence ({px_m} px vs {px_o} px), on a page the board treats as "
                  f"ANIMATED. {cause} {detail}")

    # ---- the verdict. Order mirrors VERDICT_RANK, so the first clause that fires is the governing one.
    if roi_split:
        # FIRST, ahead of every other clause: this is the one finding the whole-frame numbers actively
        # HIDE, so anything that reads them — including the frames_agree test below — would overrule it.
        verdict, why = FAIL, "roi-split"
    elif mismatch and not authored_asymmetry:
        verdict, why = FAIL, "mismatch"
    elif both_frozen:
        # INVALID, never FAIL: two frozen columns agree perfectly and would otherwise read as a
        # confident green. Nothing was demonstrated about the port either way — the evidence is
        # missing, and the why-code says which kind of missing.
        verdict, why = INVALID, (WHY_NOT_DRIVEN if driven_page else WHY_NO_SCENARIO)
    elif phase_only:
        verdict, why = INCONCLUSIVE, "phase-only"
    elif authored_asymmetry:
        verdict, why = INCONCLUSIVE, "twin-cannot-react"
    elif frames_agree:
        verdict, why = PASS, ""
    else:
        # Valid evidence, both columns moved, and the frames still do not agree to the board's green
        # bar. That IS the demonstrated difference FAIL is for. It does not force the cell red — the
        # board's own thresholds decide the colour — so a FAIL on a yellow cell is a real and useful
        # statement: the frames genuinely disagree, and the static bands call that minor.
        verdict, why = FAIL, "frames-disagree"

    # Only what a caller USES: pixel_score writes {status, review} into the slot, so every number is
    # carried by `detail` (the review sentence) rather than duplicated into keys nothing reads back.
    # `evidence` is the exception and is deliberately STRUCTURED: pixel_score records it in the
    # committed comparison.json so a verdict outlives the gitignored run directory it came from.
    return {"ssim": round(ssims[worst_i], 4), "diff_pct": round(scores[worst_i]["diff_pct"], 2),
            "detail": detail, "mismatch": mismatch, "both_frozen": both_frozen,
            "phase_only": phase_only, "authored_asymmetry": authored_asymmetry,
            "verdict": verdict, "why": why,
            "evidence": {"run": run.name, "commit": meta.get("commit", "?"),
                         "captured_at": str(meta.get("captured_at", "?"))[:10]}}


# --------------------------------------------------------------------------- cross-run stability
def stability(comp=COMP, only=None, platforms=None, max_runs=4) -> int:
    """`python3 lib/motion_score.py --stability` — does a cell's verdict depend on WHICH RUN it reads?

    THE GATE THIS REPLACES, AND WHY THE OBVIOUS ONE IS USELESS. The stated intent was "require two
    consecutive scoring runs to agree". But scoring is a pure function of the frames on disk: run it
    twice on unchanged input and it agrees BY CONSTRUCTION, so that gate can only ever pass. It would
    have been a green check that tested nothing — the exact failure shape this whole pass is against.

    The flapping that was actually observed (13 cells changing verdict in one rescore with no logic
    change; three cells differing between byte-identical board states) came from WHICH RUN DIRECTORY
    got selected, not from the arithmetic. So the variable to vary is the run. This scores every motion
    cell against the newest `max_runs` directories that hold both its columns' frames, and reports each
    cell whose verdict is not the same in all of them.

    A disagreeing cell is not automatically a bug — a genuine recapture between runs SHOULD change a
    verdict. It is a cell whose reported verdict is a function of run selection, which is the thing
    that must be known before anyone ANDs this layer into the board's colour. Read-only: no writes.
    """
    import pixel_score  # noqa: PLC0415  see _compare on the import cycle

    pages = json.loads((comp / "comparison.json").read_text())
    keys = set(ANIMATED) | pixel_score.driven_pages()
    flapped, checked, single = [], 0, 0
    for page in pages:
        if page["name"] not in keys or (only and page["name"] not in only):
            continue
        for plat, pv in (page.get("platforms") or {}).items():
            if pv is None or (platforms and plat not in platforms):
                continue
            crop_top = 140 if plat == "android" else 0
            for fw in ("cpp", "xaml"):
                for theme in ("light", "dark"):
                    runs = _run_dirs(comp, plat, page["name"])[:max_runs]
                    seen = {}
                    for run in runs:
                        r = score_cell(page["name"], plat, fw, theme, crop_top,
                                       {"ssim": 1.0, "diff_pct": 0.0}, comp, only_run=run)
                        if r and r.get("evidence"):          # this run really did hold the frames
                            seen[run.name] = r["verdict"]
                    if not seen:
                        continue
                    checked += 1
                    if len(seen) == 1:
                        single += 1        # only one run has these frames: nothing to disagree with
                    elif len(set(seen.values())) > 1:
                        flapped.append((f"{page['name']}/{plat}/{fw}/{theme}", seen))
    for name, seen in flapped:
        print(f"  UNSTABLE {name}: " + ", ".join(f"{r}={v}" for r, v in sorted(seen.items())))
    print(f"--stability: {checked} cell(s) scored across up to {max_runs} runs each; "
          f"{single} had only ONE run to read (not a stability measurement); "
          f"{len(flapped)} disagreed across runs")
    return 1 if flapped else 0


# --------------------------------------------------------------------------- self-check
def _selftest() -> int:
    """python3 lib/motion_score.py — synthetic frame sequences, no device and no board writes.

    Drives score_cell itself (not its helpers) so a future rewrite of the selection rule is checked
    rather than the internals it happens to use today."""
    import shutil  # noqa: PLC0415  selftest-only
    import tempfile  # noqa: PLC0415

    from PIL import Image  # noqa: PLC0415

    ok = True

    def check(what, got, want):
        nonlocal ok
        if got != want:
            print(f"  FAIL {what}: got {got!r}, want {want!r}")
            ok = False

    BOX = 50   # 50x50 on a 240x320 page = 6.5% when it moves; the size is LOAD-BEARING, not arbitrary.

    def frame(path, boxes, size=BOX):
        """A 240x320 white page with `size`x`size` black boxes at `boxes` — big enough for the 11x11
        SSIM window, and big enough to clear MOVED_PCT.

        The verdict thresholds are FRACTIONS of the frame (see MOVED_PCT/FROZEN_PCT), so the box is a
        real parameter of the test rather than a drawing choice: one moving between two
        non-overlapping positions changes 2*size^2 of 76800 px, while a column that keeps its box
        changes 0. Case (8) drives `size` down on purpose — a widget small enough that the bounds'
        previous ABSOLUTE form (800 px) swallowed it whole."""
        im = Image.new("RGB", (240, 320), "white")
        for x, y in boxes:
            for dx in range(size):
                for dy in range(size):
                    im.putpixel((x + dx, y + dy), (0, 0, 0))
        im.save(path)

    def faint_frame(path, boxes, size, rgb):
        """A 240x320 white page with `size`x`size` boxes in `rgb` — for the SUB-THRESHOLD cases.

        The existing `frame` draws BLACK on white, a per-pixel delta of 255, which makes any threshold
        between 1 and 254 behave identically. That is why cases 21/22 passed with ROI_DIFF_THRESHOLD
        forced back to 25 and with the un-freeze disabled: their fixture could not tell the two apart.
        This one draws the amplitude the real defect had."""
        im = Image.new("RGB", (240, 320), "white")
        for x, y in boxes:
            for dx in range(size):
                for dy in range(size):
                    im.putpixel((x + dx, y + dy), rgb)
        im.save(path)

    def faint_unit(run, key, col, theme, frames, size, rgb, plat="maccatalyst"):
        d = run / key / plat / col
        d.mkdir(parents=True, exist_ok=True)
        for n, (step, boxes) in enumerate(frames, 1):
            faint_frame(d / f"{n:04d}.png", boxes, size, rgb)
            (d / f"{n:04d}.json").write_text(json.dumps(
                {"theme": theme, "step": step, "frame": n, "commit": "deadbeef",
                 "captured_at": "2026-08-05T00:00:00+03:00"}))
        return d

    def unit(run, key, col, theme, frames, size=BOX, plat="maccatalyst"):
        """frames: [(step, [boxes])] -> the runner's NNNN.png + NNNN.json pairs."""
        d = run / key / plat / col
        d.mkdir(parents=True, exist_ok=True)
        for n, (step, boxes) in enumerate(frames, 1):
            frame(d / f"{n:04d}.png", boxes, size)
            (d / f"{n:04d}.json").write_text(json.dumps(
                {"theme": theme, "step": step, "frame": n, "commit": "deadbeef",
                 "captured_at": "2026-08-05T00:00:00+03:00"}))
        return d

    def publish(comp, key, fw, theme, src, plat="maccatalyst"):
        dst = comp / "captures" / plat / fw / f"{key}_{theme}.png"
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, dst)

    STILL = {"ssim": 1.0, "diff_pct": 0.0}
    # An UNDRIVEN animated page: burst_frames drops the `initial` frame and keeps the gifNN burst.
    def seq(offsets):
        return [("initial", [(10, 10)])] + [(f"gif{i:02d}", [(x, 10)]) for i, x in enumerate(offsets, 1)]

    with tempfile.TemporaryDirectory() as tmp:
        comp = Path(tmp)
        run = comp / "2026-08-05-10_00_00"

        # (1) IDENTICAL sequences: both animate the same way -> perfect score, no mismatch.
        moving = seq([10, 40, 70])
        dm = unit(run, "same", "maui_xaml", "light", moving)
        do = unit(run, "same", "cpp", "light", moving)
        publish(comp, "same", "maui", "light", dm / "0001.png")
        publish(comp, "same", "cpp", "light", do / "0001.png")
        r = score_cell("same", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("identical: frames scored", "MOTION 3 frames" in r["detail"], True)
        check("identical: worst SSIM", r["ssim"], 1.0)
        check("identical: no mismatch", r["mismatch"], False)

        # (2) ONE COLUMN STATIC — the finding this tool exists for. MAUI animates, the port is frozen
        #     on its first frame, and the mismatch must be flagged loudly, not left to the SSIM.
        dm = unit(run, "frozen", "maui_xaml", "light", moving)
        do = unit(run, "frozen", "cpp", "light", seq([10, 10, 10]))
        publish(comp, "frozen", "maui", "light", dm / "0001.png")
        publish(comp, "frozen", "cpp", "light", do / "0001.png")
        r = score_cell("frozen", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("frozen column: flagged", r["mismatch"], True)
        check("frozen column: says so first", r["detail"].startswith("!! MOTION MISMATCH"), True)
        # Both measurements must be IN THE TEXT, so a forced red can be argued from the review alone.
        # Asserted as a PERCENT (with the pixel count alongside) because the percent is what the
        # verdict is taken on — quoting only a count would hide the number that decided the outcome,
        # and a count alone is not comparable across lanes of different pixel density at all.
        check("frozen column: names the frozen side", "cpp IS FROZEN" in r["detail"], True)
        check("frozen column: prints the port's own motion", "cpp 0.0000% (0 px)" in r["detail"], True)

        # (3) SHIFTED: both animate, but the port's box sits 8px lower throughout. Real difference,
        #     NOT a mismatch — both columns moved, so the loud flag must stay off.
        dm = unit(run, "shift", "maui_xaml", "light", moving)
        do = unit(run, "shift", "cpp", "light",
                  [("initial", [(10, 18)])] + [(f"gif{i:02d}", [(x, 18)]) for i, x in enumerate([10, 40, 70], 1)])
        publish(comp, "shift", "maui", "light", dm / "0001.png")
        publish(comp, "shift", "cpp", "light", do / "0001.png")
        r = score_cell("shift", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("shifted: not a mismatch", r["mismatch"], False)
        check("shifted: worst SSIM below green", r["ssim"] < 0.98, True)
        # The shifted case is ALSO the control for the phase gate: it moves the same distance as MAUI
        # but starts 8px lower, so its RESTING frame already disagrees. That is a real port difference
        # and must not be forgiven as phase.
        check("shifted: not phase-only", r["phase_only"], False)

        # (3b) PHASE ONLY — the fling. Same start, same end, same distance travelled; only the middle
        #      sample lands somewhere else, exactly what a non-reproducible coast does. Must be flagged
        #      and must NOT read as a mismatch.
        for lane in ("android", "ios"):
            dm = unit(run, "phase", "maui_xaml", "light", moving, plat=lane)      # 10 -> 40 -> 70
            do = unit(run, "phase", "cpp", "light", seq([10, 55, 70]), plat=lane) # 10 -> 55 -> 70
            publish(comp, "phase", "maui", "light", dm / "0001.png", plat=lane)
            publish(comp, "phase", "cpp", "light", do / "0001.png", plat=lane)
        r = score_cell("phase", "android", "cpp", "light", 0, STILL, comp)
        check("phase: flagged", r["phase_only"], True)
        check("phase: says so first", r["detail"].startswith("!! PHASE ONLY"), True)
        check("phase: not a mismatch", r["mismatch"], False)
        check("phase: the middle frame really does differ", r["ssim"] < 0.98, True)
        # (3b-ii) THE SAME FRAMES ON A REPRODUCIBLE LANE. The signature is identical, so only the
        #         measured per-lane fact separates them — and iOS's drive IS reproducible (both columns
        #         0.00% against themselves), which makes an equal-distance disagreement a finding rather
        #         than a sampling artifact. Without this clause the gate forgives three real iOS cells.
        r = score_cell("phase", "ios", "cpp", "light", 0, STILL, comp)
        check("phase on a reproducible lane: NOT forgiven", r["phase_only"], False)
        check("phase on a reproducible lane: no phase banner", r["detail"].startswith("!! PHASE ONLY"), False)
        # (3b-iii) A cell whose frames AGREE established parity — it must stay clean, not carry a
        #          "not decidable" banner. Identical sequences on the non-reproducible lane.
        dm = unit(run, "agree", "maui_xaml", "light", moving, plat="android")
        do = unit(run, "agree", "cpp", "light", moving, plat="android")
        publish(comp, "agree", "maui", "light", dm / "0001.png", plat="android")
        publish(comp, "agree", "cpp", "light", do / "0001.png", plat="android")
        r = score_cell("agree", "android", "cpp", "light", 0, STILL, comp)
        check("agreeing frames: not flagged as phase", r["phase_only"], False)
        check("agreeing frames: perfect score", r["ssim"], 1.0)

        # (3c) HALF THE DISTANCE — the case the phase gate must never swallow. Same resting frame, but
        #      the port travels 30px where MAUI travels 60. A port that scrolls half as far is a defect,
        #      and the self-motion spread is what separates it from (3b).
        dm = unit(run, "halfway", "maui_xaml", "light", moving)               # 10 -> 70, travels 60
        do = unit(run, "halfway", "cpp", "light", seq([10, 25, 40]))          # 10 -> 40, travels 30
        publish(comp, "halfway", "maui", "light", dm / "0001.png")
        publish(comp, "halfway", "cpp", "light", do / "0001.png")
        r = score_cell("halfway", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("half distance: NOT phase-only", r["phase_only"], False)
        check("half distance: not a mismatch either (both moved)", r["mismatch"], False)

        # (4) DIFFERENT LENGTHS: the port dropped gif03. The two surviving pairs are scored, the
        #     orphan is reported — never re-aligned onto gif02 the way index pairing would.
        dm = unit(run, "short", "maui_xaml", "light", moving)
        do = unit(run, "short", "cpp", "light", seq([10, 40]))
        publish(comp, "short", "maui", "light", dm / "0001.png")
        publish(comp, "short", "cpp", "light", do / "0001.png")
        r = score_cell("short", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("unequal lengths: only pairs scored", "MOTION 2 frames" in r["detail"], True)
        check("unequal lengths: orphan reported", "1 frame(s) had no partner" in r["detail"], True)

        # (5) NO STEP NAME IN COMMON: refuse. Scoring "0 frames" or falling back to the still without
        #     saying so are both the silent-wrong-number failure this file is written against.
        dm = unit(run, "alien", "maui_xaml", "light", moving)
        do = unit(run, "alien", "cpp", "light",
                  [("initial", [(10, 10)])] + [(f"other{i:02d}", [(x, 10)]) for i, x in enumerate([10, 40, 70], 1)])
        publish(comp, "alien", "maui", "light", dm / "0001.png")
        publish(comp, "alien", "cpp", "light", do / "0001.png")
        r = score_cell("alien", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("no common step: NOT motion-scored", "NOT motion-scored" in r["detail"], True)
        check("no common step: keeps the still number", (r["ssim"], r["diff_pct"]), (1.0, 0.0))
        check("no common step: claims no motion number", "MOTION" in r["detail"], False)

        # (6) NO RUN DIR AT ALL — iOS and Android every time. Labelled fallback, never a bare number.
        r = score_cell("absent", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("no run dir: labelled", "NOT motion-scored" in r["detail"], True)
        check("no run dir: keeps the still", r["ssim"], 1.0)
        check("no run dir: no comparable pair stays blank",
              score_cell("absent", "maccatalyst", "cpp", "light", 0, None, comp), None)

        # (7) STALENESS: the frames are there, but captures/ holds a DIFFERENT still — that run is not
        #     the one behind the board, so pairing them would score two unrelated builds.
        dm = unit(run, "stale", "maui_xaml", "light", moving)
        do = unit(run, "stale", "cpp", "light", moving)
        publish(comp, "stale", "maui", "light", dm / "0001.png")
        frame(comp / "captures" / "maccatalyst" / "cpp" / "stale_light.png", [(90, 90)])
        r = score_cell("stale", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("stale run: refused", "CURRENTLY PUBLISHED" in r["detail"], True)

        # (8) A SMALL WIDGET ON A 1x LANE, and the reason MOVED_PCT/FROZEN_PCT are fractions. A 4x4 box
        #     moving to a non-overlapping position changes 32 of 76800 px = 0.042% — real motion, in
        #     both columns, matching. Under the bounds' previous ABSOLUTE form (frozen below 800 px)
        #     32 px read as FROZEN on BOTH sides, so this scored "!! NOTHING MOVED" and was capped
        #     yellow. That is not hypothetical: it is maccatalyst activity_indicator, whose five
        #     UIActivityIndicator rings are 15x15 px on the 1x mac window and moved a measured 231 px
        #     of 819200 — every animated maccatalyst cell on the board was condemned by it.
        tiny = seq([10, 40, 70])
        dm = unit(run, "tiny", "maui_xaml", "light", tiny, size=4)
        do = unit(run, "tiny", "cpp", "light", tiny, size=4)
        publish(comp, "tiny", "maui", "light", dm / "0001.png")
        publish(comp, "tiny", "cpp", "light", do / "0001.png")
        r = score_cell("tiny", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("small widget: counted as motion", r["both_frozen"], False)
        check("small widget: not called NOTHING MOVED", "NOTHING MOVED" in r["detail"], False)
        check("small widget: no false mismatch", r["mismatch"], False)
        # The percentage is what the verdict was taken on, so the review has to print enough of it to
        # argue with — "0.04%" at the old 2 decimals rounds a live signal down toward zero.
        check("small widget: percent quoted at full precision", "0.0417%" in r["detail"], True)

        # (9) …and the frozen side of the SAME small scale is still caught, so (8) did not simply
        #     disarm the detector: the port holds its 4x4 box still while MAUI moves it.
        dm = unit(run, "tinyfrozen", "maui_xaml", "light", tiny, size=4)
        do = unit(run, "tinyfrozen", "cpp", "light", seq([10, 10, 10]), size=4)
        publish(comp, "tinyfrozen", "maui", "light", dm / "0001.png")
        publish(comp, "tinyfrozen", "cpp", "light", do / "0001.png")
        r = score_cell("tinyfrozen", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("small widget frozen column: flagged", r["mismatch"], True)
        check("small widget frozen column: names the frozen side", "cpp IS FROZEN" in r["detail"], True)

        # (13) A PURE SAMPLING DRIFT IS NOT A DEFECT. Both columns move the box through the SAME
        #      positions; column B is simply sampled one step later. Index pairing scores that as a
        #      large per-frame difference and reds the cell — which is exactly what happened to
        #      gestures/android (worst SSIM 0.8820, 34.42% differing on a frame both columns call
        #      `at-rest`) on a page where direct adb injection proves the port behaves identically.
        drift_a = seq([10, 40, 70, 100])
        drift_b = seq([40, 70, 100, 130])          # same trajectory, one sample late
        dm = unit(run, "drift", "maui_xaml", "light", drift_a)
        do = unit(run, "drift", "cpp", "light", drift_b)
        publish(comp, "drift", "maui", "light", dm / "0001.png")
        publish(comp, "drift", "cpp", "light", do / "0001.png")
        r = score_cell("drift", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("sampling drift: realigned, not called a mismatch", r["mismatch"], False)
        check("sampling drift: says it realigned", "realigned by" in r["detail"], True)
        check("sampling drift: scores as agreeing", r["ssim"] > 0.99, True)

        # (14) …and the alignment does NOT forgive a real divergence. Column B visits DIFFERENT places,
        #      so no shift within MAX_PHASE_SHIFT can make the sequences agree. Without this the fix
        #      would buy green cells by sliding frames until something matched.
        dm = unit(run, "diverge", "maui_xaml", "light", seq([10, 40, 70, 100]))
        do = unit(run, "diverge", "cpp", "light", seq([10, 15, 20, 25]))
        publish(comp, "diverge", "maui", "light", dm / "0001.png")
        publish(comp, "diverge", "cpp", "light", do / "0001.png")
        r = score_cell("diverge", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("real divergence: still scores badly", r["ssim"] < 0.99, True)

        # (10) STEP-PAIRED frames have no encoder in them, so they get no percentage floor at all —
        #      moved iff any pixel changed. A 2x2 box is 8 changed px of 76800 = 0.0104%, UNDER
        #      FROZEN_PCT: as a burst that is indistinguishable from iOS's H.264 speckle, but as two
        #      PNG screenshots it is a real reaction. This is stepper, whose entire intrinsic signal is
        #      one "-" glyph re-enabling: 22 px on Catalyst, 35 on Windows, 245 on iOS, all three
        #      columns agreeing, every one of them previously reported "!! NOTHING MOVED".
        def steps(offsets):
            return [("initial", [(10, 10)])] + [(f"step{i:02d}", [(x, 10)]) for i, x in enumerate(offsets, 1)]

        tiny_steps = steps([40])
        dm = unit(run, "steppaired", "maui_xaml", "light", tiny_steps, size=2)
        do = unit(run, "steppaired", "cpp", "light", tiny_steps, size=2)
        publish(comp, "steppaired", "maui", "light", dm / "0001.png")
        publish(comp, "steppaired", "cpp", "light", do / "0001.png")
        r = score_cell("steppaired", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("step-paired: sub-floor reaction is motion", r["both_frozen"], False)
        check("step-paired: not called NOTHING MOVED", "NOTHING MOVED" in r["detail"], False)
        check("step-paired: no false mismatch", r["mismatch"], False)

        # (11) …and the exactness cuts BOTH ways: the same 8-px reaction present in MAUI and absent in
        #      the port is now a MISMATCH. Under the 0.012% burst floor both sides read "frozen" and
        #      this scored a confident green — the switch defect, at the scale where it hid longest.
        dm = unit(run, "steppairedfrozen", "maui_xaml", "light", tiny_steps, size=2)
        do = unit(run, "steppairedfrozen", "cpp", "light", steps([10]), size=2)
        publish(comp, "steppairedfrozen", "maui", "light", dm / "0001.png")
        publish(comp, "steppairedfrozen", "cpp", "light", do / "0001.png")
        r = score_cell("steppairedfrozen", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("step-paired frozen column: flagged", r["mismatch"], True)
        check("step-paired frozen column: names the frozen side", "cpp IS FROZEN" in r["detail"], True)

        # (12) A genuinely dead step-paired page is STILL caught: nothing moved anywhere, no threshold
        #      needed to say so, and the message quotes pixels rather than a percentage bound.
        dead = steps([10])
        dm = unit(run, "steppaireddead", "maui_xaml", "light", dead, size=2)
        do = unit(run, "steppaireddead", "cpp", "light", dead, size=2)
        publish(comp, "steppaireddead", "maui", "light", dm / "0001.png")
        publish(comp, "steppaireddead", "cpp", "light", do / "0001.png")
        r = score_cell("steppaireddead", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("step-paired dead page: still flagged", "NO MOTION EVIDENCE" in r["detail"], True)
        check("step-paired dead page: quotes a single pixel", "a single pixel" in r["detail"], True)

        # ------------------------------------------------------------------ the verdict lattice
        # Each of these reproduces a failure this board actually shipped, so the gate is checked
        # against history rather than against its own definitions.

        # (15) A DRIVEN PAGE WHOSE RUN DIRECTORY IS GONE MUST NOT SCORE GREEN OFF ONE STILL. This was
        #      6 live cells: `not_scored` returns the single-frame number, the cell scored a confident
        #      green, and the only trace was the words "NOT motion-scored" inside prose that nothing
        #      aggregated. The verdict is what makes it visible to a caller.
        r = score_cell("vanished", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("expired evidence: INVALID", r["verdict"], INVALID)
        check("expired evidence: marked EXPIRED, not contradicted", r["why"] in EXPIRED_WHY, True)
        check("expired evidence: carries no run pointer", r["evidence"], None)
        # …and (advisor's catch) an ABSENT PAIR IS NOT INVALID EVIDENCE. A theme with no comparable
        # screenshot is no cell at all; stamping a verdict on it would turn every page missing a dark
        # capture into a motion finding and drown the 6 real ones.
        check("no comparable pair: still blank, still verdict-free",
              score_cell("vanished", "maccatalyst", "cpp", "light", 0, None, comp), None)

        # (16) PROVENANCE MISMATCH IS ALSO EXPIRED, NOT CONTRADICTED. Frames exist; none belong to the
        #      published stills. That says nothing about whether the port moved — so it must be
        #      eligible for carry-forward, exactly like a pruned directory. (Reuses case 7's fixture.)
        r = score_cell("stale", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("provenance mismatch: INVALID", r["verdict"], INVALID)
        check("provenance mismatch: EXPIRED class", r["why"], WHY_PROVENANCE)

        # (17) BOTH COLUMNS FROZEN, WITH AN ACTION AIMED AT THE PAGE — the actionable half. Something
        #      was injected and nothing reacted anywhere: the aim missed, or the interaction is not
        #      reachable on this lane. INVALID rather than FAIL, because two frozen columns are
        #      byte-identical and demonstrate nothing about the port in either direction.
        (comp / "scenarios").mkdir(exist_ok=True)
        (comp / "scenarios" / "steppaireddead.toml").write_text(
            '[[steps]]\naction = "tap"\nat = [0.5, 0.5]\n')
        r = score_cell("steppaireddead", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("frozen + action authored: INVALID", r["verdict"], INVALID)
        check("frozen + action authored: named as not-driven", r["why"], WHY_NOT_DRIVEN)
        check("frozen + action authored: says the action was injected",
              "An action WAS injected here" in r["detail"], True)

        # (18) …and the SAME FRAMES with no scenario at all are a different statement entirely: nobody
        #      ever aimed at this page. 80 of the 139 cells carrying the old "NOTHING MOVED" banner
        #      were this — including 10 of the 14 hard-coded ANIMATED pages — reading like a port
        #      finding when the missing artifact was a scenario file.
        (comp / "scenarios" / "steppaireddead.toml").unlink()
        r = score_cell("steppaireddead", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("frozen + no scenario: INVALID", r["verdict"], INVALID)
        check("frozen + no scenario: named as such", r["why"], WHY_NO_SCENARIO)
        check("frozen + no scenario: refuses to blame the port",
              "NOT a port finding" in r["detail"], True)

        # (19) PASS IS CONJUNCTIVE. Case (1)'s identical sequences have valid evidence AND agreeing
        #      frames. Case (3)'s 8px-shifted port has evidence just as valid and frames that do not
        #      agree — if PASS meant only "validly compared", that cell would advertise PASS while the
        #      board rendered it red, and every reader of the field would conclude the motion matched.
        check("identical sequences: PASS", score_cell("same", "maccatalyst", "cpp", "light", 0,
                                                      STILL, comp)["verdict"], PASS)
        check("8px-shifted port: NOT PASS", score_cell("shift", "maccatalyst", "cpp", "light", 0,
                                                       STILL, comp)["verdict"], FAIL)
        check("one column frozen: FAIL", score_cell("frozen", "maccatalyst", "cpp", "light", 0,
                                                    STILL, comp)["verdict"], FAIL)
        check("fling phase: INCONCLUSIVE", score_cell("phase", "android", "cpp", "light", 0,
                                                      STILL, comp)["verdict"], INCONCLUSIVE)

        # (21) THE DECLARED REACTION REGION — reproducing the green that had to be reverted.
        #      button/maccatalyst: both columns MOVED, so no mismatch; the frames agreed to 0.40%
        #      differing at SSIM 0.9897, comfortably green; and MAUI's readout still said "Taps: 0"
        #      while the port's said "Taps: 1". The whole-frame numbers cannot carry a one-digit
        #      difference — 41 px of 819,200 — so the scenario has to say WHERE the reaction belongs.
        #
        #      Built to that shape deliberately: BOTH columns change by a large amount OUTSIDE the roi
        #      (so every existing clause is satisfied and would score PASS), and only one changes
        #      INSIDE it.
        (comp / "scenarios").mkdir(exist_ok=True)
        (comp / "scenarios" / "roisplit.toml").write_text(
            '[[steps]]\nname = "initial"\n\n[[steps]]\nname = "after-tap"\naction = "click"\n'
            'at = [0.5, 0.5]\nroi = [0.0, 0.0, 0.25, 0.25]\n')

        def roi_frames(readout_moves):
            # a big shared box far from the roi (both columns move it), plus a small box INSIDE the
            # roi that only moves when readout_moves.
            after = [(120, 120)] + ([(20, 20)] if readout_moves else [(5, 5)])
            return [("initial", [(60, 120), (5, 5)]), ("after-tap", after)]

        # size=3 IS THE POINT, not a detail. The first cut of this fixture used the default 50px box and
        # the case failed for the WRONG REASON — a 50px box shifted 15px is a big whole-frame difference,
        # so `frames-disagree` fired and the ROI clause was never what caught it. That would have been a
        # test passing while proving nothing. At size 3 the in-roi difference is ~18 px of 76,800
        # (0.023%), under the 1.0% green bar and invisible to SSIM — the same relationship the real
        # button/maccatalyst digit had to its 819,200-pixel frame.
        dm = unit(run, "roisplit", "maui_xaml", "light", roi_frames(False), size=3)   # readout frozen
        do = unit(run, "roisplit", "cpp", "light", roi_frames(True), size=3)          # readout moved
        publish(comp, "roisplit", "maui", "light", dm / "0001.png")
        publish(comp, "roisplit", "cpp", "light", do / "0001.png")
        r = score_cell("roisplit", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("roi split: FAIL, not PASS", r["verdict"], FAIL)
        check("roi split: named", r["why"], "roi-split")
        check("roi split: says which side was silent", "changed NONE" in r["detail"], True)
        check("roi split: leads with it", r["detail"].startswith("!! DECLARED REACTION REGION"), True)

        # (22) …and a region BOTH columns react in is not flagged. Without this the gate would red every
        #      driven page that declares an roi at all, which is worse than not having it.
        dm = unit(run, "roiok", "maui_xaml", "light", roi_frames(True), size=3)
        do = unit(run, "roiok", "cpp", "light", roi_frames(True), size=3)
        publish(comp, "roiok", "maui", "light", dm / "0001.png")
        publish(comp, "roiok", "cpp", "light", do / "0001.png")
        (comp / "scenarios" / "roiok.toml").write_text(
            (comp / "scenarios" / "roisplit.toml").read_text())
        r = score_cell("roiok", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("both react in the roi: not flagged", r["why"] == "roi-split", False)
        check("both react in the roi: PASS", r["verdict"], PASS)

        # (23) A LARGE SELF-MOTION ASYMMETRY IS REPORTED BUT DOES NOT CHANGE THE VERDICT. Both columns
        #      visit the same positions, so every paired frame agrees and the cell is a legitimate PASS
        #      — but one column ALSO moves a second box the other never touches, so its own sequence
        #      covers far more ground. That is exactly gestures/android's shape (247x, and innocent on
        #      the paired evidence), and before this flag the review said nothing about it.
        # Shaped to gestures/android's ACTUAL structure, which the first cut of this fixture missed: the
        # extra motion must live in an UNPAIRED frame, so every paired frame still agrees and the cell is
        # a legitimate PASS. (The first version simply gave MAUI a bigger box in a PAIRED frame — that
        # scores frames-disagree, and would have tested the wrong clause. It also only reached 1.5x,
        # under the flag, so it failed outright rather than passing hollowly.)
        #
        # gif02 is a 2px shift of a 50px box = ~200 changed px; gif03 is a full 5000px move (at (180,250) — the frame is 240x320, so a 50px box at
        # (200,200) would overrun it) that only
        # MAUI has. Paired: gif01+gif02, identical in both. Self-motion: MAUI 5000, port 200 -> 25x.
        wide = [("initial", [(10, 10)]), ("gif01", [(10, 10)]), ("gif02", [(12, 10)]),
                ("gif03", [(180, 250)])]
        narrow = [("initial", [(10, 10)]), ("gif01", [(10, 10)]), ("gif02", [(12, 10)])]
        dm = unit(run, "asym", "maui_xaml", "light", wide)
        do = unit(run, "asym", "cpp", "light", narrow)
        publish(comp, "asym", "maui", "light", dm / "0001.png")
        publish(comp, "asym", "cpp", "light", do / "0001.png")
        r = score_cell("asym", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("asymmetry: reported", "SELF-MOTION ASYMMETRY" in r["detail"], True)
        check("asymmetry: says it is not a defect", "not treated as a defect" in r["detail"], True)
        check("asymmetry: the cell still PASSES on its paired frames", r["verdict"], PASS)
        check("asymmetry: not a mismatch (both moved)", r["mismatch"], False)
        check("asymmetry: names the unpaired frames", "never compared at all" in r["detail"], True)

        # (24) …and a SYMMETRIC pair says nothing, or every cell would carry the banner.
        dm = unit(run, "sym", "maui_xaml", "light", narrow)
        do = unit(run, "sym", "cpp", "light", narrow)
        publish(comp, "sym", "maui", "light", dm / "0001.png")
        publish(comp, "sym", "cpp", "light", do / "0001.png")
        r = score_cell("sym", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("symmetric: silent", "SELF-MOTION ASYMMETRY" in r["detail"], False)
        check("symmetric: PASS", r["verdict"], PASS)

        # (25) A SUB-THRESHOLD REACTION INSIDE A DECLARED REGION IS STILL A REACTION.
        #      picker/windows exactly: the WinUI ComboBox's focus state changes 29,434 px — the whole
        #      control — by an amplitude of SIX. _self_motion asks the VISIBILITY question at
        #      25/channel, reads 0 for every column, and the cell scores both_frozen ->
        #      INVALID/`not-driven` on a cell where all three columns agree TO THE PIXEL.
        #
        #      The fixture draws (249,249,249) on white: a delta of 6, matching the real amplitude.
        #      Cases 21/22 could not have caught this — their boxes are BLACK on white, delta 255, so
        #      every threshold from 1 to 254 behaves the same and both halves of the fix could be
        #      disabled with the suite still green. That was verified by break-testing before this
        #      case existed.
        (comp / "scenarios").mkdir(exist_ok=True)
        (comp / "scenarios" / "faint.toml").write_text(
            '[[steps]]\nname = "initial"\n\n[[steps]]\nname = "after-tap"\naction = "click"\n'
            'at = [0.5, 0.5]\nroi = [0.0, 0.0, 0.5, 0.5]\n')
        faint = [("initial", [(10, 10)]), ("after-tap", [(60, 10)])]
        dm = faint_unit(run, "faint", "maui_xaml", "light", faint, 40, (249, 249, 249))
        do = faint_unit(run, "faint", "cpp", "light", faint, 40, (249, 249, 249))
        publish(comp, "faint", "maui", "light", dm / "0001.png")
        publish(comp, "faint", "cpp", "light", do / "0001.png")
        r = score_cell("faint", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("faint reaction: NOT called frozen", r["both_frozen"], False)
        check("faint reaction: not a mismatch (both reacted)", r["mismatch"], False)
        check("faint reaction: PASSES", r["verdict"], PASS)
        check("faint reaction: no roi split", r["why"] == "roi-split", False)

        # (26) …and a page that is genuinely dead INSIDE the region is still caught, so (25) did not
        #      simply disarm the frozen detector by declaring a region.
        dead_faint = [("initial", [(10, 10)]), ("after-tap", [(10, 10)])]
        dm = faint_unit(run, "faintdead", "maui_xaml", "light", dead_faint, 40, (249, 249, 249))
        do = faint_unit(run, "faintdead", "cpp", "light", dead_faint, 40, (249, 249, 249))
        publish(comp, "faintdead", "maui", "light", dm / "0001.png")
        publish(comp, "faintdead", "cpp", "light", do / "0001.png")
        (comp / "scenarios" / "faintdead.toml").write_text(
            (comp / "scenarios" / "faint.toml").read_text())
        r = score_cell("faintdead", "maccatalyst", "cpp", "light", 0, STILL, comp)
        check("faint dead page: still frozen", r["both_frozen"], True)
        check("faint dead page: INVALID", r["verdict"], INVALID)

        # (20) PRECEDENCE. A cell green in light and frozen in dark is governed by the dark theme —
        #      FAIL > INVALID > INCONCLUSIVE > PASS, so no theme's finding can be averaged away.
        check("precedence: FAIL outranks PASS", worst_verdict([PASS, FAIL]), FAIL)
        check("precedence: INVALID outranks INCONCLUSIVE", worst_verdict([INCONCLUSIVE, INVALID]), INVALID)
        check("precedence: INVALID outranks PASS", worst_verdict([PASS, INVALID]), INVALID)
        check("precedence: FAIL outranks INVALID", worst_verdict([INVALID, FAIL]), FAIL)
        check("precedence: nothing in, nothing out", worst_verdict([None, None]), None)

    print("motion_score selftest:", "OK" if ok else "FAILED")
    return 0 if ok else 1


def _csv(s):
    """"a,b" -> {"a","b"}; "" -> None. An EMPTY filter must mean "no filter", never "match nothing" —
    `--only ""` silently selecting zero cells and printing a cheerful "0 disagreed" is the same
    successful-looking-run-that-did-nothing shape as `--platform macos` scoring zero pages."""
    return {p for p in (s or "").split(",") if p.strip()} or None


if __name__ == "__main__":
    import argparse  # noqa: PLC0415  CLI only

    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--stability", action="store_true",
                    help="cross-run verdict stability instead of the selftest (read-only)")
    ap.add_argument("--only", default="", help="comma-separated page keys")
    ap.add_argument("--platform", default="", help="comma-separated board platforms")
    ap.add_argument("--runs", type=int, default=4, help="how many run dirs per cell (default 4)")
    a = ap.parse_args()
    sys.exit(stability(only=_csv(a.only), platforms=_csv(a.platform), max_runs=a.runs)
             if a.stability else _selftest())
