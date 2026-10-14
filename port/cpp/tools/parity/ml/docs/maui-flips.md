# 50 flips: --verify vs comparison.json (2026-10-13)

## Root cause
All 50 flipped cells are animated/driven (motion-scored) pages, and **all 50 have stills fingerprints identical to the stored ones**, so no captures changed. Where the stored score came from stills, the fresh still SSIM/diff matches it to 4 decimals, so the scorer is deterministic (no resize/library/float drift).
Cause: stored statuses came from per-frame MOTION scoring of run directories (`run 2026-08-2x…`) that are gitignored and missing from a fresh clone. With no frames, motion_score returns an EXPIRED INVALID. `carry_forward()` restores the old motion *verdict* but **not** the status, so the status is recomputed from the single resting still (and capped/uncapped differently). Result: green<->yellow flips in both directions (stored FAIL/PASS-from-frames vs fresh still-only). These are not pages sitting at a threshold: most fresh stills are at SSIM ~0.99-1.00, well clear of 0.98/1%.
4 cells (basic_grouping ios/maccatalyst) had no stored motion block (scored as stills before the page got a scenario), so now they are INVALID and capped yellow. Nothing exists to carry forward for them.
Real defects visible in the stills (independent of the flip): scroll_to_group android dark (C++ capture has the **soft keyboard open**, 22.5% px differ), switch android dark (5.4%), check_box android dark (3.3%), ios_blur_effect android (0.7%, SSIM 0.97). Heatmaps are in maui-flips/ (MAUI | C++ | diff, yellow = >25/255).

## Fix
In pixel_score.main/carry_forward: when every theme's motion is EXPIRED and the stills fingerprint equals the stored one, carry forward the whole cell (status+review+motion), not just the verdict. Optionally `--verify` prints `EXPIRED (carried)` separately from `CHANGED`. Longer-term: commit or archive the per-frame score summaries (small JSON) instead of relying on gitignored run dirs. Hysteresis or pinned deps would not help, since the scorer isn't nondeterministic.
Prototype (simulated on the flip list, not patched into the repo): 50 flips -> **4** (the basic_grouping cells with no prior motion block).

## Per-cell (fresh still SSIM/diff% per theme; stored motion verdict)
| cell | stored -> fresh | stored motion | fresh still light | dark |
|---|---|---|---|---|
| activity_indicator/android/pixel | yellow -> green | FAIL | [0.9986, 0.09] | [0.9984, 0.1] |
| activity_indicator/android/pixel_xaml | yellow -> green | FAIL | [0.9986, 0.09] | [0.9984, 0.1] |
| basic_grouping/ios/pixel | green -> yellow | None | [1.0, 0.0] | [1.0, 0.0] |
| basic_grouping/ios/pixel_xaml | green -> yellow | None | [1.0, 0.0] | [1.0, 0.0] |
| basic_grouping/maccatalyst/pixel | green -> yellow | None | [0.9978, 0.09] | [0.9978, 0.08] |
| basic_grouping/maccatalyst/pixel_xaml | green -> yellow | None | [0.9965, 0.12] | [0.9966, 0.11] |
| basic_grouping/windows/pixel | yellow -> green | FAIL | [1.0, 0.01] | [0.9999, 0.01] |
| basic_grouping/windows/pixel_xaml | yellow -> green | FAIL | [1.0, 0.01] | [0.9999, 0.01] |
| box_view/android/pixel | yellow -> green | PASS | [1.0, 0.0] | [1.0, 0.0] |
| box_view/android/pixel_xaml | yellow -> green | PASS | [1.0, 0.0] | [1.0, 0.0] |
| carousel_page/ios/pixel | yellow -> green | FAIL | [1.0, 0.0] | [1.0, 0.0] |
| carousel_page/ios/pixel_xaml | yellow -> green | FAIL | [1.0, 0.0] | [1.0, 0.0] |
| carousel_page/maccatalyst/pixel | green -> yellow | INVALID | [0.9978, 0.09] | [0.9979, 0.08] |
| carousel_page/maccatalyst/pixel_xaml | green -> yellow | INVALID | [0.9966, 0.12] | [0.9966, 0.11] |
| carousel_page/android/pixel | yellow -> green | PASS | [0.992, 0.35] | [0.9924, 0.36] |
| carousel_page/android/pixel_xaml | yellow -> green | PASS | [0.992, 0.35] | [0.9924, 0.36] |
| check_box/android/pixel | green -> yellow | PASS | [0.9984, 0.09] | [0.9821, 3.43] |
| check_box/android/pixel_xaml | green -> yellow | PASS | [1.0, 0.0] | [0.9835, 3.35] |
| entry/maccatalyst/pixel | green -> yellow | INVALID | [0.9978, 0.09] | [0.9978, 0.08] |
| entry/maccatalyst/pixel_xaml | green -> yellow | INVALID | [0.9965, 0.12] | [0.9966, 0.11] |
| ios_blur_effect/android/pixel | green -> yellow | INVALID | [0.976, 0.48] | [0.97, 0.64] |
| ios_blur_effect/android/pixel_xaml | green -> yellow | INVALID | [0.976, 0.48] | [0.97, 0.64] |
| ios_date_picker/ios/pixel | yellow -> green | FAIL | [1.0, 0.0] | [1.0, 0.0] |
| ios_date_picker/ios/pixel_xaml | yellow -> green | FAIL | [1.0, 0.0] | [1.0, 0.0] |
| radio_content_properties/maccatalyst/pixel | green -> yellow | INVALID | [0.9893, 0.56] | [0.991, 0.49] |
| radio_content_properties/maccatalyst/pixel_xaml | green -> yellow | INVALID | [0.9881, 0.59] | [0.9898, 0.52] |
| scroll_to_group/android/pixel | green -> yellow | PASS | [0.9999, 0.0] | [0.7988, 23.8] |
| scroll_to_group/android/pixel_xaml | green -> yellow | PASS | [0.9999, 0.0] | [0.7988, 23.8] |
| scroll_view/ios/pixel_xaml | yellow -> green | FAIL | [1.0, 0.0] | [1.0, 0.0] |
| scroll_view/android/pixel | yellow -> green | FAIL | [1.0, 0.0] | [1.0, 0.0] |
| scroll_view/android/pixel_xaml | yellow -> green | FAIL | [1.0, 0.0] | [1.0, 0.0] |
| search_bar/windows/pixel | green -> yellow | INVALID | [1.0, 0.0] | [1.0, 0.0] |
| search_bar/windows/pixel_xaml | green -> yellow | INVALID | [1.0, 0.0] | [1.0, 0.0] |
| selection_synchronization/ios/pixel_xaml | red -> green | FAIL | [1.0, 0.0] | [1.0, 0.0] |
| selection_synchronization/maccatalyst/pixel | green -> yellow | INVALID | [0.9978, 0.08] | [0.9978, 0.08] |
| selection_synchronization/maccatalyst/pixel_xaml | green -> yellow | INVALID | [0.9966, 0.11] | [0.9966, 0.11] |
| selection_synchronization/android/pixel | yellow -> green | INCONCLUSIVE | [0.9878, 0.73] | [0.9873, 0.8] |
| selection_synchronization/android/pixel_xaml | yellow -> green | INCONCLUSIVE | [0.9878, 0.73] | [0.9873, 0.8] |
| semantics/windows/pixel | green -> yellow | INVALID | [1.0, 0.01] | [1.0, 0.01] |
| semantics/windows/pixel_xaml | green -> yellow | INVALID | [1.0, 0.01] | [1.0, 0.01] |
| stepper/android/pixel | green -> yellow | INVALID | [0.9999, 0.0] | [0.9988, 0.03] |
| stepper/android/pixel_xaml | green -> yellow | INVALID | [0.9999, 0.0] | [0.9988, 0.03] |
| swipe_refresh/ios/pixel | yellow -> green | FAIL | [1.0, 0.0] | [1.0, 0.0] |
| swipe_refresh/ios/pixel_xaml | yellow -> green | FAIL | [1.0, 0.0] | [1.0, 0.0] |
| swipe_refresh/maccatalyst/pixel | green -> yellow | INVALID | [0.9978, 0.08] | [0.9978, 0.08] |
| swipe_refresh/maccatalyst/pixel_xaml | green -> yellow | INVALID | [0.9966, 0.11] | [0.9966, 0.11] |
| swipe_refresh/windows/pixel | green -> yellow | INVALID | [0.9943, 0.13] | [0.994, 0.13] |
| swipe_refresh/windows/pixel_xaml | green -> yellow | INVALID | [0.9943, 0.13] | [0.994, 0.13] |
| switch/android/pixel | green -> yellow | PASS | [1.0, 0.0] | [0.9754, 5.59] |
| switch/android/pixel_xaml | green -> yellow | PASS | [1.0, 0.0] | [0.9754, 5.59] |
