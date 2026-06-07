#pragma once
// maui::core::manual_dispatcher — the headless/test i_dispatcher, driven by a virtual clock.
//
// dispatch()/dispatch_delayed() enqueue work against a manually advanced clock instead of a real
// run loop: run_pending() runs everything due at the current virtual time, and advance(delta) moves
// the clock forward, running work (and timer ticks) as it comes due. No wall-clock, no threads — so
// dispatcher-driven behavior (delayed work, timers) is deterministic and testable without flakiness.
// is_dispatch_required() reports whether the caller is off the thread that constructed the dispatcher.

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

#include "maui/core/dispatcher.hpp"

namespace maui::core
{
    class manual_dispatcher final : public i_dispatcher
    {
    public:
        manual_dispatcher();

        [[nodiscard]] bool is_dispatch_required() const override;
        bool dispatch(dispatcher_action action) override;
        bool dispatch_delayed(std::chrono::milliseconds delay, dispatcher_action action) override;
        std::unique_ptr<i_dispatcher_timer> create_timer() override;

        // ---- headless test pump ----
        // Run every action due at or before the current virtual time, in (due, enqueue-order); work
        // queued by a running action is also run if already due. Returns how many actions ran.
        std::size_t run_pending();
        // Advance the virtual clock by `delta`, running work (and timer ticks) at each due time it
        // passes (so a repeating timer fires once per interval crossed).
        void advance(std::chrono::milliseconds delta);
        [[nodiscard]] std::chrono::milliseconds now() const;
        [[nodiscard]] std::size_t pending_count() const;

    private:
        struct entry
        {
            std::chrono::milliseconds due;
            std::uint64_t seq;
            dispatcher_action action;
        };
        std::size_t run_due(); // run all entries with due <= now_

        std::vector<entry> queue_;
        std::chrono::milliseconds now_{std::chrono::milliseconds::zero()};
        std::uint64_t next_seq_ = 0;
        std::thread::id owner_thread_;
    };
} // namespace maui::core
