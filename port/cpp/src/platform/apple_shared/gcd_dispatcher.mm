// maui::core::gcd_dispatcher — the GCD main-queue dispatcher + its dispatch-source timer, SHARED by the
// apple (AppKit) and ios (UIKit) backends: GCD is identical on both, so this one Obj-C++ TU sits in both
// backends' source lists (the analog of a single Dispatcher.MaciOS.cs partial; gcd_dispatcher.hpp).
//
// Ported from src/Core/src/Dispatching/Dispatcher.iOS.cs:
//   - DispatchImplementation        -> dispatch_async(dispatch_get_main_queue(), …)
//   - DispatchDelayedImplementation -> dispatch_after(DispatchTime.Now + delay, main queue, …)
//   - IsDispatchRequiredImplementation compares queue labels; the port's dispatcher is ALWAYS the main
//     queue, so it reduces to "not on the main thread" via pthread_main_np (the supported stand-in for
//     the deprecated DispatchQueue.CurrentQueue).
//   - DispatcherTimer re-arms a DispatchBlock per tick; the port keeps one repeating DISPATCH SOURCE
//     instead (no per-tick re-arm churn) and mirrors C#'s OnTimerTick at delivery: bail when stopped,
//     raise Tick, then Stop() when !IsRepeating — so the observable Start/Stop/Tick/IsRunning behavior
//     matches, including flipping is_repeating mid-run.
//
// A dispatcher_action (move_only_function) cannot be captured by an ObjC block directly (blocks copy
// their captures; the action is move-only), so each action is boxed in a shared_ptr first — the one heap
// hop the C# DispatchAsync(() => action()) closure also pays. Compiled with ARC: the dispatch_source_t
// member is ARC-managed (OS_OBJECT_USE_OBJC), released when the timer cancels/destructs.

#include "maui/core/gcd_dispatcher.hpp"

#import <Foundation/Foundation.h>

#include <dispatch/dispatch.h>
#include <pthread.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <utility>

#include "maui/core/event.hpp"

namespace
{
    // Box a move-only action so an ObjC block (which copies its captures) can carry it.
    dispatch_block_t box_action(maui::core::dispatcher_action action)
    {
        auto boxed = std::make_shared<maui::core::dispatcher_action>(std::move(action));
        return ^{
          (*boxed)();
        };
    }

    std::uint64_t to_nanoseconds(std::chrono::milliseconds value)
    {
        const auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(value).count();
        return nanoseconds > 0 ? static_cast<std::uint64_t>(nanoseconds) : 0;
    }

    // The main-queue dispatcher timer (<= Microsoft.Maui.Dispatching.DispatcherTimer, Dispatcher.iOS.cs)
    // over a repeating dispatch source. Main-thread confined (gcd_dispatcher.hpp): the source delivers on
    // the main queue, and stop()/teardown cancel it there, so a delivery can never race the teardown.
    class gcd_dispatcher_timer final : public maui::core::i_dispatcher_timer
    {
    public:
        gcd_dispatcher_timer() = default;
        gcd_dispatcher_timer(const gcd_dispatcher_timer&) = delete;
        gcd_dispatcher_timer(gcd_dispatcher_timer&&) = delete;
        gcd_dispatcher_timer& operator=(const gcd_dispatcher_timer&) = delete;
        gcd_dispatcher_timer& operator=(gcd_dispatcher_timer&&) = delete;
        ~gcd_dispatcher_timer() override
        {
            stop();
        }

        [[nodiscard]] std::chrono::milliseconds interval() const override
        {
            return interval_;
        }
        // C# applies a changed Interval at the next re-arm; one repeating source has no per-tick re-arm,
        // so a running timer is re-armed NOW (the current period restarts at the new length) — the
        // closest dispatch-source equivalent, documented deviation.
        void set_interval(std::chrono::milliseconds value) override
        {
            interval_ = value;
            if (is_running_)
            {
                arm();
            }
        }

        [[nodiscard]] bool is_repeating() const override
        {
            return is_repeating_;
        }
        void set_is_repeating(bool value) override
        {
            is_repeating_ = value;
        }

        [[nodiscard]] bool is_running() const override
        {
            return is_running_;
        }

        // DispatcherTimer.Start: no-op when already running; otherwise create + arm the source.
        void start() override
        {
            if (is_running_)
            {
                return;
            }
            is_running_ = true;
            source_ = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
            dispatch_source_set_event_handler(source_, ^{
              on_tick();
            });
            arm();
            dispatch_resume(source_);
        }

        // DispatcherTimer.Stop: no-op when not running; otherwise cancel + drop the source (after
        // dispatch_source_cancel returns, the event handler is never invoked again).
        void stop() override
        {
            if (!is_running_)
            {
                return;
            }
            is_running_ = false;
            dispatch_source_cancel(source_);
            source_ = nil;
        }

        [[nodiscard]] maui::core::event<>& tick() override
        {
            return tick_;
        }

    private:
        // Re-target the source at interval_ from now, repeating (one-shot is decided at TICK time, like
        // C#'s OnTimerTick). GCD requires a non-zero repeat interval; clamp to 1ns (C# with a zero
        // Interval also re-fires immediately).
        void arm()
        {
            const std::uint64_t nanoseconds = std::max<std::uint64_t>(to_nanoseconds(interval_), 1);
            dispatch_source_set_timer(source_, dispatch_time(DISPATCH_TIME_NOW, static_cast<std::int64_t>(nanoseconds)),
                                      nanoseconds, 0);
        }

        // C# DispatcherTimer.OnTimerTick, verbatim: bail when stopped, raise Tick, then either let the
        // source repeat (IsRepeating) or Stop().
        void on_tick()
        {
            if (!is_running_)
            {
                return;
            }
            tick_.raise();
            if (!is_repeating_)
            {
                stop();
            }
        }

        maui::core::event<> tick_;
        dispatch_source_t source_ = nil;
        std::chrono::milliseconds interval_{0};
        bool is_repeating_ = true; // C# IsRepeating default
        bool is_running_ = false;
    };
} // namespace

namespace maui::core
{
    bool gcd_dispatcher::is_dispatch_required() const
    {
        return pthread_main_np() == 0;
    }

    bool gcd_dispatcher::dispatch(dispatcher_action action)
    {
        dispatch_async(dispatch_get_main_queue(), box_action(std::move(action)));
        return true; // DispatchImplementation always schedules
    }

    bool gcd_dispatcher::dispatch_delayed(std::chrono::milliseconds delay, dispatcher_action action)
    {
        const auto when = dispatch_time(DISPATCH_TIME_NOW, static_cast<std::int64_t>(to_nanoseconds(delay)));
        dispatch_after(when, dispatch_get_main_queue(), box_action(std::move(action)));
        return true; // DispatchDelayedImplementation always schedules
    }

    std::unique_ptr<i_dispatcher_timer> gcd_dispatcher::create_timer()
    {
        return std::make_unique<gcd_dispatcher_timer>();
    }
} // namespace maui::core
