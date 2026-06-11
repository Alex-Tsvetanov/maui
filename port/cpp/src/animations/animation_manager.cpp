// maui::animations::animation_manager — see include/maui/animations/animation_manager.hpp. Ported
// from src/Core/src/Animations/AnimationManager.cs.
#include "maui/animations/animation_manager.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "maui/animations/animation.hpp"
#include "maui/animations/i_ticker.hpp"

namespace maui::animations
{
    namespace
    {
        // C# static GetCurrentTick (Environment.TickCount) — the non-virtual default the constructor
        // may safely use (the virtual seam re-syncs in start()).
        std::int64_t steady_tick_milliseconds()
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now().time_since_epoch())
                .count();
        }
    } // namespace

    animation_manager::animation_manager(std::shared_ptr<i_ticker> ticker)
        : ticker_(std::move(ticker)), last_update_(steady_tick_milliseconds())
    {
        ticker_->set_fire([this] { on_fire(); });
    }

    animation_manager::~animation_manager()
    {
        ticker_->set_fire({});
        ticker_->stop();
    }

    i_ticker& animation_manager::ticker() const
    {
        return *ticker_;
    }

    double animation_manager::speed_modifier() const
    {
        return speed_modifier_;
    }

    void animation_manager::set_speed_modifier(double value)
    {
        speed_modifier_ = value;
    }

    bool animation_manager::auto_start_ticker() const
    {
        return auto_start_ticker_;
    }

    void animation_manager::set_auto_start_ticker(bool value)
    {
        auto_start_ticker_ = value;
    }

    void animation_manager::add(std::shared_ptr<animation> animation_to_add)
    {
        // If animations are disabled, don't do anything.
        if (!ticker_->system_enabled())
        {
            return;
        }
        if (std::ranges::find(animations_, animation_to_add) == animations_.end())
        {
            animations_.push_back(std::move(animation_to_add));
        }
        if (!ticker_->is_running() && auto_start_ticker_)
        {
            start();
        }
    }

    void animation_manager::remove(animation& animation_to_remove)
    {
        std::erase_if(animations_, [&animation_to_remove](const std::shared_ptr<animation>& candidate) {
            return candidate.get() == &animation_to_remove;
        });
        if (animations_.empty())
        {
            end();
        }
    }

    double animation_manager::adjust_speed(double elapsed_milliseconds) const
    {
        return elapsed_milliseconds * speed_modifier_;
    }

    std::int64_t animation_manager::current_tick_milliseconds() const
    {
        return steady_tick_milliseconds();
    }

    void animation_manager::start()
    {
        last_update_ = current_tick_milliseconds();
        ticker_->start();
    }

    void animation_manager::end()
    {
        ticker_->stop();
    }

    void animation_manager::on_fire()
    {
        if (!ticker_->system_enabled())
        {
            // The ticker detected that animations are no longer enabled and fired one last time:
            // force every running animation to its finished state and stop (the C# "hack" path).
            force_finish_animations();
            return;
        }

        const std::int64_t now = current_tick_milliseconds();
        const auto milliseconds = static_cast<double>(now - last_update_);
        last_update_ = now;

        // Snapshot: an animation's step may add/remove animations mid-tick.
        const std::vector<std::shared_ptr<animation>> snapshot(animations_);
        for (const auto& current : snapshot)
        {
            if (current->has_finished())
            {
                try_remove(*current);
                current->remove_from_parent();
                continue;
            }
            current->tick(adjust_speed(milliseconds));
            if (current->has_finished())
            {
                try_remove(*current);
                current->remove_from_parent();
            }
        }

        if (animations_.empty())
        {
            end();
        }
    }

    void animation_manager::try_remove(const animation& animation_to_remove)
    {
        std::erase_if(animations_, [&animation_to_remove](const std::shared_ptr<animation>& candidate) {
            return candidate.get() == &animation_to_remove;
        });
    }

    void animation_manager::force_finish_animations()
    {
        const std::vector<std::shared_ptr<animation>> snapshot(animations_);
        for (const auto& current : snapshot)
        {
            current->force_finish();
            try_remove(*current);
            current->remove_from_parent();
        }
        end();
    }
} // namespace maui::animations
