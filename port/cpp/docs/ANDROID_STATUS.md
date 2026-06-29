# Android backend status — 2026-06-29

## What works
- **The whole framework cross-compiles for `arm64-android`** (NDK r27.2, `cmake --preset android`,
  `MAUI_BACKEND=android`). The blocker was floating-point `std::from_chars` (the NDK's libc++ 18 deletes it);
  fixed by routing the two offending sites through the `maui::detail::from_chars_general` shim — see commit
  `372a9ff9c8`. `libmaui_{graphics,core,layouts,controls,xaml,essentials,hosting}.a` all link.
- **Core unit suites PASS on the emulator** (the pre-installed `maui-test` AVD, android-34 google_apis
  arm64-v8a, run via `tools/android-emu-run.sh`):
  - `maui_graphics_tests` — **207/207**
  - `maui_core_tests` — **146/146**
  - `maui_layout_tests` — **116/116**
  - `maui_animations_tests` — **40/40**
  - = **509 cross-platform-logic tests green on-device.**
- Emulator infra is ready: SDK at `/opt/homebrew/share/android-commandlinetools` (adb, emulator,
  system-image), the `maui-test` AVD, and `MAUI_ANDROID_SDK_ROOT` is all `android-emu-run.sh` needs.

## Gaps (deferred — resume later)
- **UI handlers are largely unimplemented.** `src/platform/android/` has only `button`, `navigation`, and
  `window` handlers (+ `android_{view,visual,semantics}_ops.hpp`, the JNI layer, and a `testhost`). The other
  ~27 controls have no Android (JNI/View) handler yet — so handler/render unit suites and any real UI are
  out of scope until those are ported. Porting them is the bulk of the remaining Android work.
- **No Android app/gallery** — the example galleries are desktop/iOS/Catalyst only; there is no
  Android `.apk` host, so there is no on-device *visual* parity capture for Android yet (the macOS/iOS
  parity boards don't extend here).
- **The `maui_ui` build-time codegen target fails for android** (`generated/maui_ui/counter_page.gen.hpp`,
  exit 2) — this is the experimental build-time XAML→`maui::ui` codegen path, separate from the runtime
  loader and the `#embed`/`build_page` path; it does not affect the framework libs or the core tests.

## How to resume
1. Boot: `emulator -avd maui-test -no-window -no-audio -no-boot-anim &` (or let `android-emu-run.sh` boot it).
2. Build: `cmake --preset android && cmake --build --preset android -j` (skip the maui_ui codegen target).
3. Test on-device: `MAUI_ANDROID_SDK_ROOT=/opt/homebrew/share/android-commandlinetools \
   tools/android-emu-run.sh build/android/<suite>` or `ctest --preset android`.
4. Next implementation step: port the remaining control handlers under `src/platform/android/` (mirror the
   existing `button_handler.cpp` JNI recipe) and stand up an Android app host for visual parity.
