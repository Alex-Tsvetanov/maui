#!/usr/bin/env python3
"""Where does vertical drift ENTER, and does it accumulate per section? Windowed ink-profile match.

rowdiff.py says which bands differ; it cannot separate a layout SHIFT from per-element error,
because a shift makes every band below it differ too. This answers the follow-up: offset(y), the
displacement between the two captures as a function of height. Constant offset below some y = ONE
wrong extent at that y, everything under it merely carried along. Offset growing in steps = each
section adding its own error, and each step names an accumulation point.

Two cheaper signals were tried and MEASURED USELESS on these captures first:
  * per-row coarse luma signature -- a row is mostly background, so it matches almost any other
    row; residual came back ~1.0 at every offset in +-60px, i.e. no discrimination at all.
  * ink-BLOCK landmarks -- a dense CollectionView carries ink on essentially every row, so the
    whole page collapses into one block and there is nothing to pair.
What does discriminate is the per-row ink COUNT as a 1-D signal: text rows run ~200px of ink,
inter-item gaps ~0, so the profile has real structure to align. Ink is "differs from this row's
modal colour", which is background-agnostic -- it does not care that the two columns disagree
about the page background by a few levels.
"""
import sys

sys.path.insert(0, __file__.rsplit("/", 1)[0])
from rowdiff import rows  # noqa: E402  (shared PNG decoder — one in the tree, not two)

INK_TOL = 24   # per-channel delta from the row's modal colour to count a pixel as ink
SEARCH = 60    # px of offset to search; drift beyond this is a different bug than drift
WINDOW = 48    # rows per correlation window — a couple of list rows, so offset(y) stays local


def ink_per_row(w, h, ch, data):
    out = []
    for y in range(h):
        line = data[y]
        counts = {}
        for x in range(0, w, 3):  # modal colour from a stride sample — the background dominates
            px = line[x * ch:x * ch + 3]
            counts[px] = counts.get(px, 0) + 1
        bg = max(counts, key=counts.get)
        n = 0
        for x in range(w):
            px = line[x * ch:x * ch + 3]
            if max(abs(px[k] - bg[k]) for k in range(3)) > INK_TOL:
                n += 1
        out.append(n)
    return out


def offset_at(pa, pb, y, h):
    """Best offset for pb's window at y against pa, plus how decisive that best was."""
    lo, hi = y, min(y + WINDOW, h)
    win = pb[lo:hi]
    if max(win) - min(win) < 8:
        return None, 0.0  # flat window (pure background) — any offset fits, report nothing
    scored = []
    for off in range(-SEARCH, SEARCH + 1):
        if lo + off < 0 or hi + off > h:
            continue
        ref = pa[lo + off:hi + off]
        scored.append((sum(abs(u - v) for u, v in zip(win, ref)) / len(win), off))
    scored.sort()
    best, off = scored[0]
    # Decisiveness: how much worse the best offset OUTSIDE +-3px of the winner is. Low margin means
    # the alignment is ambiguous and the offset should not be trusted.
    other = next((s for s, o in scored if abs(o - off) > 3), best)
    return off, other - best


def main():
    wa, ha, ca, a = rows(sys.argv[1])
    wb, hb, cb, b = rows(sys.argv[2])
    h = min(ha, hb)
    pa = ink_per_row(wa, ha, ca, a)
    pb = ink_per_row(wb, hb, cb, b)
    step = int(sys.argv[3]) if len(sys.argv) > 3 else 16
    global SEARCH
    if len(sys.argv) > 4:
        SEARCH = int(sys.argv[4])
    print(f"a={wa}x{ha}  b={wb}x{hb}  window={WINDOW}  search=+-{SEARCH}")
    print("   y   drift  margin   (drift = px the PORT sits BELOW maui; margin<3 = untrustworthy)")
    for y in range(0, h - WINDOW, step):
        off, margin = offset_at(pa, pb, y, h)
        if off is None:
            print(f"{y:5d}     --      --   (flat window)")
        else:
            # port row y shows maui row y+off, so the port sits -off px LOWER than maui.
            edge = "   <- AT SEARCH EDGE (raise arg 4)" if abs(off) == SEARCH else ""
            print(f"{y:5d}  {-off:+5d}  {margin:6.1f}"
                  f"{'   <- ambiguous' if margin < 3 else ''}{edge}")


if __name__ == "__main__":
    main()
