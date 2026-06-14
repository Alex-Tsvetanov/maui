#pragma once
// maui::tests::run_loop_pump — a shared on-device test helper that pumps the main run loop so the
// platform's deferred work (UICollectionView/NSCollectionView layout, cell realization, async
// completion blocks, GCD main-queue posts) actually runs before the test inspects the result. UIKit
// and AppKit do their layout/cell-realization lazily inside the run loop, so a test that mutates the
// model and immediately reads back the native state sees nothing until the loop turns; these helpers
// turn it deterministically.
//
// Two shapes, both pumping NSDefaultRunLoopMode in small slices up to a generous deadline (a positive
// completion condition keeps the on-simulator suite non-flaky — the same recipe the web_view_ios suite
// proved):
//   - pump_run_loop(seconds): spin the loop for at least `seconds` (a fixed settle — for native work
//     with no observable boolean, e.g. "let the compositional layout run a pass");
//   - pump_until(predicate[, seconds]): spin until predicate() turns true or the deadline elapses,
//     returning predicate()'s final value (the caller ASSERT_TRUEs it).
//
// Header-only and Objective-C++ (uses Foundation's NSRunLoop/NSDate). Include it only from a `.mm`
// compiled for the apple or ios backend — guarded so a plain C++ TU that includes it by accident is a
// no-op rather than a compile error (the predicate is evaluated once and returned).

#include <chrono>

#if defined(__OBJC__) && (defined(MAUI_PLATFORM_IOS) || defined(MAUI_PLATFORM_APPLE))
    #import <Foundation/Foundation.h>
#endif

namespace maui::tests
{
    // The default per-call deadline. CollectionView realization is fast, but the first compositional
    // layout pass after a fresh window mount can take a beat on a cold simulator; 5s is comfortably
    // generous while still failing fast if a positive condition genuinely never arrives.
    inline constexpr double k_default_pump_seconds = 5.0;

    // The slice the loop runs each turn (50ms — small enough to re-check the predicate promptly,
    // large enough to let UIKit actually do work between checks).
    inline constexpr double k_pump_slice_seconds = 0.05;

#if defined(__OBJC__) && (defined(MAUI_PLATFORM_IOS) || defined(MAUI_PLATFORM_APPLE))

    // Spin the main run loop for at least `seconds` (a fixed settle when there is no boolean to wait on).
    inline void pump_run_loop(double seconds = k_pump_slice_seconds)
    {
        NSDate* const deadline = [NSDate dateWithTimeIntervalSinceNow:seconds];
        while (deadline.timeIntervalSinceNow > 0)
        {
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode
                                     beforeDate:[NSDate dateWithTimeIntervalSinceNow:k_pump_slice_seconds]];
        }
    }

    // Spin the main run loop until `done()` returns true or the deadline elapses; returns done()'s
    // final value (an early-out the moment the condition holds keeps the suite fast).
    template <typename Predicate> bool pump_until(Predicate done, double seconds = k_default_pump_seconds)
    {
        NSDate* const deadline = [NSDate dateWithTimeIntervalSinceNow:seconds];
        while (!done() && deadline.timeIntervalSinceNow > 0)
        {
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode
                                     beforeDate:[NSDate dateWithTimeIntervalSinceNow:k_pump_slice_seconds]];
        }
        return done();
    }

#else

    // Non-apple / non-Objective-C fallbacks: no run loop to pump. pump_run_loop is a no-op; pump_until
    // evaluates the predicate once (so a headless caller still gets the current state, never blocks).
    inline void pump_run_loop(double /*seconds*/ = k_pump_slice_seconds)
    {
    }

    template <typename Predicate> bool pump_until(Predicate done, double /*seconds*/ = k_default_pump_seconds)
    {
        return done();
    }

#endif
} // namespace maui::tests
