#pragma once
// maui::animations::manual_ticker — the deterministic test/headless ticker (the W1-14 analog of the
// C# test tickers in src/Controls/tests/Core.UnitTests/TestClasses/{BlockingTicker,AsyncTicker}.cs,
// and the port's PlatformTicker.Standard.cs stand-in).
//
// It is the base `ticker` constructed over the port's deterministic dispatcher
// (maui::core::manual_dispatcher): start() arms a repeating dispatcher timer at 1000/max_fps ms, and
// the test pumps time with dispatcher.advance(delta) — the timer fires once per interval crossed, so
// every tick is reproducible with no wall clock and no threads. Pair it with an animation manager
// whose adjust_speed override pins the per-tick elapsed time (the C# TestAnimationManager pattern)
// for fully deterministic animation runs.
//
// The only addition over the base is the PUBLIC set_system_enabled (C# AsyncTicker.SetEnabled): tests
// flip it to drive the manager's force-finish path.

#include "maui/animations/ticker.hpp"

namespace maui::animations
{
    class manual_ticker final : public ticker
    {
    public:
        using ticker::ticker;
        // Tests flip the system-enabled flag directly (protected on the production base).
        using ticker::set_system_enabled;
    };
} // namespace maui::animations
