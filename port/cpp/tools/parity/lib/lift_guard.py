#!/usr/bin/env python3
"""Find android frames carrying the white SRC_OVER alpha-31/255 lift, so they can be re-shot.

WHAT THE ARTIFACT IS. Some android captures come back with a region composited under pure white at
8-bit alpha 31/255 = 12.157%:

    dst = round(src * 224/255 + 31)

That reads on the board as the dark surface #121212 (18) turning into #2F2F2F (47), which is how it was
first noticed -- and how it was first MISDIAGNOSED, three times, as a theme value, a Material elevation
overlay and a device-state wash. It is none of those, and one line rules them all out: OPAQUE
FOREGROUND pixels move too, per channel, by the same law -- (128,0,128) -> (143,31,143),
#FF0000 -> (255,31,31). No background colour, no elevation overlay on a surface and no scrim over a
background can lift the green and blue channels of opaque red. The alpha was pinned by fitting ~100
independent (in,out) channel mappings: the feasible round-to-nearest interval is ~1.5e-4 wide, contains
31/255, and EXCLUDES 0.12, 30/255, 32/255, MDC 8dp (0.118875) and MDC 9dp (0.123616). A truncation
model -- which is what MDC's own ColorUtils.blendARGB uses -- gives an EMPTY interval, so it is not
MDC's overlay path either. The port links no Material at all and shows the byte-identical transform.

IT IS A CAPTURE ARTIFACT AND IT IS INTERMITTENT. Relaunching the same installed APK and screencapping
gives the correct colours with identical pixel counts, and the same pages re-shot minutes later come
back clean. So the repair is to DETECT and RE-SHOOT, not to change any port code. Measured 2026-08-24:
103 full-window frames over 48 pages plus 20 partial pairs over 10 pages -> 0, in three targeted passes.

TWO DETECTORS, BECAUSE ONE IS NOT ENOUGH.

  scan()  FULL-WINDOW. The transform's floor is 31, so a lifted region cannot contain a pixel below it.
          When the lift covers the whole app window the only sub-31 pixels left are OUTSIDE it (the
          navigation bar), a constant ~0.025 of the frame. That makes the lifted population a SPIKE at
          one value rather than a tail, which is what makes a threshold here a detector and not a tuned
          knob. NOTE the predicate this replaces: "global minimum channel == 31" is exactly true in
          theory and matched ZERO of 542 real frames, because the nav bar keeps its true blacks.

  pairs() PARTIAL. A lift confined to one view's rect -- a CheckBox, one cell of a grid, a ScrollView --
          leaves true blacks elsewhere and slips past scan() entirely. There the reference is THE OTHER
          COLUMN: two columns of one page render the same content, so where they differ, ask whether
          the difference IS the transform, in either direction. A per-channel affine map with a fixed
          slope and intercept is not something a genuine rendering difference satisfies by accident,
          and the direction of fit names which column is lifted.

CALIBRATION, and the false positive that set it. FLAT_MAX was 0.10 and swept up swipe_item_position at
0.0672 -- an already-GREEN cell -- sending a 30-minute re-shoot after nothing. It is not lifted, and the
proof is one number: its extrema minimum is 0, and a lifted frame cannot contain a 0. The spike sits at
0.0249 and the next real value on the board is 0.0672, so the window is kept tight around the spike.

    python3 lib/lift_guard.py <captures/android>            # report
    python3 lib/lift_guard.py <captures/android> --pages    # comma-separated keys, for --examples
"""
import glob
import os
import sys

LIFT_MIN = 31        # the transform's floor: round(src*224/255 + 31) can never emit less
FLAT_MAX = 0.04      # full-window: spike at 0.0249, next real value 0.0672 -- see CALIBRATION above
FIT_MIN = 0.80       # partial: frac of differing pixels that must satisfy the identity
MIN_DIFF = 0.02      # partial: ignore pairs that barely differ -- nothing to explain
GRID = 260           # sampling grid; the decision is a FRACTION, so it does not need full resolution


def _px(path, grid=GRID):
    from PIL import Image  # lazy so --help works without Pillow
    return list(Image.open(path).convert("RGB").resize((grid, grid), Image.NEAREST).getdata())


def frac_below(path, thresh=LIFT_MIN):
    d = _px(path)
    return sum(1 for p in d if min(p) < thresh) / len(d)


def is_lift(a, b):
    """b == the lift of a, per channel, +/-1 for rounding."""
    return all(abs(bc - round(ac * 224 / 255 + 31)) <= 1 for ac, bc in zip(a, b))


def scan(root):
    """Fully-lifted frames: [(page, column, frac)]."""
    out = []
    for col in ("maui", "cpp", "xaml"):
        for p in sorted(glob.glob(os.path.join(root, col, "*_dark.png"))):
            f = frac_below(p)
            if f < FLAT_MAX:
                out.append((os.path.basename(p)[:-9], col, f))
    return out


def pairs(root):
    """Partially-lifted cells, judged against the other column: [(page, column, lifted, fit, diff)]."""
    out = []
    for p in sorted(glob.glob(os.path.join(root, "maui", "*_dark.png"))):
        key = os.path.basename(p)[:-9]
        da = _px(p)
        for col in ("cpp", "xaml"):
            q = os.path.join(root, col, f"{key}_dark.png")
            if not os.path.exists(q):
                continue
            db = _px(q)
            diff = [(x, y) for x, y in zip(da, db) if x != y]
            if len(diff) / len(da) < MIN_DIFF:
                continue
            fwd = sum(1 for x, y in diff if is_lift(x, y)) / len(diff)
            rev = sum(1 for x, y in diff if is_lift(y, x)) / len(diff)
            if max(fwd, rev) >= FIT_MIN:
                out.append((key, col, col if fwd >= rev else "maui",
                            max(fwd, rev), len(diff) / len(da)))
    return out


def main(argv):
    root = argv[1] if len(argv) > 1 else "docs/comparison/captures/android"
    full, part = scan(root), pairs(root)
    keys = sorted({k for k, *_ in full} | {k for k, *_ in part})
    if len(argv) > 2 and argv[2] == "--pages":
        print(",".join(keys))
        return 0
    print(f"{len(full)} fully-lifted frame(s), {len(part)} partially-lifted pair(s), "
          f"{len(keys)} page(s) affected")
    for k, c, f in full:
        print(f"  FULL    {k:32} {c:5} frac_below31={f:.4f}")
    for k, c, who, fit, d in part:
        print(f"  PARTIAL {k:32} maui-vs-{c:4} lifted={who:5} fit={fit:.3f} diff={d:.3f}")
    return 1 if keys else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
