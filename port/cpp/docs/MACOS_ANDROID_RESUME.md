# macOS (Catalyst + AppKit) & Android — session resume

Branch `cpp-port-kit-qol-changes`. Everything below is committed + pushed to origin. This doc is the single
entry point; see also `docs/comparison/maccatalyst/APPKIT_FINDINGS.md` and `docs/ANDROID_STATUS.md`.

## Milestones (this session, on the branch)
| commit | what |
|---|---|
| `ce09ec5cfc` | Mac Catalyst backend + standalone `examples/gallery_xaml` + 3-way parity board + 8 fixes |
| `6008b15ff6` | AppKit (native macOS) built + tested — capture harness + board + findings |
| `372a9ff9c8` | portable float `from_chars` for the Android NDK — framework cross-compiles for arm64-android |
| `7067d1ce01` | Android status doc — core suites green on the emulator |
| `9df29cad8f` | Mac Catalyst **dark-theme** board |
| `db2433923a` | **fix**: AppKit shapes/BoxView filled the whole window in layouts (draw over `self.bounds`) |

## 1. Mac Catalyst (UIKit on macOS) — the strict-parity target ✅
.NET MAUI's macOS IS Mac Catalyst, so this is what matches MAUI. New `maccatalyst` preset + toolchain
(`cmake/maccatalyst.toolchain.cmake`, target `arm64-apple-ios26.0-macabi`) + triplet; reuses the iOS UIKit
handlers; `maui_add_app` Catalyst `.app` branch. The whole framework + all 12 examples build and render
real UIKit on macOS. 3-way board (MAUI | C++ builder | C++ & XAML) in `docs/comparison/maccatalyst/`,
**light + dark**, captured by `tools/parity/capture_maccatalyst.py`. ~25 pages are clean 3-way matches;
**8 diffs fixed** (input_controls, border_styles, border_resize_content, pickers TimePicker, radio_button_group,
search_bar, radio ring tint, + loader `DatePicker.Date`/`TimePicker.Time`). Headless stayed 3241/3241.

**Remaining Catalyst diffs (deferred, root-caused in memory):** collectionview/items row spacing
(get_desired_size fixed-100px/entry → needs native content size), stepper bg-width, host resize-relayout,
image_button GIF decode. Plus MAUI-quirks (per policy — keep the port): the `button` settings-icon and
image_button Custom-Size speck (the maui-compare app ships no image asset, so MAUI shows nothing — the
port is arguably *more* correct), DatePicker/TimePicker boxed vs MAUI's borderless-inline.

## 2. AppKit (native NSViews) — the native-look target ✅ (tested; one big fix landed)
Build: `cmake -S examples -B examples/build-apple -G Ninja -DMAUI_BACKEND=apple -DMAUI_EXAMPLES_FRAMEWORK_DIR=.. -DVCPKG_MANIFEST_DIR=<port/cpp> -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/...vcpkg.cmake`
then `cmake --build examples/build-apple --target gallery gallery_xaml`. Capture: `tools/parity/capture_appkit.py`.
Criterion (per user): every coded/XAML element PRESENT in all renders; `appkit_cpp == appkit_xaml`; the
*look* may differ vs MAUI (different framework).
- **FIXED:** the systemic "BoxView/shape fills the whole window" bug (~10 pages) — `graphics_host.mm`
  `drawRect:` drew over the OS `dirtyRect` (the whole window on a flipped/nested NSView) instead of
  `self.bounds`. See `db2433923a` + APPKIT_FINDINGS.md.
- **Remaining AppKit gaps (deferred, diagnosed):**
  - **switch** — FIXED: `switch_handler::platform_arrange` now frames the CONTAINER (the wrapper the
    layout positioned via NeedsContainer) and fills it with the NSSwitch, instead of framing the bare inner
    NSSwitch. All toggles render; no regression on controls_stack/value_controls.
  - **image** — NSImageView shows no image (UriSource/FileSource); investigate the apple image handler.
  - **radio_template_from_style** — builder ControlTemplate renders a solid-blue box (only cpp-vs-xaml diff).

## 3. Android — builds + core verified on-device ✅ (UI handlers deferred)
`cmake --preset android && cmake --build --preset android -j` (skip the experimental `maui_ui` codegen
target). Cross-compiles for arm64-android (NDK r27.2). Core suites pass on the `maui-test` emulator:
graphics 207, core 146, layout 116, animations 40 = **509 on-device**
(`MAUI_ANDROID_SDK_ROOT=/opt/homebrew/share/android-commandlinetools tools/parity/... tools/android-emu-run.sh build/android/<suite>`).
**Gaps (the bulk of remaining Android work):** only button/navigation/window handlers exist under
`src/platform/android/` (~27 controls need JNI/View handlers); no Android app host (so no on-device visual
parity yet). See `docs/ANDROID_STATUS.md`.

## Build-system note
Each backend builds in its own dir via a CMake option/preset (headless/apple/ios/maccatalyst/android),
per the chosen "enabling options, keep separate dirs" approach.

## Suggested resume order
1. AppKit `image` rendering (NSImageView measures to 0 before the async load).
3. Catalyst collectionview/items spacing (native get_desired_size).
4. Android: port more control handlers + stand up an Android app host for visual parity.
