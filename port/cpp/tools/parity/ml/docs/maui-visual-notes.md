# MAUI C++ port – visual regression notes (cpp-port-kit, port/cpp)
## Scoring
- tools/parity/lib/pixel_score.py: MAUI (ground truth) vs cpp ("pixel" slot) and vs xaml ("pixel_xaml"), light+dark (Android light only).
- Metrics: SSIM (11x11 window, grayscale luma) + diff_pct (% px with max channel diff >25/255). Dim mismatch -> LANCZOS resize to smaller (no alignment).
- Thresholds: green SSIM>=0.98 & diff<=1.0%; yellow SSIM>=0.90 & diff<=8%; else red. Missing side -> "blank".
- Animated pages: lib/motion_score.py frame-by-frame worst frame; PASS/FAIL/INVALID/INCONCLUSIVE under `motion` key; non-PASS can't be green.
- Orchestration: tools/parity/review.py (pixel first, then LLM judges sonnet/gemini via lib/gemini.py), recapture.py, full_board.sh.
- Results: docs/comparison/comparison.json -> rendered by docs/comparison/tools/gen_readme.py into README.md.
## Pairs (docs/comparison/captures/<platform>/{maui,cpp,xaml}/*.png)
- 2960 candidate pairs (maui vs cpp/xaml, 4 platforms); 2864 usable (both present, same dims, non-empty).
- 96 unusable, all one side missing: android cpp 4, android xaml 48, maccatalyst cpp 22, windows cpp 20, windows xaml 2. 0 empty, 0 dim mismatch.
- cpp-only usable: 1412. (maccatalyst appkit_* columns not counted.)
## Next step
Run `python3 tools/parity/lib/pixel_score.py --verify` (no write) to get a baseline of current scores vs comparison.json, then recapture the 96 missing frames.
