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
  src/graphics/point.cpp
  src/graphics/point_f.cpp
  src/graphics/size.cpp
  src/graphics/size_f.cpp
  src/graphics/rect.cpp
  src/graphics/rect_f.cpp
  src/graphics/path_f.cpp
  src/graphics/path_builder.cpp
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
)
target_link_libraries(maui_graphics_tests PRIVATE
  maui_graphics
  maui_sanitizers
  GTest::gmock_main   # transitively pulls gmock + gtest + main(); single chain avoids duplicate-lib warnings
)
gtest_discover_tests(maui_graphics_tests)

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
add_library(maui_core STATIC
  src/core/manual_dispatcher.cpp
  src/core/bindable_object.cpp
  src/core/thickness.cpp
  src/core/font.cpp
  src/core/property_mapper.cpp
  src/core/command_mapper.cpp
  src/core/handler_registry.cpp
)
target_include_directories(maui_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# Layer 1 depends on layer 0: core contracts expose graphics value types (color in i_text_style,
# rect/size in i_view). PUBLIC so consumers of maui_core link maui_graphics transitively.
target_link_libraries(maui_core PUBLIC maui_graphics PRIVATE maui_sanitizers)
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
)
target_link_libraries(maui_core_tests PRIVATE
  maui_core
  maui_sanitizers
  GTest::gmock_main
)
gtest_discover_tests(maui_core_tests)
