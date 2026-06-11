#pragma once
// maui::animations::animation_manager  <=  Microsoft.Maui.Animations.AnimationManager
//
// The concrete i_animation_manager: keeps the list of running animations, ticks each one from the
// ticker's fire callback with the elapsed milliseconds (times speed_modifier), auto-starts the
// ticker when the first animation arrives (auto_start_ticker) and stops it when the last one
// finishes or is removed. When the ticker reports the system disabled animations mid-run, every
// running animation is forced to its end state and the ticker is stopped (the C# "hack" path).
// Ported from src/Core/src/Animations/AnimationManager.cs.
//
// Ownership: the manager OWNS its ticker (shared_ptr — C# Dispose disposes it; tests keep their own
// handle to drive the manual ticker) and shares ownership of every added animation (the GC analog).
// Determinism seam: C#'s `internal virtual AdjustSpeed` and the static Environment.TickCount read
// become the two protected virtuals adjust_speed / current_tick_milliseconds — the C#
// TestAnimationManager overrides the former to pin every tick to 16 ms; a clock override makes the
// default speed-modifier path testable without wall-clock (the port's injected-clock doctrine, cf.
// image_source_loader).

#include <cstdint>
#include <memory>
#include <vector>

#include "maui/animations/animation.hpp"
#include "maui/animations/i_animation_manager.hpp"
#include "maui/animations/i_ticker.hpp"

namespace maui::animations
{
    class animation_manager : public i_animation_manager
    {
    public:
        explicit animation_manager(std::shared_ptr<i_ticker> ticker);
        animation_manager(const animation_manager&) = delete;
        animation_manager& operator=(const animation_manager&) = delete;
        animation_manager(animation_manager&&) = delete;
        animation_manager& operator=(animation_manager&&) = delete;
        // Detaches the fire callback (which captures `this`) and stops the ticker, so a test-held
        // ticker can never fire into a destroyed manager.
        ~animation_manager() override;

        [[nodiscard]] i_ticker& ticker() const override;
        [[nodiscard]] double speed_modifier() const override;
        void set_speed_modifier(double value) override;
        [[nodiscard]] bool auto_start_ticker() const override;
        void set_auto_start_ticker(bool value) override;

        // C# Add: no-op while the system has animations disabled; otherwise register the animation
        // (once) and auto-start the ticker. Remove: drop it and stop the ticker when none remain.
        void add(std::shared_ptr<animation> animation_to_add) override;
        void remove(animation& animation_to_remove) override;

    protected:
        // C# internal virtual AdjustSpeed(elapsedMilliseconds) — the per-tick elapsed time handed to
        // each animation; the default scales by speed_modifier. Test managers override (=> 16).
        [[nodiscard]] virtual double adjust_speed(double elapsed_milliseconds) const;
        // C# static GetCurrentTick (Environment.TickCount), virtual here for clock injection.
        [[nodiscard]] virtual std::int64_t current_tick_milliseconds() const;

    private:
        void start();
        void end();
        void on_fire();
        void force_finish_animations();
        // C#'s list TryRemove: erase without the public remove()'s "stop the ticker when empty" step.
        void try_remove(const animation& animation_to_remove);

        std::shared_ptr<i_ticker> ticker_;
        std::vector<std::shared_ptr<animation>> animations_;
        std::int64_t last_update_ = 0;
        double speed_modifier_ = 1.0;
        bool auto_start_ticker_ = true;
    };
} // namespace maui::animations
