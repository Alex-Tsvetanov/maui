# ML-based UI parity judging: C++ port vs .NET MAUI (2026-10-13, box only)

## 1. Research summary
- **Pixel/structural (SSIM, diff%)**: cheap and deterministic, but they penalise sub-perceptual shifts and blur and are blind to meaning (a missing label scores the same as AA noise).
- **Learned perceptual**: LPIPS (low-level, AlexNet/VGG), DISTS (structure+texture, tolerant of resampling), **DreamSim** (CLIP+OpenCLIP+DINO ensemble tuned on ~20k human triplets; 96% agreement on NIGHTS vs ~90% for raw DINO; single-branch variants ~3x faster). DINOv2 patch tokens give a *localised* semantic distance plus a heatmap for free.
- **UI-specific**: element detection (OmniParser v2 YOLO + caption, or the platform a11y tree if the capture harness can dump it) → match elements by text/role/IoU → report missing/extra/moved/restyled elements. OCR text-set match catches wrong or missing strings directly. Pros (mizchi/vrt, vlm-diff 2026): run deterministic structure/pixel diff first and send only changed regions plus a "what changed" hint to a VLM. That reached 83% vs 67-75% for raw full-image VLM, and at about half the tokens.
- **VLM judge**: pairwise "would a user consider A and B equivalent? SAME/DIFFERENT + list differences". Known failure modes: position bias (swap order, accept only if consistent), hallucinated differences, missed subtle colour shifts. Mitigation: cross-check with measured delta (refute DIFFERENT when delta≈0, contradict SAME when delta is large). Repo already has lib/gemini.py and review.py LLM slots (sonnet/gemini) that are currently empty.
- **Calibration**: build a labelled set (~300 pairs: the reviewed board cells, plus synthetic edits and synthetic noise); label each SAME/MINOR/DIFFERENT with 2 raters (target Cohen's kappa >0.7). Fit thresholds or a logistic regression on the metric features and report ROC-AUC, precision at a fixed recall, and per-platform splits. For VLM judges, measure agreement with human labels and swap-consistency. Re-validate whenever the model or prompt changes. Keep a frozen golden set as a regression gate.

## 2. Prototype (CPU, venv /workspace/mlenv; script /workspace/mlproto.py; rows ml_rows.json)
Sample: 100 MAUI-vs-C++ still pairs (4 known real diffs + 96 random across platforms/themes), resized to 512 px wide, Android status bar cropped.
Metrics: SSIM, diff%, LPIPS(alex), DINOv2-S/14 patch cosine distance (mean and 99th percentile), RapidOCR text similarity.
Synthetic test (MAUI vs perturbed MAUI): 67 user-visible changes (drop a text label, add a coloured box) vs 67 noise (2-4 px shift, JPEG q60, gamma ±6%, blur 0.8).
Runtime: 175 s for 234 comparisons, about 0.75 s/pair on 6 CPU threads (DINO+LPIPS+OCR). The full board (~1,400 cpp + 1,400 xaml) would take about 35 min on CPU, or seconds per page on a GPU.

| metric | AUC change vs noise | ranks of 4 known real diffs among 100 real pairs |
|---|---|---|
| 1-SSIM | 0.415 | 1,2,5,6 |
| diff% | 0.389 | **1,2,3,5** |
| LPIPS | 0.582 | 1,3,4,8 |
| DINOv2 mean | 0.693 | 1,2,3,22 |
| **DINOv2 p99 patch** | **0.772** | 1,3,4,33 |
| OCR text | 0.450 | 10,47,48,49 (noisy) |
| naive combo | 0.605 | 1,17,20,46 |

Findings:
- SSIM/diff% are *worse than chance* at separating real edits from harmless noise: a 3 px shift gives 3.6% diff and SSIM 0.90 (would be YELLOW/RED), while a deleted label is a tiny pixel change. DINOv2 patch p99 is clearly best here (0.77) and LPIPS is moderate.
- On the current real board the obvious defects are large (open keyboard, wrong dark theme), so pixel diff already ranks them top. ML adds no ranking gain on these 4. ios_blur_effect (a subtle blur rendering difference) drops in DINO, which is the expected semantic-vs-texture trade-off.
- OCR at 512 px is unreliable (maccatalyst small text). It needs full-res crops and token-set (bag-of-words) matching, not raw string ratio. Do not use it as-is.
- Caveats: only 4 real positives, synthetic labels, and no human labels. This is evidence of a direction, not a validated result.

## 3. Recommended pipeline
1. **Tier 0 (all pairs, ms)**: existing SSIM/diff% plus DINOv2 patch-distance map (or DreamSim single-branch). Auto-GREEN when *both* are clean; auto-flag when both are bad. Shift-tolerant: SSIM after a ±4 px alignment search.
2. **Tier 1 (flagged/disagreeing pairs, ~1 s)**: element and text diff. Full-res OCR token sets (missing/extra/changed strings) plus OmniParser or a11y-tree element matching (missing/extra/moved >N px/recoloured). Output structured differences, plus DINO heatmap boxes.
3. **Tier 2 (borderline only, ~5-10% of pairs)**: VLM pairwise judge (Gemini Flash / Claude Sonnet via the existing review.py slots). Input: side-by-side full images plus cropped diff regions plus the Tier-1 hints. Forced SAME/MINOR/DIFFERENT with a list of differences, run in both orders, then cross-checked against Tier 0 (refute or contradict). Cost is roughly 1-3k tokens/pair, about $0.5-3 per full board run if ~150 pairs reach Tier 2.
4. **Validation**: label ~300 pairs (start from the board plus the 4 known diffs plus synthetic edits), fit tier thresholds for ≥95% recall of DIFFERENT, and track precision. Add a golden-set CI gate; re-check swap consistency and agreement after any model or prompt change.

## Next step
Build the ~300-pair human-labelled set (spreadsheet: pair, SAME/MINOR/DIFFERENT, note). Then add DreamSim plus shift-aligned SSIM and full-res OCR token match as Tier 0/1 features and measure AUC on real labels before wiring in the VLM.
