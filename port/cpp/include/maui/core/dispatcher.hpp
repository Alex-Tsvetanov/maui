#pragma once
// maui::core::i_dispatcher        <=  Microsoft.Maui.Dispatching.IDispatcher
// maui::core::i_dispatcher_timer  <=  Microsoft.Maui.Dispatching.IDispatcherTimer
//
// The UI-thread message-pump abstraction (PROFILE.md §5). Cross-thread work is marshalled back onto
// the dispatcher's thread via dispatch(); each backend supplies a concrete dispatcher over its native
// loop (GCD / DispatcherQueue / Looper). The headless backend's deterministic, virtual-clock
// implementation is `manual_dispatcher` (manual_dispatcher.hpp).
//
// Mirrors the C# contract: IDispatcher.{IsDispatchRequired, Dispatch, DispatchDelayed, CreateTimer}
// and IDispatcherTimer.{Interval, IsRepeating, IsRunning, Tick, Start, Stop}. C# properties become
// get/set accessors here because the contract is a runtime-polymorphic interface (PROFILE.md §11
// "per-type rule"), not a concrete bindable type.

#include <chrono>
#include <memory>

#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::core
{
    // A unit of work posted to a dispatcher.
    using dispatcher_action = move_only_function<void()>;

    class i_dispatcher_timer
    {
    public:
        virtual ~i_dispatcher_timer() = default;

        [[nodiscard]] virtual std::chrono::milliseconds interval() const = 0;
        virtual void set_interval(std::chrono::milliseconds value) = 0;
        [[nodiscard]] virtual bool is_repeating() const = 0;
        virtual void set_is_repeating(bool value) = 0;
        [[nodiscard]] virtual bool is_running() const = 0;

        virtual void start() = 0;
        virtual void stop() = 0;

        // Fired once per elapsed interval (the C# Tick event).
        virtual event<> &tick() = 0;

    protected:
        i_dispatcher_timer() = default;
        i_dispatcher_timer(const i_dispatcher_timer &) = default;
        i_dispatcher_timer(i_dispatcher_timer &&) = default;
        i_dispatcher_timer &operator=(const i_dispatcher_timer &) = default;
        i_dispatcher_timer &operator=(i_dispatcher_timer &&) = default;
    };

    class i_dispatcher
    {
    public:
        virtual ~i_dispatcher() = default;

        // True when the caller is NOT on the dispatcher's thread (so work must be dispatch()'d).
        [[nodiscard]] virtual bool is_dispatch_required() const = 0;

        // Post an action onto the dispatcher's thread; returns true if it was scheduled.
        virtual bool dispatch(dispatcher_action action) = 0;

        // Post an action to run after at least `delay`; returns true if it was scheduled.
        virtual bool dispatch_delayed(std::chrono::milliseconds delay, dispatcher_action action) = 0;

        // Create a timer bound to this dispatcher.
        virtual std::unique_ptr<i_dispatcher_timer> create_timer() = 0;

    protected:
        i_dispatcher() = default;
        i_dispatcher(const i_dispatcher &) = default;
        i_dispatcher(i_dispatcher &&) = default;
        i_dispatcher &operator=(const i_dispatcher &) = default;
        i_dispatcher &operator=(i_dispatcher &&) = default;
    };
} // namespace maui::core
