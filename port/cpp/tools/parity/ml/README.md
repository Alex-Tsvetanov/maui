# ML parity judging (experimental)

Learned and region-aware MAUI-vs-C++ screenshot comparison, calibrated against human labels. It reads the existing captures in `docs/comparison/captures/` via `comparison.json`. It does **not** write comparison.json or copy screenshots.

## Setup (CPU is fine)
    python3 -m venv mlenv && . mlenv/bin/activate && pip install -r requirements.txt
DINOv2 is fetched via torch.hub and DreamSim/LPIPS weights download on first run.

## Pipeline
| step | script | output |
|---|---|---|
| prototype (synthetic edits vs noise) | `mlproto.py` | `data/ml_rows.json`, `data/ml_eval.txt` |
| score all MAUI-vs-C++ pairs (SSIM, diff%, DINOv2) | `score_all.py` | `allscores.json` |
| pick 300 pairs, uncertain first | `select.py` | `pairs.json` |
| labelling web UI (SAME/MINOR/DIFFERENT, keys 1/2/3, flicker overlay) | `labeler/server.py` + `index.html` | `labels.json` |
| v1 global metrics + eval | `eval.py`, `analyze.py` | `eval_rows.json` |
| v2 region features (OCR word diff, tile LPIPS/DINO max, motion mask, AA-tolerant blobs) + eval | `v2feat.py`, `analyze2.py` | `v2_rows.json`, `v2_results.txt` |

Paths are currently hard-coded to the original working dir (`/workspace/labeler`, `/workspace/maui/...`). Edit `D`/`COMP` at the top of each script, or run from a matching layout.
The labeler needs a `token` file (random string; not committed). Requests must carry `?t=<token>`. It serves on 127.0.0.1:8787 (expose with a tunnel if needed).

## Results so far (76 labels, LOO-CV)
SSIM AUC 0.40 and diff% 0.53 (no better than chance); v1 combined 0.81; **v2 region features 0.91 (0.83-0.97)**. See `docs/maui-ml-v2.md`, `docs/maui-ml-eval.md` and `docs/maui-ml-parity.md`. `docs/maui-flips.md` and `docs/maui-baseline.md` cover pixel_score `--verify` flips (cause: motion run dirs missing in a fresh clone).
