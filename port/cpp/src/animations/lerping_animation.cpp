// maui::animations::lerping_animation — see include/maui/animations/lerping_animation.hpp. Ported
// from src/Core/src/Animations/LerpingAnimation.cs.
#include "maui/animations/lerping_animation.hpp"

#include <any>
#include <memory>
#include <optional>
#include <typeindex>
#include <utility>
#include <vector>

#include "maui/animations/animation.hpp"
#include "maui/animations/easing.hpp"
#include "maui/animations/lerp.hpp"

namespace maui::animations
{
    lerping_animation::lerping_animation() = default;

    lerping_animation::lerping_animation(step_fn callback, double start, double end,
                                         std::optional<easing> easing_function, finished_fn finished)
        : animation(std::move(callback), start, end, std::move(easing_function), std::move(finished))
    {
    }

    lerping_animation::lerping_animation(std::vector<std::shared_ptr<animation>> animations)
        : animation(std::move(animations))
    {
    }

    void lerping_animation::set_value_changed(value_changed_fn value)
    {
        value_changed_ = std::move(value);
    }

    const std::any& lerping_animation::start_value() const
    {
        return start_value_;
    }

    void lerping_animation::set_start_value(std::any value)
    {
        start_value_ = std::move(value);
    }

    const std::any& lerping_animation::end_value() const
    {
        return end_value_;
    }

    void lerping_animation::set_end_value(std::any value)
    {
        end_value_ = std::move(value);
    }

    const std::any& lerping_animation::current_value() const
    {
        return current_value_;
    }

    const lerp* lerping_animation::current_lerp() const
    {
        if (lerp_)
        {
            return &*lerp_;
        }
        // C#: var type = StartValue?.GetType() ?? EndValue?.GetType(); null type => null lerp.
        if (start_value_.has_value())
        {
            return &lerp::get(std::type_index(start_value_.type()));
        }
        if (end_value_.has_value())
        {
            return &lerp::get(std::type_index(end_value_.type()));
        }
        return nullptr;
    }

    void lerping_animation::set_lerp(lerp value)
    {
        lerp_ = std::move(value);
    }

    void lerping_animation::update(double percent)
    {
        try
        {
            animation::update(percent);
            const lerp* resolved = current_lerp();
            if (resolved != nullptr && resolved->calculate && start_value_.has_value() && end_value_.has_value())
            {
                current_value_ = resolved->calculate(start_value_, end_value_, progress_);
                if (value_changed_)
                {
                    value_changed_();
                }
            }
        }
        catch (...)
        {
            has_finished_ = true;
        }
    }
} // namespace maui::animations
