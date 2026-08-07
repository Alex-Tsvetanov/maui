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
motion-scored" and why. A single-still number wearing a motion label is the one outcome this file
exists to prevent.

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

NOT EVERY PLATFORM CAN BE MOTION-SCORED TODAY. Only the VM lanes (maccatalyst, windows) keep full-res
per-step frames. `capture_ios.capture_gif` deletes its mp4 after the ffmpeg conversion, and
`capture_android.capture_gif` writes its burst into a `tempfile.TemporaryDirectory` — both discard the
full-res frames, so iOS and Android animated cells get the labelled "NOT motion-scored" fallback. Those
two functions are the change that would enable them; it is outside this file's scope.

Self-check (no device, no run dir, no board writes):  python3 tools/parity/lib/motion_score.py
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
# And only say it when it CHANGES anything. A cell whose frames already agree to the green bar has
# established frame parity — stamping "not decidable" on it would be false, and would bury the real
# instances among a dozen green ones. These mirror pixel_score.classify's green test.
PHASE_ONLY_SSIM = 0.98
PHASE_ONLY_DIFF_PCT = 1.0
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
# How far back to look for the run that produced the board's capture. Run dirs accumulate for weeks;
# without a bound, a cell whose run was deleted would read every surviving run's frames to prove it.
MAX_RUNS_SCANNED = 20


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


def find_frames(key, plat_dir, fw_dir, theme, published_maui, published_other, comp=COMP):
    """The newest run holding BOTH columns' frames for this cell behind the published stills.

    -> (run_dir, shots_maui, shots_other, why_not). Exactly one of run_dir / why_not is set."""
    col_m, col_o = FW_TO_COL["maui"], FW_TO_COL.get(fw_dir, fw_dir)
    saw_frames = False
    scanned = _run_dirs(comp, plat_dir, key)
    for run in scanned:
        dm, do = run / key / plat_dir / col_m, run / key / plat_dir / col_o
        if not (dm.is_dir() and do.is_dir()):
            continue
        sm, so = _shots(dm, theme), _shots(do, theme)
        if not (sm and so):
            continue
        saw_frames = True
        # BOTH columns must be the ones behind the board, not just one: a mixed pairing would compare
        # two different runs' builds and report the difference as a port bug.
        if _is_published_run(published_maui, sm) and _is_published_run(published_other, so):
            return run, sm, so, None
    if saw_frames:
        # The scan window is named, because "I did not find it" and "I stopped looking" are different
        # claims and this file's whole point is that no failure reads as something it is not.
        capped = " (the newest %d were scanned)" % MAX_RUNS_SCANNED if len(scanned) == MAX_RUNS_SCANNED else ""
        return None, None, None, (f"no run directory holds the frames behind the CURRENTLY PUBLISHED "
                                  f"stills for both columns{capped} — their frames do not match "
                                  f"captures/ byte-for-byte, so re-capture this page")
    return None, None, None, (f"no run directory under docs/comparison/ has {theme} frames for both "
                              f"columns of this cell (run dirs are per-run and gitignored; iOS and "
                              f"Android keep no run dir at all)")


def score_cell(key, plat_dir, fw_dir, theme, crop_top, still, comp=COMP, fw_label=None):
    """The motion score for one (page, platform, framework, theme), shaped for pixel_score.classify.

    Returns the same {"ssim", "diff_pct"} contract classify() already reads — with `ssim`/`diff_pct`
    set to the WORST frame, so the existing thresholds judge the worst moment rather than an average
    that a long static tail can hide — plus:
      detail    the review sentence (mean/worst/per-frame diffs/frame counts/provenance)
      mismatch  True when one column moved and the other did not; pixel_score forces RED on it

    `still` is the single-frame score pixel_score already computed. It is returned UNCHANGED except
    for a `detail` that says the page was NOT motion-scored whenever the frames are unavailable, and
    None stays None (a cell with no comparable pair is blank, exactly as before)."""
    label = fw_label or fw_dir

    def not_scored(why):
        if still is None:
            return None
        # "single frame" rather than "still": on a cell whose PNG is missing, full_res() leaves
        # pixel_score scoring the 400px GIF, and calling that a still would misstate it twice over.
        return dict(still, detail=f"SSIM {still['ssim']:.4f}, {still['diff_pct']:.2f}% pixels differ "
                                  f"(single frame only) — NOT motion-scored: {why}")

    pub_m = str(comp / "captures" / plat_dir / "maui" / f"{key}_{theme}.png")
    pub_o = str(comp / "captures" / plat_dir / fw_dir / f"{key}_{theme}.png")
    run, shots_m, shots_o, why = find_frames(key, plat_dir, fw_dir, theme, pub_m, pub_o, comp)
    if run is None:
        return not_scored(why)

    # The GIF's own frames, in the GIF's own order, via the GIF's own selector.
    dm = run / key / plat_dir / FW_TO_COL["maui"]
    do = run / key / plat_dir / FW_TO_COL.get(fw_dir, fw_dir)
    burst_m = {p: s for s, p, _ in shots_m}
    burst_o = {p: s for s, p, _ in shots_o}
    sel_m = [(burst_m.get(p, ""), p) for p in burst_frames(dm, theme)]
    sel_o = [(burst_o.get(p, ""), p) for p in burst_frames(do, theme)]
    pairs = _pair(sel_m, sel_o)
    if not pairs:
        return not_scored(f"run {run.name} has {len(sel_m)} MAUI and {len(sel_o)} {label} frames but "
                          f"NO step name occurs in both, so nothing can be paired — a comparison by "
                          f"frame index would be a guess. Re-capture this page")

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
    if step_paired:
        m_moved, m_frozen = px_m > 0, px_m == 0
        o_moved, o_frozen = px_o > 0, px_o == 0
    else:
        m_moved, m_frozen = move_m >= MOVED_PCT, move_m <= FROZEN_PCT
        o_moved, o_frozen = move_o >= MOVED_PCT, move_o <= FROZEN_PCT
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
    frames_disagree = ssims[worst_i] < PHASE_ONLY_SSIM or scores[worst_i]["diff_pct"] > PHASE_ONLY_DIFF_PCT
    phase_only = (plat_dir in NON_REPRODUCIBLE_DRIVE and frames_disagree
                  and not mismatch and m_moved and o_moved
                  and spread <= PHASE_SELF_MOTION_TOL and at_rest_diff <= PHASE_AT_REST_PCT)

    meta = shots_m[0][2]
    prov = (f"run {run.name}, commit {meta.get('commit', '?')}, "
            f"{str(meta.get('captured_at', '?'))[:10]}")
    dropped = (len(sel_m) - len(pairs)) + (len(sel_o) - len(pairs))
    unpaired = (f"; {dropped} frame(s) had no partner and were NOT scored" if dropped else "")
    per_frame = "/".join(f"{s['diff_pct']:.2f}" for s in scores)
    detail = (f"MOTION {len(pairs)} frames paired by step ({prov}){unpaired} — "
              f"worst SSIM {ssims[worst_i]:.4f} at frame {worst_i + 1} '{pairs[worst_i][0]}' "
              f"({scores[worst_i]['diff_pct']:.2f}% pixels differ), mean SSIM {mean_ssim:.4f}; "
              f"per-frame diff% {per_frame}; self-motion MAUI {move_m:.4f}% ({px_m} px) vs "
              f"{label} {move_o:.4f}% ({px_o} px)")
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
        detail = (f"!! NOTHING MOVED: neither MAUI nor {label} changed by more than {bound} "
                  f"across the sequence ({px_m} px vs {px_o} px), on a page "
                  f"the board treats as ANIMATED. The two columns agree perfectly because both are "
                  f"still — this scores no motion parity at all. Either the page needs a scenario "
                  f"step to drive it, or its interaction is not reachable on this lane. {detail}")
    # Only what a caller USES: pixel_score writes {status, review} into the slot, so every number is
    # carried by `detail` (the review sentence) rather than duplicated into keys nothing reads back.
    return {"ssim": round(ssims[worst_i], 4), "diff_pct": round(scores[worst_i]["diff_pct"], 2),
            "detail": detail, "mismatch": mismatch, "both_frozen": both_frozen,
            "phase_only": phase_only, "authored_asymmetry": authored_asymmetry}


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
        check("step-paired dead page: still NOTHING MOVED", "NOTHING MOVED" in r["detail"], True)
        check("step-paired dead page: quotes a single pixel", "a single pixel" in r["detail"], True)

    print("motion_score selftest:", "OK" if ok else "FAILED")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(_selftest())
