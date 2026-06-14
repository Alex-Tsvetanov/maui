#pragma once
// maui::controls::shell_navigating_deferral  <=  Microsoft.Maui.Controls.ShellNavigatingDeferral
//
// The token a Navigating subscriber takes (shell_navigating_event_args::get_deferral) to hold a
// pending navigation open; complete() releases it (idempotent — the C# Interlocked.Exchange
// collapses to a plain swap on the single UI thread). When the LAST outstanding token completes,
// the navigation continues (or stays cancelled). Ported from ShellNavigatingDeferral.cs.

#include <functional>
#include <utility>

namespace maui::controls
{
    class shell_navigating_event_args; // mints deferrals

    class shell_navigating_deferral
    {
    public:
        // Signal that the deferred navigation may proceed. Only the first call does anything.
        void complete()
        {
            if (completed_)
            {
                return;
            }
            completed_ = true;
            if (completed_callback_)
            {
                std::function<void()> callback = std::exchange(completed_callback_, nullptr);
                callback();
            }
        }

        [[nodiscard]] bool is_completed() const
        {
            return completed_;
        }

    private:
        friend class shell_navigating_event_args;
        explicit shell_navigating_deferral(std::function<void()> completed) : completed_callback_(std::move(completed))
        {
        }

        std::function<void()> completed_callback_;
        bool completed_ = false;
    };
} // namespace maui::controls
