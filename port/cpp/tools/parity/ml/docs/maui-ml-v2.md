# Region-aware comparator v2 (2026-10-14, box only)
Labels: labeler/labels.json, 76 pairs (38 DIFFERENT, 37 SAME, 1 MINOR; MINOR→SAME). Leave-one-out logistic regression (standardised, C=1), 1000-resample bootstrap CI. Code: labeler/v2feat.py (features → v2_rows.json), labeler/analyze2.py (results → v2_results.txt, v2_wrong.json).

## New features
1. **OCR word diff** (full-res RapidOCR, word boxes): match by text then nearest position → missing / extra / moved (>2% of screen) / changed (unmatched word at the same spot), plus unmatched fraction.
2. **Worst-region scores**: LPIPS spatial map → max and p95 over 32 px tiles (512 px-wide images); DINOv2 patch-distance max.
3. **Motion mask**: per-pixel std across the board GIF frames (MAUI ∪ C++), dilated, excluded from tiles/DINO/element diff. Only 9/76 labelled pairs have GIFs; run-frame dirs are not in the clone.
4. **AA-tolerant element diff**: Gaussian blur σ1.5 on both, |Δ|>40, opening, connected components ≥40 px → count, largest blob %, total %.

## Results (n=76)
| model | AUC | 95% CI | acc @0.5 |
|---|---|---|---|
| v1 baseline (8 global metrics) | 0.806 | 0.70-0.90 | 0.74 |
| **v2 region features only (12)** | **0.907** | **0.83-0.97** | **0.84** |
| v1+v2 (20) | 0.891 | 0.82-0.96 | 0.79 |
Best single v2 features: DINO patch max (masked) 0.76, element largest blob 0.76, LPIPS tile p95 0.74, OCR unmatched frac 0.70. Largest LR weights: DINO max, element max area, LPIPS p95, OCR unmatched frac, OCR changed (element count is negative: many small blobs = AA noise).
MINOR→DIFFERENT gives the same picture (v2 0.915).

## The 14 v1 errors
- **Fixed (11):** border_resize_content/android/dark, border_stroke/android/dark, cv_visual_states/windows/dark, formatted_text/android/dark+light, hybrid_web_view/android/dark+light, hybrid_web_view/ios/dark, image_button/android/dark, scattered_radio_button/android/light, time_picker/ios/dark.
- **Still wrong (3):** carousel_page/android/dark (animation phase; no GIF mask on that capture), header_footer_grid_horizontal/windows/light and header_footer_view/windows/light (rotated vs unrotated footer: same words, so OCR and blobs see little).
- **New errors (9):** absolute_layout ios/dark + windows/dark (box size change: mostly empty colour area, small blobs), device/android/dark (version/platform strings; OCR tokenisation of numbers), scattered_radio_button/android/dark (margin change), polyline_gallery/maccatalyst/light (MINOR margin), and false alarms on header_footer_grid ios light/dark, border_playground/android/light, path_transform_string/android/light (stroke/shape rendering differences Alex accepts).
- Net: 14 → 12 errors, mainly a shift from "text misses" to "geometry of one element".

## Remaining failure types
1. **Geometry/orientation of a single element** (rotation, box size, margins/indent): needs element matching with bounding boxes (OmniParser/contour pairs) and comparing size/position/angle, not appearance.
2. **Animation phase** where no frame sequence exists: capture at a settled state, or use run frames.
3. **Shape/stroke rendering tolerance**: needs calibrated per-element tolerance, or a VLM judge on the flagged region.

## Caveat
76 labels; CI ±0.07. v2 vs v1 (+0.10) is likely real (CIs barely overlap), but feature selection was guided by these same errors, so expect some optimism. Validate on new labels Alex adds.

## Update 2026-10-14 09:50: held-out test + 108 labels
Labels now 108 (32 new since the v2 build: 9 DIFFERENT, 23 SAME). Script: labeler/analyze3.py → v3_results.txt.
- **Held-out** (fit on the original 76, score the 32 new): v2 AUC **0.850 (0.70-0.96)**, acc 0.75; v1 baseline 0.734 (0.51-0.91), acc 0.69. The drop from 0.91 LOO confirms mild optimism, but v2 still beats v1.
- **LOO on all 108**: v2 **0.909 (0.85-0.96)**, acc 0.83, 18 errors; v1 0.815 (0.73-0.90), acc 0.78, 24 errors; v1+v2 0.904.
- Held-out errors (8):
  - *Missed DIFFERENT, small text/state:* scroll_mode_test/maccatalyst/light (text inside an input box), empty_view_rtl/ios/dark (field filled vs empty), containers/ios/dark ("Scrolled to" numbers differ: digits poorly tokenised).
  - *Missed DIFFERENT, rotation:* header_footer_view/windows/dark (same known failure).
  - *False DIFFERENT on Mac Catalyst dark (4):* relative_layout, animation, custom_layout, staggered_layout. A systematic maccatalyst-dark rendering offset or AA that Alex accepts, so a per-platform calibration or offset feature is needed.
