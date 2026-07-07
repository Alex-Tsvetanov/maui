# parity tooling (legacy helpers)

The canonical parity flow is now **`port/tools/e2e/e2e.py`** (`gen`/`lint`/`keys`/`capture`/`board`/
`pixel`/`consistency`). The scripts here are the pieces e2e.py has not yet absorbed, plus the shared
path model:

| File | Role |
| --- | --- |
| `comparison_paths.py` | Shared capture/verdict path model — **used by** `docs/comparison/tools/build_comparison_json.py`. |
| `capture_all_cpp.py`, `capture_all_csharp_android.sh`, `build_android_apphost*.sh` | iOS + Android capture (e2e.py `capture` currently covers maccatalyst only). |
| `gemini_compare.py` | Standalone Gemini vision judge for ONE page: `gemini_compare.py <key>` → JSON verdict (exit 75 on quota). `gemini_*_sweep.py` batch it per platform. |
| `pixel_score.py`, `capture_appkit.py`, `capture_maccatalyst.py`, `capture_all.py` | Older per-column capture/scoring, mostly superseded by e2e.py — check before relying on them. |

Gemini API key lives **outside the repo**: `~/.config/maui-parity/gemini_api_key` (mode 600);
resolution `$GEMINI_API_KEY` → that file.

⚠️ **iOS MAUI capture calibration:** the MAUI app MUST declare `UILaunchScreen` in
`Platforms/iOS/Info.plist`, else iOS runs it letterboxed at ~320pt (upscaled ~1.25×) and every MAUI
control renders ~1.25× larger than the native C++ gallery — a whole class of bogus "MAUI larger" diffs.
Both stacks must render native. A `dotnet build -t:Rebuild` is required for the plist change to take.

> Retired (superseded by e2e.py / the 2026-07-01 restructure): `run_parity.py`, `gen_review_md.py`,
> `analyze_gemini.py`, `capture_examples.py`, `capture_all_csharp.py`, `set_claude.py`, `winid.m`,
> and the flat `parity_status.json` / `PARITY_REVIEW.md` board they wrote. Restore from git history if needed.
