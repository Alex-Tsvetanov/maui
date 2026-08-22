#!/usr/bin/env python3
"""Compute a real pixel-diff / SSIM score between each page's MAUI and C++ screenshots, and write
the result into comparison.json as a review-like model ("pixel" / "pixel_xaml") alongside
"sonnet"/"sonnet_xaml" and "gemini"/"gemini_xaml" — same {status, review} shape, so gen_readme.py's
existing status_emoji()/review_body() renders it for free.

**The four required comparisons** (port/CLAUDE.md "Parity comparison policy" §5, 2026-07-05 ruling —
applies to EVERY review model, not just this one): MAUI is ground truth, judged independently against
BOTH framework columns, in both themes:
  1. MAUI light vs C++ light            -> written to the "pixel" slot
  2. MAUI light vs C++ & XAML light      -> written to the "pixel_xaml" slot
  3. MAUI dark  vs C++ dark              -> written to the "pixel" slot
  4. MAUI dark  vs C++ & XAML dark       -> written to the "pixel_xaml" slot
This mirrors the existing cpp->bare / xaml->`_xaml` slot convention `comparison_paths.review_slot()`
already encodes for sonnet/gemini. Android has no dark theme, so only comparisons 1 and 2 apply there.

Dependency-light: PIL + numpy only (no scipy/scikit-image on this machine). SSIM uses a proper 11x11
uniform-window average (via an integral image, not a naive global mean) per Wang et al. 2004, computed
on grayscale luma. Also reports diff_pct: the percentage of pixels whose per-channel max absolute
difference exceeds a visibility threshold (25/255) — a blunter, more intuitive companion number.

**Animated pages are scored FRAME BY FRAME**, at full resolution, by lib/motion_score.py — worst-frame
and mean SSIM across the sequence, so a port that reaches the right END state through the wrong
intermediate frames is caught. The worst frame supplies the {ssim, diff_pct} the thresholds below judge.

**Motion carries its own four-verdict result** (PASS / FAIL / INVALID / INCONCLUSIVE — the lattice is
defined and justified in motion_score.py) and it is written into comparison.json under a `motion` key
BESIDE the status, never merged into it. Three consequences, each deliberate:

  * a non-PASS cell can never be GREEN — including the case that used to slip through, where the run
    frames had expired and the cell quietly scored a confident green off ONE resting still while its
    review prose said "NOT motion-scored" to nobody in particular (measured: 6 cells);
  * a non-PASS cell is NOT forced red either. The recovery plan asks for "static green AND motion PASS";
    that AND is not adopted, because motion covers 24.6% of board cells and the layer still moves
    (13 cells changed verdict in one rescore with no logic change). classify() has the full argument;
  * a verdict OUTLIVES the gitignored run directory that produced it. See carry_forward — without that,
    every pruned run would degrade another cell and a fresh clone would cap the entire animated board
    yellow, which is a fact about the checkout rather than about the port.

Modes: `--verify` scores without writing and reports which cells moved (a CHANGE gate for scorer edits);
`--selftest` checks the verdict plumbing with no images. For run-to-run stability of the verdicts
themselves, see `motion_score.py --stability`, which varies the run directory rather than the clock.

Mismatched image dimensions are resized (LANCZOS) to the smaller common size before comparing — a
known limitation: this is a global comparison, not a registered/aligned diff, so a uniform outer-inset
shift (exempt per parity policy) can still show up as a nonzero diff_pct/SSIM<1. Missing screenshots
(either side null, or file absent) produce no score for that theme; a page with NO computable theme for
a given framework gets status "blank" on that framework's slot (mirrors the sonnet/gemini convention).

Usage: python3 tools/parity/pixel_score.py [--only key1,key2] [--platform ios,maccatalyst,android,windows]
Writes results directly into docs/comparison/comparison.json (preserves everything else). Run
docs/comparison/tools/gen_readme.py afterward to render the scores.
"""
import argparse
import glob
import json
import os
import sys
import tomllib

import numpy as np
from PIL import Image

import motion_score

HERE = os.path.dirname(os.path.abspath(__file__))
CPP_ROOT = os.path.normpath(os.path.join(HERE, "..", "..", ".."))
COMP = os.path.join(CPP_ROOT, "docs", "comparison")
JSON = os.path.join(COMP, "comparison.json")

_DRIVEN = None   # driven_pages() cache

PLATFORMS = ("ios", "maccatalyst", "android", "windows")
THEMES = ("light", "dark")
DIFF_THRESHOLD = 25  # per-channel 0-255 abs-diff above this counts as a "visibly different" pixel
WINDOW = 11  # the standard SSIM window size (Wang et al. 2004)
C1 = (0.01 * 255) ** 2
C2 = (0.03 * 255) ** 2


def to_luma(img):
    """RGB (or RGBA, alpha dropped) PIL Image -> float64 grayscale array via ITU-R BT.601."""
    arr = np.asarray(img.convert("RGB"), dtype=np.float64)
    return arr[..., 0] * 0.299 + arr[..., 1] * 0.587 + arr[..., 2] * 0.114


def box_filter(a, k):
    """Uniform k x k box filter over a 2D array via an integral image (reflect-padded at the
    border so the output stays the same size as the input, like scipy.ndimage.uniform_filter)."""
    pad = k // 2
    padded = np.pad(a, pad, mode="reflect")
    ii = np.pad(padded, ((1, 0), (1, 0))).cumsum(0).cumsum(1)
    H, W = a.shape
    # sum over each k x k window ending at (i+k, j+k) in the padded+integral coordinate frame
    total = ii[k:, k:] - ii[:H, k:] - ii[k:, :W] + ii[:H, :W]
    return total / (k * k)


def ssim(a, b):
    """Mean windowed SSIM between two same-size float64 grayscale arrays (Wang et al. 2004)."""
    mu_a, mu_b = box_filter(a, WINDOW), box_filter(b, WINDOW)
    mu_a2, mu_b2, mu_ab = mu_a * mu_a, mu_b * mu_b, mu_a * mu_b
    var_a = box_filter(a * a, WINDOW) - mu_a2
    var_b = box_filter(b * b, WINDOW) - mu_b2
    cov_ab = box_filter(a * b, WINDOW) - mu_ab
    num = (2 * mu_ab + C1) * (2 * cov_ab + C2)
    den = (mu_a2 + mu_b2 + C1) * (var_a + var_b + C2)
    return float(np.mean(num / den))


def load_pair(path_a, path_b):
    """Load two images, resize to a shared (smaller) size if dimensions differ, return RGB arrays."""
    ia, ib = Image.open(path_a), Image.open(path_b)
    if ia.size != ib.size:
        w = min(ia.size[0], ib.size[0])
        h = min(ia.size[1], ib.size[1])
        ia, ib = ia.resize((w, h), Image.LANCZOS), ib.resize((w, h), Image.LANCZOS)
    return ia, ib


def score_images(ia, ib, crop_top=0):
    """Two same-size PIL Images -> {"ssim", "diff_pct"}. Split out of score_theme so motion_score can
    score a pair of RUN-DIRECTORY frames through exactly this code rather than a second copy of it.

    crop_top: rows to drop from the TOP of both images before comparing — used on Android to exclude the
    system STATUS BAR (clock/battery/wifi), which differs between captures purely because they were shot at
    different times, not because of any port rendering (the same capture-chrome exemption the iOS harness
    inset gets under the capture-chrome carve-out). Measured: the status bar occupies rows 0..~135 and differs on 100% of pages;
    the page content below it aligns. Both hosts run NoActionBar, so there is no app title bar to keep.

    THERE IS NO MACCATALYST EQUIVALENT, AND THE OBVIOUS ONE WAS TRIED AND REJECTED (2026-08-22).
    Do not re-add it without reading this. On maccatalyst the three columns are three APPS with three
    window titles (gallery / gallery_xaml / MauiReference), so every cell carries a constant ~771 px
    (0.09%) mismatch in the title bar. That much is true, it IS chrome and not app content, and it looks
    like the same carve-out the Android status bar gets above. It fails for three separate reasons, any
    one of which is sufficient:

      1. THE REGION IS NOT PAGE-INDEPENDENT. Hashing the candidate box per column across all 172 pages,
         13 of them carry page-SPECIFIC content there: borderless, clipping, data_template_selector,
         empty_view{,_swap,_template,_view}, gap_menu_bar, GAP_TITLE_BAR, header_footer_string,
         ios_search_bar, swipe_transition_mode, table_view. Masking would hide the subject matter of
         `gap_title_bar`, a page that exists to test the title bar. This one alone ends it.
      2. THE BOX IS EASY TO MIS-MEASURE. The first sizing said x 96-211 — that sampled only the `cpp`
         column. `xaml`'s title is longer and the true union across all three is x 96-262, y 11-23. Size
         against every column, never one.
      3. THE ONLY CELLS IT MOVES ARE ONES THAT MUST NOT MOVE THIS WAY. Measured by monkeypatching this
         function and running the real classifier over the whole lane: 4 cells, all yellow->green, all
         hair's-breadth crossings on pages with separately-measured REAL differences.
         varied_size_selector dark crosses on diff_pct by 0.01pp (1.07% -> 0.99%), on the page where MAUI
         draws a cell boundary at lum 12 against endpoints [30,215] that the port does not;
         header_footer_grid_horizontal dark crosses on SSIM by 0.0013 (0.9791 -> 0.9813), on a pair whose
         columns came from different runs, at different commits, at different display modes. A scorer
         change whose entire observable outcome is greening cells with known defects is not one we make.

    Isolate before believing a count here: a first pass measured 16 moved cells, but 12 of those were an
    unrelated ANIMATED-set change showing through a comparison.json written before it. Run the no-mask
    control and subtract, or the change looks 4x more consequential than it is, in its own favour.

    (A self-validating variant — mask only where the region matches that column's modal title, so a
    content-bearing page scores normally — answers 1 and 2 but NOT 3: all four of those pages are in the
    common-title group, so it greens exactly the same four cells.)"""
    if crop_top > 0:
        w, h = ia.size
        if h > crop_top:
            ia, ib = ia.crop((0, crop_top, w, h)), ib.crop((0, crop_top, w, h))
    a_rgb = np.asarray(ia.convert("RGB"), dtype=np.int16)
    b_rgb = np.asarray(ib.convert("RGB"), dtype=np.int16)
    # diff_pct IS A PERCENTAGE OF FRAME AREA, AND THE FOUR LANES' FRAMES DIFFER BY ~4x
    # (ios 1206x2622 = 3.16M px, android 1080x2340 = 2.53M, maccatalyst/windows 1024x800 = 819K). The
    # SAME absolute defect therefore scores differently per lane: border_stroke differs by 14,592 px on
    # ios and 21,334 px on maccatalyst — comparable — yet reads 0.46% GREEN and 2.60% yellow.
    # So a green on a LARGE-frame lane is weaker evidence than the same green on a small-frame one, and
    # "lane A is green, so the cause is lane-B-specific" is INVALID unless normalised. Compare lanes in
    # ABSOLUTE differing pixels. Worked example + the numbers: src/platform/ios/ios_border_ops.hpp, at
    # the `stroke_layer.zPosition` line.
    diff_pct = float(np.mean(np.max(np.abs(a_rgb - b_rgb), axis=-1) > DIFF_THRESHOLD) * 100)
    s = ssim(to_luma(ia), to_luma(ib))
    return {"ssim": round(s, 4), "diff_pct": round(diff_pct, 2)}


def score_theme(maui_path, other_path, crop_top=0):
    """One (page, platform, theme) MAUI-vs-<framework> score, or None if either file is missing."""
    if not maui_path or not other_path:
        return None
    abs_maui = os.path.join(COMP, maui_path)
    abs_other = os.path.join(COMP, other_path)
    if not (os.path.isfile(abs_maui) and os.path.isfile(abs_other)):
        return None
    return score_images(*load_pair(abs_maui, abs_other), crop_top)


def full_res(rel_path):
    """Score from the PNG even when the board DISPLAYS a GIF.

    An animated page's board cell points at `<key>_<theme>.gif`, which ffmpeg wrote at 400px wide —
    2.7x smaller in each dimension than the 1080px still beside it. Scoring that downscale throws away
    6/7 of the pixels and inflates SSIM: activity_indicator scored 0.9942 from its GIF while the
    equivalent still-vs-still comparison sees the real detail. The GIF stays the thing humans look at;
    the numbers come from the full-resolution still whenever one exists.
    """
    if rel_path and rel_path.endswith(".gif"):
        png = rel_path[:-4] + ".png"
        if os.path.isfile(os.path.join(COMP, png)):
            return png
    return rel_path


def classify(theme_scores):
    """theme_scores: {"light": {...}|None, "dark": {...}|None}. -> (status, review text, motion).

    `motion` is the four-verdict block (or None on a page with no motion evidence at all):
    {"verdict", "themes": {theme: verdict}, "why", "run", "commit", "captured_at"} — see
    motion_score's lattice block. It is returned SEPARATELY from the status, and that separation is
    deliberate: see the cap rules below.
    """
    have = {t: v for t, v in theme_scores.items() if v is not None}
    if not have:
        return "blank", "No comparable MAUI/C++ screenshot pair exists for this page on this platform.", None
    worst_ssim = min(v["ssim"] for v in have.values())
    worst_diff = max(v["diff_pct"] for v in have.values())
    if worst_ssim >= motion_score.GREEN_SSIM and worst_diff <= motion_score.GREEN_DIFF:
        status = "green"
    elif worst_ssim >= 0.90 and worst_diff <= 8.0:
        status = "yellow"
    else:
        status = "red"
    # One column ANIMATES and the other is FROZEN is a parity failure by definition, and the per-frame
    # SSIM cannot be trusted to expose it — a spinner is a few hundred pixels, so a frozen port can
    # still score 0.99 on every frame. So the verdict is forced rather than inferred; motion_score puts
    # both self-motion percentages in the review text, so a forced red is arguable from that one line.
    if any(v.get("mismatch") for v in have.values()):
        status = "red"
    # NEITHER column moved, on a page the board calls animated. Two frozen columns are byte-identical,
    # so this arrives as a perfect score — the single most misleading green the board can produce, and
    # exactly the state the whole interaction pass exists to eliminate. It is capped at yellow rather
    # than forced red because it is not evidence of a PORT defect: it says the page was never driven
    # (no scenario, or an interaction this lane cannot reach). Yellow puts it in front of a human
    # without accusing the port of something the capture never tested.
    # Both columns moved the same distance from the same resting frame, and only the PHASE differs —
    # which on a fling this lane cannot reproduce in either column (motion_score's PHASE_* block has the
    # control measurements). A red there would assert a port defect the evidence does not support, so it
    # is capped at yellow. Never promoted TO green: frame parity was not established either. Requires
    # EVERY scored theme to be phase-only, so one genuinely red theme still reds the cell.
    #
    # A THEME THAT **PASSED** COUNTS TOWARD THE CAP, and that clause is not a loosening — it repairs a
    # way this test could punish an IMPROVEMENT. `all(phase_only)` conflates "not phase-only" with
    # "genuinely red", and a PASS is the opposite of red. Measured 2026-08-20 on clip/android/pixel: the
    # drive-landing correction (motion_score._drive_shift) took dark from 4.96% to 0.72%, i.e. dark went
    # INCONCLUSIVE -> PASS. That flipped `all(phase_only)` to False, removed the cap, and turned the
    # cell yellow -> RED — on a run where nothing rendered worse and one theme rendered measurably
    # better. Four cells moved that way (clip, clip_gallery, path_gallery on android). Only a theme that
    # actually DEMONSTRATES a failure should block the cap; that is what FAIL means in this lattice.
    def _capped(v):
        return bool(v.get("phase_only")) or v.get("verdict") == motion_score.PASS
    if status == "red" and have and all(_capped(v) for v in have.values()):
        status = "yellow"
    # The port reacted and the ground truth could not, because the shared XAML twin omits the handler
    # (motion_score's `twin_cannot_react` block). Capped at yellow on the same reasoning as phase_only:
    # a red would accuse the port of the TWIN's omission, and a green would claim a motion parity that
    # this page structurally cannot demonstrate. Every scored theme must be exempt, so a genuinely red
    # theme still reds the cell.
    if status == "red" and have and all(v.get("authored_asymmetry") for v in have.values()):
        status = "yellow"
    # WAS MOTION EVER EXPECTED HERE? Computed once, because TWO different caps below would otherwise
    # colour a page nobody ever drove — and the second one silently defeated the first when this ruling
    # was first written (the rescore moved 0 cells until this hoist landed).
    #
    # `why == no-scenario` comes from motion_score's `driven_page = _has_action_scenario(key, comp)`,
    # which reads the CHECKED-IN scenario files rather than the run directory. That provenance is the
    # whole safety argument: a pruned or expired run directory yields `no-frames` / `unpairable` /
    # `not-driven`, none of which is exempt, so no amount of missing evidence can manufacture this state.
    # Requires EVERY scored theme to agree, so a half-authored scenario cannot buy a green.
    never_expected = bool(have) and all(v.get("why") == motion_score.WHY_NO_SCENARIO for v in have.values())

    # NEITHER column moved, on a page the board calls animated. Two frozen columns are byte-identical, so
    # this arrives as a perfect score — the single most misleading green the board can produce when the
    # page WAS driven. But this cap's own reasoning ("it says the page was never driven — no scenario, or
    # an interaction this lane cannot reach") lumps together two states that deserve opposite colours:
    #   - an interaction this lane could not reach  -> yellow: a reaction was authored and never observed;
    #   - NO SCENARIO AT ALL                        -> both columns frozen is the CORRECT result. There
    #     was no motion to miss. USER RULING 2026-08-10: do not expect motion that was never expected.
    # This was the dominant distortion on the board: 70 cells, every one with a clean still (worst 0.60%
    # at SSIM 0.9708, median 0.12%), coloured as though something differed.
    # USER RULING 2026-08-16: BOTH COLUMNS FROZEN NO LONGER CAPS THE COLOUR AT ALL.
    # "'neither moves' should not make a green image comparison yellow just because no motion is
    # detected on neither party."
    #
    # The 2026-08-10 ruling above had already exempted `no-scenario`; this removes the remainder. The
    # cap's own justification was to "put it in front of a human without accusing the port" -- but a
    # COLOUR is the wrong instrument for that, because colour is what the board is graded on, and this
    # one fires on a state that is SYMMETRIC: neither column moved, so there is no evidence of a port
    # defect in either direction. The stills were compared and they matched; that is a green.
    #
    # What replaced it is strictly better at the job the cap was doing: gen_readme now prints the motion
    # STATE in every example header (motion_state / MOTION_STATE), so "⏸ neither moves" is visible on
    # the page itself without repainting a cell that has nothing wrong with it. A reader who wants
    # coverage gaps reads the badge; a reader who wants parity reads the colour. Conflating them cost
    # this board real greens.
    #
    # The ASYMMETRIC cases are untouched and still decide colour, because those ARE evidence: one
    # column animating while the other is frozen is forced RED by the `mismatch` rule above.
    _ = never_expected  # kept: still used by the INVALID cap below
    # ---- the motion verdict, and WHY IT IS NOT ANDed INTO THE COLOUR ----------------------------
    # The recovery plan this implements asks for "only static green AND motion PASS may render a green
    # cell". That AND is the one part of it not adopted, on measured grounds: motion evidence covers
    # 338 of 1376 board cells (24.6%), and that layer demonstrably moves — 13 cells changed verdict in
    # a single rescore with NO logic change, and three flipped between byte-identical runs. ANDing a
    # flapping 24.6% layer into the 100% layer converts every motion flake into a board regression, and
    # the board is what a human reads to decide where to work.
    #
    # So the verdict is REPORTED (its own field, rendered as its own badge by gen_readme) and the
    # colour keeps the cap rules above, which already encode the honest half of the plan's intent: a
    # non-PASS cell can never be GREEN, but neither is it forced red by a metric that is still settling.
    # The one gap that closes here is INVALID -> capped yellow, below.
    verdicts = {t: v.get("verdict") for t, v in have.items() if v.get("verdict")}
    motion = None
    if verdicts:
        governing = motion_score.worst_verdict(verdicts.values())
        # The evidence pointer, from whichever theme actually supplied one. It is written into the
        # COMMITTED comparison.json precisely so a verdict outlives its gitignored run directory —
        # main() reads it back to tell "the frames expired" from "the frames disagreed".
        ev = next((v["evidence"] for v in have.values() if v.get("evidence")), {}) or {}
        why = next((v.get("why") for t, v in have.items()
                    if v.get("verdict") == governing and v.get("why")), "")
        # Self-motion from the GOVERNING theme, so the figure agrees with the verdict beside it rather
        # than averaging a moving light frame with a frozen dark one.
        gov = next((v for t, v in have.items() if v.get("verdict") == governing and v.get("self")), None)
        motion = {"verdict": governing, "themes": verdicts, "why": why,
                  "run": ev.get("run"), "commit": ev.get("commit"),
                  "captured_at": ev.get("captured_at")}
        if gov and gov.get("self"):
            motion["self"] = gov["self"]
        # A DECLARED-REGION SPLIT CANNOT BE GREEN EITHER, and this is a deliberate exception to the
        # "do not AND motion into the colour" rule above. That rule exists because the motion layer
        # FLAPS — 13 cells moved in one rescore — and a flapping signal must not drive the 100% layer.
        # `roi-split` is not that kind of signal: the scenario AUTHORED the region its reaction must
        # appear in, and one column did not change it. There is nothing to wobble.
        #
        # Capped at YELLOW rather than forced red on purpose. The pixels genuinely do agree (on
        # button/maccatalyst, 0.40% differing at SSIM 0.9897) — the columns differ by one glyph, which
        # is a real parity failure and a small visual one. Yellow says "not verified"; red would claim a
        # magnitude the frames do not support.
        if any(v.get("why") == "roi-split" for v in have.values()) and status == "green":
            status = "yellow"
        # INVALID CANNOT BE GREEN — EXCEPT WHERE NO MOTION WAS EVER EXPECTED.
        #
        # The rule exists to close a real hole: a DRIVEN page whose run directory was pruned fell through
        # to `not_scored`, which returns the SINGLE-STILL number — so the cell scored a confident green off
        # one resting frame while its review text said, in prose nobody aggregates, "NOT motion-scored".
        # Measured at 6 cells. That hole stays shut below.
        #
        # But the rule was over-broad, and it was the single largest distortion on the board. USER RULING
        # 2026-08-10: "we should not expect motions that are not expected." A page with NO action scenario
        # was never driven by anything; there is no motion to have failed. Colouring it the same as a page
        # that WAS driven and did not react states a difference the board never measured, and it did so on
        # 70 cells — every one of which has a clean still (worst 0.60% at SSIM 0.9708; the median is 0.12%).
        #
        # THE EXEMPTION IS SAFE BECAUSE OF WHERE THE DISCRIMINATOR COMES FROM. `why == no-scenario` is
        # decided by motion_score's `driven_page = _has_action_scenario(key, comp)`, which reads the
        # CHECKED-IN scenario files — not the run directory. So the pruned-run case that motivated the
        # original rule is `not-driven` / `unpairable` / `no-frames`, all still capped: a page that
        # declares an action must still prove it reacted. Only the "nothing was ever aimed at this page"
        # case is exempt, and no amount of missing or expired frames can manufacture that state.
        #
        # ALL scored themes must agree it was undriven — one theme carrying a real not-driven result still
        # caps the cell, so a half-authored scenario cannot buy a green.
        # USER RULING 2026-08-16 EXTENDS THE EXEMPTION TO ANY SYMMETRIC NO-MOTION CELL.
        # "'neither moves' should not make a green image comparison yellow just because no motion is
        # detected on neither party."
        #
        # Removing the `both_frozen` cap alone did NOT deliver that: a driven-but-frozen page is also
        # INVALID/`not-driven`, so it fell straight through to this cap and stayed yellow by another
        # route. (The selftest kept passing for exactly that reason -- worth stating, because a green
        # suite made the first half of this change look complete when it moved nothing.)
        #
        # So `both_frozen` now exempts here too. The discriminator stays honest: both_frozen means
        # motion_score MEASURED both columns and found neither moved -- a symmetric, positive
        # observation, not missing evidence. An expired or pruned run yields no such measurement, so the
        # pruned-run hole this cap was built for stays shut, and an ASYMMETRIC result is still forced RED
        # by the `mismatch` rule earlier.
        # NEITHER `any` NOR `all` — per-theme, exactly like `_capped` above (line ~211), and for the same
        # measured reason.
        #
        # `any` was wrong: ONE theme carrying the positive observation exempted the cell while the OTHER
        # theme's genuine INVALID — missing evidence — was waved through with it. That is the pruned-run
        # hole this cap exists to shut, reopened from the side.
        #
        # `all(both_frozen)` was ALSO wrong, and is the same error `_capped`'s comment already documents
        # one cap higher: it conflates "not frozen" with "genuinely INVALID". A theme that MOVED AND
        # MATCHED is not frozen, and that is the opposite of a problem. MEASURED 2026-08-22 on
        # stepper/android (both columns): light scored a real 13-frame motion at worst SSIM 0.9997 / 0.00%
        # while dark measured both columns frozen. Under `all`, light's PASS — an excellent result —
        # cancelled dark's exemption and took the cell green -> YELLOW with a byte-identical review string.
        # Two cells moved that way on one rescore with no capture change.
        #
        # So a theme counts toward the exemption when it is EITHER frozen-symmetric OR not INVALID at all.
        # The pruned-run hole stays shut: a pruned or expired run yields `no-frames` / `unpairable` /
        # `not-driven`, which are INVALID and not both_frozen, so the predicate is False and the cap fires.
        def _frozen_ok(v):
            return bool(v.get("both_frozen")) or v.get("verdict") != motion_score.INVALID
        frozen_both = bool(have) and all(_frozen_ok(v) for v in have.values())
        if (governing == motion_score.INVALID and status == "green"
                and not never_expected and not frozen_both):
            status = "yellow"
    parts = []
    for t in THEMES:
        v = theme_scores.get(t)
        if v is not None:
            # `detail` is motion_score's sentence (a frame-by-frame score, or the still number plus the
            # reason it could NOT be motion-scored). Absent on the ~158 non-animated pages.
            parts.append(f"{t.capitalize()}: " + (v.get("detail") or
                         f"SSIM {v['ssim']:.4f}, {v['diff_pct']:.2f}% pixels differ"))
        elif t in theme_scores:
            parts.append(f"{t.capitalize()}: no comparable pair")
    review = " · ".join(parts)
    return status, review, motion


def driven_pages():
    """Page keys whose authored scenario performs at least one ACTION — i.e. pages the harness drives.

    Cached: main() calls this per page x platform x framework, and the answer cannot change mid-run.

    Why a scenario rather than a list: a driven page is animated in the only sense this scorer cares
    about — its frames differ because something happened to it. Keeping a second hard-coded set beside
    ANIMATED would go stale the first time anyone authored a scenario without remembering to update it,
    which is exactly how the motion of 26 driven pages went unscored on the first driven sweep.
    """
    global _DRIVEN
    if _DRIVEN is None:
        _DRIVEN = set()
        scen = os.path.join(COMP, "scenarios")
        for f in glob.glob(os.path.join(scen, "*.toml")):
            try:
                with open(f, "rb") as fh:
                    steps = tomllib.load(fh).get("steps") or []
            except Exception:            # noqa: BLE001  a malformed scenario is the runner's error to
                continue                 # report, not this scorer's — it simply is not driven here
            if any(s.get("action") for s in steps):
                _DRIVEN.add(os.path.splitext(os.path.basename(f))[0])
    return _DRIVEN


def stills_fingerprint(paths):
    """sha256 over the published stills a cell is scored from — the thing a carried-forward verdict is
    pinned to.

    NOT a hash of the run frames: those are gitignored and pruned, so hashing them would pin a verdict
    to evidence guaranteed to vanish. The published stills ARE committed, so this asks the only
    question that matters when the frames are gone: *are the pictures on the board still the same
    pictures the recorded verdict was taken on?* If yes the verdict still describes them; if a
    recapture landed, the hash moves and the verdict is recomputed or refused."""
    import hashlib  # noqa: PLC0415  only main() needs it
    h = hashlib.sha256()
    for rel in paths:
        if not rel:
            continue
        p = os.path.join(COMP, rel)
        h.update(rel.encode())
        h.update(open(p, "rb").read() if os.path.isfile(p) else b"<missing>")
    return h.hexdigest()[:16]


def carry_forward(theme_scores, prior, fingerprint):
    """Replace EXPIRED motion INVALIDs with the verdict previously recorded for these same stills.

    THE TIME-RATCHET THIS EXISTS TO PREVENT. Run directories are per-run, gitignored and pruned (see
    motion_score's module header for why that source was chosen). Without this, "the frames are gone"
    and "the frames disagreed" both land as INVALID, so:

      * every pruned run silently degrades another cell to a yellow cap, and
      * ON A FRESH CLONE, where there are no run directories at all, EVERY motion cell is INVALID and
        the whole animated board caps yellow — a statement about the checkout, not about the port.

    The board would decay with the calendar. So the two are separated at the source (motion_score's
    EXPIRED_WHY codes) and rejoined here: an expired verdict falls back to the one recorded in the
    COMMITTED comparison.json, but only while `fingerprint` proves the published stills have not moved
    since. A recapture changes the stills, the fingerprint changes with them, and nothing is carried.

    Only EXPIRED codes are eligible. A cell that was driven and did not react, or whose frames cannot
    be paired, has real evidence saying so — that is a finding, and a finding is never overwritten by
    an older one."""
    if not prior or prior.get("stills") != fingerprint:
        return theme_scores
    was = prior.get("themes") or {}
    out = {}
    for t, v in theme_scores.items():
        old = was.get(t)
        if (v is not None and v.get("verdict") == motion_score.INVALID
                and v.get("why") in motion_score.EXPIRED_WHY and old):
            v = dict(v, verdict=old, why=prior.get("why", ""), carried=True,
                     evidence={"run": prior.get("run"), "commit": prior.get("commit"),
                               "captured_at": prior.get("captured_at")},
                     detail=(f"{v['detail']} — CARRIED FORWARD: the published stills are byte-identical "
                             f"to those behind the recorded {old} verdict from run "
                             f"{prior.get('run')} ({prior.get('captured_at')}), so the evidence "
                             f"EXPIRED rather than disagreed. Re-capture to re-derive it."))
        out[t] = v
    return out


def lane_status():
    """docs/comparison/lane_status.toml -> {platform: {...}}, or {} when the file is absent.

    A lane whose CAPTURES cannot be trusted for a reason no pixel can see — the classic being a build
    guest running source older than the tree, which renders old code perfectly and hashes perfectly.
    Declared in one file so retiring it is a one-line edit rather than a sweep over 172 pages."""
    p = os.path.join(COMP, "lane_status.toml")
    if not os.path.isfile(p):
        return {}
    with open(p, "rb") as fh:
        return tomllib.load(fh)


def declare_lane(theme_scores, lane):
    """Apply a lane_status.toml declaration: force every motion verdict on this lane to INVALID.

    For the failure no frame hash can detect — a lane whose captures render SOURCE THAT IS NOT THIS
    TREE. The Windows guest is the measured case: `C:/maui-src` is a tarball copy, not a checkout, and
    was found six days behind. Old code renders perfectly and hashes perfectly; every provenance check
    in this pipeline passes, because they all bind a capture to its own run and none of them binds a
    run to the source it was built from. Only a human who checks SYNC_STAMP.txt can know, so the
    declaration is where that knowledge is written down."""
    if not lane or not lane.get("motion_invalid"):
        return theme_scores
    why = lane.get("reason", "declared stale in lane_status.toml")
    return {t: (v if v is None or not v.get("verdict") else
                dict(v, verdict=motion_score.INVALID, why=motion_score.WHY_PROVENANCE, carried=False,
                     detail=f"{v['detail']} — LANE DECLARED STALE: {why}"))
            for t, v in theme_scores.items()}


# (screenshot framework key, comparison.json slot) — the two required MAUI-vs-<framework> pairs
# per port/CLAUDE.md "Parity comparison policy" §5: cpp -> "pixel" (comparisons 1/3), xaml ->
# "pixel_xaml" (comparisons 2/4). Mirrors comparison_paths.review_slot()'s cpp->bare/xaml->_xaml.
# MODULE SCOPE so there is exactly ONE definition of the pairing: freshness.py reads it back to check
# a recorded verdict against the stills it names, and a second hand-copied list would drift silently.
SLOTS = [("cpp", "pixel"), ("xaml", "pixel_xaml")]
FW_LABEL = {"cpp": "C++", "xaml": "C++ & XAML"}   # how the motion review names the port column


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", default="", help="comma-separated page keys (default: all)")
    ap.add_argument("--platform", default=",".join(PLATFORMS), help="comma-separated platforms")
    ap.add_argument("--verify", action="store_true",
                    help="score into memory and DIFF against the committed comparison.json instead of "
                         "writing it; exits 1 if any cell's status or motion verdict moved. This is a "
                         "CHANGE gate ('did my edit move the board?'), not a stability gate — for that "
                         "see `motion_score.py --stability`, which varies the run directory.")
    args = ap.parse_args()

    # A name that is not a board platform is a MISTAKE, not a filter. Silently dropping it made
    # `--platform macos` (the lane's name for the maccatalyst board column) print "scored 0" and
    # exit 0 — a successful-looking run that scored nothing.
    plats = [p.strip() for p in args.platform.split(",") if p.strip()]
    unknown = [p for p in plats if p not in PLATFORMS]
    if unknown:
        ap.error(f"unknown platform(s) {','.join(unknown)}; known: {','.join(PLATFORMS)}")
    pages = json.load(open(JSON, encoding="utf-8"))
    want = set(k.strip() for k in args.only.split(",") if k.strip()) if args.only else None

    lanes = lane_status()
    scored = 0
    changes = []
    # PROGRESS, FLUSHED PER PAGE. Not cosmetic: recapture.py runs this step under a watchdog that
    # terminates it after 2700s WITHOUT OUTPUT, and until this line existed the only prints in the whole
    # program were the change list and the final total — both after the last page. A motion-heavy lane
    # takes longer than the watchdog, so the step was killed every time, and because the write is a
    # single json.dump at the end, a killed run banks NOTHING. That is silent: the run reports one
    # failed step, the board keeps its OLD verdicts, and the fresh captures underneath them are scored
    # by nobody. Measured on the 2026-08-16 android recapture — 609 new frames, 0-byte score log,
    # verdicts byte-identical to the previous commit.
    todo = [p for p in pages if want is None or p["name"] in want]
    for i, page in enumerate(todo, 1):
        print(f"  [{i}/{len(todo)}] {page['name']}", flush=True)
        for plat in plats:
            platform = page["platforms"].get(plat)
            if platform is None:
                continue
            sc = platform["screenshots"]
            maui = sc.get("maui", {})
            themes = THEMES  # Android is now captured in both light + dark (like iOS/macOS)
            for fw, slot in SLOTS:
                other = sc.get(fw, {})
                crop_top = 140 if plat == "android" else 0  # exclude the Android status bar (see score_images)
                paths = [full_res(maui.get(t)) for t in themes] + [full_res(other.get(t)) for t in themes]
                fingerprint = stills_fingerprint(paths)   # hoisted: EVERY cell is stamped, see below
                theme_scores = {t: score_theme(full_res(maui.get(t)), full_res(other.get(t)), crop_top)
                                for t in themes}
                if page["name"] in motion_score.ANIMATED or page["name"] in driven_pages():
                    # The trigger is the ANIMATED set OR an authored scenario that performs an action —
                    # NOT "captures/ holds a .gif". Two reasons, and the second one was measured:
                    #
                    # A page whose GIF assembly FAILED is still an animated page, and the run frames can
                    # still score its motion.
                    #
                    # And a page that is DRIVEN is animated in every sense that matters here, even
                    # though it is not in the hard-coded ANIMATED list. The first driven iOS sweep made
                    # this concrete: 234 scenario steps executed with 0 failures across 26 pages, every
                    # extra frame landed in the run dir — and 58 of 62 cells were then scored from a
                    # SINGLE STILL, because the gate only knew about the 14 hard-coded pages. The
                    # motion was captured and silently ignored, which is the same shape of blind spot
                    # as the sanitizer's CXX-only language gate: the tool was not looking where the
                    # work had been done.
                    #
                    # Keying on the scenario means a page becomes motion-scored the moment someone
                    # authors a drive for it, with no second list to remember to update.
                    theme_scores = {t: motion_score.score_cell(page["name"], plat, fw, t, crop_top, v,
                                                               fw_label=FW_LABEL[fw])
                                    for t, v in theme_scores.items()}
                    prior = (platform.get(slot) or {}).get("motion")
                    theme_scores = carry_forward(theme_scores, prior, fingerprint)
                    theme_scores = declare_lane(theme_scores, lanes.get(plat))
                was = platform.get(slot) or {}
                status, review, motion = classify(theme_scores)
                # STAMP EVERY CELL, not just the motion ones. The fingerprint used to be computed only
                # inside the motion branch, so a STILL cell's verdict recorded no evidence of which
                # pictures it was taken on -- and a recorded number nobody can tie to a picture is
                # indistinguishable from a current one. That is a second staleness axis, separate from
                # the stale-BINARY problem freshness.py's header describes, and it cost real time on
                # 2026-08-22: an agent was sent to debug 14 android cells of which 11 were ALREADY
                # GREEN on the stills then on disk (`border` published 0.9605/3.49%, actual
                # 0.9943/0.29%) because comparison.json lagged a recapture that had landed 20 minutes
                # earlier. build_comparison_json.py CARRIES pixel scores over -- it cannot recompute
                # them -- so the file's own mtime is no evidence at all: it is bumped by every board
                # refresh while the carried score underneath stays as old as it was. Only the stills'
                # own hash can answer it, which is exactly what stills_fingerprint already asks.
                # freshness.py reads this back; a cell without it reports as unverified, not as fresh.
                cell = {"status": status, "review": review, "stills": fingerprint}
                if motion:
                    motion["stills"] = fingerprint
                    cell["motion"] = motion
                if (was.get("status"), (was.get("motion") or {}).get("verdict")) != \
                        (status, (motion or {}).get("verdict")):
                    changes.append((f"{page['name']}/{plat}/{slot}",
                                    was.get("status"), (was.get("motion") or {}).get("verdict"),
                                    status, (motion or {}).get("verdict")))
                platform[slot] = cell
                scored += 1

    # The change list is printed either way. A full rescore of this board is a ~12-minute SSIM grind,
    # so making the operator run it TWICE — once to see the delta, once to keep it — is a real cost for
    # no information: `--verify` differs only in withholding the write.
    for name, os_, ov, ns, nv in changes:
        moved = [f"status {os_} -> {ns}"] if os_ != ns else []
        if ov != nv:
            moved.append(f"motion {ov} -> {nv}")
        print(f"  CHANGED {name}: " + ", ".join(moved))
    if args.verify:
        print(f"--verify: {scored} scored, {len(changes)} cell(s) differ from {JSON} (nothing written)")
        return 1 if changes else 0

    json.dump(pages, open(JSON, "w", encoding="utf-8"), indent=2)
    print(f"scored {scored} page x platform x framework comparisons "
          f"({len(changes)} changed) -> {JSON}")
    return 0


def _selftest() -> int:
    """python3 tools/parity/lib/pixel_score.py --selftest — the verdict plumbing, no board, no images.

    motion_score's own selftest proves a single cell's verdict. These prove what this module does with
    verdicts once it HAS them: aggregate them, cap the colour on them, and carry them forward. Each
    case is a failure mode that was reasoned about rather than observed, which is exactly why it needs
    a check that fails when the reasoning is wrong."""
    ok = True

    def check(what, got, want):
        nonlocal ok
        if got != want:
            print(f"  FAIL {what}: got {got!r}, want {want!r}")
            ok = False

    GREEN = {"ssim": 1.0, "diff_pct": 0.0}
    RED = {"ssim": 0.5, "diff_pct": 30.0}

    def cell(base, **kw):
        return dict(base, detail="d", evidence={"run": "R", "commit": "c", "captured_at": "2026-08-08"}, **kw)

    # (1) INVALID CANNOT BE GREEN — the 6-cell hole. Perfect pixels, no usable motion evidence.
    st, _rev, mo = classify({"light": cell(GREEN, verdict=motion_score.INVALID, why="no-frames"),
                             "dark": None})
    check("INVALID caps a perfect cell at yellow", st, "yellow")
    check("INVALID is reported as the verdict", mo["verdict"], motion_score.INVALID)

    # (1b) …BUT `no-scenario` IS EXEMPT (user ruling 2026-08-10: do not expect motion that was never
    #      expected). Nothing was ever aimed at the page, so there is no motion to have failed, and the
    #      still is what the cell actually measured. NOTE the fixture above uses `no-frames` — a DRIVEN
    #      page whose frames expired — which is exactly the 6-cell hole and stays capped. These two cases
    #      differ only in the why-code, which is why the pair has to exist: widening the exemption to all
    #      INVALID passes (1b) and fails (1), and the version that caps everything passes (1) and fails
    #      (1b). Neither test alone pins the rule.
    #      EVERY FIXTURE HERE SETS both_frozen=True, and that is load-bearing rather than incidental: an
    #      undriven page's two columns ARE both frozen, and the `both_frozen` cap runs BEFORE the INVALID
    #      one. The first cut of these tests omitted it, so they passed against a rule that moved ZERO of
    #      the 70 real cells — the earlier cap had already turned the cell yellow and the exemption under
    #      test was never reached. A fixture that does not reproduce the cell's real shape proves nothing.
    st, _rev, mo = classify({"light": cell(GREEN, verdict=motion_score.INVALID, both_frozen=True,
                                           why=motion_score.WHY_NO_SCENARIO), "dark": None})
    check("no-scenario INVALID with a clean still is GREEN", st, "green")
    check("no-scenario INVALID still reports its verdict", mo["verdict"], motion_score.INVALID)

    # (1c) A page that WAS driven and did not react keeps the cap, even beside an undriven theme — a
    #      half-authored scenario must not buy a green.
    st, _rev, _mo = classify({"light": cell(GREEN, verdict=motion_score.INVALID, both_frozen=True,
                                            why=motion_score.WHY_NO_SCENARIO),
                              "dark": cell(GREEN, verdict=motion_score.INVALID, both_frozen=True,
                                           why=motion_score.WHY_NOT_DRIVEN)})
    # RULING 2026-08-16: was "yellow". Both columns frozen is symmetric -- neither side moved, so
    # there is no evidence of a port defect and the stills agree. The state is still REPORTED, in the
    # example header ("neither moves"), which is where a coverage gap belongs.
    check("one not-driven theme no longer caps a frozen cell", st, "green")

    # (1d) …and a genuinely bad still is never rescued by the exemption.
    st, _rev, _mo = classify({"light": cell(RED, verdict=motion_score.INVALID, both_frozen=True,
                                            why=motion_score.WHY_NO_SCENARIO), "dark": None})
    check("no-scenario exemption does not rescue a red still", st, "red")

    # (1e) RULING 2026-08-16 INVERTED THIS CASE. It used to pin the exemption to `no-scenario` rather
    #      than to "frozen"; the ruling makes FROZEN itself sufficient, so a driven-but-frozen page is
    #      green. Kept rather than deleted because it is the pair that proves the why-code no longer
    #      decides the colour -- and because the first cut of this change removed only the `both_frozen`
    #      cap, left this cap in place, and moved NOTHING while the suite stayed green.
    st, _rev, _mo = classify({"light": cell(GREEN, verdict=motion_score.INVALID, both_frozen=True,
                                            why=motion_score.WHY_NOT_DRIVEN), "dark": None})
    check("driven-but-frozen is green (both columns still)", st, "green")

    # (1e2) THE ASYMMETRIC CASE IS UNTOUCHED AND STILL RED. This is what stops the ruling from becoming
    #       "motion never colours anything": one column animating while the other is frozen is a real,
    #       measured parity failure.
    st, _rev, _mo = classify({"light": cell(GREEN, verdict=motion_score.FAIL, mismatch=True,
                                            why=motion_score.WHY_NOT_DRIVEN), "dark": None})
    check("one column frozen, one moving is RED", st, "red")

    # (2) …and PASS does NOT cap. The cap must be the exception, not a blanket tax on motion cells.
    st, _rev, mo = classify({"light": cell(GREEN, verdict=motion_score.PASS, why=""), "dark": None})
    check("PASS leaves green alone", st, "green")
    check("PASS reported", mo["verdict"], motion_score.PASS)

    # (3) PRECEDENCE ACROSS THEMES: light passes, dark has no evidence. The cell is governed by dark.
    st, _rev, mo = classify({"light": cell(GREEN, verdict=motion_score.PASS, why=""),
                             "dark": cell(GREEN, verdict=motion_score.INVALID, why="no-scenario")})
    check("one INVALID theme governs the cell", mo["verdict"], motion_score.INVALID)
    check("per-theme verdicts both kept", mo["themes"],
          {"light": motion_score.PASS, "dark": motion_score.INVALID})
    check("the governing theme's why is the one reported", mo["why"], "no-scenario")

    # (4) A FAIL VERDICT DOES NOT FORCE RED. The plan's conjunctive rule would AND motion into the
    #     colour; measured flapping (13 cells moved in one rescore with no logic change) says a 24.6%
    #     layer must not drive the 100% layer. So a FAIL on frames the static bands call minor stays
    #     yellow AND says FAIL — which is the informative outcome, not a contradiction.
    st, _rev, mo = classify({"light": cell({"ssim": 0.95, "diff_pct": 3.0},
                                           verdict=motion_score.FAIL, why="frames-disagree"),
                             "dark": None})
    check("FAIL does not force red", st, "yellow")
    check("FAIL is still reported", mo["verdict"], motion_score.FAIL)

    # (4b) AN roi-split CAPS AT YELLOW, and is the ONE motion verdict allowed to touch the colour.
    #      Deliberate exception: unlike a whole-frame FAIL this one cannot flap — the scenario declared
    #      the region and a column did not change it. Yellow, not red: the pixels really do agree.
    st, _rev, mo = classify({"light": cell(GREEN, verdict=motion_score.FAIL, why="roi-split"),
                             "dark": None})
    check("roi-split caps a pixel-perfect cell at yellow", st, "yellow")
    check("roi-split still reports FAIL", mo["verdict"], motion_score.FAIL)

    # (5) A CELL WITH NO MOTION EVIDENCE AT ALL carries no motion block — the ~158 static pages must
    #     not acquire an empty one.
    st, _rev, mo = classify({"light": GREEN, "dark": None})
    check("static page: no motion block", mo, None)
    check("static page: unaffected", st, "green")

    # (6) CARRY-FORWARD, and the fingerprint that bounds it. Same stills -> the recorded verdict
    #     stands; the board must not decay as run directories are pruned.
    expired = {"light": cell(GREEN, verdict=motion_score.INVALID, why=motion_score.WHY_NO_FRAMES)}
    prior = {"verdict": motion_score.PASS, "themes": {"light": motion_score.PASS}, "why": "",
             "run": "2026-08-07-13_30_41", "commit": "c451d81", "captured_at": "2026-08-07",
             "stills": "abc123"}
    got = carry_forward(expired, prior, "abc123")
    check("expired + unchanged stills: prior verdict stands", got["light"]["verdict"], motion_score.PASS)
    check("carried verdicts say so", got["light"]["carried"], True)
    check("carried verdict keeps the ORIGINAL run pointer", got["light"]["evidence"]["run"],
          "2026-08-07-13_30_41")
    check("carried verdict explains itself", "CARRIED FORWARD" in got["light"]["detail"], True)
    check("a carried PASS is green again", classify(got)[0], "green")

    # (7) …and a RECAPTURE breaks the carry. Different stills, so the old verdict describes different
    #     pictures and must not survive.
    got = carry_forward(expired, prior, "DIFFERENT")
    check("stills changed: nothing carried", got["light"]["verdict"], motion_score.INVALID)

    # (8) ONLY EXPIRED CODES CARRY. A page that was driven and did not react has real evidence saying
    #     so; an older PASS must never overwrite a finding.
    finding = {"light": cell(GREEN, verdict=motion_score.INVALID, why=motion_score.WHY_NOT_DRIVEN)}
    got = carry_forward(finding, prior, "abc123")
    check("a real finding is never carried over", got["light"]["verdict"], motion_score.INVALID)
    check("a real finding keeps its why", got["light"]["why"], motion_score.WHY_NOT_DRIVEN)

    # (9) A LANE DECLARED STALE invalidates every verdict on it, including a PASS — the point of the
    #     declaration is a failure no frame hash can see, so it must beat evidence that looks clean.
    good = {"light": cell(GREEN, verdict=motion_score.PASS, why="")}
    got = declare_lane(good, {"motion_invalid": True, "reason": "guest source is stale"})
    check("declared lane: PASS invalidated", got["light"]["verdict"], motion_score.INVALID)
    check("declared lane: reason reaches the review", "guest source is stale" in got["light"]["detail"], True)
    check("undeclared lane: untouched", declare_lane(good, None)["light"]["verdict"], motion_score.PASS)

    print("pixel_score selftest:", "OK" if ok else "FAILED")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(_selftest() if "--selftest" in sys.argv else main())
