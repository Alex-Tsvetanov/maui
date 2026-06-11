// maui::animations::ticker — see include/maui/animations/ticker.hpp. Ported from
// src/Core/src/Animations/Ticker.cs (System.Timers.Timer -> the injected dispatcher's timer).
#include "maui/animations/ticker.hpp"

#include <chrono>
#include <utility>

#include "maui/core/dispatcher.hpp"

namespace maui::animations
{
    ticker::ticker(maui::core::i_dispatcher& dispatcher) : dispatcher_(&dispatcher)
    {
    }

    bool ticker::is_running() const
    {
        return timer_ != nullptr && timer_->is_running();
    }

    bool ticker::system_enabled() const
    {
        return system_enabled_;
    }

    int ticker::max_fps() const
    {
        return max_fps_;
    }

    void ticker::set_max_fps(int value)
    {
        max_fps_ = value;
    }

    void ticker::set_fire(fire_callback fire)
    {
        fire_ = std::move(fire);
    }

    void ticker::start()
    {
        if (is_running())
        {
            return;
        }
        if (!timer_)
        {
            // No token bookkeeping: the subscription lives exactly as long as the owned timer.
            timer_ = dispatcher_->create_timer();
            timer_->tick().connect([this] { invoke_fire(); });
        }
        timer_->set_interval(std::chrono::milliseconds(1000 / max_fps_));
        timer_->set_is_repeating(true);
        timer_->start();
    }

    void ticker::stop()
    {
        if (!timer_)
        {
            return;
        }
        // Stop but do NOT destroy (C# disposes and re-creates): stop() is routinely invoked from
        // INSIDE the timer's own tick — the manager ends the ticker when the last animation
        // finishes — and destroying the timer there would free the state its in-flight tick
        // continuation still reads (ASan-verified). The stopped timer is re-armed by the next
        // start(), which refreshes the interval against max_fps. Behavior through i_ticker is
        // identical to C#'s dispose-and-recreate.
        timer_->stop();
    }

    void ticker::set_system_enabled(bool value)
    {
        if (system_enabled_ == value)
        {
            return;
        }
        system_enabled_ = value;
        on_system_enabled_changed();
    }

    void ticker::on_system_enabled_changed()
    {
        if (is_running() && !system_enabled_)
        {
            // Animations were disabled mid-run: fire once more so the manager can force-finish the
            // animations in progress (the only communication channel it has — the C# comment).
            invoke_fire();
        }
    }

    void ticker::invoke_fire()
    {
        if (fire_)
        {
            fire_();
        }
    }
} // namespace maui::animations
