#pragma once
// maui::controls::shell_navigating_event_args  <=  Microsoft.Maui.Controls.ShellNavigatingEventArgs
//
// The Navigating payload: current/target states, the source, cancellation (only when can_cancel),
// and the DEFERRAL state machine — get_deferral() hands out tokens; when the last token completes,
// the registered continuation runs (C#'s _deferralFinishedTask + the DeferredTask the navigation
// manager awaits, collapsed into one synchronous continuation since the port's navigation pipeline
// is synchronous-with-suspension rather than Task-based). Ported from ShellNavigatingEventArgs.cs.
//
// Lifetime: args are heap-held (shared_ptr) by the navigation manager — each deferral token pins
// them so a token completed long after the Navigating event still finds live state.

#include <functional>
#include <memory>
#include <optional>

#include "maui/controls/shell/shell_navigating_deferral.hpp"
#include "maui/controls/shell/shell_navigation_source.hpp"
#include "maui/controls/shell/shell_navigation_state.hpp"

namespace maui::controls
{
    class shell_navigating_event_args : public std::enable_shared_from_this<shell_navigating_event_args>
    {
    public:
        // `current` is nullopt before the shell has any state (C# null Current).
        shell_navigating_event_args(std::optional<shell_navigation_state> current, shell_navigation_state target,
                                    shell_navigation_source source, bool can_cancel)
            : current_(std::move(current)), target_(std::move(target)), source_(source), can_cancel_(can_cancel)
        {
        }

        [[nodiscard]] const std::optional<shell_navigation_state>& current() const
        {
            return current_;
        }
        [[nodiscard]] const shell_navigation_state& target() const
        {
            return target_;
        }
        [[nodiscard]] shell_navigation_source source() const
        {
            return source_;
        }
        [[nodiscard]] bool can_cancel() const
        {
            return can_cancel_;
        }

        // Cancel the navigation; false (and no effect) when it cannot be cancelled.
        bool cancel()
        {
            if (!can_cancel_)
            {
                return false;
            }
            cancelled_ = true;
            return true;
        }
        [[nodiscard]] bool cancelled() const
        {
            return cancelled_;
        }

        // Take a deferral token. Returns nullptr when the navigation cannot be cancelled; throws
        // std::runtime_error once the deferral has already completed (C# InvalidOperationException).
        [[nodiscard]] std::shared_ptr<shell_navigating_deferral> get_deferral();

        // ---- the internals the navigation manager drives (C# internal members) ----
        [[nodiscard]] int deferral_count() const
        {
            return deferral_count_;
        }
        [[nodiscard]] bool navigation_delayed_or_cancelled() const
        {
            return cancelled_ || deferral_count_ > 0;
        }
        // C# DeferredTask != null — a deferral was taken at some point.
        [[nodiscard]] bool deferral_taken() const
        {
            return deferral_taken_;
        }
        [[nodiscard]] bool deferral_completed() const
        {
            return deferral_completed_;
        }
        [[nodiscard]] bool animate() const
        {
            return animate_;
        }
        void set_animate(bool value)
        {
            animate_ = value;
        }
        // C# DeferredEventArgs — true once a deferral was taken; a re-dispatched (deferred) args
        // instance skips re-raising Navigating.
        [[nodiscard]] bool deferred_event_args() const
        {
            return deferred_event_args_;
        }
        // The continuation run when the last deferral completes (C# RegisterDeferralCompletedCallBack;
        // the caller checks cancelled() inside, exactly like the C# callbacks do).
        void register_deferral_completed_callback(std::function<void()> continuation)
        {
            deferral_finished_ = std::move(continuation);
        }

    private:
        void decrement_deferral();

        std::optional<shell_navigation_state> current_;
        shell_navigation_state target_;
        shell_navigation_source source_;
        bool can_cancel_;
        bool cancelled_ = false;
        bool animate_ = true;
        bool deferred_event_args_ = false;
        bool deferral_taken_ = false;
        bool deferral_completed_ = false;
        int deferral_count_ = 0;
        std::function<void()> deferral_finished_;
    };
} // namespace maui::controls
