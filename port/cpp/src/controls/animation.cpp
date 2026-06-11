// maui::controls::animation — see include/maui/controls/animation.hpp. Ported from
// src/Controls/src/Core/Animation.cs.
#include "maui/controls/animation.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "maui/animations/animation.hpp"
#include "maui/animations/easing.hpp"
#include "maui/animations/i_ticker.hpp"
#include "maui/controls/animation_extensions.hpp"

namespace maui::controls
{
    animation::animation()
    {
        set_easing_function(maui::animations::easing::linear());
    }

    animation::animation(step_fn callback, double start, double end,
                         std::optional<maui::animations::easing> easing_function, finished_fn finished)
        : maui::animations::animation({}, start, end - start,
                                      easing_function ? std::move(easing_function)
                                                      : std::optional{maui::animations::easing::linear()},
                                      std::move(finished))
    {
        // C#: Step = f => callback(transform(f)) with transform = Interpolate(start, end).
        set_step([callback = std::move(callback), transform = interpolate(start, end)](double f) mutable {
            callback(transform(f));
        });
    }

    void animation::add(double begin_at, double finish_at, std::shared_ptr<animation> child)
    {
        if (begin_at < 0 || begin_at > 1)
        {
            throw std::out_of_range("animation::add: begin_at must be within [0, 1]");
        }
        if (finish_at < 0 || finish_at > 1)
        {
            throw std::out_of_range("animation::add: finish_at must be within [0, 1]");
        }
        if (finish_at <= begin_at)
        {
            throw std::invalid_argument("animation::add: finish_at must be greater than begin_at");
        }
        child->set_start_delay(begin_at);
        child->set_duration(finish_at - begin_at);
        children_.push_back(std::move(child));
    }

    animation& animation::insert(double begin_at, double finish_at, std::shared_ptr<animation> child)
    {
        add(begin_at, finish_at, std::move(child));
        return *this;
    }

    animation& animation::with_concurrent(std::shared_ptr<animation> child, double begin_at, double finish_at)
    {
        child->set_start_delay(begin_at);
        child->set_duration(finish_at - begin_at);
        children_.push_back(std::move(child));
        return *this;
    }

    animation& animation::with_concurrent(step_fn callback, double start, double end,
                                          std::optional<maui::animations::easing> easing_function, double begin_at,
                                          double finish_at)
    {
        auto child = std::make_shared<animation>(std::move(callback), start, end, std::move(easing_function));
        child->set_start_delay(begin_at);
        child->set_duration(finish_at - begin_at);
        children_.push_back(std::move(child));
        return *this;
    }

    animation::step_fn animation::get_callback()
    {
        // The closure shares ownership of this animation: a committed tweener keeps it alive.
        auto self = std::static_pointer_cast<animation>(shared_from_this());
        return [self = std::move(self)](double f) {
            if (self->step_)
            {
                self->step_(self->easing_function_.ease(f));
            }
            for (const auto& base_child : self->children_)
            {
                // Children added through the controls API are controls animations; anything else
                // cannot carry the finished-triggered latch, so it is skipped (the C# type system
                // makes this unrepresentable).
                const auto child = std::dynamic_pointer_cast<animation>(base_child);
                if (!child || child->finished_triggered_)
                {
                    continue;
                }
                const double val = std::max(0.0, std::min(1.0, (f - child->start_delay()) / child->duration()));
                if (val <= 0.0) // not ready to process yet
                {
                    continue;
                }
                child->get_callback()(val);
                if (val >= 1.0)
                {
                    child->finished_triggered_ = true;
                    if (child->finished_)
                    {
                        child->finished_();
                    }
                }
            }
        };
    }

    void animation::commit(element& owner, std::string name, std::uint32_t rate, std::uint32_t length,
                           std::optional<maui::animations::easing> easing_function, finished_with_result_fn finished,
                           repeat_fn repeat)
    {
        animate(owner, std::move(name), std::static_pointer_cast<animation>(shared_from_this()), rate, length,
                std::move(easing_function), std::move(finished), std::move(repeat));
    }

    bool animation::is_enabled() const
    {
        const auto manager = animation_manager_.lock();
        return manager && manager->ticker().system_enabled();
    }

    void animation::reset()
    {
        maui::animations::animation::reset();
        finished_triggered_ = false;
    }
} // namespace maui::controls
