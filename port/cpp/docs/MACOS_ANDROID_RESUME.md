# macOS (Catalyst + AppKit) & Android — session resume

Branch `cpp-port-kit-qol-changes`. Everything below is committed + pushed to origin. This doc is the single
entry point; see also `docs/comparison/maccatalyst/APPKIT_FINDINGS.md` and `docs/ANDROID_STATUS.md`.

## GOAL (user, 2026-06-30): 172 captures × 3 platforms in docs/comparison
The C++ mirror must be cross-platform like MAUI — every example buildable + runnable on every platform. The
comparison README must show **172 screenshots/recordings for EACH of iOS / macOS / Android** (3-way
MAUI ┃ C++ ┃ C++&XAML per row). Status:
- **iOS ✅ 172** (done).
- **macOS ⏳ 59→172** — `tools/parity/capture_maccatalyst.py` now keys off the full `page_keys.txt` (172). The
  RUN hijacks the host screen (`screencapture` + frontmost), so it is an **away/overnight job**:
  `python3 tools/parity/capture_maccatalyst.py` then `--theme dark`, then `gen_macos_readme_section.py` to
  regenerate the README block. (Also extend AppKit similarly via `capture_appkit.py`.)
- **Android ⏳ 0→172** — needs the **app host** (below) so pages render on the emulator; then `adb exec-out
  screencap` per page, which grabs the EMULATOR framebuffer and does NOT hijack the host screen (so it can run
  while the Mac is in use). The 9-widget handler fan-out is the prerequisite that makes pages render.

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
**slider** (SeekBar — 15) · **editor** / **switch** / **check_box** (handlers compile + wired; seam tests
deferred to the app host — see lesson 3). The recipe: port
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
   ctors of horizontal-ProgressBar (`progressBarStyleHorizontal`), SeekBar (`seekBarStyle`), EditText
   (`editTextStyle`) resolve a theme style attr. Construct theme-independently with the **4-arg
   `(Context, null, 0, R.style.X)`** ctor (defStyleRes — e.g. `Widget_ProgressBar_Horizontal`, `Widget_SeekBar`)
   so the widget keeps its drawables + intrinsic size; the 3-arg `defStyleAttr=0` form constructs but yields a
   drawable-less, **size-0** widget (slider's measure→0). Always add a plain-ctor fallback. (TextView/Button OK.)
3. **TextView-derived INTERACTIVE widgets cannot be constructed in the testhost at all** — their ctor's
   `setText` triggers a Settings/DeviceConfig ContentProvider query the shell-uid (2000) process may not reach
   → `SecurityException`. EditText: `Editor`→`SelectionActionModeHelper`→`TextClassificationConstants`.
   Switch/CheckBox: `notifyListeningManagersAfterTextChanged`→`AutofillManager`. So **editor/entry/switch/
   check_box/search_bar (EditText- or CompoundButton-based) can only be verified via a real Activity** (the app
   host); their seam-test files are kept but unwired. Non-text widgets (View/ProgressBar/ImageView/SeekBar) +
   plain Button/TextView construct fine.

**Remaining Android work:** **the Android app host** is the clear critical path (a real Activity/APK that mounts
the maui view tree + an emulator screenshot pipeline) — the ONLY route to on-device VISUAL parity (the user's
acceptance criterion) AND the only way to verify the app-host-only controls above. With ~9 handlers already
rendering, simple gallery pages can show meaningful content once it exists. Then continue the fan-out for the
rest (entry, stepper, picker, date+time_picker, search_bar, border, box_view, shapes/…) — non-text ones
seam-test in the testhost, text/interactive ones verify in the app host. See `docs/ANDROID_STATUS.md`.

**App host build path (toolchain confirmed in the env):** no gradle, but `build-tools/34.0.0` ships aapt2 + d8
+ apksigner + zipalign and `platforms/android-34/android.jar` is present — and the widget testhost already
proves a `javac → d8 → app_process` pipeline on the emulator (`tools/android-testhost-run.sh`). So the app host
= a minimal **signed APK**: `AndroidManifest.xml` + a `MauiHostActivity.java` that `System.loadLibrary`s the
android gallery lib and JNI-calls a native entry which mounts the maui view tree into the Activity's content
view; built via aapt2 link (manifest) → d8 (dex the Activity) → zip/zipalign/apksigner → `adb install` →
`am start`, then `adb exec-out screencap` per page (mirroring the macOS/iOS capture pipelines) for the 3-way
MAUI ┃ C++ ┃ C++&XAML board. The native gallery already cross-compiles for arm64-android; the ~9 wired handlers
mean simple pages render meaningful content on day one.

**Confirmed mechanism (de-risks it):** `src/platform/android/window_handler.cpp` ALREADY mounts the root page's
native View into a plain `android.widget.FrameLayout` (its `host_content` does the `SetContentView(rootView)`
dance; navigation_handler does the same for page stacks) — so the rendering plumbing EXISTS; the Activity just
needs `setContentView(window's FrameLayout)`. Four pieces to build:
1. **Native JNI `nativeMount(Activity, pageKey)`** (`src/platform/android/apphost/app_host.cpp`) — set_java_vm +
   set_app_context(NewGlobalRef(activity)); build the gallery page for the key; connect a window; mount; return
   the window's FrameLayout. **Confirmed API:** the mount driver is `maui::hosting::mount_window(app, window)` +
   `drive_layout(window, w, h)` (`include/maui/hosting/app_host.hpp` — the SAME path headless/apple `run_app`
   use; android has no `run_app` yet, so this entry IS the android boot). The page-by-key holder is
   `examples/gallery/gallery_host.hpp`'s `MAUI_GALLERY_PAGES` X-macro (runtime string → type-erased
   `gallery_page_holder` whose root the window hosts). Then return `window_platform::native` (the FrameLayout)
   as the jobject. **Resolve when writing:** the gallery page-holder factory signature, `maui_app` construction
   + `add_maui_controls_handlers` (hosting/maui_controls_handlers.hpp), the window→native accessor, and pixel
   display dims (Activity `getResources().getDisplayMetrics()` widthPixels/heightPixels). Model the JNI
   bootstrap on `src/platform/android/testhost/test_host.cpp`'s `nativeRun`. **Template the mount on the
   run_app internals, NOT on the gallery's main.cpp:** `src/platform/headless/host_run.cpp` shows the
   builder → maui_app → mount_window → drive_layout sequence, and `src/platform/apple/host_run.mm` shows
   how the platform run_app reaches + shows the native window view (the android entry returns that view to
   the Activity instead of spinning a run loop). NOTE: `examples/gallery/main.cpp`'s `gallery_app` +
   `make_selected_page` live in an anonymous namespace (not reusable), so app_host.cpp needs its own
   MAUI_GALLERY_PAGES dispatch + a maui_app subclass owning the page+window (or reuse `gallery_host.hpp`'s
   `sample_app` template, which already exposes `win()`). The window's native FrameLayout is
   `window_platform::native` — find the handler→platform_view accessor used inside window_handler.cpp.
2. **`MauiHostActivity.java` + `AndroidManifest.xml`** — onCreate: System.loadLibrary the app-host .so, call
   nativeMount, `setContentView` the returned root view. (Model the JNI bootstrap on `testhost/Bootstrap.java`.)
3. **CMake `SHARED` app-host .so target** — gallery page-builder sources + `maui_controls` + the JNI entry,
   mirroring the `maui_android_widget_tests` target. NOTE: `examples/cmake/maui_add_app.cmake` has no android
   branch yet (only headless/apple/maccatalyst) — either add one or build the .so directly in the root CMake.
4. **Build+run script** — compile .so → aapt2 link (manifest) → d8 (dex the Activity) → zip/zipalign/apksigner →
   `adb install` → `am start` → `adb exec-out screencap`. The page-by-key builder is the gallery's
   `MAUI_SAMPLE_PAGE` switcher. EditText/Switch/CheckBox-based pages will finally render + be verifiable here.

## Build-system note
Each backend builds in its own dir via a CMake option/preset (headless/apple/ios/maccatalyst/android),
per the chosen "enabling options, keep separate dirs" approach.

## Suggested resume order
1. Android: continue the handler fan-out — entry next (hand-port; EditText, no testhost seam test), then
   slider/switch/check_box/stepper/… via `code-changes` agents (brief them with the 3 lessons above).
2. Android app host: a real Activity that mounts the maui view tree + an emulator capture pipeline — the
   critical path for on-device VISUAL parity and for verifying EditText-backed controls.
3. Catalyst collectionview/items spacing (native get_desired_size — a supervised layout-model refactor).
