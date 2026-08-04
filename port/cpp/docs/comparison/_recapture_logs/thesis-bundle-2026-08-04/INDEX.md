# Parity recapture logs — 2026-08-04

Every log behind the current board, one bundle. All four platforms were recaptured on 2026-08-04
against MAUI 10.0.71; the measurement pass at the end (`2026-08-04-204818`) is the one whose numbers
match the committed `comparison.json` and `measurements.json` included here.

Tool: `port/cpp/tools/parity/recapture.py` (capture + measure) and `review.py` (consistency check).

## Which run is authoritative per platform

| Platform | Run ID | Wall clock | Notes |
|---|---|---|---|
| iOS | `2026-08-04-015802` | part of a 10h24m all-platform run | simulator, UDID-pinned |
| Windows | `2026-08-04-015802` | same run | WinUI 3 on an arm64 UTM guest, session-1 agent |
| Android | `2026-08-04-131802` | 102m 36s | API 34 emulator, demo-mode status bar |
| macOS Catalyst | `2026-08-04-152054` | 325m 4s (both macOS lanes) | UTM guest over SSH |
| macOS AppKit | `2026-08-04-152054` | same run, second lane | `appkit_cpp` / `appkit_xaml` columns |
| Measurement | `2026-08-04-204818` | 4m 34s, 0 failures | scoring + TTFF + artifact sizes |

## Board result (pixel scoring, SSIM ≥0.98 & diff ≤1.0% = green)

`pixel` = MAUI vs C++ (code-first); `pixel_xaml` = MAUI vs C++ & XAML. Both themes, per
port/CLAUDE.md ruling 5 (MAUI judged independently against both port columns).

| Platform | pixel (green/yellow/red) | pixel_xaml (green/yellow/red) |
|---|---|---|
| iOS | 159 / 12 / 1 | 160 / 11 / 1 |
| macOS Catalyst | 161 / 9 / 2 | 160 / 10 / 2 |
| Android | 159 / 10 / 3 | 160 / 10 / 2 |
| Windows | 170 / 0 / 2 | 170 / 0 / 2 |

172 example pages per platform.

## Time to first frame (cold start, 10 reps, `label` page)

Measured host-side; the `res` column is the sampler's own resolution, so differences below ~0.24 s
are not resolvable.

| Platform | Framework | n | median | p95 |
|---|---|---|---|---|
| Android | maui_xaml | 10 | 1.442 s | 1.714 s |
| Android | cpp | 10 | 1.016 s | 1.252 s |
| Android | cpp_xaml | 10 | 1.060 s | 1.184 s |
| iOS | maui_xaml | 10 | 1.990 s | 2.259 s |
| iOS | cpp | 9 | 1.148 s | 1.237 s |
| iOS | cpp_xaml | 9 | 1.210 s | 1.217 s |

macOS and Windows are reported `unmeasured`, not zero: both run on a UTM guest, and the
SSH → screencapture → scp round trip (~1–3 s) dominates any start-up figure, so a number there would
measure the transport rather than the app. That refusal is in the log verbatim.

## Release artifact sizes

| Target | Framework | Size | Stripped |
|---|---|---|---|
| macos-arm64 | maui_xaml | 78.7 MB | 47.3 MB |
| macos-arm64 | cpp | 9.1 MB | 9.1 MB |
| macos-arm64 | cpp_xaml | 22.1 MB | 22.1 MB |
| macos-appkit | appkit_cpp | 9.0 MB | 9.0 MB |
| macos-appkit | appkit_xaml | 21.9 MB | 21.9 MB |

Windows artifacts live on the guest and were not measured (path recorded in the log).

## Files

```
capture/   per-lane capture logs — one line per (platform, framework, theme, page) with elapsed time
build/     framework + gallery build logs for the macOS and iOS lanes
measure/   scoring, TTFF, artifact sizes, plus comparison.json and measurements.json
```

### Gap: iOS has no per-example capture log

The iOS lane captures in-process rather than through the VM runner, so its per-example timeline went
to **stdout only**, and the authoritative iOS run (`2026-08-04-015802`) predates the commit that began
teeing that timeline to a file. What survives for iOS is the build log, the
`promote-reference-captures` log, and `capture/PARTIAL-ios-…-004836.log` — a **partial** (43 of 172
pages) MAUI-reference capture from an earlier aborted run, included only as a format sample.

The iOS *results* are unaffected and fully citable: the frames are on disk and the scores in
`comparison.json` were computed from them. But do not claim a complete per-page iOS capture timeline
exists — it does not. Later runs of any platform will produce one (`<RUN_ID>.log`, as `measure/`
shows for the measurement pass).

### Filtering

The six `capture/2026-08-04-131802-android--*.log` files are **filtered**: the raw logs are ~20,800
lines each, of which ~20,450 are a single benign `com.android.tools.r8.internal…: Should never be
called` stack trace repeated ~1,177 times during APK assembly. Only that trace was removed; every
capture line is intact. Raw originals remain in the parent directory.

## Known caveat: 126 "gif assembly failed" lines in the macOS logs

These are **not** capture failures. 13 of the 172 pages are marked animated and captured as a burst of
12 stills; `tools/parity/lib/gif.py` deliberately deletes any GIF whose frames are byte-identical,
because a stale or empty GIF would shadow a good PNG on the board (an earlier Android pass wrote 84
zero-byte GIFs that way). The burst scenario contains no interaction steps, so:

- **10 pages animate only under input** (`gestures`, `pan_gesture_events`, `pointer_gesture`,
  `swipe_gesture`, `swipe_item_position`, `swipe_refresh`, `ios_pan_gesture`, `ios_swipe_transition`,
  `chrome`, `ios_blur_effect`) — verified 1 distinct frame per theme. They will never animate passively.
- **3 pages animate once on load** (`animation`, `empty_view_load_simulate`, `carousel_page`) and
  finish before the burst begins; catch rate is timing luck.
- `activity_indicator` animates continuously and succeeded, except AppKit dark.

Board impact is nil: with no GIF, the board falls back to the full-resolution PNG, which is what the
pixel scorer reads anyway (`pixel_score.full_res`). Do not report these as failed captures.

## Two facts worth stating precisely in the thesis

1. **Byte-identical captures between the MAUI and port columns are exact parity, not a filing error.**
   Measured across the full board: 306 such cells on iOS, 290 on Android, 292 on Windows, 0 on
   maccatalyst. iOS/Android/Windows captures come from three independent app launches writing to
   separate directories, so identical bytes can only mean identical rendering.
2. **A run exiting 0 is not evidence the frames are correct** — the tool prints this itself on every
   completion. Every claim here is backed by a frame count or a log line, not an exit code.
