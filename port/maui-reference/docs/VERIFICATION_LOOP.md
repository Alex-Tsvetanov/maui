# The deterministic E2E verification loop

The repeatable 10-step protocol that validates the C++ port against real .NET MAUI, runnable by any
agent (human or AI) on any machine. Ground truth: **MAUI's render of the canonical shared XAML pages**
(`port/maui-reference/pages/*.xaml`) — the exact same files the port's `gallery_xaml` app `#embed`s.
See `AUTHORING.md` for how pages are written, and `port/CLAUDE.md` § "Parity comparison policy" for the
binding visual-fidelity rulings (incl. ruling 5: the four required comparisons — MAUI vs C++ **and**
MAUI vs C++&XAML, in light **and** dark, judged independently by every review model).

## Prerequisites (per machine)

- **All platforms**: python3 (stdlib + Pillow + numpy for `pixel`), Clang 19+ toolchain per
  `port/cpp/PROFILE.md`, `VCPKG_ROOT` set.
- **.NET**: .NET 10 SDK + `maui` workload; `MauiVersion` is pinned to **10.0.71** in
  `app/MauiReference.csproj` (the oracle version — do not float it).
- **iOS/macCatalyst** (macOS hosts): Xcode. The csproj sets `ValidateXcodeVersion=false` so a newer
  Xcode minor than the .NET iOS SDK's pin still builds (verified: SDK 26.5 band + Xcode 26.6).
  iOS simulator booted for captures; the app's `Info.plist` MUST keep `UILaunchScreen` (without it iOS
  letterboxes at ~320pt and every control captures ~1.25× too large).
- **Android**: SDK + emulator (this Mac: `ANDROID_HOME=/opt/homebrew/share/android-commandlinetools`,
  emulator `maui-test`); the csproj pins `EmbedAssembliesIntoApk=true` so `adb install -r` is
  self-contained.
- **Vision judge** (step 6): macOS → `pip3 install mlx-vlm` (model
  `mlx-community/Qwen2.5-VL-7B-Instruct-4bit`, ~4.5 GB, auto-downloaded) or
  `brew install ollama && ollama pull qwen2.5vl:7b`; Linux/CUDA → vllm with
  `Qwen/Qwen2.5-VL-7B-Instruct`.
- **Platform coverage is host-limited**: macOS hosts test macOS + iOS + Android; Windows hosts test
  Windows + Android; Linux hosts test Linux + Android. Capture only what the host supports; the board
  merges per-platform results.

All commands run from the repo root via the **unified tool** `port/tools/e2e/e2e.py` (see its README
for flags). Legacy per-step scripts under `port/cpp/tools/parity/` are being absorbed into it; where a
subcommand is not implemented yet, the loop lists the legacy fallback.

## The 10 steps

1. **Unit equivalence gates (first line of defense — run before any capture):**
   `port/cpp/tools/dev.sh "gallery_twin|gallery_structure|xaml_parity"`
   - load gate: every page hydrates per its `pages/manifest.json` `expected_port_status`;
   - render gate: hydrated pages measure/arrange to non-degenerate frames headlessly;
   - structure equivalence: for every `builder_twin: true` page, the C++ builder tree ==
     the XAML-hydrated tree (`describe()` normalization) — proving C++-only and C++&XAML UI
     definitions are interchangeable;
   - compile-time: generated TUs `static_assert` the naming triple on the embedded bytes.
   A red gate is fixed (or the manifest updated, if a gap genuinely opened/closed) BEFORE capturing.
2. **Capture MAUI** screenshots (+ recordings, step-P5) for all host-supported platforms, light+dark:
   `e2e.py capture --framework maui --platform <p> --theme <t> [keys…]`
   → `port/maui-reference/captures/<platform>/<key>_<theme>.png`
   (fallback until absorbed: `port/cpp/tools/parity/capture_maccatalyst.py --framework maui`, iOS
   `capture_all.py --apps maui`, Android `capture_all_csharp_android.sh`).
3. **Capture C++ & XAML** (`gallery_xaml`): `e2e.py capture --framework xaml …`
   → `port/cpp/docs/comparison/captures/<platform>/xaml/`.
4. **Capture C++-only** (builder `gallery`): `e2e.py capture --framework cpp …`
   → `port/cpp/docs/comparison/captures/<platform>/cpp/`.
5. **Pixel-perfect score**: `e2e.py pixel` (fallback: `port/cpp/tools/parity/pixel_score.py`) — SSIM +
   diff% for all four ruling-5 comparisons → `pixel`/`pixel_xaml` slots in `comparison.json`.
6. **Open-source vision judge**: `e2e.py vision --backend mlx|ollama|vllm` — Qwen2.5-VL judges the same
   four comparisons → its own review slots. Local + free: run it exhaustively before spending API quota.
7. **Sonnet agents**: dispatch Claude (Sonnet 5) agents over the same four comparisons per page
   (batched, vision-capable) → `sonnet`/`sonnet_xaml` slots. (Gemini sweeps remain available as an
   additional independent judge → `gemini`/`gemini_xaml`.)
8. **Review**: regenerate the board (`e2e.py board`; fallback `build_comparison_json.py` +
   `gen_readme.py`) and triage every non-green verdict into: port bug | MAUI quirk (needs a user
   ruling, per CLAUDE.md ruling 3) | capture artifact (recapture) | expected gap (manifest).
9. **Fix**: code fixes in the port (or page/manifest corrections), each verified by re-running step 1
   and re-capturing only the affected keys.
10. **Repeat** from step 1 until the board is green (or every residual is a ruled MAUI quirk /
    documented gap). Update `docs/CONTROL_CATALOG.md` (`e2e.py catalog`) so progress is visible.

## Invariants

- `port/cpp/docs/comparison/captures/*/maui/` is **frozen history** — nothing writes there anymore;
  fresh MAUI captures land ONLY in `port/maui-reference/captures/`.
- MAUI ground truth is XAML-only (no C# twin pages in the reference app).
- The page-key universe = `e2e.py keys` (derived from `pages/*.xaml` + not-yet-migrated legacy twins);
  `page_keys.txt` is deprecated.
- Gap pages (`gap_*.xaml`) are FEATURES of the corpus: they must render in MAUI and fail on the port
  exactly as `manifest.json` records; both directions of drift turn tests red.
