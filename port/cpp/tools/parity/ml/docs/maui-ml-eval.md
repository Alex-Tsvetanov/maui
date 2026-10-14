# ML parity eval vs Alex's labels (2026-10-14)
Labels: /workspace/labeler/labels.json, 71 at run time (36 DIFFERENT, 34 SAME, 1 MINOR; he labelled 2 more while it ran). Features computed at 512 px wide (Android status bar cropped). OCR uses full-res RapidOCR word sets. DreamSim = open_clip_vitb32 single-branch on CPU. Shift-SSIM = best SSIM over ±4 px shifts. Scripts: labeler/eval.py and analyze.py; rows in labeler/eval_rows.json. Bootstrap = 2000 resamples; LR = standardised logistic regression with leave-one-out CV.


### MINOR->SAME: n=71, positives=36
| metric | AUC | 95% CI | best thr acc |
|---|---|---|---|
| 1-SSIM | 0.398 | 0.27-0.54 | 0.55 @ 0.0055 |
| diff% | 0.525 | 0.38-0.66 | 0.59 @ 0.57 |
| 1-shiftSSIM | 0.400 | 0.26-0.54 | 0.55 @ 0.0055 |
| LPIPS | 0.737 | 0.61-0.85 | 0.70 @ 0.01483 |
| DINO mean | 0.640 | 0.50-0.77 | 0.69 @ 0.005051 |
| DINO p99 | 0.651 | 0.53-0.77 | 0.63 @ 0.09472 |
| DreamSim | 0.667 | 0.54-0.79 | 0.68 @ 0.005489 |
| 1-OCR jaccard | 0.663 | 0.55-0.78 | 0.65 @ 0.1429 |
| LR all (LOO-CV) | 0.814 | 0.70-0.91 | 0.80 @ 0.5 |
| LR DINO p99+OCR+diff (LOO-CV) | 0.685 | 0.55-0.81 | 0.65 @ 0.5 |

### MINOR->DIFFERENT: n=71, positives=37
| metric | AUC | 95% CI | best thr acc |
|---|---|---|---|
| 1-SSIM | 0.420 | 0.29-0.55 | 0.56 @ 0.0055 |
| diff% | 0.537 | 0.40-0.68 | 0.61 @ 0.15 |
| 1-shiftSSIM | 0.414 | 0.27-0.55 | 0.56 @ 0.0055 |
| LPIPS | 0.732 | 0.60-0.84 | 0.69 @ 0.01483 |
| DINO mean | 0.660 | 0.53-0.79 | 0.70 @ 0.005051 |
| DINO p99 | 0.645 | 0.51-0.77 | 0.65 @ 0.05029 |
| DreamSim | 0.679 | 0.55-0.79 | 0.69 @ 0.005489 |
| 1-OCR jaccard | 0.686 | 0.57-0.79 | 0.66 @ 0.1429 |
| LR all (LOO-CV) | 0.804 | 0.69-0.91 | 0.79 @ 0.5 |
| LR DINO p99+OCR+diff (LOO-CV) | 0.710 | 0.59-0.82 | 0.65 @ 0.5 |

## Key findings
- **Pixel metrics are useless against human judgement here.** SSIM AUC is 0.40 (inverted, worse than chance) and diff% is 0.53. Many SAME pairs have large pixel noise (animation phase, fonts/AA), while many DIFFERENT pairs are small text/label changes.
- **Best single metric: LPIPS**, AUC 0.74 (0.61-0.85), 70% accuracy at threshold 0.0148. DreamSim 0.67, OCR word mismatch 0.66, DINO 0.64-0.65.
- **Best overall: logistic regression on all 8 features**, LOO-CV AUC 0.81 (0.70-0.91), 80% accuracy. MINOR handling hardly matters (0.80 either way).

## What the metrics miss (14 LOO errors, contact sheet maui-ml-misclassified.png)
- **Missed DIFFERENT (5): small semantic changes** in a large static screen. Footer text rotated vs not (header_footer_grid_horizontal, header_footer_view), a missing hour value (time_picker), image aspect fill (image_button), one shape out of three (border_resize_content), and ellipsis vs overflow (hybrid_web_view ios). These are local, low-area changes; a global score dilutes them.
- **False DIFFERENT (9): pixel-heavy but user-equivalent.** Animation phase (carousel_page; Alex's note on controls_stack names this as *the* main problem), web content and loading state (hybrid_web_view ×2), text rendering/font differences (formatted_text ×2), radio/border AA (scattered_radio_button, border_stroke), visual-state hover (cv_visual_states).
- Alex's notes: ~60% of DIFFERENT are **text or label presence/content** (labels missing or extra, platform/version strings, picker state), ~25% are **geometry of one element** (box size, rotation, indent/margin), and a few are **image scaling**. Notably 3 notes say the **C++ port is more correct** than MAUI. A 'port better' label would be useful.
- Implication: the next gains come from **element/text-level diffing** (OCR line alignment with positions, per-element bounding boxes), **max-over-regions** instead of global scores, **masking animated or web regions** (or capturing at a settled state), and a VLM judge for local semantics.

## Is 69-71 enough?
No, not for picking among metrics. The 95% CIs are ±0.12-0.15 AUC and all the single metrics except SSIM overlap. CI half-width scales ~1/√n, so: ~150 labels → ±0.09; **~300 → ±0.06** (enough to separate 0.74 from 0.81 and to tune per-tier thresholds); 600 → ±0.045. The current 71 are enough to conclude that SSIM/diff% fail and that a learned combination gets ~0.8. Recommend ≥150 more, prioritising the remaining uncertain pairs (already ordered first in the labeller).

