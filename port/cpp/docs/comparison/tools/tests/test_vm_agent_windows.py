#!/usr/bin/env python3
"""Off-guest checks for vm_agent_windows.py's pure helpers.

Runs on the macOS/Linux dev machine (the agent module imports anywhere; only its Win32 calls need a
guest), so the two places a Windows capture can be silently WRONG are verified before the VM exists:

  1. _bgra_to_rgb_rows — a B<->R swap here would tint every screenshot, and the result would look
     completely legitimate: a plausible screenshot with red and blue exchanged, scored against the
     MAUI reference as a real diff. This is exactly the "capture fabricates plausible data" failure
     class the project has already been bitten by, so it gets an explicit per-channel assertion.
  2. _write_png — a malformed chunk/CRC yields a file that exists (so the agent reports ok) but that
     the scorer cannot read. Verified by decoding the bytes back with Pillow, the same library
     pixel_score.py uses, and comparing pixels.

Run: python3 tests/test_vm_agent_windows.py
"""
from __future__ import annotations

import importlib.util
import os
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
AGENT = os.path.join(os.path.dirname(HERE), "vm_agent_windows.py")


def _load_agent():
    spec = importlib.util.spec_from_file_location("vm_agent_windows", AGENT)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)  # imports fine off-Windows thanks to the _IS_WINDOWS guard
    return mod


def test_bgra_to_rgb_rows(agent) -> None:
    """A 2x2 BGRA buffer with one pure channel per pixel must come out as RGB in row order."""
    # DIB memory is B, G, R, A per pixel. Pick colours where a swap is unmistakable.
    px = {
        "red":   bytes((0, 0, 255, 255)),      # B=0   G=0   R=255
        "blue":  bytes((255, 0, 0, 255)),      # B=255 G=0   R=0
        "green": bytes((0, 255, 0, 255)),      # B=0   G=255 R=0
        "grey":  bytes((128, 128, 128, 255)),
    }
    buf = px["red"] + px["blue"] + px["green"] + px["grey"]  # row0: red, blue | row1: green, grey
    rows = agent._bgra_to_rgb_rows(buf, w=2, h=2)

    assert len(rows) == 2, f"expected 2 rows, got {len(rows)}"
    assert all(len(r) == 6 for r in rows), f"expected 3 bytes/px: {[len(r) for r in rows]}"
    assert rows[0] == bytes((255, 0, 0, 0, 0, 255)), (
        f"row0 must be RGB red then RGB blue, got {list(rows[0])} — a B<->R swap tints every capture")
    assert rows[1] == bytes((0, 255, 0, 128, 128, 128)), f"row1 wrong: {list(rows[1])}"
    print("  ok  _bgra_to_rgb_rows: channel order is RGB, alpha dropped, rows top-down")


def test_write_png_roundtrip(agent) -> None:
    """The stdlib PNG must be decodable by Pillow (what pixel_score.py uses) with exact pixels."""
    try:
        from PIL import Image
    except ImportError:
        print("  SKIP _write_png roundtrip: Pillow not installed")
        return

    w, h = 3, 2
    rows = [bytes((255, 0, 0, 0, 255, 0, 0, 0, 255)),        # red, green, blue
            bytes((255, 255, 255, 0, 0, 0, 17, 34, 51))]     # white, black, #112233
    with tempfile.TemporaryDirectory() as d:
        out = os.path.join(d, "probe.png")
        agent._write_png(out, w, h, rows)
        assert os.path.getsize(out) > 0, "PNG is empty"
        im = Image.open(out)
        assert im.mode == "RGB", f"expected RGB, got {im.mode}"
        assert im.size == (w, h), f"expected {(w, h)}, got {im.size}"
        got = im.convert("RGB").tobytes()
        want = bytes((255, 0, 0, 0, 255, 0, 0, 0, 255,
                      255, 255, 255, 0, 0, 0, 17, 34, 51))
        assert got == want, f"pixels differ:\n got {got}\nwant {want}"
    print("  ok  _write_png: Pillow decodes it; pixels exact")


def test_png_of_a_realistic_shot(agent) -> None:
    """End-to-end sanity at capture scale: a 1024x800 BGRA buffer must convert+encode quickly and
    decode to the right size. Guards the performance fix (slice ops, not a per-pixel loop) — the naive
    version took seconds per shot, which is unusable across every step of every scenario."""
    try:
        from PIL import Image
    except ImportError:
        print("  SKIP realistic shot: Pillow not installed")
        return
    import time

    w, h = 1024, 800
    buf = (bytes((200, 100, 50, 255)) * (w * h))  # B=200 G=100 R=50 -> RGB (50,100,200)
    t0 = time.monotonic()
    rows = agent._bgra_to_rgb_rows(buf, w, h)
    with tempfile.TemporaryDirectory() as d:
        out = os.path.join(d, "shot.png")
        agent._write_png(out, w, h, rows)
        dt = time.monotonic() - t0
        im = Image.open(out)
        assert im.size == (w, h), f"expected {(w, h)}, got {im.size}"
        assert im.convert("RGB").getpixel((0, 0)) == (50, 100, 200), (
            f"colour wrong: {im.convert('RGB').getpixel((0, 0))}")
    assert dt < 5.0, f"convert+encode took {dt:.2f}s for one 1024x800 shot — too slow for the runner"
    print(f"  ok  1024x800 shot: convert+encode {dt:.2f}s, size + colour correct")


def test_subcommand_surface(agent) -> None:
    """The agent must expose exactly the macOS agent's subcommands, or the shared host orchestrator
    breaks on whichever one is missing."""
    want = {"set-resolution", "clean", "launch", "window-id", "present",
            "click", "type", "scroll", "shot", "stop"}
    import argparse
    parsers: set[str] = set()
    real_add = argparse._SubParsersAction.add_parser

    def spy(self, name, **kw):
        parsers.add(name)
        return real_add(self, name, **kw)

    argparse._SubParsersAction.add_parser = spy
    try:
        try:
            agent.main(["--help"])
        except SystemExit:
            pass
    finally:
        argparse._SubParsersAction.add_parser = real_add
    missing = want - parsers
    assert not missing, f"missing subcommands vs the macOS agent: {sorted(missing)}"
    print(f"  ok  subcommand surface: all {len(want)} macOS-parity subcommands present")


def main() -> int:
    agent = _load_agent()
    print(f"vm_agent_windows.py checks (host={sys.platform}, agent DPI mode={agent.DPI_MODE})")
    failures = 0
    for fn in (test_bgra_to_rgb_rows, test_write_png_roundtrip,
               test_png_of_a_realistic_shot, test_subcommand_surface):
        try:
            fn(agent)
        except AssertionError as e:
            failures += 1
            print(f"  FAIL {fn.__name__}: {e}")
    print("PASS" if not failures else f"{failures} FAILURE(S)")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
