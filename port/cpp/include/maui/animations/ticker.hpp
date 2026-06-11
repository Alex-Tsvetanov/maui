#pragma once
// maui::animations::ticker  <=  Microsoft.Maui.Animations.Ticker
//
// The cross-platform base ticker: fires the registered callback once per 1000/max_fps milliseconds
// while started. Ported from src/Core/src/Animations/Ticker.cs with ONE structural mapping: C#'s
// System.Timers.Timer (a threadpool timer) becomes a repeating i_dispatcher_timer from the injected
// dispatcher — every backend already supplies a dispatcher over its native loop (PROFILE §5), which
// keeps ticks on the UI thread (C# tickers fire off-thread and the animation code compensates with
// locks the port doesn't need). Over the headless manual_dispatcher this base IS deterministic:
// advancing the dispatcher's virtual clock fires the tick per interval crossed (see
// manual_ticker.hpp). The Apple backends subclass and replace start/stop with their native frame
// sources (platform_ticker.hpp), exactly like C#'s PlatformTicker partials.
//
// Lifetime: the dispatcher is borrowed (non-owning) and must outlive the ticker; the timer (and with
// it the tick subscription) is owned and torn down in stop().

#include <memory>

#include "maui/animations/i_ticker.hpp"
#include "maui/core/dispatcher.hpp"

namespace maui::animations
{
    class ticker : public i_ticker
    {
    public:
        explicit ticker(maui::core::i_dispatcher& dispatcher);

        [[nodiscard]] bool is_running() const override;
        [[nodiscard]] bool system_enabled() const override;
        [[nodiscard]] int max_fps() const override;
        void set_max_fps(int value) override;
        void set_fire(fire_callback fire) override;

        // C# Ticker.Start/Stop: idempotent — Start while running and Stop while stopped are no-ops.
        void start() override;
        void stop() override;

    protected:
        // C# Ticker.SystemEnabled protected setter: flipping the flag notifies
        // on_system_enabled_changed (which force-fires once when running animations get disabled, so
        // the manager can finish them — the C# "hack" comment).
        void set_system_enabled(bool value);
        virtual void on_system_enabled_changed();
        // Raise the registered fire callback, if any (the seam the platform tickers' native frame
        // callbacks invoke).
        void invoke_fire();

    private:
        maui::core::i_dispatcher* dispatcher_; // non-owning; must outlive the ticker
        std::unique_ptr<maui::core::i_dispatcher_timer> timer_;
        fire_callback fire_;
        int max_fps_ = 60;
        bool system_enabled_ = true;
    };
} // namespace maui::animations
