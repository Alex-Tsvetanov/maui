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
  - **image** — FIXED: apple image_handler::get_desired_size was a {0,0} stub; now returns the loaded
    NSImage's aspect-fit size, so Images render (UriSource/FileSource/Font). (image_button GIF still draws
    black — a separate image_button decode issue.)
  - **radio_template_from_style** — FIXED by the shape-fill fix (radio ring/dot are shapes); cpp==xaml.
  - **image_button Animated GIF** — draws black (route the apple image_button load through the GIF-aware
    CGImageSource decode the image handler uses). The only remaining AppKit element gap.

## 3. Android — handler fan-out underway, widget-tested on the emulator ✅
Build + run the widget seam suite (needs `VCPKG_ROOT` + `MAUI_ANDROID_SDK_ROOT=/opt/homebrew/share/android-commandlinetools`):
`cmake --preset android && cmake --build --preset android --target maui_android_widget_tests && ctest --test-dir build/android -R android_testhost_widget_suite --output-on-failure`
(the host is `app_process` via `tools/android-testhost-run.sh`). Cross-compiles for arm64-android (NDK 27.2);
core suites still **509 on-device** (graphics 207 / core 146 / layout 116 / animations 40).

**Real `android.widget` handlers** (each = a `src/platform/android/<ctrl>_handler.cpp` JNI partial + a header
`#ifdef MAUI_PLATFORM_ANDROID` override block + a CMake fan-out pair + emulator seam tests):
button, navigation, window (pre-existing) · **label** (TextView — 14 tests) · **progress_bar** (ProgressBar
horizontal — 7) · **activity_indicator** (ProgressBar indeterminate — 4) · **image** (ImageView — 6) ·
**editor** (EditText — handler compiles + wired; seam test deferred, see lesson 3). The recipe: port
`<Ctrl>Handler.Android.cs` → a partial mirroring `button_handler.cpp` (scoped_env/app_context VM-less
guards, headless-mirror-first then widget push, `default_jni_cache`, global-ref lifecycle, to_pixels/density).
Hand-port the foundational ones; agent-port the rest via `code-changes` agents (button + label + editor are
the templates). The cross-platform `<ctrl>_handler.cpp` + mapper registration are reused unchanged — only the
platform partial swaps via the CMake `list(REMOVE_ITEM headless…)/list(APPEND android…)` pair.

**LESSONS the emulator surfaced — apply to every new handler:**
1. **Static fields need `GetStaticFieldID` + `GetStatic*Field`, NOT `jni_cache::field()`** (that is
   `GetFieldID` = instance-only → returns null for statics; there is no `static_field()` — call `env`
   directly). Bit progress_bar (`R.style`) + activity_indicator (`PorterDuff.Mode`).
2. **Theme-dependent widget ctors throw in the bare `app_process` testhost** (no Activity theme): the default
   ctors of horizontal-ProgressBar (`progressBarStyleHorizontal`) and EditText (`editTextStyle`) resolve a
   theme style attr. Construct theme-independently — 4-arg `(Context, null, 0, R.style.X)` or 3-arg
   `(Context, null, 0)` with `defStyleAttr=0` — plus a plain-ctor fallback. (TextView/Button are fine.)
3. **EditText cannot be constructed in the `app_process` testhost at all** — its `Editor` eagerly queries
   Settings/DeviceConfig (`SelectionActionModeHelper` → `TextClassificationConstants`), which the shell-uid
   (2000) process may not reach → `SecurityException`. So **editor/entry/search_bar (any EditText) can only
   be verified via a real Activity** (the Android app host). Their seam-test files are kept but unwired.

**Remaining Android work:** entry (hand-port the EditText handler — no testhost seam test possible) + ~20 more
controls (slider/switch/check_box/stepper/picker/date+time_picker/search_bar/border/box_view/shapes/…) +
**the Android app host** (a real Activity that mounts the view tree) — now the critical path, since it is the
only route to on-device VISUAL parity (the user's acceptance criterion) AND the only way to verify
EditText-backed controls. See `docs/ANDROID_STATUS.md`.

## Build-system note
Each backend builds in its own dir via a CMake option/preset (headless/apple/ios/maccatalyst/android),
per the chosen "enabling options, keep separate dirs" approach.

## Suggested resume order
1. Android: continue the handler fan-out — entry next (hand-port; EditText, no testhost seam test), then
   slider/switch/check_box/stepper/… via `code-changes` agents (brief them with the 3 lessons above).
2. Android app host: a real Activity that mounts the maui view tree + an emulator capture pipeline — the
   critical path for on-device VISUAL parity and for verifying EditText-backed controls.
3. Catalyst collectionview/items spacing (native get_desired_size — a supervised layout-model refactor).
