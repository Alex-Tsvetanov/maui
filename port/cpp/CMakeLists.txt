# Root build for the C++23 MAUI port. Selects a native backend by preset/option
# (MAUI_BACKEND), mirroring MAUI's cross-platform/platform split (PROFILE.md §3-§4).
cmake_minimum_required(VERSION 3.28)
project(maui_cpp LANGUAGES CXX)

# ---- Language standard (PROFILE.md §1) ----
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Backend selection (headless first; macOS/iOS/Windows/Android/Linux later) ----
set(MAUI_BACKEND "headless" CACHE STRING "Native backend: headless|apple|windows|android|linux")
set_property(CACHE MAUI_BACKEND PROPERTY STRINGS headless apple windows android linux)
message(STATUS "MAUI_BACKEND = ${MAUI_BACKEND}")

# ---- Warnings (PROFILE.md §1: treat as errors in CI once green) ----
add_compile_options(-Wall -Wextra -Wpedantic)

enable_testing()

# ---- Dependencies (vcpkg manifest: gtest, benchmark) ----
find_package(GTest CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)
include(GoogleTest)

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
  src/graphics/path_f.cpp
  src/graphics/path_builder.cpp
  src/graphics/solid_paint.cpp
  src/graphics/gradient_paint.cpp
  src/graphics/linear_gradient_paint.cpp
  src/graphics/radial_gradient_paint.cpp
  src/graphics/shapes/round_rectangle.cpp
)
target_include_directories(maui_graphics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(maui_graphics PRIVATE maui_sanitizers)

# clang-tidy runs on the port's own sources (its headers are reached transitively); tests and
# benchmarks are excluded to keep the lint focused on the implementation.
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_graphics PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# ---- Graphics tests (GoogleTest, discovered into ctest) ----
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
)
target_link_libraries(maui_graphics_tests PRIVATE
  maui_graphics
  maui_sanitizers
  GTest::gmock_main   # transitively pulls gmock + gtest + main(); single chain avoids duplicate-lib warnings
)
gtest_discover_tests(maui_graphics_tests)
# Lint the test TUs too (kept clean: 0 findings). Same guard as the library targets above.
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_graphics_tests PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
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
    src/platform/apple/image_handler.mm
    src/platform/apple/image_source_services.mm
    src/platform/apple/layout_handler.mm
    src/platform/apple/content_page_handler.mm
    src/platform/apple/navigation_page_handler.mm)
  set_source_files_properties(${MAUI_CORE_PLATFORM_SOURCES} PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
else()
  set(MAUI_CORE_PLATFORM_SOURCES
    src/platform/headless/button_handler.cpp
    src/platform/headless/label_handler.cpp
    src/platform/headless/entry_handler.cpp
    src/platform/headless/image_handler.cpp
    src/platform/headless/image_source_services.cpp
    src/platform/headless/layout_handler.cpp
    src/platform/headless/content_page_handler.cpp
    src/platform/headless/navigation_page_handler.cpp)
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
  src/core/image_handler.cpp
  src/core/image_source_loader.cpp
  src/core/image_source_service_registry.cpp
  src/core/image_source_services.cpp
  src/core/uri_bytes.cpp
  src/core/layout_handler.cpp
  src/core/content_page_handler.cpp
  src/core/navigation_page_handler.cpp
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
)
target_link_libraries(maui_core_tests PRIVATE
  maui_core
  maui_sanitizers
  GTest::gmock_main
)
gtest_discover_tests(maui_core_tests)
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_core_tests PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# ======================================================================
# Layer 2 - Controls  (maui::controls  <=  Microsoft.Maui.Controls : the developer-facing controls)
# ======================================================================
add_library(maui_controls STATIC
  src/controls/element.cpp
  src/controls/view.cpp
  src/controls/button.cpp
  src/controls/label.cpp
  src/controls/entry.cpp
  src/controls/image.cpp
  src/controls/file_image_source.cpp
  src/controls/vertical_stack_layout.cpp
  src/controls/horizontal_stack_layout.cpp
  src/controls/grid.cpp
  src/controls/content_page.cpp
  src/controls/navigation_page.cpp
  src/controls/style.cpp
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
    tests/controls/label_apple_tests.mm
    tests/controls/view_mapper_apple_tests.mm
    tests/controls/entry_apple_tests.mm
    tests/controls/image_apple_tests.mm
    tests/controls/font_image_source_apple_tests.mm
    tests/controls/layout_apple_tests.mm
    tests/controls/grid_apple_tests.mm
    tests/controls/content_page_apple_tests.mm
    tests/controls/navigation_page_apple_tests.mm
    tests/controls/style_tests.cpp   # backend-agnostic; compiled as C++ within the apple test exe
    tests/controls/trigger_tests.cpp
    tests/controls/visual_state_manager_tests.cpp
    tests/controls/lifecycle_tests.cpp)
  # tests/core/image_source_tests.cpp + font_image_source_tests.cpp are HEADLESS-ONLY: they assert on the
  # headless image_source_result mirror + the headless "any non-empty bytes => loaded" decode (raw test
  # bytes are not decodable NSImages on apple). The apple backend's native rasterize is covered by
  # tests/controls/font_image_source_apple_tests.mm instead.
  set_source_files_properties(
    tests/controls/button_apple_tests.mm
    tests/controls/label_apple_tests.mm
    tests/controls/view_mapper_apple_tests.mm
    tests/controls/entry_apple_tests.mm
    tests/controls/image_apple_tests.mm
    tests/controls/font_image_source_apple_tests.mm
    tests/controls/layout_apple_tests.mm
    tests/controls/grid_apple_tests.mm
    tests/controls/content_page_apple_tests.mm
    tests/controls/navigation_page_apple_tests.mm
    PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
else()
  add_executable(maui_controls_tests
    tests/controls/button_tests.cpp
    tests/controls/label_tests.cpp
    tests/controls/view_mapper_tests.cpp
    tests/controls/entry_tests.cpp
    tests/controls/image_tests.cpp
    tests/controls/layout_tests.cpp
    tests/controls/grid_tests.cpp
    tests/controls/content_page_tests.cpp
    tests/controls/navigation_page_tests.cpp
    tests/controls/style_tests.cpp
    tests/controls/trigger_tests.cpp
    tests/controls/visual_state_manager_tests.cpp
    tests/controls/lifecycle_tests.cpp
    tests/core/image_source_tests.cpp     # loader cache TTL + the typed service-provider
    tests/core/font_image_source_tests.cpp)
endif()
target_include_directories(maui_controls_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) # for tests/layouts/*.hpp
# The apple .mm tests share the backend's text/visual op headers (apple_text_ops.hpp etc.), which live
# beside the handler .mm sources; put that dir on the include path for the apple test exe only.
if(MAUI_BACKEND STREQUAL "apple")
  target_include_directories(maui_controls_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/apple)
endif()
target_link_libraries(maui_controls_tests PRIVATE
  maui_controls
  maui_sanitizers
  GTest::gmock_main
)
gtest_discover_tests(maui_controls_tests)
# The controls tests are .cpp on headless and .mm (OBJCXX) on apple; lint whichever language is built.
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_controls_tests PROPERTIES
    CXX_CLANG_TIDY "${CLANG_TIDY_EXE}"
    OBJCXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# ---- Sample app (macOS only): a real NSWindow hosting the cross-platform button ----
if(MAUI_BACKEND STREQUAL "apple")
  add_executable(maui_button_sample src/samples/button_sample.mm)
  set_source_files_properties(src/samples/button_sample.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
  target_link_libraries(maui_button_sample PRIVATE maui_controls maui_sanitizers)
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
)
target_include_directories(maui_layout_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) # for tests/layouts/*.hpp
target_link_libraries(maui_layout_tests PRIVATE
  maui_controls
  maui_sanitizers
  GTest::gmock_main
)
gtest_discover_tests(maui_layout_tests)
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  set_target_properties(maui_layout_tests PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()
