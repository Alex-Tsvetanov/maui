// maui::animations::animation — see include/maui/animations/animation.hpp. Ported from
// src/Core/src/Animations/Animation.cs.
#include "maui/animations/animation.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "maui/animations/easing.hpp"
#include "maui/animations/i_animation_manager.hpp"
#include "maui/animations/i_animator.hpp"

namespace maui::animations
{
    animation::animation() = default;

    animation::animation(step_fn callback, double start, double duration, std::optional<easing> easing_function,
                         finished_fn finished)
        : step_(std::move(callback)), finished_(std::move(finished)), start_delay_(start), duration_(duration)
    {
        // The member initializer already holds Easing.Default (CubicInOut); only override when given.
        if (easing_function)
        {
            easing_function_ = std::move(*easing_function);
        }
    }

    animation::animation(std::vector<std::shared_ptr<animation>> animations) : children_(std::move(animations))
    {
    }

    const std::string& animation::name() const
    {
        return name_;
    }

    void animation::set_name(std::string value)
    {
        name_ = std::move(value);
    }

    double animation::start_delay() const
    {
        return start_delay_;
    }

    void animation::set_start_delay(double value)
    {
        start_delay_ = value;
    }

    double animation::duration() const
    {
        return duration_;
    }

    void animation::set_duration(double value)
    {
        duration_ = value;
    }

    double animation::current_time() const
    {
        return current_time_;
    }

    double animation::progress() const
    {
        return progress_;
    }

    const easing& animation::easing_function() const
    {
        return easing_function_;
    }

    void animation::set_easing_function(easing value)
    {
        easing_function_ = std::move(value);
    }

    bool animation::has_finished() const
    {
        return has_finished_;
    }

    bool animation::repeats() const
    {
        return repeats_;
    }

    void animation::set_repeats(bool value)
    {
        repeats_ = value;
    }

    bool animation::is_paused() const
    {
        return paused_;
    }

    bool animation::is_disposed() const
    {
        return disposed_;
    }

    void animation::set_step(step_fn value)
    {
        step_ = std::move(value);
    }

    void animation::set_finished(finished_fn value)
    {
        finished_ = std::move(value);
    }

    std::shared_ptr<i_animation_manager> animation::animation_manager() const
    {
        return animation_manager_.lock();
    }

    void animation::set_parent(std::weak_ptr<i_animator> parent)
    {
        parent_ = std::move(parent);
    }

    const std::vector<std::shared_ptr<animation>>& animation::children() const
    {
        return children_;
    }

    void animation::add(double begin_at, double duration, std::shared_ptr<animation> child)
    {
        if (begin_at < 0 || begin_at > 1)
        {
            throw std::out_of_range("animation::add: begin_at must be within [0, 1]");
        }
        if (duration < 0 || duration > 1)
        {
            throw std::out_of_range("animation::add: duration must be within [0, 1]");
        }
        if (duration <= begin_at)
        {
            throw std::invalid_argument("animation::add: duration must be greater than begin_at");
        }
        child->start_delay_ = begin_at;
        child->duration_ = duration;
        children_.push_back(std::move(child));
    }

    void animation::tick(double milliseconds)
    {
        if (paused_)
        {
            return;
        }
        if (ticking_)
        {
            // The animation is lagging behind (a re-entrant tick): catch up on the next one.
            skipped_milliseconds_ += milliseconds;
            return;
        }
        ticking_ = true;
        on_tick(skipped_milliseconds_ + milliseconds);
        skipped_milliseconds_ = 0;
        ticking_ = false;
    }

    void animation::on_tick(double milliseconds_since_last_update)
    {
        if (has_finished_)
        {
            return;
        }

        const double seconds_since_last_update = milliseconds_since_last_update / 1000.0;
        current_time_ += seconds_since_last_update;

        if (!children_.empty())
        {
            bool finished = true;
            for (const auto& child : children_)
            {
                child->on_tick(milliseconds_since_last_update);
                if (!child->has_finished_)
                {
                    finished = false;
                }
            }
            has_finished_ = finished;
        }
        else
        {
            if (current_time_ < start_delay_)
            {
                return;
            }
            const double start = current_time_ - start_delay_;
            const double percent = std::min(start / duration_, 1.0);
            update(percent);
        }

        if (has_finished_)
        {
            if (finished_)
            {
                finished_();
            }
            if (repeats_)
            {
                reset();
            }
        }
    }

    void animation::update(double percent)
    {
        try
        {
            progress_ = easing_function_.ease(percent);
            if (step_)
            {
                step_(progress_);
            }
            has_finished_ = percent == 1;
        }
        catch (...)
        {
            // C# swallows a throwing Step and finishes the animation.
            has_finished_ = true;
        }
    }

    void animation::commit(const std::shared_ptr<i_animation_manager>& manager)
    {
        animation_manager_ = manager;
        manager->add(shared_from_this());
    }

    std::shared_ptr<animation> animation::create_auto_reversing()
    {
        auto reversed = create_reverse();
        auto parent =
            std::make_shared<animation>(std::vector<std::shared_ptr<animation>>{shared_from_this(), reversed});
        parent->duration_ = reversed->start_delay_ + reversed->duration_;
        parent->repeats_ = repeats_;
        repeats_ = false;
        return parent;
    }

    std::shared_ptr<animation> animation::create_reverse()
    {
        // C# CreateReverse: the SAME child instances, in reverse order (shared ownership).
        std::vector<std::shared_ptr<animation>> reversed_children{children_.rbegin(), children_.rend()};
        auto reversed = std::make_shared<animation>(std::move(reversed_children));
        reversed->easing_function_ = easing_function_;
        reversed->duration_ = duration_;
        reversed->start_delay_ = start_delay_ + duration_;
        return reversed;
    }

    void animation::reset()
    {
        current_time_ = 0;
        has_finished_ = false;
        for (const auto& child : children_)
        {
            child->reset();
        }
    }

    void animation::pause()
    {
        paused_ = true;
        if (const auto manager = animation_manager_.lock())
        {
            manager->remove(*this);
        }
    }

    void animation::resume()
    {
        paused_ = false;
        if (const auto manager = animation_manager_.lock())
        {
            manager->add(shared_from_this());
        }
    }

    void animation::remove_from_parent()
    {
        if (const auto parent = parent_.lock())
        {
            parent->remove_animation(*this);
        }
    }

    void animation::force_finish()
    {
        if (progress_ < 1.0)
        {
            update(1.0);
        }
    }

    void animation::dispose()
    {
        if (disposed_)
        {
            return;
        }
        for (const auto& child : children_)
        {
            child->dispose();
        }
        children_.clear();
        disposed_ = true;
        if (const auto manager = animation_manager_.lock())
        {
            manager->remove(*this);
        }
        finished_ = nullptr;
        step_ = nullptr;
    }
} // namespace maui::animations
