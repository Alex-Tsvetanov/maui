#pragma once
// maui::core::gcd_dispatcher  <=  Microsoft.Maui.Dispatching.Dispatcher (Dispatcher.iOS.cs)
//
// The Apple-backend i_dispatcher: the platform twin of the headless manual_dispatcher, posting work to
// the GCD MAIN queue (the UI thread on both AppKit and UIKit). One Obj-C++ definition is SHARED by the
// apple and ios backends (src/platform/apple_shared/gcd_dispatcher.mm) — GCD is identical on both, the
// analog of a single Foo.MaciOS.cs partial — so this class only exists in those builds (a headless
// consumer gets a link error, exactly like referencing any other platform partial off-backend).
//
// Mirrors Dispatcher.iOS.cs: dispatch() = DispatchQueue.MainQueue.DispatchAsync, dispatch_delayed() =
// DispatchAfter(Now + delay), create_timer() mints the dispatcher timer (timers fire on the main
// queue). is_dispatch_required() is C#'s "not on the dispatcher's queue" — the port's dispatcher is
// always the main queue, so it reduces to "not on the main thread" (pthread_main_np), the supported
// stand-in for the deprecated DispatchQueue.CurrentQueue label compare.
//
// The class is stateless (the main queue is a process global); the timer state lives in the
// per-instance timers create_timer() returns. Threading doctrine (PROFILE §8): create/destroy timers on
// the main thread — the timer's teardown cancels its dispatch source, which must not race a delivery.

#include <chrono>
#include <memory>

#include "maui/core/dispatcher.hpp"

namespace maui::core
{
    class gcd_dispatcher final : public i_dispatcher
    {
    public:
        // True when the caller is off the main thread (work must be dispatch()'d to reach the UI).
        [[nodiscard]] bool is_dispatch_required() const override;

        // Post onto the main queue (DispatchQueue.MainQueue.DispatchAsync). Always schedules.
        bool dispatch(dispatcher_action action) override;

        // Post onto the main queue after at least `delay` (DispatchAfter). Always schedules.
        bool dispatch_delayed(std::chrono::milliseconds delay, dispatcher_action action) override;

        // Mint a main-queue timer (DispatcherTimer over a dispatch source).
        [[nodiscard]] std::unique_ptr<i_dispatcher_timer> create_timer() override;
    };
} // namespace maui::core
