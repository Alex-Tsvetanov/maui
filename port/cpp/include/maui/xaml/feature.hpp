#pragma once
// maui/xaml/feature.hpp — feature-test gating for the COMPILE-TIME XAML path (PUBLIC_API_DESIGN.md §6).
//
// The port offers XAML three ways, in ascending "magic" and descending portability:
//   1. runtime loader   (maui_xaml, xaml_loader::load_into)  — always available, parses at startup.
//   2. build-time codegen (maui_xaml_codegen)                — a host tool emits maui::ui C++; needs a
//                                                              runnable host binary, so it does NOT
//                                                              cross-compile (the iOS app build can't
//                                                              run an arm64-sim codegen tool mid-build).
//   3. compile-time      (this layer: #embed + build_page<VM, fixed_string>) — the compiler itself
//                                                              embeds the raw .xaml bytes into the TU
//                                                              and hydrates them; NO host tool, so it
//                                                              cross-compiles to iOS cleanly. This is
//                                                              why the user asked for it: it is the one
//                                                              path that gives the "C++ & XAML" iOS
//                                                              column without a build-system codegen step.
//
// Two capabilities gate path 3, probed via standard feature-test macros so the surface degrades
// gracefully on an older toolchain rather than failing to compile:
//
//   MAUI_HAS_EMBED            — the compiler implements #embed (C23/C++26 P1967). Required to pull the
//                               raw markup into the binary with no external tooling. Apple clang 21: yes
//                               (as a -Wc23-extensions extension under -std=c++2c).
//   MAUI_HAS_XAML_REFLECTION  — static reflection (P2996, std::meta) is available, so a `{Binding Path}`
//                               can be resolved to a view-model member BY NAME at compile time, making a
//                               typo / missing member a COMPILE ERROR with no registration boilerplate.
//                               Apple clang 21: NO (__cpp_reflection undefined) — so the auto-resolving
//                               binding path is compiled out and callers wire bindings explicitly via the
//                               typed ui::bind layer (still compile-checked, just not name-driven).
//
// MAUI_HAS_COMPILE_TIME_XAML is the umbrella: structural compile-time XAML (layout + literal props, no
// name-driven bindings) needs only #embed and class-type NTTPs, both present on Apple clang 21.

// ---- #embed ------------------------------------------------------------------------------------------
#if defined(__has_embed)
    #define MAUI_HAS_EMBED 1
#else
    #define MAUI_HAS_EMBED 0
#endif

// ---- static reflection (P2996) -----------------------------------------------------------------------
#if defined(__cpp_reflection) && __cpp_reflection >= 202400L
    #define MAUI_HAS_XAML_REFLECTION 1
#else
    #define MAUI_HAS_XAML_REFLECTION 0
#endif

// ---- class-type NTTP (P0732/P1907) — the vehicle for build_page<VM, fixed_string> ---------------------
// Apple clang under-reports __cpp_nontype_template_args (201411, the C++17 level) yet accepts class-type
// NTTPs; we therefore treat C++20 (__cplusplus >= 202002L) as sufficient and let a real failure surface
// at the build_page instantiation rather than gating it off on the macro alone.
#if defined(__cplusplus) && __cplusplus >= 202002L
    #define MAUI_HAS_CLASS_NTTP 1
#else
    #define MAUI_HAS_CLASS_NTTP 0
#endif

// ---- the umbrella ------------------------------------------------------------------------------------
#if MAUI_HAS_EMBED && MAUI_HAS_CLASS_NTTP
    #define MAUI_HAS_COMPILE_TIME_XAML 1
#else
    #define MAUI_HAS_COMPILE_TIME_XAML 0
#endif
