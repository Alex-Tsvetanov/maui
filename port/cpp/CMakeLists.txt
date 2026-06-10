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
    src/platform/apple/image_handler.mm
    src/platform/apple/image_source_services.mm
    src/platform/apple/layout_handler.mm
    src/platform/apple/content_page_handler.mm
    src/platform/apple/navigation_page_handler.mm
    src/platform/apple/window_handler.mm
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
    src/platform/ios/image_handler.mm
    src/platform/ios/image_source_services.mm
    src/platform/ios/layout_handler.mm
    src/platform/ios/content_page_handler.mm
    src/platform/ios/navigation_page_handler.mm
    src/platform/ios/window_handler.mm
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
    src/platform/headless/image_handler.cpp
    src/platform/headless/image_source_services.cpp
    src/platform/headless/layout_handler.cpp
    src/platform/headless/content_page_handler.cpp
    src/platform/headless/navigation_page_handler.cpp
    src/platform/headless/window_handler.cpp)
endif()

add_library(maui_core STATIC
  src/core/manual_dispatcher.cpp
  src/core/bindable_object.cpp
  src/core/thickness.cpp
  src/core/font.cpp
  src/core/property_mapper.cpp
  src/core/command_mapper.cpp
  src/core/handler_registry.cpp
  src/core/view_platform_base.cpp
  src/core/view_mapper.cpp
  src/core/shadow.cpp
  src/core/button_handler.cpp
  src/core/label_handler.cpp
  src/core/entry_handler.cpp
  src/core/switch_handler.cpp
  src/core/check_box_handler.cpp
  src/core/image_handler.cpp
  src/core/image_source_loader.cpp
  src/core/image_source_service_registry.cpp
  src/core/image_source_services.cpp
  src/core/uri_bytes.cpp
  src/core/uri_image_disk_cache.cpp
  src/core/crc64.cpp
  src/core/layout_handler.cpp
  src/core/layout_z_order.cpp
  src/core/content_page_handler.cpp
  src/core/navigation_page_handler.cpp
  src/core/window_handler.cpp
  src/core/grid_length.cpp
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
  target_link_libraries(maui_core PUBLIC "-framework Cocoa" "-framework QuartzCore")
  target_compile_definitions(maui_core PUBLIC MAUI_PLATFORM_APPLE)
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
  target_link_libraries(maui_core PUBLIC
    "-framework UIKit" "-framework QuartzCore" "-framework Foundation" "-framework CoreFoundation"
    "-framework ImageIO" "-framework CoreGraphics" objc)
  target_compile_definitions(maui_core PUBLIC MAUI_PLATFORM_IOS)
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
  tests/core/contracts_tests.cpp
  tests/core/handler_tests.cpp
  tests/core/binding_tests.cpp
  tests/core/crc64_tests.cpp
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
    tests/platform/android_widget_tests.cpp)
  target_include_directories(maui_android_widget_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/android)
  target_link_libraries(maui_android_widget_tests PRIVATE GTest::gtest maui_sanitizers)
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

add_library(maui_controls STATIC
  src/controls/element.cpp
  src/controls/view.cpp
  src/controls/gestures/gesture_recognizers.cpp
  src/controls/gestures/gesture_recognizer_collection.cpp
  src/controls/gestures/gesture_platform_manager.cpp
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
  src/controls/entry.cpp
  src/controls/toggle_switch.cpp
  src/controls/check_box.cpp
  src/controls/image.cpp
  src/controls/file_image_source.cpp
  src/controls/vertical_stack_layout.cpp
  src/controls/horizontal_stack_layout.cpp
  src/controls/grid.cpp
  src/controls/content_page.cpp
  src/controls/navigation_page.cpp
  src/controls/style.cpp
  src/controls/resource_dictionary.cpp
  src/controls/merged_style.cpp
  src/controls/visual_state_manager.cpp
  src/controls/window.cpp
  src/controls/application.cpp
)
target_include_directories(maui_controls PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# Controls depend on Core (handlers, contracts, the property system); the layout controls additionally
# use the maui_layouts measure/arrange managers (their create_layout_manager() instantiates them).
target_link_libraries(maui_controls PUBLIC maui_core maui_layouts PRIVATE maui_sanitizers)
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_controls PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# ---- Controls tests (GoogleTest, discovered into ctest) ----
# The button control + the handler seam (control -> handler -> platform -> back). The seam test is
# backend-specific: headless drives the fake platform; apple drives a real NSButton via performClick.
if(MAUI_BACKEND STREQUAL "apple")
  add_executable(maui_controls_tests
    tests/controls/button_apple_tests.mm
    tests/controls/gesture_apple_tests.mm # E1: native recognizer attachment + target/action drives
    tests/controls/label_apple_tests.mm
    tests/controls/view_mapper_apple_tests.mm
    tests/controls/entry_apple_tests.mm
    tests/controls/toggle_switch_apple_tests.mm
    tests/controls/check_box_apple_tests.mm
    tests/controls/image_apple_tests.mm
    tests/controls/font_image_source_apple_tests.mm
    tests/controls/layout_apple_tests.mm
    tests/controls/grid_apple_tests.mm
    tests/controls/content_page_apple_tests.mm
    tests/controls/navigation_page_apple_tests.mm
    tests/controls/window_apple_tests.mm
    tests/controls/window_handler_tests.cpp # the portable seam mirrors (real NSWindow behind them here)
    tests/controls/style_tests.cpp   # backend-agnostic; compiled as C++ within the apple test exe
    tests/controls/trigger_tests.cpp
    tests/controls/visual_state_manager_tests.cpp
    tests/controls/resource_dictionary_tests.cpp
    tests/controls/dynamic_resource_tests.cpp
    tests/controls/implicit_style_tests.cpp
    tests/controls/advanced_trigger_tests.cpp
    tests/controls/size_request_tests.cpp # backend-agnostic; the VisualElement size-request surface
    tests/controls/lifecycle_tests.cpp
    tests/controls/data_template_tests.cpp    # backend-agnostic (W1-09 templates)
    tests/controls/control_template_tests.cpp)
  # tests/core/image_source_tests.cpp + font_image_source_tests.cpp are HEADLESS-ONLY: they assert on the
  # headless image_source_result mirror + the headless "any non-empty bytes => loaded" decode (raw test
  # bytes are not decodable NSImages on apple). The apple backend's native rasterize is covered by
  # tests/controls/font_image_source_apple_tests.mm instead.
  set_source_files_properties(
    tests/controls/button_apple_tests.mm
    tests/controls/gesture_apple_tests.mm
    tests/controls/label_apple_tests.mm
    tests/controls/view_mapper_apple_tests.mm
    tests/controls/entry_apple_tests.mm
    tests/controls/toggle_switch_apple_tests.mm
    tests/controls/check_box_apple_tests.mm
    tests/controls/image_apple_tests.mm
    tests/controls/font_image_source_apple_tests.mm
    tests/controls/layout_apple_tests.mm
    tests/controls/grid_apple_tests.mm
    tests/controls/content_page_apple_tests.mm
    tests/controls/navigation_page_apple_tests.mm
    tests/controls/window_apple_tests.mm
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
    tests/controls/entry_ios_tests.mm
    tests/controls/toggle_switch_ios_tests.mm
    tests/controls/check_box_ios_tests.mm
    tests/controls/image_ios_tests.mm
    tests/controls/font_image_source_ios_tests.mm
    tests/controls/image_sources_ios_tests.mm
    tests/controls/layout_ios_tests.mm
    tests/controls/grid_ios_tests.mm
    tests/controls/content_page_ios_tests.mm
    tests/controls/navigation_page_ios_tests.mm
    tests/controls/window_ios_tests.mm)
  add_executable(maui_controls_tests
    ${MAUI_IOS_TEST_MM_SOURCES}
    tests/controls/style_tests.cpp
    tests/controls/trigger_tests.cpp
    tests/controls/visual_state_manager_tests.cpp
    tests/controls/resource_dictionary_tests.cpp
    tests/controls/dynamic_resource_tests.cpp
    tests/controls/implicit_style_tests.cpp
    tests/controls/advanced_trigger_tests.cpp
    tests/controls/size_request_tests.cpp # backend-agnostic (compares against the handler's own size)
    tests/controls/lifecycle_tests.cpp
    tests/controls/data_template_tests.cpp    # backend-agnostic (W1-09 templates)
    tests/controls/control_template_tests.cpp)
  set_source_files_properties(${MAUI_IOS_TEST_MM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
else()
  add_executable(maui_controls_tests
    tests/controls/button_tests.cpp
    tests/controls/gesture_recognizer_tests.cpp # the E1 gesture state machines + synthetic pipeline
    tests/controls/label_tests.cpp
    tests/controls/view_mapper_tests.cpp
    tests/controls/entry_tests.cpp
    tests/controls/toggle_switch_tests.cpp
    tests/controls/check_box_tests.cpp
    tests/controls/image_tests.cpp
    tests/controls/layout_tests.cpp
    tests/controls/grid_tests.cpp
    tests/controls/content_page_tests.cpp
    tests/controls/navigation_page_tests.cpp
    tests/controls/style_tests.cpp
    tests/controls/trigger_tests.cpp
    tests/controls/visual_state_manager_tests.cpp
    tests/controls/resource_dictionary_tests.cpp
    tests/controls/dynamic_resource_tests.cpp
    tests/controls/implicit_style_tests.cpp
    tests/controls/advanced_trigger_tests.cpp
    tests/controls/size_request_tests.cpp # the VisualElement size-request surface (view<> IView mapping)
    tests/controls/lifecycle_tests.cpp
    tests/controls/data_template_tests.cpp    # backend-agnostic (W1-09 templates)
    tests/controls/control_template_tests.cpp
    tests/controls/window_handler_tests.cpp # the window_handler seam over the headless mirror
    tests/core/image_source_tests.cpp     # loader cache TTL + the typed service-provider
    tests/core/font_image_source_tests.cpp)
endif()
target_include_directories(maui_controls_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) # for tests/layouts/*.hpp
# The apple .mm tests share the backend's text/visual op headers (apple_text_ops.hpp etc.), which live
# beside the handler .mm sources; put that dir on the include path for the apple test exe only.
if(MAUI_BACKEND STREQUAL "apple")
  target_include_directories(maui_controls_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/apple)
endif()
# Same convention for the ios .mm tests (ios_text_ops.hpp / ios_conversions.hpp live beside the
# UIKit handler sources).
if(MAUI_BACKEND STREQUAL "ios")
  target_include_directories(maui_controls_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/ios)
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
  tests/layouts/grid_layout_manager_tests.cpp
  tests/layouts/constraint_tests.cpp
  tests/layouts/z_index_tests.cpp
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
  src/xaml/name_scope.cpp
  src/xaml/markup_extensions.cpp
  src/xaml/xaml_runtime_environment.cpp
  src/xaml/xaml_static_registry.cpp
  src/xaml/xaml_visitors.cpp
  src/xaml/xaml_binding_applier.cpp
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
  geocoding)

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
    "-framework IOKit" "-framework SystemConfiguration" "-framework CoreLocation")
endif()
# The iOS partials add CoreMotion (sensors), CoreLocation (compass/geolocation/geocoding),
# AVFoundation (flashlight torch) and AudioToolbox (vibration) on top of maui_core's UIKit set.
if(MAUI_BACKEND STREQUAL "ios")
  target_link_libraries(maui_essentials PUBLIC
    "-framework CoreMotion" "-framework CoreLocation" "-framework AVFoundation" "-framework AudioToolbox")
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
    tests/essentials/essentials_apple_tests.mm)
  set_source_files_properties(tests/essentials/essentials_apple_tests.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
elseif(MAUI_BACKEND STREQUAL "ios")
  add_executable(maui_essentials_tests
    tests/essentials/types_tests.cpp
    tests/essentials/accelerometer_queue_tests.cpp
    tests/essentials/essentials_ios_tests.mm)
  set_source_files_properties(tests/essentials/essentials_ios_tests.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
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
    tests/essentials/geocoding_tests.cpp)
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
