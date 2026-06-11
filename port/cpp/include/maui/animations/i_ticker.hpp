#pragma once
// maui::animations::i_ticker  <=  Microsoft.Maui.Animations.ITicker
//
// A ticker makes sure animations get triggered to advance through their stages: the animation
// manager registers its fire callback (C# ITicker.Fire), starts the ticker while animations are
// running, and stops it when none remain. Each backend supplies a concrete ticker (see ticker.hpp
// and platform_ticker.hpp). Ported from src/Core/src/Animations/ITicker.cs; the settable C# `Fire`
// property becomes set_fire (a move_only_function callback, per the port's callback doctrine).

#include "maui/core/move_only_function.hpp"

namespace maui::animations
{
    class i_ticker
    {
    public:
        using fire_callback = maui::core::move_only_function<void()>;

        virtual ~i_ticker() = default;

        // C# ITicker.IsRunning: whether this ticker is currently running.
        [[nodiscard]] virtual bool is_running() const = 0;
        // C# ITicker.SystemEnabled: false when the system disabled animations (e.g. energy saving);
        // the manager force-finishes running animations when this drops while ticking.
        [[nodiscard]] virtual bool system_enabled() const = 0;
        // C# ITicker.MaxFps: the maximum frames per second this ticker can handle (default 60).
        [[nodiscard]] virtual int max_fps() const = 0;
        virtual void set_max_fps(int value) = 0;
        // C# ITicker.Fire: the callback triggered each time the ticker interval elapses.
        virtual void set_fire(fire_callback fire) = 0;

        virtual void start() = 0;
        virtual void stop() = 0;

    protected:
        i_ticker() = default;
        i_ticker(const i_ticker&) = default;
        i_ticker(i_ticker&&) = default;
        i_ticker& operator=(const i_ticker&) = default;
        i_ticker& operator=(i_ticker&&) = default;
    };
} // namespace maui::animations
