// maui::controls::detail::tweener — see tweener.hpp. Ported from src/Controls/src/Core/Tweener.cs.
#include "tweener.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>

#include "maui/animations/i_animation_manager.hpp"
#include "maui/animations/i_ticker.hpp"

namespace maui::controls::detail
{
    tweener_animation::tweener_animation(raw_step_fn step) : raw_step_(std::move(step))
    {
    }

    void tweener_animation::on_tick(double milliseconds_since_last_update)
    {
        const bool running = raw_step_(static_cast<std::int64_t>(milliseconds_since_last_update));
        has_finished_ = !running;
        if (has_finished_ && finished_)
        {
            finished_();
        }
    }

    void tweener_animation::force_finish()
    {
        if (has_finished_)
        {
            return;
        }
        has_finished_ = true;
        // The in-band signal that the step should jump to the end of the animation.
        (void)raw_step_(tweener::finish_signal);
    }

    tweener::tweener(std::uint32_t length, std::shared_ptr<maui::animations::i_animation_manager> manager)
        : manager_(std::move(manager)), length_(length)
    {
    }

    tweener::tweener(std::uint32_t length, std::uint32_t rate,
                     std::shared_ptr<maui::animations::i_animation_manager> manager)
        : manager_(std::move(manager)), length_(length), rate_(rate)
    {
    }

    tweener::~tweener()
    {
        // The C# finalizer: a still-attached animation is silently removed from the manager (no
        // events) — deterministic teardown when the owning element dies mid-animation.
        if (animation_)
        {
            manager_->remove(*animation_);
            animation_ = nullptr;
        }
    }

    void tweener::start()
    {
        pause();

        last_milliseconds_ = 0;
        frames_ = 0;

        if (!manager_->ticker().system_enabled())
        {
            // Animations are disabled system-wide: move straight to the finished state.
            finish_immediately();
            return;
        }

        animation_ = std::make_shared<tweener_animation>([this](std::int64_t ms) { return step(ms); });
        animation_->commit(manager_);
        if (!manager_->ticker().is_running())
        {
            manager_->ticker().start();
        }
    }

    void tweener::stop()
    {
        pause();
        const std::weak_ptr<int> alive = life_;
        finished.raise();
        if (alive.expired())
        {
            return; // a finished handler destroyed this tweener (its owning registry entry)
        }
        value_ = 0.0;
    }

    void tweener::pause()
    {
        if (animation_)
        {
            manager_->remove(*animation_);
            animation_ = nullptr;
        }
    }

    bool tweener::step(std::int64_t milliseconds)
    {
        if (milliseconds == finish_signal)
        {
            // The tweener is being forced into the finished state (the ticker was disabled).
            finish_immediately();
            return false;
        }

        const std::int64_t ms = milliseconds + last_milliseconds_;
        value_ = std::min(1.0, static_cast<double>(ms) / static_cast<double>(length_));
        last_milliseconds_ = ms;

        // LIVENESS (PROFILE §8; the manual_dispatcher_timer pattern): the value_updated/finished
        // handlers may tear this tweener down (the named-animation registry erases the entry that
        // owns it when a run completes or is replaced). In C# the GC keeps the about-to-be-orphaned
        // Tweener reachable through the in-flight call; here a weak guard bails out of the member
        // writes after any raise that freed us.
        const std::weak_ptr<int> alive = life_;

        const std::int64_t wanted_frames = (last_milliseconds_ / rate_) + 1;
        if (wanted_frames > frames_ || value_ >= 1.0)
        {
            value_updated.raise();
            if (alive.expired())
            {
                return false;
            }
        }
        frames_ = wanted_frames;

        if (value_ >= 1.0)
        {
            if (loop_)
            {
                last_milliseconds_ = 0;
                value_ = 0.0;
                return true;
            }

            finished.raise();
            if (alive.expired())
            {
                return false;
            }
            value_ = 0.0;
            animation_ = nullptr; // C#'s _animationManagerKey = 0 (manager drops the finished animation)
            return false;
        }

        return true;
    }

    void tweener::finish_immediately()
    {
        value_ = 1.0;
        const std::weak_ptr<int> alive = life_; // see step() — the raises may destroy this tweener
        value_updated.raise();
        if (alive.expired())
        {
            return;
        }
        finished.raise();
        if (alive.expired())
        {
            return;
        }
        value_ = 0.0;
        animation_ = nullptr;
    }
} // namespace maui::controls::detail
