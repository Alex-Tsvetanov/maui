#!/usr/bin/env python3
"""GIF assembly for the animated parity pages — the one ffmpeg seam every lane shares.

Each platform can only produce motion its capture path allows, so the INPUT differs (a simulator
recording on iOS, a screenrecord mp4 on Android, a burst of stills on the VM lanes) but the OUTPUT is
always the same: captures/<platform>/<framework>/<key>_<theme>.gif, paletted, ~400px wide.

`drop_stale()` is not housekeeping — it is the rule that keeps a failed recording honest.
comparison_paths.find_capture() and build_comparison_json.py both prefer a `.gif` over a `.png`, so a
run that refreshes the PNG and then fails to record leaves the board rendering the PREVIOUS run's GIF
behind a green log line. Delete the old GIF FIRST; a missing GIF falls back to the fresh still, which
is honest, while a stale GIF is a lie.
"""
from __future__ import annotations

import os
import shutil
import subprocess
import tempfile

# 400px wide, paletted: crisp inline playback in the README without a multi-MB cell.
_SCALE = "scale=400:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse"


def drop_stale(gif_path: str) -> None:
    """Remove an existing GIF so a failed recording can never shadow the fresh PNG."""
    if os.path.exists(gif_path):
        os.remove(gif_path)


def _distinct_frames(gif_path: str) -> int:
    """How many DIFFERENT frames the GIF actually holds."""
    try:
        from PIL import Image
    except ImportError:                      # pragma: no cover - PIL ships with the scoring tools
        return 2                             # cannot check; assume it is fine rather than delete work
    seen = set()
    try:
        with Image.open(gif_path) as im:
            while True:
                seen.add(im.convert("RGB").tobytes())
                im.seek(im.tell() + 1)
    except EOFError:
        pass
    except Exception:
        return 0
    return len(seen)


def _ffmpeg(args: list[str], out: str) -> bool:
    """Run ffmpeg and KEEP the output only if it is a real animation.

    ffmpeg creates its output file before it knows whether it can encode anything, so a failed
    conversion leaves a 0-BYTE .gif behind — and since the board prefers .gif over .png, that empty
    file then shadows a perfectly good still and takes pixel_score.py down with an
    UnidentifiedImageError. Measured: one Android board pass wrote 84 of them. So on any failure, and
    on a "GIF" that turns out to be a single repeated frame, the file is DELETED: no GIF means the
    fresh still is used, which is the honest outcome.
    """
    os.makedirs(os.path.dirname(out), exist_ok=True)
    r = subprocess.run(["ffmpeg", "-y", *args, out], capture_output=True, text=True)
    ok = r.returncode == 0 and os.path.exists(out) and os.path.getsize(out) > 0
    if ok and _distinct_frames(out) < 2:
        ok = False                           # nothing moved — a still wearing an animation's name
    if not ok and os.path.exists(out):
        os.remove(out)
    return ok


def video_to_gif(video: str, out: str, fps: int = 12) -> bool:
    """mp4 (simctl recordVideo / adb screenrecord) -> paletted GIF."""
    if not os.path.exists(video) or os.path.getsize(video) == 0:
        return False      # screenrecord fails by writing a 0-byte file, not by erroring
    return _ffmpeg(["-i", video, "-vf", f"fps={fps},{_SCALE}"], out)


def frames_to_gif(frames: list[str], out: str, fps: int = 3) -> bool:
    """A burst of stills -> paletted GIF. The VM lanes can only capture discrete frames, so their
    GIFs are slideshows at the burst's own rate rather than smooth video."""
    frames = [f for f in frames if os.path.exists(f)]
    if len(frames) < 2:
        return False      # one frame is a PNG, not an animation — say so by failing
    with tempfile.TemporaryDirectory() as tmp:
        for i, f in enumerate(frames):
            shutil.copyfile(f, os.path.join(tmp, f"{i:04d}.png"))
        return _ffmpeg(["-framerate", str(fps), "-i", os.path.join(tmp, "%04d.png"),
                        "-vf", _SCALE], out)
