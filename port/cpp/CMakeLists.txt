# Root build for the C++23 MAUI port. Selects a native backend by preset/option
# (MAUI_BACKEND), mirroring MAUI's cross-platform/platform split (PROFILE.md §3-§4).
cmake_minimum_required(VERSION 3.28)
project(maui_cpp LANGUAGES CXX)

# ---- Language standard (PROFILE.md §1) ----
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Backend selection (headless first; macOS/iOS done, Windows/Android/Linux later) ----
set(MAUI_BACKEND "headless" CACHE STRING "Native backend: headless|apple|ios|windows|android|linux")
set_property(CACHE MAUI_BACKEND PROPERTY STRINGS headless apple ios windows android linux)
message(STATUS "MAUI_BACKEND = ${MAUI_BACKEND}")

# ---- Warnings (PROFILE.md §1: treat as errors in CI once green) ----
add_compile_options(-Wall -Wextra -Wpedantic)

enable_testing()

# ---- Dependencies (vcpkg manifest: gtest, benchmark) ----
find_package(GTest CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)
include(GoogleTest)

# gtest discovery, backend-aware: the ios preset cross-compiles, so the test binaries cannot run on
# the build host — discovery is deferred to ctest time (PRE_TEST) and runs through the simulator
# runner (CMAKE_CROSSCOMPILING_EMULATOR = tools/ios-sim-run.sh), whose boot-and-spawn round trip
# needs far more than the default 5s discovery timeout. Other backends keep the stock behavior.
function(maui_discover_tests target)
  if(MAUI_BACKEND STREQUAL "ios")
    gtest_discover_tests(${target} DISCOVERY_MODE PRE_TEST DISCOVERY_TIMEOUT 180)
  elseif(MAUI_BACKEND STREQUAL "android")
    # The android runner (tools/android-emu-run.sh) may have to CREATE the maui-test AVD and cold-boot
    # the emulator inside the first discovery call, so its timeout dwarfs even the ios one.
    gtest_discover_tests(${target} DISCOVERY_MODE PRE_TEST DISCOVERY_TIMEOUT 600)
  else()
    gtest_discover_tests(${target})
  endif()
endfunction()

# ---- Sanitizers (Sanitizers.md): target-level flags via an interface library, never global. ----
# Enable one personality at a time via the asan-ubsan / tsan / msan presets. ASan+UBSan and TSan
# are usable on AppleClang/macOS; MSan needs Clang + an instrumented libc++ (Linux only).
option(ENABLE_ASAN_UBSAN "Build with AddressSanitizer + UndefinedBehaviorSanitizer" OFF)
option(ENABLE_TSAN "Build with ThreadSanitizer" OFF)
option(ENABLE_MSAN "Build with MemorySanitizer (Clang/Linux only; unsupported on AppleClang)" OFF)

add_library(maui_sanitizers INTERFACE)
target_compile_options(maui_sanitizers INTERFACE
  $<$<COMPILE_LANGUAGE:CXX>:-g>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-omit-frame-pointer>
)
if(ENABLE_ASAN_UBSAN)
  target_compile_options(maui_sanitizers INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-O1>
    $<$<COMPILE_LANGUAGE:CXX>:-fsanitize=address,undefined>
    $<$<COMPILE_LANGUAGE:CXX>:-fsanitize-address-use-after-scope>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-sanitize-recover=all>)  # UBSan findings abort -> fail tests
  target_link_options(maui_sanitizers INTERFACE -fsanitize=address,undefined)
  message(STATUS "Sanitizers: ASan + UBSan")
endif()
if(ENABLE_TSAN)
  target_compile_options(maui_sanitizers INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-O1>
    $<$<COMPILE_LANGUAGE:CXX>:-fsanitize=thread>)
  target_link_options(maui_sanitizers INTERFACE -fsanitize=thread)
  message(STATUS "Sanitizers: TSan")
endif()
if(ENABLE_MSAN)
  target_compile_options(maui_sanitizers INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-O1>
    $<$<COMPILE_LANGUAGE:CXX>:-fsanitize=memory>
    $<$<COMPILE_LANGUAGE:CXX>:-fsanitize-memory-track-origins=2>)
  target_link_options(maui_sanitizers INTERFACE -fsanitize=memory)
  message(STATUS "Sanitizers: MSan")
endif()

# ---- clang-tidy guardrails (optional; off by default to keep builds fast). Uses port/cpp/.clang-tidy ----
option(ENABLE_CLANG_TIDY "Run clang-tidy on the library sources during compilation" OFF)
if(ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy
    HINTS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin /opt/homebrew/bin)
  if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy: ${CLANG_TIDY_EXE}")
  else()
    message(WARNING "ENABLE_CLANG_TIDY=ON but clang-tidy was not found")
  endif()
endif()

# ======================================================================
# Layer 0 - Graphics  (maui::graphics  <=  Microsoft.Maui.Graphics)
# ======================================================================
# ---- BEGIN apple_shared (canvas core F1): the CoreGraphics canvas, ONE .mm for BOTH Apple
# backends (mirrors C#'s Platforms/MaciOS folder inside the Graphics assembly). OBJCXX must be
# enabled before the maui_graphics target adds the .mm (the later per-backend blocks re-enable it,
# which is a no-op). ----
if(MAUI_BACKEND STREQUAL "apple" OR MAUI_BACKEND STREQUAL "ios")
  enable_language(OBJCXX)
  set(CMAKE_OBJCXX_STANDARD 23)
  set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
  set(MAUI_GRAPHICS_PLATFORM_SOURCES
    src/platform/apple_shared/coregraphics_canvas.mm)
  set_source_files_properties(${MAUI_GRAPHICS_PLATFORM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
else()
  set(MAUI_GRAPHICS_PLATFORM_SOURCES "")
endif()
# ---- END apple_shared ----

add_library(maui_graphics STATIC
  src/graphics/color.cpp
  src/graphics/corner_radius.cpp
  src/graphics/point.cpp
  src/graphics/point_f.cpp
  src/graphics/size.cpp
  src/graphics/size_f.cpp
  src/graphics/rect.cpp
  src/graphics/rect_f.cpp
  src/graphics/vector2.cpp
  src/graphics/matrix3x2.cpp
  src/graphics/path_f.cpp
  src/graphics/path_builder.cpp
  src/graphics/solid_paint.cpp
  src/graphics/gradient_paint.cpp
  src/graphics/linear_gradient_paint.cpp
  src/graphics/radial_gradient_paint.cpp
  src/graphics/picture_pattern.cpp
  src/graphics/paint_pattern.cpp
  src/graphics/pattern_paint.cpp
  src/graphics/shapes/round_rectangle.cpp
  src/graphics/font.cpp
  src/graphics/canvas_state.cpp
  src/graphics/recording_canvas.cpp
  src/graphics/text/text_attributes.cpp
  src/graphics/text/attributed_text_run.cpp
  src/graphics/text/attributed_text.cpp
  ${MAUI_GRAPHICS_PLATFORM_SOURCES}
)
target_include_directories(maui_graphics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(maui_graphics PRIVATE maui_sanitizers)
# ---- BEGIN apple_shared link/include: the CoreGraphics canvas needs CoreGraphics + CoreText +
# Foundation (PUBLIC: the .mm tests and any backend consumer reach CGContext/CTLine through it);
# its header lives beside the .mm, so consumers inside this build use the source-dir include. ----
if(MAUI_BACKEND STREQUAL "apple" OR MAUI_BACKEND STREQUAL "ios")
  target_link_libraries(maui_graphics PUBLIC
    "-framework CoreGraphics" "-framework CoreText" "-framework Foundation")
  target_include_directories(maui_graphics PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/apple_shared)
endif()
# ---- END apple_shared link/include ----

# clang-tidy runs on the port's own sources (its headers are reached transitively); tests and
# benchmarks are excluded to keep the lint focused on the implementation. The graphics library is
# .cpp-only on headless and gains the apple_shared .mm on apple/ios — lint both languages.
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_graphics PROPERTIES
    CXX_CLANG_TIDY "${CLANG_TIDY_EXE}"
    OBJCXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# ---- Graphics tests (GoogleTest, discovered into ctest) ----
# ---- BEGIN apple_shared test sources: the CGBitmapContext pixel-readback tests for the shared
# CoreGraphics canvas run on BOTH Apple backends (.mm, ARC); headless skips them. ----
if(MAUI_BACKEND STREQUAL "apple" OR MAUI_BACKEND STREQUAL "ios")
  set(MAUI_GRAPHICS_PLATFORM_TEST_SOURCES
    tests/graphics/coregraphics_canvas_tests.mm)
  set_source_files_properties(${MAUI_GRAPHICS_PLATFORM_TEST_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
else()
  set(MAUI_GRAPHICS_PLATFORM_TEST_SOURCES "")
endif()
# ---- END apple_shared test sources ----
add_executable(maui_graphics_tests
  tests/graphics/smoke_test.cpp
  tests/graphics/color_tests.cpp
  tests/graphics/color_converter_tests.cpp
  tests/graphics/point_size_tests.cpp
  tests/graphics/rect_tests.cpp
  tests/graphics/path_tests.cpp
  tests/graphics/path_builder_tests.cpp
  tests/graphics/numerics_tests.cpp
  tests/graphics/round_rectangle_tests.cpp
  tests/graphics/gradient_paint_tests.cpp
  tests/graphics/image_pattern_paint_tests.cpp
  tests/graphics/canvas_tests.cpp
  tests/graphics/attributed_text_tests.cpp
  ${MAUI_GRAPHICS_PLATFORM_TEST_SOURCES}
)
# The apple_shared canvas header lives beside its .mm (src/platform/apple_shared), same convention
# as the apple/ios control tests including their backend op headers.
if(MAUI_BACKEND STREQUAL "apple" OR MAUI_BACKEND STREQUAL "ios")
  target_include_directories(maui_graphics_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/apple_shared)
endif()
target_link_libraries(maui_graphics_tests PRIVATE
  maui_graphics
  maui_sanitizers
  GTest::gmock_main   # transitively pulls gmock + gtest + main(); single chain avoids duplicate-lib warnings
)
maui_discover_tests(maui_graphics_tests)
# Lint the test TUs too (kept clean: 0 findings). Same guard as the library targets above.
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_graphics_tests PROPERTIES
    CXX_CLANG_TIDY "${CLANG_TIDY_EXE}"
    OBJCXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# ---- Graphics benchmarks (Google Benchmark; not a ctest test, run manually) ----
add_executable(maui_graphics_benchmarks
  benchmarks/graphics/color_bench.cpp
  benchmarks/graphics/path_bench.cpp
)
target_link_libraries(maui_graphics_benchmarks PRIVATE
  maui_graphics
  maui_sanitizers
  benchmark::benchmark_main   # provides main(); transitively pulls benchmark (single chain)
)

# ======================================================================
# Layer 1 - Core  (maui::core  <=  Microsoft.Maui : property/handler infra, events, dispatcher)
# ======================================================================
# Platform partials: exactly one backend's set is compiled (PROFILE §5's partial-class split). The
# headless backend is the deterministic unit-test target; `apple` swaps in the AppKit .mm twin.
if(MAUI_BACKEND STREQUAL "apple")
  enable_language(OBJCXX)
  set(CMAKE_OBJCXX_STANDARD 23)
  set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
  set(MAUI_CORE_PLATFORM_SOURCES
    src/platform/apple/button_handler.mm
    src/platform/apple/label_handler.mm
    src/platform/apple/entry_handler.mm
    src/platform/apple/switch_handler.mm
    src/platform/apple/check_box_handler.mm
    src/platform/apple/radio_button_handler.mm
    src/platform/apple/slider_handler.mm
    src/platform/apple/stepper_handler.mm
    src/platform/apple/picker_handler.mm
    src/platform/apple/date_picker_handler.mm
    src/platform/apple/time_picker_handler.mm
    src/platform/apple/editor_handler.mm
    # (W7 keyboard-automanager) the AppKit no-op stub of the iOS keyboard scroll-avoidance engine
    # (macOS has no soft keyboard); the real engine is the ios partial.
    src/platform/apple/ios_keyboard_auto_manager.mm
    src/platform/apple/search_bar_handler.mm
    src/platform/apple/progress_bar_handler.mm
    src/platform/apple/activity_indicator_handler.mm
    src/platform/apple/image_handler.mm
    src/platform/apple/image_button_handler.mm
    src/platform/apple/image_source_services.mm
    src/platform/apple/font_manager.mm
    src/platform/apple/layout_handler.mm
    src/platform/apple/content_page_handler.mm
    # (U04 hide-soft-input-on-tapped) AppKit setup_native hook (empty token — macOS has no soft keyboard);
    # the cross-platform tracking is in src/core, the real wiring in the ios partial.
    src/platform/apple/hide_soft_input_on_tapped_manager.cpp
    # (W1-07 containers) the NSView border host (CAShapeLayer stroke via apple_border_ops.hpp) and the
    # NSScrollView scroller (content_view reuses the content_page partial).
    src/platform/apple/border_handler.mm
    # (W2-23 shapes) the shared NSView drawing host + the graphics/shape handlers over it.
    src/platform/apple/graphics_host.mm
    src/platform/apple/graphics_view_handler.mm
    src/platform/apple/shape_view_handler.mm
    src/platform/apple/scroll_view_handler.mm
    # (W2-20 swipe + refresh) the NSView swipe host (pan-driven reveal, documented AppKit deviation) and
    # the refresh host (AppKit has no native pull — IsRefreshing drives a spinner-overlay flag).
    src/platform/apple/swipe_view_handler.mm
    # (W7-U09) the swipe menu-item NSButton handler (AppKit twin of the iOS UIButton recipe).
    src/platform/apple/swipe_item_menu_item_handler.mm
    src/platform/apple/refresh_view_handler.mm
    src/platform/apple/navigation_page_handler.mm
    # (W1-10 tabbed + flyout pages) NSTabViewController / NSSplitViewController hosts.
    src/platform/apple/tabbed_page_handler.mm
    src/platform/apple/flyout_page_handler.mm
    # (W1-11 chrome) the shared NSMenu builder + the NSView toolTip/menu push.
    src/platform/apple/apple_menu_ops.mm
    src/platform/apple/view_chrome_ops.mm
    # (W8-53 focus) the NSWindow makeFirstResponder: focus/unfocus push behind the view_command_mapper.
    src/platform/apple/view_focus_ops.mm
    # (W1-08 web_view) the WKWebView recipe, ONE .mm shared with the ios backend (WebKit's API is
    # identical on AppKit and UIKit — the gcd_dispatcher convention).
    src/platform/apple_shared/web_view_handler.mm
    # (G5 hybrid_web_view) the WKWebView + WKScriptMessageHandler bridge, ONE .mm shared with ios (the
    # script-message handler is identical on AppKit and UIKit — the web_view_handler convention).
    src/platform/apple_shared/hybrid_web_view_handler.mm
    src/platform/apple/window_handler.mm
    # (W1-14 animations) the NSTimer platform ticker (AppKit has no CADisplayLink; see the .mm).
    src/platform/apple/platform_ticker.mm
    # (M-L hosting-app-builder) the GCD main-queue dispatcher, SHARED with the ios backend (one .mm —
    # GCD is identical on both; the analog of a single Dispatcher.MaciOS.cs partial).
    src/platform/apple_shared/gcd_dispatcher.mm)
  set_source_files_properties(${MAUI_CORE_PLATFORM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
elseif(MAUI_BACKEND STREQUAL "ios")
  enable_language(OBJCXX)
  set(CMAKE_OBJCXX_STANDARD 23)
  set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
  # CMake defaults executables to .app bundles on Apple embedded platforms; the test executables are
  # plain console binaries run via `simctl spawn` (tools/ios-sim-run.sh), not installable apps.
  set(CMAKE_MACOSX_BUNDLE OFF)
  # M6 fan-out COMPLETE: every control has a real UIKit partial (no headless fallbacks remain) — the
  # button (scaffold), label/entry/image + the image source services, the three containers (layout /
  # content_page / navigation_page), and the window.
  set(MAUI_IOS_MM_SOURCES
    src/platform/ios/button_handler.mm
    src/platform/ios/label_handler.mm
    src/platform/ios/entry_handler.mm
    src/platform/ios/switch_handler.mm
    src/platform/ios/check_box_handler.mm
    src/platform/ios/radio_button_handler.mm
    src/platform/ios/slider_handler.mm
    src/platform/ios/stepper_handler.mm
    src/platform/ios/picker_handler.mm
    src/platform/ios/date_picker_handler.mm
    src/platform/ios/time_picker_handler.mm
    src/platform/ios/editor_handler.mm
    # (W7 keyboard-automanager) the KeyboardAutoManagerScroll scroll-avoidance engine
    # (KeyboardAutoManagerScroll.cs); the next-responder walk lives header-only in
    # ios_keyboard_manager_ops.hpp (KeyboardAutoManager.cs).
    src/platform/ios/ios_keyboard_auto_manager.mm
    src/platform/ios/search_bar_handler.mm
    src/platform/ios/progress_bar_handler.mm
    src/platform/ios/activity_indicator_handler.mm
    src/platform/ios/image_handler.mm
    src/platform/ios/image_button_handler.mm
    src/platform/ios/image_source_services.mm
    src/platform/ios/font_manager.mm
    src/platform/ios/layout_handler.mm
    src/platform/ios/content_page_handler.mm
    # (U04 hide-soft-input-on-tapped) the iOS setup_native hook (HideSoftInputOnTappedChangedManager.iOS.cs
    # SetupHideSoftInputOnTapped(UIView)) + the resign-first-responder tap recognizer it arms
    # (ResignFirstResponderTouchGestureRecognizer.iOS.cs); the cross-platform tracking is in src/core.
    src/platform/ios/hide_soft_input_on_tapped_manager.mm
    src/platform/ios/resign_first_responder_touch_gesture_recognizer.mm
    # (W1-07 containers) the UIView border host (CAShapeLayer stroke via ios_border_ops.hpp) and the
    # UIScrollView scroller (content_view reuses the content_page partial).
    src/platform/ios/border_handler.mm
    # (W2-23 shapes) the shared UIView drawing host + the graphics/shape handlers over it.
    src/platform/ios/graphics_host.mm
    src/platform/ios/graphics_view_handler.mm
    src/platform/ios/shape_view_handler.mm
    src/platform/ios/scroll_view_handler.mm
    # (W2-20 swipe + refresh) the UIView swipe host (pan-driven reveal) + the UIRefreshControl host.
    src/platform/ios/swipe_view_handler.mm
    # (W7-U09) the swipe menu-item UIButton handler (UserInteractionEnabled=false + frame KVO + parent notify).
    src/platform/ios/swipe_item_menu_item_handler.mm
    src/platform/ios/refresh_view_handler.mm
    src/platform/ios/navigation_page_handler.mm
    # (W1-10 tabbed + flyout pages) UITabBarController / UISplitViewController child-VC hosts.
    src/platform/ios/tabbed_page_handler.mm
    src/platform/ios/flyout_page_handler.mm
    # (W1-11 chrome) tooltip documented no-op + the UIContextMenuInteraction attach.
    src/platform/ios/view_chrome_ops.mm
    # (W8-53 focus) the UIView becomeFirstResponder/resignFirstResponder push behind the view_command_mapper.
    src/platform/ios/view_focus_ops.mm
    # (W1-08 web_view) the WKWebView .mm shared with the apple backend (see above).
    src/platform/apple_shared/web_view_handler.mm
    # (G5 hybrid_web_view) the WKWebView + WKScriptMessageHandler bridge .mm shared with apple (see above).
    src/platform/apple_shared/hybrid_web_view_handler.mm
    src/platform/ios/window_handler.mm
    # (W1-14 animations) the CADisplayLink platform ticker (PlatformTicker.iOS.cs, 1:1).
    src/platform/ios/platform_ticker.mm
    # (M-L hosting-app-builder) the GCD dispatcher .mm shared with the apple backend (see above).
    src/platform/apple_shared/gcd_dispatcher.mm)
  set(MAUI_CORE_PLATFORM_SOURCES
    ${MAUI_IOS_MM_SOURCES})
  set_source_files_properties(${MAUI_IOS_MM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
else()
  # headless AND android (M-android scaffold): the android preset compiles these pure-C++ headless
  # partials unchanged and runs them on the emulator — exactly how the ios bring-up started before its
  # per-control .mm fan-out. The JNI fan-out (src/platform/android/) swaps them out control by control.
  set(MAUI_CORE_PLATFORM_SOURCES
    src/platform/headless/button_handler.cpp
    src/platform/headless/label_handler.cpp
    src/platform/headless/entry_handler.cpp
    src/platform/headless/switch_handler.cpp
    src/platform/headless/check_box_handler.cpp
    src/platform/headless/radio_button_handler.cpp
    src/platform/headless/slider_handler.cpp
    src/platform/headless/stepper_handler.cpp
    src/platform/headless/picker_handler.cpp
    src/platform/headless/date_picker_handler.cpp
    src/platform/headless/time_picker_handler.cpp
    src/platform/headless/editor_handler.cpp
    # (W7 keyboard-automanager) the headless no-op stub of the iOS keyboard scroll-avoidance engine
    # (no native windowing/keyboard); lets the suite link and run. The real engine is the ios partial.
    src/platform/headless/ios_keyboard_auto_manager.cpp
    src/platform/headless/search_bar_handler.cpp
    src/platform/headless/progress_bar_handler.cpp
    src/platform/headless/activity_indicator_handler.cpp
    src/platform/headless/image_handler.cpp
    src/platform/headless/image_button_handler.cpp
    src/platform/headless/image_source_services.cpp
    src/platform/headless/font_manager.cpp
    src/platform/headless/layout_handler.cpp
    src/platform/headless/content_page_handler.cpp
    # (U04 hide-soft-input-on-tapped) headless setup_native hook (empty token — no soft keyboard off-
    # device); the cross-platform tracking is in src/core, the real wiring in the ios partial.
    src/platform/headless/hide_soft_input_on_tapped_manager.cpp
    src/platform/headless/border_handler.cpp
    # (W2-23 shapes) the op-recording graphics/shape drawing seats.
    src/platform/headless/graphics_view_handler.cpp
    src/platform/headless/shape_view_handler.cpp
    src/platform/headless/scroll_view_handler.cpp
    # (W2-20 swipe + refresh) the swipe state machine (synthetic-offset driven) + the refresh pull mirror.
    src/platform/headless/swipe_view_handler.cpp
    # (W7-U09) the swipe menu-item button mirror (records the mapped text/colour/font/source/visibility).
    src/platform/headless/swipe_item_menu_item_handler.cpp
    src/platform/headless/refresh_view_handler.cpp
    src/platform/headless/navigation_page_handler.cpp
    # (W1-10 tabbed + flyout pages) the tab-host + two-pane mirrors.
    src/platform/headless/tabbed_page_handler.cpp
    src/platform/headless/flyout_page_handler.cpp
    src/platform/headless/view_chrome_ops.cpp # (W1-11 chrome) tooltip/context-flyout native push (no-op)
    src/platform/headless/view_focus_ops.cpp # (W8-53 focus) first-responder push (no-op; focus always succeeds)
    src/platform/headless/web_view_handler.cpp
    # (G5 hybrid_web_view) the message-bridge mirror (records sent/received messages, the invoke-task table).
    src/platform/headless/hybrid_web_view_handler.cpp
    src/platform/headless/window_handler.cpp
    # (W1-14 animations) the deterministic manual_ticker is the headless platform ticker.
    src/platform/headless/platform_ticker.cpp)
  if(MAUI_BACKEND STREQUAL "android")
    # M-android per-control fan-out (unit 28): the JNI partial REPLACES the headless one control by
    # control, exactly how the ios .mm partials displaced their headless twins. The android partial
    # keeps the headless mirrors live (the pure-native cross-platform suite runs on the emulator
    # WITHOUT a Java VM) and additionally drives the real android.widget view when a VM + app
    # context exist (the widget test host / a real app host).
    list(REMOVE_ITEM MAUI_CORE_PLATFORM_SOURCES src/platform/headless/button_handler.cpp)
    list(APPEND MAUI_CORE_PLATFORM_SOURCES src/platform/android/button_handler.cpp)
    # (W4-34d) navigation + window: the FrameLayout-based page-stack container + the Activity
    # content-view stand-in. The android partials keep the headless mirrors live (VM-less suite) and
    # additionally drive the real FrameLayout child swap when a VM + app context exist (the test host).
    list(REMOVE_ITEM MAUI_CORE_PLATFORM_SOURCES src/platform/headless/navigation_page_handler.cpp)
    list(APPEND MAUI_CORE_PLATFORM_SOURCES src/platform/android/navigation_handler.cpp)
    list(REMOVE_ITEM MAUI_CORE_PLATFORM_SOURCES src/platform/headless/window_handler.cpp)
    list(APPEND MAUI_CORE_PLATFORM_SOURCES src/platform/android/window_handler.cpp)
  endif()
endif()

add_library(maui_core STATIC
  src/core/manual_dispatcher.cpp
  src/core/bindable_object.cpp
  src/core/property_path.cpp
  src/core/binding_expression.cpp
  src/core/thickness.cpp
  src/core/font.cpp
  src/core/font_attributes.cpp
  src/core/font_file.cpp
  src/core/font_registrar.cpp
  src/core/font_manager.cpp
  src/core/date_time.cpp
  src/core/property_mapper.cpp
  src/core/command_mapper.cpp
  src/core/handler_registry.cpp
  src/core/view_platform_base.cpp
  src/core/view_mapper.cpp
  src/core/view_command_mapper.cpp # (W8-53 focus) the shared IView Focus/Unfocus command mapper
  src/core/shadow.cpp
  src/core/button_handler.cpp
  src/core/label_handler.cpp
  src/core/entry_handler.cpp
  src/core/switch_handler.cpp
  src/core/check_box_handler.cpp
  src/core/radio_button_handler.cpp
  src/core/slider_handler.cpp
  src/core/stepper_handler.cpp
  src/core/picker_handler.cpp
  src/core/date_picker_handler.cpp
  src/core/time_picker_handler.cpp
  src/core/editor_handler.cpp
  src/core/search_bar_handler.cpp
  src/core/progress_bar_handler.cpp
  src/core/activity_indicator_handler.cpp
  src/core/image_handler.cpp
  src/core/image_button_handler.cpp
  src/core/image_source_loader.cpp
  src/core/image_source_service_registry.cpp
  src/core/image_source_services.cpp
  src/core/uri_bytes.cpp
  src/core/uri_image_disk_cache.cpp
  src/core/crc64.cpp
  src/core/layout_handler.cpp
  src/core/layout_z_order.cpp
  src/core/content_page_handler.cpp
  # (U04 hide-soft-input-on-tapped) the cross-platform HideSoftInputOnTapped tracking
  # (HideSoftInputOnTappedChangedManager.Platform.cs); the native gesture wiring (setup_native) is the
  # per-backend partial under src/platform/<backend>/hide_soft_input_on_tapped_manager.{cpp,mm}.
  src/core/hide_soft_input_on_tapped_manager.cpp
  src/core/border_handler.cpp # (W1-07 containers)
  # (W2-23 shapes) the canvas-drawn + shape-rendering handlers and the shape drawable.
  src/core/shape_drawable.cpp
  src/core/graphics_view_handler.cpp
  src/core/shape_view_handler.cpp
  src/core/scroll_view_handler.cpp
  # (W2-20 swipe + refresh) the cross-platform mapper/command tables + the shared swipe state machine.
  src/core/swipe_view_handler.cpp
  src/core/swipe_view_machine.cpp
  # (W7-U09) the swipe menu-item (UIButton) handler — mapper table + GetTextColor extension.
  src/core/swipe_item_menu_item_handler.cpp
  src/core/refresh_view_handler.cpp
  src/core/navigation_page_handler.cpp
  # (W1-10 tabbed + flyout pages) the cross-platform mapper tables.
  src/core/tabbed_page_handler.cpp
  src/core/flyout_page_handler.cpp
  src/core/web_view_handler.cpp
  src/core/web_view_helper.cpp
  # (G5 hybrid_web_view) the cross-platform JS↔native message protocol (HybridWebView.js host side).
  src/core/hybrid_web_view_protocol.cpp
  src/core/window_handler.cpp
  src/core/grid_length.cpp
  # (W1-14 animations) maui::animations <= Microsoft.Maui.Animations (the Core animation engine).
  src/animations/easing.cpp
  src/animations/lerp.cpp
  src/animations/animation.cpp
  src/animations/lerping_animation.cpp
  src/animations/ticker.cpp
  src/animations/animation_manager.cpp
  ${MAUI_CORE_PLATFORM_SOURCES}
)
target_include_directories(maui_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# Layer 1 depends on layer 0: core contracts expose graphics value types (color in i_text_style,
# rect/size in i_view). PUBLIC so consumers of maui_core link maui_graphics transitively.
target_link_libraries(maui_core PUBLIC maui_graphics PRIVATE maui_sanitizers)
# The Apple backend's .mm partials use AppKit/Foundation; Cocoa is PUBLIC so test/app consumers link it.
# QuartzCore (PUBLIC) supplies the CATransform3D* C functions apple_view_ops.hpp's apply_transform uses
# to build the render transform (Cocoa does not transitively link those symbols).
# MAUI_PLATFORM_APPLE is PUBLIC so every TU in the apple build (controls, tests) sees the same platform
# struct definitions (with the view_platform_base update_* overrides) — keeping the class ODR-consistent
# across the build (only one backend is ever compiled/linked together).
if(MAUI_BACKEND STREQUAL "apple")
  # WebKit carries the WKWebView the (W1-08) web_view handler drives.
  target_link_libraries(maui_core PUBLIC "-framework Cocoa" "-framework QuartzCore" "-framework WebKit")
  target_compile_definitions(maui_core PUBLIC MAUI_PLATFORM_APPLE)
  # (W2-23) the drawing host renders through the shared CoreGraphics canvas, whose header lives
  # beside its .mm (the maui_graphics apple_shared convention).
  target_include_directories(maui_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/apple_shared)
endif()
# The iOS twin of the apple block: UIKit instead of Cocoa (QuartzCore again for the layer C functions),
# plus Foundation/CoreFoundation/libobjc explicitly — unlike macOS's Cocoa umbrella, iOS's UIKit does
# not re-export them, and the ARC'd .mm code calls straight into all three (NSString/NSAttributedString,
# CFRelease for the retained native slot, objc_retain/objc_msgSend/objc_setAssociatedObject).
# ImageIO + CoreGraphics carry the CGImageSource* / CGImage* decode the image services use (and the
# tests' CGImageDestination GIF fixture). MAUI_PLATFORM_IOS (PUBLIC, sibling of MAUI_PLATFORM_APPLE)
# selects the ios update_* override blocks in the shared platform structs for every TU of the build.
if(MAUI_BACKEND STREQUAL "ios")
  # ImageIO + CoreGraphics: the image services decode through CGImageSource and ios_visual_ops.hpp
  # calls the CGPath*/CGColor* C functions directly (UIKit re-exports neither, unlike macOS's Cocoa
  # umbrella); PUBLIC so the .mm tests (which also encode fixtures via CGImageDestination) link them.
  # WebKit (like on apple) carries the WKWebView the (W1-08) web_view handler drives.
  target_link_libraries(maui_core PUBLIC
    "-framework UIKit" "-framework QuartzCore" "-framework Foundation" "-framework CoreFoundation"
    "-framework ImageIO" "-framework CoreGraphics" "-framework WebKit" objc)
  target_compile_definitions(maui_core PUBLIC MAUI_PLATFORM_IOS)
  # (W2-23) the drawing host renders through the shared CoreGraphics canvas (see the apple block).
  target_include_directories(maui_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/apple_shared)
endif()
# The android twin of the two blocks above: MAUI_PLATFORM_ANDROID (PUBLIC, the same ODR rule)
# selects the android update_* override blocks in the shared platform structs for every TU of the
# build; the JNI seam headers (src/platform/android/jni) are PRIVATE — only the android partials
# reach them (jni.h itself comes from the NDK sysroot, no extra link library needed: every JNI call
# goes through the JNIEnv*/JavaVM* function tables).
if(MAUI_BACKEND STREQUAL "android")
  target_compile_definitions(maui_core PUBLIC MAUI_PLATFORM_ANDROID)
  target_include_directories(maui_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/android)
endif()
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_core PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# ---- Core tests (GoogleTest, discovered into ctest) ----
add_executable(maui_core_tests
  tests/core/event_tests.cpp
  tests/core/dispatcher_tests.cpp
  tests/core/specificity_tests.cpp
  tests/core/bindable_tests.cpp
  tests/core/primitives_tests.cpp
  tests/core/keyboard_tests.cpp # (W8-53) the keyboard value type + keyboard_flags
  tests/core/safe_area_tests.cpp # (X4) safe_area_regions + safe_area_edges value types
  tests/core/date_time_tests.cpp
  tests/core/contracts_tests.cpp
  tests/core/handler_tests.cpp
  tests/core/binding_tests.cpp
  tests/core/property_path_tests.cpp
  tests/core/crc64_tests.cpp
  tests/core/font_manager_tests.cpp
)
target_link_libraries(maui_core_tests PRIVATE
  maui_core
  maui_sanitizers
  GTest::gmock_main
)
maui_discover_tests(maui_core_tests)
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_core_tests PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()
# (M-L hosting-app-builder) the GCD dispatcher seam on a pumped main run loop. ONE .mm shared by the
# apple + ios presets — the dispatcher under test (src/platform/apple_shared/gcd_dispatcher.mm) is
# shared the same way, and the test pumps CFRunLoop (Foundation), identical on AppKit/UIKit.
if(MAUI_BACKEND STREQUAL "apple" OR MAUI_BACKEND STREQUAL "ios")
  target_sources(maui_core_tests PRIVATE tests/core/gcd_dispatcher_tests.mm)
  set_source_files_properties(tests/core/gcd_dispatcher_tests.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
  if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
    set_target_properties(maui_core_tests PROPERTIES OBJCXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
  endif()
endif()

# ---- Android JNI seam + the app_process widget TEST HOST (M-android milestone 3) ----
if(MAUI_BACKEND STREQUAL "android")
  # The seam's pure transcoders (jni_string.hpp's UTF-8 ⇄ UTF-16) run as ordinary on-emulator gtest
  # cases in the regular suite — no Java VM needed; the jstring/widget round trip is covered by the
  # test host below. The seam headers live beside the future android handler sources
  # (src/platform/android), mirroring how the apple/ios test exes reach their op headers.
  target_sources(maui_core_tests PRIVATE tests/platform/jni_string_tests.cpp)
  target_include_directories(maui_core_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/android)

  # The PROVEN test-host decision that gates the android fan-out (the full rationale is in
  # tools/android-testhost-run.sh's header): gtest cases drive REAL android.widget views inside an
  # app_process-spawned ART runtime — no APK, no gradle. The suite is a SHARED library
  # (System.load()ed by testhost/Bootstrap.java, which hands a themed Context + argv across JNI to
  # test_host.cpp's gtest main); per-control android units add their widget-test TUs here.
  # Registered as ONE ctest test through the host-side script — deliberately NOT wrapped by the
  # cross-compiling emulator (the script owns the device round trip), hence plain add_test instead
  # of maui_discover_tests.
  add_library(maui_android_widget_tests SHARED
    src/platform/android/testhost/test_host.cpp
    tests/platform/android_widget_tests.cpp
    # (unit 28) the button Rosetta: the real control + handler drive a real android.widget.Button
    # both directions (property pushes read back over JNI; performClick → clicked). Links
    # maui_controls so the cross-platform seam is the one under test.
    tests/platform/android_button_tests.cpp
    # (W4-34d) navigation + window: the real navigation_page / window controls + handlers drive a real
    # android.widget.FrameLayout — set window content → the root container hosts it; push/pop → the
    # visible native child swaps; push/pop modal → the overlay child appears/clears.
    tests/controls/navigation_window_android_tests.cpp
    # (W4-34e) the shared generic-IView view-mapper + semantics: render transform / flow direction /
    # background / semantics pushed to a real android.view.View via the shared android_*_ops helpers,
    # read back over JNI.
    tests/platform/android_view_mapper_tests.cpp)
  target_include_directories(maui_android_widget_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/android)
  target_link_libraries(maui_android_widget_tests PRIVATE GTest::gtest maui_controls maui_sanitizers)
  if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
    set_target_properties(maui_android_widget_tests PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
  endif()
  add_test(NAME android_testhost_widget_suite
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/android-testhost-run.sh
            $<TARGET_FILE:maui_android_widget_tests>
            ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/android/testhost/Bootstrap.java)
  # The first run may create the AVD + cold-boot the emulator inside the test (exactly like the
  # PRE_TEST gtest discovery above) and compiles/dexes the bootstrap — same generous ceiling.
  set_tests_properties(android_testhost_widget_suite PROPERTIES TIMEOUT 600)
endif()

# ---- Animations tests (W1-14: the maui::animations engine; GoogleTest, discovered into ctest) ----
# Backend-agnostic suites (easing value tables, the animation/composition semantics, the manager over
# the deterministic manual_ticker + manual_dispatcher) run everywhere; each Apple backend adds a short
# real-time assertion that its native frame source actually fires (generous deadlines, no exact frame
# counts — see the .mm files).
add_executable(maui_animations_tests
  tests/animations/easing_tests.cpp
  tests/animations/animation_tests.cpp
  tests/animations/animation_manager_tests.cpp
)
if(MAUI_BACKEND STREQUAL "apple")
  target_sources(maui_animations_tests PRIVATE tests/animations/ticker_apple_tests.mm)
  set_source_files_properties(tests/animations/ticker_apple_tests.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
elseif(MAUI_BACKEND STREQUAL "ios")
  target_sources(maui_animations_tests PRIVATE tests/animations/ticker_ios_tests.mm)
  set_source_files_properties(tests/animations/ticker_ios_tests.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
endif()
target_link_libraries(maui_animations_tests PRIVATE
  maui_core
  maui_sanitizers
  GTest::gmock_main
)
maui_discover_tests(maui_animations_tests)
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_animations_tests PROPERTIES
    CXX_CLANG_TIDY "${CLANG_TIDY_EXE}"
    OBJCXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# ======================================================================
# Layer 2 - Controls  (maui::controls  <=  Microsoft.Maui.Controls : the developer-facing controls)
# ======================================================================
# The gesture platform partial mirrors C#'s GesturePlatformManager.{iOS,Standard}.cs split: the
# cross-platform manager core lives in src/controls/gestures/, and exactly one backend's native
# attach/detach partial is compiled INTO maui_controls (the C# partials are part of Controls.Core too —
# unlike the handler partials, the gesture manager needs the controls-layer recognizer types).
if(MAUI_BACKEND STREQUAL "apple")
  set(MAUI_CONTROLS_GESTURE_PLATFORM_SOURCES src/platform/apple/gesture_platform_manager.mm)
  set_source_files_properties(${MAUI_CONTROLS_GESTURE_PLATFORM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
elseif(MAUI_BACKEND STREQUAL "ios")
  set(MAUI_CONTROLS_GESTURE_PLATFORM_SOURCES src/platform/ios/gesture_platform_manager.mm)
  set_source_files_properties(${MAUI_CONTROLS_GESTURE_PLATFORM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
else()
  set(MAUI_CONTROLS_GESTURE_PLATFORM_SOURCES src/platform/headless/gesture_platform_manager.cpp)
endif()
# (W2-19 items core) the collection_view_handler's per-backend partial: the simulator is
# cross-platform (src/controls/items/); each backend only mints the platform struct (+ a placeholder
# native view on apple/ios until the wave-3 NSCollectionView/UICollectionView hosts land). It lives
# in maui_controls because the handler consumes the controls-layer template machinery — exactly where
# C# keeps it (Controls.Handlers.Items).
if(MAUI_BACKEND STREQUAL "apple")
  set(MAUI_CONTROLS_ITEMS_PLATFORM_SOURCES src/platform/apple/collection_view_handler.mm)
  set_source_files_properties(${MAUI_CONTROLS_ITEMS_PLATFORM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
elseif(MAUI_BACKEND STREQUAL "ios")
  set(MAUI_CONTROLS_ITEMS_PLATFORM_SOURCES src/platform/ios/collection_view_handler.mm)
  set_source_files_properties(${MAUI_CONTROLS_ITEMS_PLATFORM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
else()
  set(MAUI_CONTROLS_ITEMS_PLATFORM_SOURCES src/platform/headless/collection_view_handler.cpp)
endif()
# (W3-33 indicator_view) the indicator_view_handler's per-backend partial: the cross-platform mapper +
# GetMaximumVisible/IsCircleShape helpers live in src/core/indicator_view_handler.cpp; each backend
# supplies the dot recipe — a real UIPageControl (ios), an NSStackView-of-dots (apple), or the headless
# dot mirror recording count/position/shape.
if(MAUI_BACKEND STREQUAL "apple")
  set(MAUI_CONTROLS_INDICATOR_PLATFORM_SOURCES src/platform/apple/indicator_view_handler.mm)
  set_source_files_properties(${MAUI_CONTROLS_INDICATOR_PLATFORM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
elseif(MAUI_BACKEND STREQUAL "ios")
  set(MAUI_CONTROLS_INDICATOR_PLATFORM_SOURCES src/platform/ios/indicator_view_handler.mm)
  set_source_files_properties(${MAUI_CONTROLS_INDICATOR_PLATFORM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
else()
  set(MAUI_CONTROLS_INDICATOR_PLATFORM_SOURCES src/platform/headless/indicator_view_handler.cpp)
endif()
# (W3-31 table_view) the table_view_handler's per-backend partial: the cross-platform mapper tables +
# cell-text helpers live in src/controls/; each backend supplies the realization recipe — a real
# UITableView (ios) / NSTableView (apple) with datasource/delegate + cell reuse + selection, or the
# headless row-realization simulator. It lives in maui_controls because the handler reads the
# controls-layer table_model (exactly like the items handler).
if(MAUI_BACKEND STREQUAL "apple")
  set(MAUI_CONTROLS_TABLE_PLATFORM_SOURCES src/platform/apple/table_view_handler.mm)
  set_source_files_properties(${MAUI_CONTROLS_TABLE_PLATFORM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
elseif(MAUI_BACKEND STREQUAL "ios")
  set(MAUI_CONTROLS_TABLE_PLATFORM_SOURCES src/platform/ios/table_view_handler.mm)
  set_source_files_properties(${MAUI_CONTROLS_TABLE_PLATFORM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
else()
  set(MAUI_CONTROLS_TABLE_PLATFORM_SOURCES src/platform/headless/table_view_handler.cpp)
endif()
# (W3-32) the native shell chrome handler's per-backend partial: the cross-platform mapper + model→tree
# rebuild lives in src/core/shell_handler.cpp (compiled into maui_controls below); each backend's recipe
# (create the container + realize_tree() materializing the VC hierarchy from the tree) lives here. It is
# in maui_controls (not maui_core) because it consumes the controls-layer shell model — exactly like the
# items handler above, and like C# (Controls.Handlers / Controls.Compatibility own the Shell renderers).
if(MAUI_BACKEND STREQUAL "apple")
  set(MAUI_CONTROLS_SHELL_PLATFORM_SOURCES src/platform/apple/shell_handler.mm)
  set_source_files_properties(${MAUI_CONTROLS_SHELL_PLATFORM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
elseif(MAUI_BACKEND STREQUAL "ios")
  set(MAUI_CONTROLS_SHELL_PLATFORM_SOURCES src/platform/ios/shell_handler.mm)
  set_source_files_properties(${MAUI_CONTROLS_SHELL_PLATFORM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
else()
  set(MAUI_CONTROLS_SHELL_PLATFORM_SOURCES src/platform/headless/shell_handler.cpp)
endif()

add_library(maui_controls STATIC
  src/controls/element.cpp
  src/controls/setter.cpp # (U-TN) Setter.TargetName string resolution via the element-side namescope
  # (G3 effects) the abstract effect base + Resolve, the explicit effect registry, the routing_effect
  # wrapper, and the element-side Effects collection + attach/detach lifecycle (element_effects.cpp).
  src/controls/effect.cpp
  src/controls/effect_registry.cpp
  src/controls/routing_effect.cpp
  src/controls/platform_effect.cpp
  src/controls/element_effects.cpp
  src/controls/bindings/binding_diagnostics.cpp
  src/controls/bindings/binding_base.cpp
  src/controls/bindings/binding.cpp
  src/controls/bindings/multi_binding.cpp
  src/controls/bindings/relative_binding_source.cpp
  src/controls/view.cpp
  src/controls/gestures/gesture_recognizers.cpp
  src/controls/gestures/gesture_recognizer_collection.cpp
  src/controls/gestures/gesture_platform_manager.cpp
  src/controls/gestures/drag_drop_recognizers.cpp # --- drag&drop (W2-22): send_* bodies ---
  src/controls/data_package.cpp # --- drag&drop (W2-22): DataPackage cluster ---
  src/controls/drag_drop_data.cpp # --- drag&drop (W2-22): the cross-control text/image seam ---
  ${MAUI_CONTROLS_GESTURE_PLATFORM_SOURCES}
  src/controls/layout.cpp
  src/controls/templates/element_templates.cpp # the element-side W1-09 templated-parent members
  src/controls/templates/element_template.cpp
  src/controls/templates/data_template.cpp
  src/controls/templates/data_template_selector.cpp
  src/controls/templates/template_utilities.cpp
  src/controls/templates/templated_view.cpp
  src/controls/templates/templated_page.cpp
  src/controls/templates/content_presenter.cpp
  src/controls/button.cpp
  src/controls/label.cpp
  src/controls/span.cpp
  src/controls/formatted_string.cpp
  src/controls/entry.cpp
  src/controls/toggle_switch.cpp
  src/controls/check_box.cpp
  src/controls/radio_button.cpp
  src/controls/radio_button_group.cpp
  src/controls/slider.cpp
  src/controls/stepper.cpp
  src/controls/picker.cpp
  src/controls/date_picker.cpp
  src/controls/time_picker.cpp
  src/controls/editor.cpp
  src/controls/search_bar.cpp
  src/controls/progress_bar.cpp
  src/controls/activity_indicator.cpp
  src/controls/image.cpp
  src/controls/image_button.cpp
  src/controls/file_image_source.cpp
  # (U-SL) the generic StackLayout (runtime-flippable Orientation) + its orientation-dispatching
  # manager (Controls-level: picks the vertical/horizontal Core manager per pass; needs the concrete
  # stack_layout for Orientation, so it lives with the controls lib rather than maui_layouts).
  src/controls/stack_layout.cpp
  src/controls/stack_layout_manager.cpp
  src/controls/vertical_stack_layout.cpp
  src/controls/horizontal_stack_layout.cpp
  src/controls/grid.cpp
  # (G2) absolute_layout + flex_layout controls (over the absolute/flex managers in maui_layouts).
  src/controls/absolute_layout.cpp
  src/controls/flex_layout.cpp
  src/controls/content_page.cpp
  # (W1-07 containers) content_view (over the W1-09 templated_view base) + border + the frame facade.
  src/controls/content_view.cpp
  src/controls/border.cpp
  src/controls/frame.cpp
  src/controls/scroll_view.cpp
  # (W2-20 swipe + refresh) the swipe item family + the two content-host controls.
  src/controls/swipe_item.cpp
  src/controls/swipe_items.cpp
  src/controls/swipe_item_view.cpp
  src/controls/swipe_view.cpp
  src/controls/refresh_view.cpp
  src/controls/navigation_page.cpp
  # (W1-10 tabbed + flyout pages) multi_page<TPage> is header-only; these are the concrete pages.
  src/controls/tabbed_page.cpp
  src/controls/flyout_page.cpp
  src/controls/web_view.cpp
  src/controls/url_web_view_source.cpp
  src/controls/html_web_view_source.cpp
  # (G5 hybrid_web_view) the hybrid web view control + its cross-platform handler tables/protocol routing.
  src/controls/hybrid_web_view.cpp
  src/controls/hybrid_web_view_handler.cpp
  # (W1-11 chrome/menus) toolbar items + menu bar/flyout family + trackers + window chrome.
  src/controls/menu_item.cpp
  src/controls/toolbar_item.cpp
  src/controls/menu_bar_item.cpp
  src/controls/menu_bar.cpp
  src/controls/toolbar_tracker.cpp
  src/controls/menu_bar_tracker.cpp
  src/controls/title_bar.cpp
  src/controls/style.cpp
  src/controls/resource_dictionary.cpp
  src/controls/merged_style.cpp
  src/controls/visual_state_manager.cpp
  src/controls/behavior.cpp      # W1-15 styles tail: Behavior/AttachedCollection
  src/controls/state_trigger.cpp # W1-15 styles tail: StateTriggerBase + the concrete state triggers
  src/controls/window.cpp
  src/controls/window_overlay.cpp # (G4) the concrete window overlay + adorner draw seam
  src/controls/application.cpp
  # (W2-19 items core) the Items layer: layouts + the items_view hierarchy + the source family + the
  # collection_view handler (cross-platform simulator; per-backend partial below).
  src/controls/items/items_layout.cpp
  src/controls/items/linear_items_layout.cpp
  src/controls/items/grid_items_layout.cpp
  src/controls/items/items_source_factory.cpp
  src/controls/items/items_view.cpp
  src/controls/items/structured_items_view.cpp
  src/controls/items/selection_list.cpp
  src/controls/items/selectable_items_view.cpp
  src/controls/items/groupable_items_view.cpp
  src/controls/items/reorderable_items_view.cpp
  src/controls/items/collection_view.cpp
  src/controls/items/collection_view_handler.cpp
  # (W3-33) carousel_view reuses the collection_view virtualization handler; indicator_view has its own
  # handler (per-backend partial below: UIPageControl / NSStackView-of-dots / headless dot mirror).
  src/controls/items/carousel_view.cpp
  src/controls/indicator_view.cpp
  src/core/indicator_view_handler.cpp
  ${MAUI_CONTROLS_INDICATOR_PLATFORM_SOURCES}
  ${MAUI_CONTROLS_ITEMS_PLATFORM_SOURCES}
  # (W3-31 table_view + cells) the obsolete-but-ported Cell family + the TableView model layer + control
  # + handler (ListView deferred → CollectionView; see STATUS).
  src/controls/cells/cell.cpp
  src/controls/cells/text_cell.cpp
  src/controls/cells/switch_cell.cpp
  src/controls/cells/entry_cell.cpp
  src/controls/cells/image_cell.cpp
  src/controls/cells/view_cell.cpp
  src/controls/table_section_base.cpp
  src/controls/table_section.cpp
  src/controls/table_root.cpp
  src/controls/table_view.cpp
  src/controls/table_view_handler.cpp
  ${MAUI_CONTROLS_TABLE_PLATFORM_SOURCES}
  # (X1 brushes) the controls-level Brush family (solid/gradient/image) + the named statics + the
  # Brush<->Paint bridge. ImageBrush is header-only; the others have out-of-line .cpp bodies.
  src/controls/brushes/brush.cpp
  src/controls/brushes/solid_color_brush.cpp
  src/controls/brushes/gradient_stop.cpp
  src/controls/brushes/gradient_brush.cpp
  src/controls/brushes/linear_gradient_brush.cpp
  src/controls/brushes/radial_gradient_brush.cpp
  src/controls/brushes/brush_paint_bridge.cpp
  src/controls/brushes/brush_type_converter.cpp # X1: BrushTypeConverter (string -> Brush), Controls-level
  # (W2-23 shapes) the Shapes geometry/transform model + the path markup parser + the controls.
  src/controls/graphics_view.cpp
  src/controls/box_view.cpp
  src/controls/shapes/shape.cpp
  src/controls/shapes/rectangle.cpp
  src/controls/shapes/ellipse.cpp
  src/controls/shapes/line.cpp
  src/controls/shapes/polyline.cpp
  src/controls/shapes/polygon.cpp
  src/controls/shapes/path.cpp
  src/controls/shapes/matrix.cpp
  src/controls/shapes/geometry_helper.cpp
  src/controls/shapes/path_geometry.cpp
  src/controls/shapes/path_markup_parser.cpp
  # (W2-21) the shell model + routing (HEADLESS-ONLY — the native shell chrome is a later wave)
  src/controls/shell/shell_uri.cpp
  src/controls/shell/shell_route_parameters.cpp
  src/controls/shell/routing.cpp
  src/controls/shell/base_shell_item.cpp
  src/controls/shell/shell_group_item.cpp
  src/controls/shell/shell_content.cpp
  src/controls/shell/shell_section.cpp
  src/controls/shell/shell_item.cpp
  src/controls/shell/shell_navigation_state.cpp
  src/controls/shell/shell_navigating_event_args.cpp
  src/controls/shell/route_request_builder.cpp
  src/controls/shell/request_definition.cpp
  src/controls/shell/shell_uri_handler.cpp
  src/controls/shell/shell_navigation_manager.cpp
  # (W7-50/X2) the Shell SearchHandler model (BindableObject) + list_proxy; the Shell.SearchHandler
  # attached property lives in shell.cpp; the chrome's search-box wiring is in shell_handler.{cpp,mm}.
  src/controls/shell/search_handler.cpp
  # (W7-51) the resolved Shell appearance value type (ShellAppearance.Ingest); the Shell.* color attached
  # properties + the GetAppearanceForPivot resolution walk live in shell.cpp.
  src/controls/shell/shell_appearance.cpp
  src/controls/shell/shell.cpp
  # (W3-32) the native shell chrome handler (cross-platform mapper KEYS + the model→render-tree rebuild);
  # the per-backend platform recipe is in src/platform/<backend>/shell_handler.{cpp,mm} (selected above).
  src/core/shell_handler.cpp
  ${MAUI_CONTROLS_SHELL_PLATFORM_SOURCES}
  # (W1-14 animations) the Controls-layer animation surface: Controls.Animation + the
  # AnimationExtensions pump (+ the internal tweener/per-element registry) + ViewExtensions.
  src/controls/animation.cpp
  src/controls/animation_extensions.cpp
  src/controls/view_extensions.cpp
  src/controls/detail/tweener.cpp
  src/controls/detail/element_animations.cpp
)
target_include_directories(maui_controls PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# Controls depend on Core (handlers, contracts, the property system); the layout controls additionally
# use the maui_layouts measure/arrange managers (their create_layout_manager() instantiates them).
# maui_essentials joins for the state triggers (DeviceStateTrigger/OrientationStateTrigger read the
# device_info / device_display facades — the same dependency direction as C# Controls.Core → Essentials).
target_link_libraries(maui_controls PUBLIC maui_core maui_layouts maui_essentials PRIVATE maui_sanitizers)
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_controls PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# ---- Controls tests (GoogleTest, discovered into ctest) ----
# The button control + the handler seam (control -> handler -> platform -> back). The seam test is
# backend-specific: headless drives the fake platform; apple drives a real NSButton via performClick.
if(MAUI_BACKEND STREQUAL "apple")
  add_executable(maui_controls_tests
    tests/controls/button_apple_tests.mm
    tests/controls/effect_apple_tests.mm # (G3) effect attaching to a real native-handled NSButton
    tests/controls/effect_tests.cpp # (G3) effects: registry + resolve + element attach/detach (backend-agnostic)
    tests/controls/gesture_apple_tests.mm # E1: native recognizer attachment + target/action drives
    tests/controls/label_apple_tests.mm
    tests/controls/formatted_text_tests.cpp # G1: span/formatted_string/label.formatted_text (backend-agnostic)
    tests/controls/formatted_text_apple_tests.mm # G1: the real NSAttributedString seam
    tests/controls/view_mapper_apple_tests.mm
    tests/controls/entry_apple_tests.mm
    tests/controls/toggle_switch_apple_tests.mm
    tests/controls/check_box_apple_tests.mm
    tests/controls/radio_button_apple_tests.mm
    tests/controls/slider_apple_tests.mm
    tests/controls/stepper_apple_tests.mm
    tests/controls/picker_apple_tests.mm
    tests/controls/date_picker_apple_tests.mm
    tests/controls/time_picker_apple_tests.mm
    tests/controls/editor_apple_tests.mm
    tests/controls/search_bar_apple_tests.mm
    tests/controls/progress_bar_apple_tests.mm
    tests/controls/activity_indicator_apple_tests.mm
    tests/controls/image_apple_tests.mm
    tests/controls/image_button_apple_tests.mm
    tests/controls/font_image_source_apple_tests.mm
    tests/controls/font_manager_apple_tests.mm
    tests/controls/image_source_paint_apple_tests.mm
    tests/controls/layout_apple_tests.mm
    tests/controls/grid_apple_tests.mm
    tests/controls/content_page_apple_tests.mm
    tests/controls/border_apple_tests.mm      # (W1-07 containers)
    tests/controls/graphics_view_apple_tests.mm # (W2-23) the NSView drawing host + CG readback
    tests/controls/shape_view_apple_tests.mm    # (W2-23) the shape host CG readback
    tests/controls/scroll_view_apple_tests.mm # (W1-07 containers)
    # (W2-20 swipe + refresh) the backend-agnostic control + state-machine suites + the apple seam.
    tests/controls/swipe_view_tests.cpp
    # (W7-U09) the swipe menu-item handler contract + mapper-mirror suite (backend-agnostic).
    tests/controls/swipe_item_menu_item_tests.cpp
    tests/controls/refresh_view_tests.cpp
    tests/controls/swipe_refresh_page_tests.cpp
    tests/controls/swipe_refresh_apple_tests.mm
    tests/controls/navigation_page_apple_tests.mm
    tests/controls/tabbed_page_apple_tests.mm # (W1-10) the real NSTabViewController seam
    tests/controls/flyout_page_apple_tests.mm # (W1-10) the real NSSplitViewController seam
    # (W3-32) the shell chrome: the real NSSplitViewController sidebar + NSTabView seam + route→tab e2e.
    tests/controls/shell_handler_apple_tests.mm
    # (W7-50/X2) the shell search box: the real NSSearchField seam + native edit/confirm round-trip.
    tests/controls/search_handler_apple_tests.mm
    tests/controls/tabbed_page_tests.cpp # (W1-10) the MultiPage/TabbedPage control suite + the mirrors
    tests/controls/flyout_page_tests.cpp # (W1-10) the FlyoutPage control suite + the mirrors
    tests/controls/web_view_apple_tests.mm # (W1-08) the real WKWebView seam, run-loop pumped
    tests/controls/hybrid_web_view_apple_tests.mm # (G5) the real WKWebView + WKScriptMessageHandler bridge
    tests/controls/window_apple_tests.mm
    tests/controls/chrome_apple_tests.mm # (W1-11) real NSMenu/NSToolbar/NSView.menu/toolTip/titlebar accessory
    tests/controls/window_handler_tests.cpp # the portable seam mirrors (real NSWindow behind them here)
    tests/controls/window_modal_events_tests.cpp # (G4) backend-agnostic: the Window modal events ordering
    tests/controls/window_overlay_tests.cpp # (G4) backend-agnostic: add/remove overlay + adorner list + invalidate->draw
    tests/controls/window_overlay_apple_tests.mm # (G4) the overlay draws through the real graphics host
    tests/controls/content_view_tests.cpp # (W1-07) backend-agnostic: the templated content host over mocks
    tests/controls/drag_drop_tests.cpp # (W2-22) backend-agnostic: data_package + drag/drop recognizers + wiring
    # (W1-11 chrome/menus) the backend-agnostic chrome suites (real natives behind the mirrors here).
    tests/controls/menu_item_tests.cpp
    tests/controls/menu_bar_tests.cpp
    tests/controls/menu_tracker_tests.cpp
    tests/controls/chrome_tests.cpp
    tests/controls/chrome_page_tests.cpp
    tests/controls/value_controls_page_tests.cpp # the W1-04 demo page wiring (backend-agnostic)
    tests/controls/formatted_text_page_tests.cpp # the G1 formatted-text demo page wiring (backend-agnostic)
    tests/controls/containers_page_tests.cpp # the W1-07 demo page wiring (backend-agnostic)
    tests/controls/pickers_page_tests.cpp # the W1-06 demo page wiring (backend-agnostic)
    tests/controls/web_view_page_tests.cpp # the W1-08 demo page wiring (backend-agnostic)
    tests/controls/input_controls_page_tests.cpp # the W1-05 demo page wiring (backend-agnostic)
    tests/controls/tabbed_flyout_page_tests.cpp # the W1-10 demo page wiring (backend-agnostic)
    tests/controls/shapes_tests.cpp # (W2-23 shapes) geometry/transform model + path markup parser
    tests/controls/graphics_view_tests.cpp # (W2-23) graphics_view + box_view + the golden-op seat
    tests/controls/shape_view_tests.cpp # (W2-23) the shape control family + shape_view_handler
    tests/controls/shapes_page_tests.cpp # the W2-23 demo page wiring (backend-agnostic)
    tests/controls/brush_tests.cpp # (X1) the controls Brush family + paint bridge + XAML converter (backend-agnostic)
    tests/controls/style_tests.cpp   # backend-agnostic; compiled as C++ within the apple test exe
    tests/controls/trigger_tests.cpp
    tests/controls/visual_state_manager_tests.cpp
    tests/controls/resource_dictionary_tests.cpp
    tests/controls/platform_configuration_tests.cpp # (W2-24) the platform-config mechanism + *Specific knob sets (backend-agnostic)
    tests/controls/dynamic_resource_tests.cpp
    tests/controls/implicit_style_tests.cpp
    tests/controls/advanced_trigger_tests.cpp
    tests/controls/setter_target_name_tests.cpp # (U-TN) Setter.TargetName resolution via element namescope
    tests/controls/behavior_tests.cpp # W1-15: Behavior/AttachedCollection (backend-agnostic)
    tests/controls/event_trigger_tests.cpp # W1-15: EventTrigger over the named-event seam
    tests/controls/state_trigger_tests.cpp # W1-15: StateTriggers driving visual states
    tests/controls/size_request_tests.cpp # backend-agnostic; the VisualElement size-request surface
    tests/controls/lifecycle_tests.cpp
    tests/controls/data_template_tests.cpp    # backend-agnostic (W1-09 templates)
    # (W2-19 items core) backend-agnostic: the items hierarchy + sources + the simulator + demo page.
    tests/controls/items_view_tests.cpp
    tests/controls/items_sources_tests.cpp
    tests/controls/selectable_items_view_tests.cpp
    tests/controls/collection_view_tests.cpp
    tests/controls/collection_view_appkit_tests.mm # (W3-30) the real NSCollectionView FlowLayout reuse/selection/supplementaries/reorder seam
    # (W3-33) carousel_view + indicator_view — backend-agnostic behavior over the W3-29 items infra.
    tests/controls/carousel_view_tests.cpp
    tests/controls/indicator_view_tests.cpp
    tests/controls/items_page_tests.cpp
    tests/controls/control_template_tests.cpp
    tests/controls/binding_engine_tests.cpp # backend-agnostic; the W1-02 string-path binding engine
    tests/controls/multi_binding_tests.cpp
    tests/controls/relative_source_binding_tests.cpp
    tests/controls/view_extensions_tests.cpp # (W1-14) backend-agnostic animation surface
    # (W3-31 table_view + cells) backend-agnostic: the cell family + the table model/control/handler.
    tests/controls/cells_tests.cpp
    tests/controls/table_view_tests.cpp
    tests/controls/table_view_apple_tests.mm # the real NSTableView realization + reuse + selection seam
    tests/controls/indicator_view_apple_tests.mm # (W3-33) the real NSStackView-of-dots indicator row
)
  # tests/core/image_source_tests.cpp + font_image_source_tests.cpp are HEADLESS-ONLY: they assert on the
  # headless image_source_result mirror + the headless "any non-empty bytes => loaded" decode (raw test
  # bytes are not decodable NSImages on apple). The apple backend's native rasterize is covered by
  # tests/controls/font_image_source_apple_tests.mm instead.
  set_source_files_properties(
    tests/controls/button_apple_tests.mm
    tests/controls/effect_apple_tests.mm # (G3)
    tests/controls/gesture_apple_tests.mm
    tests/controls/label_apple_tests.mm
    tests/controls/formatted_text_apple_tests.mm # G1
    tests/controls/view_mapper_apple_tests.mm
    tests/controls/entry_apple_tests.mm
    tests/controls/toggle_switch_apple_tests.mm
    tests/controls/check_box_apple_tests.mm
    tests/controls/radio_button_apple_tests.mm
    tests/controls/slider_apple_tests.mm
    tests/controls/stepper_apple_tests.mm
    tests/controls/picker_apple_tests.mm
    tests/controls/date_picker_apple_tests.mm
    tests/controls/time_picker_apple_tests.mm
    tests/controls/editor_apple_tests.mm
    tests/controls/search_bar_apple_tests.mm
    tests/controls/progress_bar_apple_tests.mm
    tests/controls/activity_indicator_apple_tests.mm
    tests/controls/image_apple_tests.mm
    tests/controls/image_button_apple_tests.mm
    tests/controls/font_image_source_apple_tests.mm
    tests/controls/font_manager_apple_tests.mm
    tests/controls/image_source_paint_apple_tests.mm
    tests/controls/layout_apple_tests.mm
    tests/controls/grid_apple_tests.mm
    tests/controls/content_page_apple_tests.mm
    tests/controls/border_apple_tests.mm
    tests/controls/graphics_view_apple_tests.mm
    tests/controls/shape_view_apple_tests.mm
    tests/controls/scroll_view_apple_tests.mm
    tests/controls/navigation_page_apple_tests.mm
    tests/controls/tabbed_page_apple_tests.mm
    tests/controls/flyout_page_apple_tests.mm
    tests/controls/shell_handler_apple_tests.mm # (W3-32)
    tests/controls/search_handler_apple_tests.mm # (W7-50/X2)
    tests/controls/web_view_apple_tests.mm
    tests/controls/window_apple_tests.mm
    tests/controls/chrome_apple_tests.mm # (W1-11)
    tests/controls/window_overlay_apple_tests.mm # (G4) the overlay CG readback smoke
    tests/controls/table_view_apple_tests.mm # (W3-31)
    tests/controls/collection_view_appkit_tests.mm # (W3-30)
    tests/controls/indicator_view_apple_tests.mm # (W3-33)
    PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
elseif(MAUI_BACKEND STREQUAL "ios")
  # Every control is a real UIKit partial on ios (M6 fan-out complete), so the suite runs each control's
  # on-simulator *_ios_tests.mm twin instead of the headless-mirror .cpp: button (button_tests.cpp +
  # view_mapper_tests.cpp assert the in-memory mirrors), label/entry/image (+ font_image_source), the
  # image SERVICES (image_sources_ios_tests.mm covers the services + loader seam with decodable PNG/GIF
  # fixtures; tests/core/image_source_tests.cpp + font_image_source_tests.cpp are HEADLESS-ONLY, exactly
  # as on apple), the three containers (layout / grid — grid reuses layout_handler — / content_page /
  # navigation_page), and the window (window_ios_tests.mm drives the genuine UIWindow). The *_ios files
  # also cover their generic-IView pushes, standing in for view_mapper coverage until a
  # view_mapper_ios_tests.mm arrives. The backend-agnostic suites (styles/triggers/VSM/resources/
  # size-requests/lifecycle) run unchanged.
  set(MAUI_IOS_TEST_MM_SOURCES
    tests/controls/button_ios_tests.mm
    tests/controls/gesture_ios_tests.mm # E1: native recognizer attachment + registered-target drives
    tests/controls/label_ios_tests.mm
    tests/controls/formatted_text_ios_tests.mm # G1: the real UILabel attributed-text seam
    tests/controls/entry_ios_tests.mm
    tests/controls/toggle_switch_ios_tests.mm
    tests/controls/check_box_ios_tests.mm
    tests/controls/radio_button_ios_tests.mm
    tests/controls/slider_ios_tests.mm
    tests/controls/stepper_ios_tests.mm
    tests/controls/picker_ios_tests.mm
    tests/controls/date_picker_ios_tests.mm
    tests/controls/time_picker_ios_tests.mm
    tests/controls/editor_ios_tests.mm
    # (W7 keyboard-automanager) the KeyboardAutoManager next-responder walk over raw UIKit hierarchies +
    # the scroll-engine connect/disconnect idempotence smoke (KeyboardAutoManager.cs / KeyboardAutoManagerScroll.cs).
    tests/controls/keyboard_auto_manager_ios_tests.mm
    tests/controls/search_bar_ios_tests.mm
    tests/controls/progress_bar_ios_tests.mm
    tests/controls/activity_indicator_ios_tests.mm
    tests/controls/image_ios_tests.mm
    tests/controls/image_button_ios_tests.mm
    tests/controls/font_image_source_ios_tests.mm
    tests/controls/font_manager_ios_tests.mm
    tests/controls/image_source_paint_ios_tests.mm
    tests/controls/image_sources_ios_tests.mm
    tests/controls/layout_ios_tests.mm
    tests/controls/grid_ios_tests.mm
    tests/controls/content_page_ios_tests.mm
    tests/controls/border_ios_tests.mm      # (W1-07 containers)
    tests/controls/graphics_view_ios_tests.mm # (W2-23) the UIView drawing host + CG readback
    tests/controls/shape_view_ios_tests.mm    # (W2-23) the shape host CG readback
    tests/controls/scroll_view_ios_tests.mm # (W1-07 containers)
    tests/controls/swipe_refresh_ios_tests.mm # (W2-20) the UIKit seam (UIRefreshControl + the swipe host)
    tests/controls/swipe_item_menu_item_ios_tests.mm # (W7-U09) the UIButton swipe-item handler seam + pan reveal
    tests/controls/navigation_page_ios_tests.mm
    tests/controls/web_view_ios_tests.mm # (W1-08) the real WKWebView seam, run-loop pumped on-simulator
    tests/controls/hybrid_web_view_ios_tests.mm # (G5) the real JS↔native bridge round trip on-simulator
    tests/controls/tabbed_page_ios_tests.mm # (W1-10) the real UITabBarController child-VC seam
    tests/controls/flyout_page_ios_tests.mm # (W1-10) the real UISplitViewController column seam
    # (W3-32) the shell chrome: the real container split + UITabBarController + per-section
    # UINavigationController seam, with the route→VC-stack e2e (the on-simulator gate).
    tests/controls/shell_handler_ios_tests.mm
    # (W7-50/X2) the shell search box: the real UISearchController install into the nav item + native
    # edit/confirm round-trip on-simulator.
    tests/controls/search_handler_ios_tests.mm
    tests/controls/chrome_ios_tests.mm # (W1-11) real bar buttons + UIContextMenuInteraction attach + inert mirrors
    # (W3-29 items) the REAL UICollectionView native virtualization (Items2 compositional path): cell
    # reuse under programmatic scroll, selection single/multi, grouping supplementaries, reorder,
    # scroll-to, empty-view — run-loop pumped via tests/support/run_loop_pump.hpp.
    tests/controls/collection_view_ios_tests.mm
    # (W3-33) the carousel rides the real compositional UICollectionView; the indicator is the real
    # MauiPageControl (UIPageControl) with the native page-tap write-back.
    tests/controls/carousel_view_ios_tests.mm
    tests/controls/indicator_view_ios_tests.mm
    tests/controls/table_view_ios_tests.mm # (W3-31) the real UITableView realization + reuse + selection seam
    tests/controls/window_ios_tests.mm)
  add_executable(maui_controls_tests
    ${MAUI_IOS_TEST_MM_SOURCES}
    tests/controls/formatted_text_tests.cpp # G1: span/formatted_string/label.formatted_text (backend-agnostic)
    tests/controls/window_modal_events_tests.cpp # (G4) backend-agnostic: the Window modal events ordering
    tests/controls/window_overlay_tests.cpp # (G4) backend-agnostic: add/remove overlay + adorner list + invalidate->draw
    # (W2-20 swipe + refresh) the backend-agnostic control + state-machine suites + the demo page.
    tests/controls/swipe_view_tests.cpp
    # (W7-U09) the swipe menu-item handler contract + mapper-mirror suite (backend-agnostic).
    tests/controls/swipe_item_menu_item_tests.cpp
    tests/controls/refresh_view_tests.cpp
    tests/controls/swipe_refresh_page_tests.cpp
    tests/controls/tabbed_page_tests.cpp # (W1-10) the MultiPage/TabbedPage control suite + the mirrors
    tests/controls/flyout_page_tests.cpp # (W1-10) the FlyoutPage control suite + the mirrors
    # (W1-11 chrome/menus) the backend-agnostic chrome suites.
    tests/controls/menu_item_tests.cpp
    tests/controls/menu_bar_tests.cpp
    tests/controls/menu_tracker_tests.cpp
    tests/controls/chrome_tests.cpp
    tests/controls/chrome_page_tests.cpp
    tests/controls/value_controls_page_tests.cpp # the W1-04 demo page wiring (backend-agnostic)
    tests/controls/formatted_text_page_tests.cpp # the G1 formatted-text demo page wiring (backend-agnostic)
    tests/controls/web_view_page_tests.cpp # the W1-08 demo page wiring (backend-agnostic)
    tests/controls/input_controls_page_tests.cpp # the W1-05 demo page wiring (backend-agnostic)
    tests/controls/tabbed_flyout_page_tests.cpp # the W1-10 demo page wiring (backend-agnostic)
    tests/controls/containers_page_tests.cpp # the W1-07 demo page wiring (backend-agnostic)
    tests/controls/content_view_tests.cpp # (W1-07) backend-agnostic: the templated content host over mocks
    tests/controls/drag_drop_tests.cpp # (W2-22) backend-agnostic: data_package + drag/drop recognizers + wiring
    tests/controls/pickers_page_tests.cpp # the W1-06 demo page wiring (backend-agnostic)
    tests/controls/shapes_tests.cpp # (W2-23 shapes) geometry/transform model + path markup parser
    tests/controls/graphics_view_tests.cpp # (W2-23) graphics_view + box_view + the golden-op seat
    tests/controls/shape_view_tests.cpp # (W2-23) the shape control family + shape_view_handler
    tests/controls/shapes_page_tests.cpp # the W2-23 demo page wiring (backend-agnostic)
    tests/controls/brush_tests.cpp # (X1) the controls Brush family + paint bridge + XAML converter (backend-agnostic)
    tests/controls/style_tests.cpp
    tests/controls/trigger_tests.cpp
    tests/controls/visual_state_manager_tests.cpp
    tests/controls/resource_dictionary_tests.cpp
    tests/controls/platform_configuration_tests.cpp # (W2-24) the platform-config mechanism + *Specific knob sets (backend-agnostic)
    tests/controls/dynamic_resource_tests.cpp
    tests/controls/implicit_style_tests.cpp
    tests/controls/advanced_trigger_tests.cpp
    tests/controls/setter_target_name_tests.cpp # (U-TN) Setter.TargetName resolution via element namescope
    tests/controls/behavior_tests.cpp # W1-15: Behavior/AttachedCollection (backend-agnostic)
    tests/controls/event_trigger_tests.cpp # W1-15: EventTrigger over the named-event seam
    tests/controls/state_trigger_tests.cpp # W1-15: StateTriggers driving visual states
    tests/controls/size_request_tests.cpp # backend-agnostic (compares against the handler's own size)
    tests/controls/lifecycle_tests.cpp
    tests/controls/effect_tests.cpp # (G3) effects (backend-agnostic): registry + resolve + element attach/detach
    tests/controls/data_template_tests.cpp    # backend-agnostic (W1-09 templates)
    # (W2-19 items core) backend-agnostic: the items hierarchy + sources + the simulator + demo page.
    tests/controls/items_view_tests.cpp
    tests/controls/items_sources_tests.cpp
    tests/controls/selectable_items_view_tests.cpp
    tests/controls/collection_view_tests.cpp
    # (W3-33) carousel_view + indicator_view — backend-agnostic behavior over the W3-29 items infra.
    tests/controls/carousel_view_tests.cpp
    tests/controls/indicator_view_tests.cpp
    tests/controls/items_page_tests.cpp
    tests/controls/control_template_tests.cpp
    tests/controls/binding_engine_tests.cpp # backend-agnostic; the W1-02 string-path binding engine
    tests/controls/multi_binding_tests.cpp
    tests/controls/relative_source_binding_tests.cpp
    tests/controls/view_extensions_tests.cpp # (W1-14) backend-agnostic animation surface
    # (W3-31 table_view + cells) backend-agnostic: the cell family + the table model/control/handler.
    tests/controls/cells_tests.cpp
    tests/controls/table_view_tests.cpp
)
  set_source_files_properties(${MAUI_IOS_TEST_MM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
else()
  add_executable(maui_controls_tests
    tests/controls/button_tests.cpp
    tests/controls/gesture_recognizer_tests.cpp # the E1 gesture state machines + synthetic pipeline
    tests/controls/drag_drop_tests.cpp # (W2-22) data_package + drag/drop recognizers + wiring
    tests/controls/label_tests.cpp
    tests/controls/formatted_text_tests.cpp # G1: span + formatted_string + label.formatted_text mirror
    tests/controls/view_mapper_tests.cpp
    tests/controls/entry_tests.cpp
    tests/controls/toggle_switch_tests.cpp
    tests/controls/check_box_tests.cpp
    tests/controls/radio_button_tests.cpp
    tests/controls/slider_tests.cpp
    tests/controls/stepper_tests.cpp
    tests/controls/picker_tests.cpp
    tests/controls/date_picker_tests.cpp
    tests/controls/time_picker_tests.cpp
    tests/controls/editor_tests.cpp
    tests/controls/search_bar_tests.cpp
    tests/controls/progress_bar_tests.cpp
    tests/controls/activity_indicator_tests.cpp
    tests/controls/value_controls_page_tests.cpp # the W1-04 demo page wiring (backend-agnostic)
    tests/controls/formatted_text_page_tests.cpp # the G1 formatted-text demo page wiring (backend-agnostic)
    tests/controls/input_controls_page_tests.cpp # the W1-05 demo page wiring (backend-agnostic)
    tests/controls/containers_page_tests.cpp # the W1-07 demo page wiring (backend-agnostic)
    tests/controls/pickers_page_tests.cpp # the W1-06 demo page wiring (backend-agnostic)
    tests/controls/image_tests.cpp
    tests/controls/image_button_tests.cpp
    tests/controls/layout_tests.cpp
    tests/controls/grid_tests.cpp
    # (G2) absolute_layout + flex_layout control surface + handler seam.
    tests/controls/absolute_layout_tests.cpp
    tests/controls/flex_layout_tests.cpp
    tests/controls/content_page_tests.cpp
    tests/controls/content_view_tests.cpp # (W1-07 containers)
    tests/controls/border_tests.cpp
    tests/controls/frame_tests.cpp
    tests/controls/scroll_view_tests.cpp
    # (W2-20 swipe + refresh) the control behavior + the swipe state-machine seam + the demo page wiring.
    tests/controls/swipe_view_tests.cpp
    # (W7-U09) the swipe menu-item handler contract + mapper-mirror suite (backend-agnostic).
    tests/controls/swipe_item_menu_item_tests.cpp
    tests/controls/refresh_view_tests.cpp
    tests/controls/swipe_refresh_page_tests.cpp
    tests/controls/navigation_page_tests.cpp
    tests/controls/tabbed_page_tests.cpp # (W1-10) MultiPageTests + TabbedPageTests + the seam mirrors
    tests/controls/flyout_page_tests.cpp # (W1-10) FlyoutPageUnitTests + the seam mirrors
    tests/controls/web_view_tests.cpp
    tests/controls/web_view_page_tests.cpp # the W1-08 demo page wiring (backend-agnostic)
    tests/controls/hybrid_web_view_tests.cpp # (G5) the hybrid web view control + the message-bridge mirror
    tests/controls/tabbed_flyout_page_tests.cpp # the W1-10 demo page wiring (backend-agnostic)
    # (W1-11 chrome/menus) menu items + containers + trackers + window/view chrome + the demo page.
    tests/controls/menu_item_tests.cpp
    tests/controls/menu_bar_tests.cpp
    tests/controls/menu_tracker_tests.cpp
    tests/controls/chrome_tests.cpp
    tests/controls/chrome_page_tests.cpp
    tests/controls/shapes_tests.cpp # (W2-23 shapes) geometry/transform model + path markup parser
    tests/controls/graphics_view_tests.cpp # (W2-23) graphics_view + box_view + the golden-op seat
    tests/controls/shape_view_tests.cpp # (W2-23) the shape control family + shape_view_handler
    tests/controls/shapes_page_tests.cpp # the W2-23 demo page wiring (backend-agnostic)
    tests/controls/brush_tests.cpp # (X1) the controls Brush family + paint bridge + XAML converter (backend-agnostic)
    tests/controls/style_tests.cpp
    tests/controls/trigger_tests.cpp
    tests/controls/visual_state_manager_tests.cpp
    tests/controls/resource_dictionary_tests.cpp
    tests/controls/platform_configuration_tests.cpp # (W2-24) the platform-config mechanism + *Specific knob sets (backend-agnostic)
    tests/controls/dynamic_resource_tests.cpp
    tests/controls/implicit_style_tests.cpp
    tests/controls/advanced_trigger_tests.cpp
    tests/controls/setter_target_name_tests.cpp # (U-TN) Setter.TargetName resolution via element namescope
    tests/controls/behavior_tests.cpp # W1-15: Behavior/AttachedCollection (backend-agnostic)
    tests/controls/event_trigger_tests.cpp # W1-15: EventTrigger over the named-event seam
    tests/controls/state_trigger_tests.cpp # W1-15: StateTriggers driving visual states
    tests/controls/size_request_tests.cpp # the VisualElement size-request surface (view<> IView mapping)
    tests/controls/view_extensions_tests.cpp # (W1-14) the animation surface over the manual ticker
    tests/controls/lifecycle_tests.cpp
    tests/controls/effect_tests.cpp # (G3) effects: registry + resolve + element attach/detach + routing/platform
    tests/controls/data_template_tests.cpp    # backend-agnostic (W1-09 templates)
    # (W2-19 items core) backend-agnostic: the items hierarchy + sources + the simulator + demo page.
    tests/controls/items_view_tests.cpp
    tests/controls/items_sources_tests.cpp
    tests/controls/selectable_items_view_tests.cpp
    tests/controls/collection_view_tests.cpp
    # (W3-33) carousel_view + indicator_view — backend-agnostic behavior over the W3-29 items infra.
    tests/controls/carousel_view_tests.cpp
    tests/controls/indicator_view_tests.cpp
    tests/controls/items_page_tests.cpp
    tests/controls/control_template_tests.cpp
    tests/controls/binding_engine_tests.cpp # backend-agnostic; the W1-02 string-path binding engine
    tests/controls/multi_binding_tests.cpp
    tests/controls/relative_source_binding_tests.cpp
    # (W3-31 table_view + cells) the cell family + the table model/control/handler simulator.
    tests/controls/cells_tests.cpp
    tests/controls/table_view_tests.cpp
    tests/controls/window_handler_tests.cpp # the window_handler seam over the headless mirror
    tests/controls/window_modal_events_tests.cpp # (G4) the Window modal events ordering
    tests/controls/window_overlay_tests.cpp # (G4) add/remove overlay + adorner list + invalidate->draw mirror
    tests/core/image_source_tests.cpp     # loader cache TTL + the typed service-provider
    tests/core/font_image_source_tests.cpp
    # (W2-21) shell model + routing — HEADLESS-ONLY this unit (the native chrome is a later wave);
    # the library sources still compile through on apple/ios.
    tests/controls/shell_tests.cpp
    tests/controls/shell_uri_handler_tests.cpp
    tests/controls/shell_navigating_tests.cpp
    # (W3-32) the native shell chrome handler — headless structure mapping + route→render-tree e2e.
    tests/controls/shell_handler_tests.cpp
    # (W7-50/X2) the Shell SearchHandler model + attached property + chrome search-box wiring.
    tests/controls/search_handler_tests.cpp
    # (W7-51) the Shell appearance model: ShellAppearance value type + the Shell.* color attached
    # properties + the GetAppearanceForPivot resolution walk.
    tests/controls/shell_appearance_tests.cpp)
endif()
target_include_directories(maui_controls_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) # for tests/layouts/*.hpp
# The apple .mm tests share the backend's text/visual op headers (apple_text_ops.hpp etc.), which live
# beside the handler .mm sources; put that dir on the include path for the apple test exe only. apple_shared
# carries the cross-Apple helpers the visual/image op headers pull in (scaled_file.hpp).
if(MAUI_BACKEND STREQUAL "apple")
  target_include_directories(maui_controls_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/apple)
  target_include_directories(maui_controls_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/apple_shared)
endif()
# Same convention for the ios .mm tests (ios_text_ops.hpp / ios_conversions.hpp live beside the
# UIKit handler sources); apple_shared carries scaled_file.hpp (ios_image_ops.hpp / ios_visual_ops.hpp pull it in).
if(MAUI_BACKEND STREQUAL "ios")
  target_include_directories(maui_controls_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/ios)
  target_include_directories(maui_controls_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/apple_shared)
endif()
target_link_libraries(maui_controls_tests PRIVATE
  maui_controls
  maui_sanitizers
  GTest::gmock_main
)
maui_discover_tests(maui_controls_tests)
# The controls tests are .cpp on headless and .mm (OBJCXX) on apple; lint whichever language is built.
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_controls_tests PROPERTIES
    CXX_CLANG_TIDY "${CLANG_TIDY_EXE}"
    OBJCXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# ---- Sample apps (macOS only) ----
# (M-L hosting-app-builder) Both samples now boot through maui_app_builder (maui_hosting, defined
# below — CMake resolves link targets at generate time, so the forward reference is fine).
if(MAUI_BACKEND STREQUAL "apple")
  # A real NSWindow hosting the cross-platform button (the M2 seam).
  add_executable(maui_button_sample src/samples/button_sample.mm)
  set_source_files_properties(src/samples/button_sample.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
  target_link_libraries(maui_button_sample PRIVATE maui_hosting maui_sanitizers)

  # The full Application → Window → ContentPage → Button slice, opened via app.open_window (the M5c
  # native NSWindow host: window_handler sets the NSWindow contentView to the page's native view).
  add_executable(maui_app_sample src/samples/maui_app_sample.mm)
  set_source_files_properties(src/samples/maui_app_sample.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
  target_link_libraries(maui_app_sample PRIVATE maui_hosting maui_sanitizers)

  # The runnable demo gallery: one NSWindow hosting whichever of the 11 curated demo pages
  # (src/samples/pages/*.hpp) the MAUI_SAMPLE_PAGE env var selects. Run e.g.
  #   MAUI_SAMPLE_PAGE=pickers ./build/apple/maui_macos_gallery
  add_executable(maui_macos_gallery src/samples/macos_gallery.mm)
  set_source_files_properties(src/samples/macos_gallery.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
  target_link_libraries(maui_macos_gallery PRIVATE maui_hosting maui_sanitizers)
endif()

# ---- Sample app (iOS only): maui_app_sample's UIKit twin, a real UIApplicationMain app ----
# Unlike the test executables (plain console binaries for `simctl spawn`), a launchable iOS app needs an
# .app bundle, so this target opts back into MACOSX_BUNDLE with a minimal iOS Info.plist (CMake produces
# the flat iOS bundle layout; simulator apps need no code signing). Run it on the booted simulator:
#   xcrun simctl install booted build/ios/maui_ios_app_sample.app
#   xcrun simctl launch --console-pty booted dev.maui-cpp.ios-app-sample
if(MAUI_BACKEND STREQUAL "ios")
  add_executable(maui_ios_app_sample src/samples/ios_app_sample.mm)
  set_source_files_properties(src/samples/ios_app_sample.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
  # (M-L hosting-app-builder) builder boot — link maui_hosting (defined below; resolved at generate).
  target_link_libraries(maui_ios_app_sample PRIVATE maui_hosting maui_sanitizers)
  set_target_properties(maui_ios_app_sample PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/samples/ios_app_sample_info.plist.in"
    MACOSX_BUNDLE_BUNDLE_NAME "maui_ios_app_sample"
    MACOSX_BUNDLE_GUI_IDENTIFIER "dev.maui-cpp.ios-app-sample"
    MACOSX_BUNDLE_BUNDLE_VERSION "1"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")

  # The runnable demo gallery — the UIKit twin of maui_macos_gallery (a real .app bundle, the same
  # minimal Info.plist as maui_ios_app_sample). Run e.g.
  #   xcrun simctl install booted build/ios/maui_ios_gallery.app
  #   SIMCTL_CHILD_MAUI_SAMPLE_PAGE=pickers xcrun simctl launch booted dev.maui-cpp.ios-gallery
  add_executable(maui_ios_gallery src/samples/ios_gallery.mm)
  set_source_files_properties(src/samples/ios_gallery.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
  target_link_libraries(maui_ios_gallery PRIVATE maui_hosting maui_sanitizers)
  set_target_properties(maui_ios_gallery PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/samples/ios_app_sample_info.plist.in"
    MACOSX_BUNDLE_BUNDLE_NAME "maui_ios_gallery"
    MACOSX_BUNDLE_GUI_IDENTIFIER "dev.maui-cpp.ios-gallery"
    MACOSX_BUNDLE_BUNDLE_VERSION "1"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
endif()

# ======================================================================
# Layouts  (maui::layouts  <=  Microsoft.Maui.Layouts : cross-platform measure/arrange algorithms)
# ======================================================================
# Pure, device-free layout algorithms over the Core layout contracts; backend-agnostic (no platform
# partials — the native layout panel that hosts the arranged children arrives with the controls at M4).
add_library(maui_layouts STATIC
  src/layouts/layout_manager.cpp
  src/layouts/stack_layout_manager.cpp
  src/layouts/vertical_stack_layout_manager.cpp
  src/layouts/horizontal_stack_layout_manager.cpp
  src/layouts/grid_layout_manager.cpp
  # (G2) absolute + flex layout managers + the vendored flex engine (src/layouts/detail/flex).
  src/layouts/absolute_layout_manager.cpp
  src/layouts/flex_layout_manager.cpp
  src/layouts/detail/flex.cpp
)
target_include_directories(maui_layouts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(maui_layouts PUBLIC maui_core PRIVATE maui_sanitizers)
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_layouts PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# ---- Layout tests (GoogleTest). The mocks reuse maui::controls::view<>, so link maui_controls (which
# now also pulls in maui_layouts transitively — listing it explicitly would duplicate the lib). ----
add_executable(maui_layout_tests
  tests/layouts/vertical_stack_layout_manager_tests.cpp
  tests/layouts/horizontal_stack_layout_manager_tests.cpp
  # (U-SL) generic StackLayout orientation dispatch + StackOrientation.
  tests/layouts/stack_layout_tests.cpp
  tests/layouts/grid_layout_manager_tests.cpp
  tests/layouts/constraint_tests.cpp
  tests/layouts/z_index_tests.cpp
  # (G2) absolute + flex layout manager measure/arrange tests.
  tests/layouts/absolute_layout_manager_tests.cpp
  tests/layouts/flex_layout_manager_tests.cpp
)
target_include_directories(maui_layout_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) # for tests/layouts/*.hpp
target_link_libraries(maui_layout_tests PRIVATE
  maui_controls
  maui_sanitizers
  GTest::gmock_main
)
maui_discover_tests(maui_layout_tests)
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_layout_tests PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# ======================================================================
# Layer 6 - XAML  (maui::xaml  <=  Microsoft.Maui.Controls.Xaml : the M7 runtime loader)
# ======================================================================
# Wave 1 of the M7 runtime loader (PROFILE §6: explicit registration replaces XamlParser.GetElementType
# / Activator.CreateInstance / the property + [ContentProperty] reflection / TypeConverter discovery):
# the type/property/converter registries + the string -> value converter functions + the XML parser +
# XAML node tree (xaml_node, xaml_parser over pugixml). Pure cross-platform C++ over the controls
# layer — no platform partials — so the library builds identically on every backend.
find_package(pugixml CONFIG REQUIRED)

add_library(maui_xaml STATIC
  src/xaml/xaml_converters.cpp
  src/xaml/xaml_type_registry.cpp
  src/xaml/xaml_property_registry.cpp
  src/xaml/xaml_converter_registry.cpp
  src/xaml/xaml_standard_types.cpp
  src/xaml/xaml_node.cpp
  src/xaml/xaml_parser.cpp
  src/xaml/i_markup_extension.cpp
  # ---- M7 wave 2: hydration + markup extensions + visitors + loader ----
  src/xaml/hydration_context.cpp
  # (name_scope is now header-only — inlined in name_scope.hpp so the controls layer can use it for
  #  element's attached scope + Setter/Trigger TargetName without a controls→xaml link cycle; U-TN)
  src/xaml/markup_extensions.cpp
  src/xaml/xaml_runtime_environment.cpp
  src/xaml/xaml_static_registry.cpp
  src/xaml/xaml_visitors.cpp
  src/xaml/xaml_binding_applier.cpp
  src/xaml/xaml_runtime_bindings.cpp
  src/xaml/xaml_loader.cpp
)
target_include_directories(maui_xaml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# pugixml stays an implementation detail of the parser .cpp (never in public headers) — PRIVATE.
target_link_libraries(maui_xaml PUBLIC maui_controls PRIVATE pugixml::pugixml maui_sanitizers)
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_xaml PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# ---- XAML tests (GoogleTest). Backend-agnostic pure C++; built on headless + apple but SKIPPED on
# ios for now (device-free logic — re-running it through the simulator round-trip adds minutes for no
# coverage; the ios lane picks it up with the M7 loader integration if desired). ----
if(NOT MAUI_BACKEND STREQUAL "ios")
  add_executable(maui_xaml_tests
    tests/xaml/converter_tests.cpp
    tests/xaml/registry_tests.cpp
    tests/xaml/parser_tests.cpp
    # ---- M7 wave 2: hydration + markup extensions + visitors + loader ----
    tests/xaml/name_scope_tests.cpp
    tests/xaml/markup_extension_tests.cpp
    tests/xaml/visitor_tests.cpp
    tests/xaml/loader_tests.cpp
  )
  target_link_libraries(maui_xaml_tests PRIVATE
    maui_xaml
    maui_sanitizers
    GTest::gmock_main
  )
  maui_discover_tests(maui_xaml_tests)
  if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
    set_target_properties(maui_xaml_tests PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
  endif()
endif()

# ======================================================================
# Layer 8 - Hosting  (maui::hosting  <=  Microsoft.Maui.Hosting : MauiAppBuilder/MauiApp + lifecycle)
# ======================================================================
# (M-L hosting-app-builder) The builder boot: maui_app_builder (UseMauiApp<TApp> / ConfigureMauiHandlers
# / ConfigureLifecycleEvents / the platform-dispatcher default) -> maui_app (owns the registries, the
# dispatcher, the lifecycle registry, and the minted application). Pure cross-platform C++ over the
# controls layer — the only backend split (which dispatcher build() mints) rides the MAUI_PLATFORM_*
# compile definitions maui_core already exports. The .NET host machinery (IHostApplicationBuilder /
# Configuration / Logging) is documented out of scope in maui_app_builder.hpp.
add_library(maui_hosting STATIC
  src/hosting/maui_app_builder.cpp
  src/hosting/maui_app.cpp
  src/hosting/maui_context.cpp
  src/hosting/maui_handlers_collection.cpp
  src/hosting/maui_controls_handlers.cpp
  src/hosting/lifecycle_event_service.cpp
  src/hosting/apple_lifecycle.cpp
  src/hosting/ios_lifecycle.cpp
)
target_include_directories(maui_hosting PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# Hosting mints the controls::application and seeds the controls handler table — PUBLIC so consumers
# (samples, tests) reach the controls/core surface transitively.
target_link_libraries(maui_hosting PUBLIC maui_controls PRIVATE maui_sanitizers)
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_hosting PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# ---- Hosting tests (GoogleTest). Backend-agnostic pure C++; wired like maui_xaml_tests — built on
# headless + apple but SKIPPED on ios (no device-specific coverage; the GCD dispatcher's on-simulator
# coverage lives in maui_core_tests' gcd_dispatcher_tests.mm instead). ----
if(NOT MAUI_BACKEND STREQUAL "ios")
  add_executable(maui_hosting_tests
    tests/hosting/app_builder_tests.cpp
    tests/hosting/lifecycle_events_tests.cpp
  )
  target_link_libraries(maui_hosting_tests PRIVATE
    maui_hosting
    maui_sanitizers
    GTest::gmock_main
  )
  maui_discover_tests(maui_hosting_tests)
  if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
    set_target_properties(maui_hosting_tests PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
  endif()
endif()

# ======================================================================
# Layer 7 - Essentials  (maui::devices[::sensors]  <=  Microsoft.Maui.Devices[.Sensors])
# ======================================================================
# --- maui_essentials: devices+sensors (W1-16) ---
# Device info/display, battery, the actuators (flashlight/vibration/haptics), the six sensors and
# geolocation/geocoding, each split C#-style: a cross-platform facade + shared logic
# (src/essentials/, detail bases under src/essentials/detail/) and one platform partial per
# backend (src/platform/<backend>/essentials_<feature>.{cpp,mm}). Feasibility follows the C#
# filename-suffix oracle, documented in include/maui/essentials/feature_not_supported.hpp; the
# headless partials are controllable fakes (essentials_fakes.hpp) that unconfigured mirror the
# *.netstandard.*.cs behavior.
set(MAUI_ESSENTIALS_FEATURES
  device_info
  device_display
  battery
  flashlight
  vibration
  haptic_feedback
  accelerometer
  gyroscope
  magnetometer
  compass
  barometer
  orientation_sensor
  geolocation
  geocoding
  # --- W1-17: app-model + storage (version_tracking is PURE managed - no platform partial) ---
  preferences
  secure_storage
  app_info
  file_system
  main_thread
  launcher
  browser
  app_actions
  permissions
  # --- W2-26: essentials networking + media ---
  connectivity
  text_to_speech
  screenshot
  media_picker
  file_picker
  # --- Unit 27: comms (clipboard/share + communication email/sms/phone_dialer/contacts +
  # authentication web_authenticator + accessibility semantic_screen_reader) ---
  clipboard
  share
  email
  sms
  phone_dialer
  contacts
  web_authenticator
  semantic_screen_reader)

set(MAUI_ESSENTIALS_PLATFORM_SOURCES "")
if(MAUI_BACKEND STREQUAL "apple")
  foreach(feature IN LISTS MAUI_ESSENTIALS_FEATURES)
    list(APPEND MAUI_ESSENTIALS_PLATFORM_SOURCES src/platform/apple/essentials_${feature}.mm)
  endforeach()
  set_source_files_properties(${MAUI_ESSENTIALS_PLATFORM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
elseif(MAUI_BACKEND STREQUAL "ios")
  foreach(feature IN LISTS MAUI_ESSENTIALS_FEATURES)
    list(APPEND MAUI_ESSENTIALS_PLATFORM_SOURCES src/platform/ios/essentials_${feature}.mm)
  endforeach()
  set_source_files_properties(${MAUI_ESSENTIALS_PLATFORM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
else()
  foreach(feature IN LISTS MAUI_ESSENTIALS_FEATURES)
    list(APPEND MAUI_ESSENTIALS_PLATFORM_SOURCES src/platform/headless/essentials_${feature}.cpp)
  endforeach()
endif()

add_library(maui_essentials STATIC
  src/essentials/device_info.cpp
  src/essentials/device_display.cpp
  src/essentials/battery.cpp
  src/essentials/flashlight.cpp
  src/essentials/vibration.cpp
  src/essentials/haptic_feedback.cpp
  src/essentials/accelerometer.cpp
  src/essentials/gyroscope.cpp
  src/essentials/magnetometer.cpp
  src/essentials/compass.cpp
  src/essentials/barometer.cpp
  src/essentials/orientation_sensor.cpp
  src/essentials/geolocation.cpp
  src/essentials/geocoding.cpp
  src/essentials/location.cpp
  src/essentials/preferences.cpp
  src/essentials/secure_storage.cpp
  src/essentials/version_tracking.cpp
  src/essentials/app_info.cpp
  src/essentials/file_system.cpp
  src/essentials/main_thread.cpp
  src/essentials/launcher.cpp
  src/essentials/browser.cpp
  src/essentials/app_actions.cpp
  src/essentials/permissions.cpp
  src/essentials/unit_converters.cpp
  src/essentials/connectivity.cpp
  src/essentials/text_to_speech.cpp
  src/essentials/screenshot.cpp
  src/essentials/media_picker.cpp
  src/essentials/file_picker.cpp
  src/essentials/file_result.cpp
  # --- Unit 27: comms facades + shared logic ---
  src/essentials/clipboard.cpp
  src/essentials/share.cpp
  src/essentials/email.cpp
  src/essentials/sms.cpp
  src/essentials/phone_dialer.cpp
  src/essentials/contacts.cpp
  src/essentials/web_authenticator.cpp
  src/essentials/semantic_screen_reader.cpp
  ${MAUI_ESSENTIALS_PLATFORM_SOURCES}
)
target_include_directories(maui_essentials PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# The internal detail bases (src/essentials/detail/) + the headless fakes header are included via
# repo-root-relative paths ("src/..."), like the tests do - PRIVATE root include dir.
target_include_directories(maui_essentials PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# Essentials sits on core (event/move_only_function/cancellation_token) which carries graphics
# (vector3/quaternion for the sensor readings) transitively.
target_link_libraries(maui_essentials PUBLIC maui_core PRIVATE maui_sanitizers)
# The macOS partials reach beyond Cocoa (already PUBLIC on maui_core): IOKit (battery power
# sources + the keep-screen-on power assertion), SystemConfiguration (SCDynamicStoreCopyComputerName)
# and CoreLocation (geolocation/geocoding).
if(MAUI_BACKEND STREQUAL "apple")
  target_link_libraries(maui_essentials PUBLIC
    "-framework IOKit" "-framework SystemConfiguration" "-framework CoreLocation"
    "-framework Security" # W1-17: secure_storage keychain
    "-framework Network" # W2-26: connectivity NWPathMonitor
    # Unit 27 comms: Contacts (contacts CNContactStore), AuthenticationServices (web_authenticator
    # ASWebAuthenticationSession). Clipboard/share/email/sms/phone_dialer/semantic_screen_reader ride
    # on Cocoa/AppKit already PUBLIC on maui_core.
    "-framework Contacts" "-framework AuthenticationServices")
endif()
# The iOS partials add CoreMotion (sensors), CoreLocation (compass/geolocation/geocoding),
# AVFoundation (flashlight torch) and AudioToolbox (vibration) on top of maui_core's UIKit set;
# W1-17 adds Security (secure_storage keychain) and SafariServices (browser SFSafariViewController).
if(MAUI_BACKEND STREQUAL "ios")
  target_link_libraries(maui_essentials PUBLIC
    "-framework CoreMotion" "-framework CoreLocation" "-framework AVFoundation" "-framework AudioToolbox"
    "-framework Security" "-framework SafariServices"
    "-framework UniformTypeIdentifiers" # U13: file_picker UTType (UIDocumentPickerViewController content types)
    "-framework Network" # W2-26: connectivity NWPathMonitor
    # Unit 27 comms: MessageUI (email MFMailComposeViewController + sms MFMessageComposeViewController),
    # Contacts (contacts CNContactStore + CNContactPickerViewController via ContactsUI, which MessageUI/
    # UIKit pull), AuthenticationServices (web_authenticator). Clipboard/share/phone_dialer/
    # semantic_screen_reader ride on UIKit already PUBLIC on maui_core.
    "-framework MessageUI" "-framework Contacts" "-framework ContactsUI" "-framework AuthenticationServices")
endif()
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_essentials PROPERTIES
    CXX_CLANG_TIDY "${CLANG_TIDY_EXE}"
    OBJCXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# ---- Essentials tests (GoogleTest, discovered into ctest) ----
# The backend-agnostic value-type + shake-queue suites run everywhere. Headless adds the full API
# coverage through the fakes (the netstandard mirrors + the configured surface); apple/ios swap in
# the on-device .mm suite (real device_info/display/battery reads, actuator support matrix, sensor
# lifecycle - the simulator delivers no sensor data, so readings stay headless-tested).
if(MAUI_BACKEND STREQUAL "apple")
  add_executable(maui_essentials_tests
    tests/essentials/types_tests.cpp
    tests/essentials/accelerometer_queue_tests.cpp
    tests/essentials/version_tracking_tests.cpp # PURE managed - runs over fakes on every backend
    tests/essentials/unit_converters_tests.cpp  # PURE managed - identical on every backend
    tests/essentials/essentials_apple_tests.mm
    tests/essentials/essentials_appmodel_apple_tests.mm
    tests/essentials/essentials_media_apple_tests.mm
    tests/essentials/comms_apple_tests.mm) # Unit 27
  set_source_files_properties(
    tests/essentials/essentials_apple_tests.mm
    tests/essentials/essentials_appmodel_apple_tests.mm
    tests/essentials/essentials_media_apple_tests.mm
    tests/essentials/comms_apple_tests.mm
    PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
elseif(MAUI_BACKEND STREQUAL "ios")
  add_executable(maui_essentials_tests
    tests/essentials/types_tests.cpp
    tests/essentials/accelerometer_queue_tests.cpp
    tests/essentials/version_tracking_tests.cpp # PURE managed - runs over fakes on every backend
    tests/essentials/unit_converters_tests.cpp  # PURE managed - identical on every backend
    tests/essentials/essentials_ios_tests.mm
    tests/essentials/essentials_appmodel_ios_tests.mm
    tests/essentials/essentials_media_ios_tests.mm
    tests/essentials/comms_ios_tests.mm) # Unit 27
  set_source_files_properties(
    tests/essentials/essentials_ios_tests.mm
    tests/essentials/essentials_appmodel_ios_tests.mm
    tests/essentials/essentials_media_ios_tests.mm
    tests/essentials/comms_ios_tests.mm
    PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
  # The iOS 26.5+ simulator runtime enforces keychain entitlements for SecItem* (-34018 without
  # them); embed them the way Xcode does for simulator builds — a __TEXT,__entitlements section
  # (see tools/ios-sim-entitlements.plist for the why; signature-carried entitlements are rejected).
  target_link_options(maui_essentials_tests PRIVATE
    "LINKER:-sectcreate,__TEXT,__entitlements,${CMAKE_CURRENT_SOURCE_DIR}/tools/ios-sim-entitlements.plist")
else()
  add_executable(maui_essentials_tests
    tests/essentials/types_tests.cpp
    tests/essentials/accelerometer_queue_tests.cpp
    tests/essentials/device_info_tests.cpp
    tests/essentials/device_display_tests.cpp
    tests/essentials/battery_tests.cpp
    tests/essentials/actuators_tests.cpp
    tests/essentials/sensors_tests.cpp
    tests/essentials/geolocation_tests.cpp
    tests/essentials/geocoding_tests.cpp
    tests/essentials/preferences_tests.cpp
    tests/essentials/secure_storage_tests.cpp
    tests/essentials/version_tracking_tests.cpp
    tests/essentials/app_info_tests.cpp
    tests/essentials/file_system_tests.cpp
    tests/essentials/main_thread_tests.cpp
    tests/essentials/launcher_tests.cpp
    tests/essentials/browser_tests.cpp
    tests/essentials/app_actions_tests.cpp
    tests/essentials/permissions_tests.cpp
    tests/essentials/unit_converters_tests.cpp
    tests/essentials/connectivity_tests.cpp
    tests/essentials/text_to_speech_tests.cpp
    tests/essentials/screenshot_tests.cpp
    tests/essentials/media_picker_tests.cpp
    tests/essentials/file_picker_tests.cpp
    tests/essentials/comms_tests.cpp) # Unit 27
endif()
target_include_directories(maui_essentials_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) # for src/... detail headers
target_link_libraries(maui_essentials_tests PRIVATE
  maui_essentials
  maui_sanitizers
  GTest::gmock_main
)
maui_discover_tests(maui_essentials_tests)
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_essentials_tests PROPERTIES
    CXX_CLANG_TIDY "${CLANG_TIDY_EXE}"
    OBJCXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()
# --- end maui_essentials: devices+sensors (W1-16) ---
