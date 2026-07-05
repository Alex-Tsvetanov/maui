# `e2e.py` — the unified MAUI-vs-C++-port E2E tool

One entrypoint for every deterministic step of the verification loop described in
[`port/maui-reference/docs/VERIFICATION_LOOP.md`](../../maui-reference/docs/VERIFICATION_LOOP.md).
It owns the **naming triple** (page key ↔ `port/maui-reference/pages/<key>.xaml` ↔
`MauiReference.Pages.<Pascal>Page` ↔ `examples::Views::<key>_page()`) and progressively absorbs the
legacy script zoo under `port/cpp/tools/parity/` and `port/cpp/docs/comparison/tools/` — each legacy
script is deleted the moment its subcommand replaces it.

Plain python3 stdlib unless noted. Run from anywhere; all paths are derived from the script location.

```sh
python3 port/tools/e2e/e2e.py <subcommand> [flags]
```

## Subcommands

### `gen` — regenerate everything derived from the pages

```sh
python3 port/tools/e2e/e2e.py gen                                    # checked-in #embed TUs + code-behinds
python3 port/tools/e2e/e2e.py gen --embed-mode=bytes --out-dir DIR   # Android NDK byte-array TUs
```

Reads the canonical shared pages (`port/maui-reference/pages/*.xaml`) **plus** any not-yet-migrated
legacy twins (`port/cpp/examples/gallery_xaml/Views/*.xaml`; a shared page supersedes a same-key twin)
and stamps:

- `Views/<key>.xaml.hpp` + `Views/<key>.xaml.cpp` — the C++ TU pair. Shared pages `#embed` the
  canonical file (relative path out of the cpp tree) and `static_assert` the naming triple on the
  embedded bytes; legacy twins keep embedding their local `Views/<key>.xaml`.
- `Views/gallery_pages.hpp` — the aggregator (includes + `MAUI_XAML_GALLERY_PAGES` X-macro).
- `app/Pages/<Pascal>Page.xaml.cs` — trivial MAUI code-behind partial per **shared** page. A
  hand-written partial (one without the GENERATED marker line) is never touched — that is how
  interactive pages wire their `x:Name`-based handlers.

`--embed-mode=bytes` instead inlines each page's bytes as an `unsigned char[]` literal into `--out-dir`
(the Android NDK's Clang 18 has no `#embed`); invoked automatically at CMake configure time by the
`maui_android_apphost_xaml` target.

Migration of a legacy twin = move+adapt the markup to `pages/<key>.xaml` (per
[`AUTHORING.md`](../../maui-reference/docs/AUTHORING.md)), delete `Views/<key>.xaml`, re-run `gen`.

### `lint` — authoring-rule enforcement

```sh
python3 port/tools/e2e/e2e.py lint
```

Checks every shared page + `pages/manifest.json`: no event attributes in markup; `x:Class` present and
matching the naming triple; winfx/**2009** `x:` namespace; `ContentPage` root; a manifest row per page
and a page per manifest row; a code-behind partial per page. Exit 1 with a list on any violation.
Wired into the port's gate (`port/cpp/tools/gate.sh`).

### `keys` — the canonical page-key universe

```sh
python3 port/tools/e2e/e2e.py keys                 # shared pages + legacy twins (the full board)
python3 port/tools/e2e/e2e.py keys --shared-only   # only migrated pages
```

One key per line. Replaces the deprecated `port/cpp/tools/parity/page_keys.txt`; shell tooling
(e.g. `build_android_apphost_xaml.sh`) shells out to this.

## Planned subcommands (absorbing the legacy scripts — see VERIFICATION_LOOP.md steps)

| Subcommand | Will replace | Loop step |
| --- | --- | --- |
| `capture --platform ios\|maccatalyst\|android\|appkit --framework maui\|cpp\|xaml --theme light\|dark [keys…]` | `capture_all.py`, `capture_all_cpp.py`, `capture_maccatalyst.py`, `capture_appkit.py`, `capture_all_csharp_android.sh` (drive half) | 2–4 |
| `record …` | (new) simctl recordVideo / adb screenrecord / macOS `screencapture -v` | 2–4 (P5) |
| `pixel` | `pixel_score.py` | 5 |
| `vision --backend mlx\|ollama\|vllm` | (new) Qwen2.5-VL judge | 6 |
| `judge --engine gemini\|claude` | `run_parity.py`, `gemini_*.py` | 7 |
| `board` | `build_comparison_json.py` + `gen_readme.py` | 8 |
| `catalog` | (new) regenerates `port/maui-reference/docs/CONTROL_CATALOG.md` | 10 |

Capture destinations: `--framework maui` → `port/maui-reference/captures/<platform>/<key>_<theme>.png`
(the mutable MAUI ground-truth tree); `cpp`/`xaml`/`appkit_*` → the existing
`port/cpp/docs/comparison/captures/<platform>/<framework>/` layout. The historical
`port/cpp/docs/comparison/captures/*/maui/` tree is FROZEN — nothing writes there.
