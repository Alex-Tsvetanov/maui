#pragma once
// maui::animations::lerping_animation  <=  Microsoft.Maui.Animations.LerpingAnimation
//
// An animation that linearly interpolates between a start and an end VALUE: each update resolves the
// lerp for the values' type (lerp::get) and publishes the interpolated current_value. Ported from
// src/Core/src/Animations/LerpingAnimation.cs. The C# object-typed StartValue/EndValue/CurrentValue
// map to std::any (the same boundary erasure lerp.hpp documents); a typed consumer reads
// current_value with std::any_cast<T>.
//
// DEVIATION (documented): C#'s CurrentValue setter short-circuits on reference equality before
// raising ValueChanged — with boxed values that comparison is virtually always false, so the port
// raises value_changed on every produced value (std::any has no general equality).

#include <any>
#include <memory>
#include <optional>
#include <vector>

#include "maui/animations/animation.hpp"
#include "maui/animations/easing.hpp"
#include "maui/animations/lerp.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::animations
{
    class lerping_animation : public animation
    {
    public:
        using value_changed_fn = maui::core::move_only_function<void()>;

        // The three C# constructors.
        lerping_animation();
        explicit lerping_animation(step_fn callback, double start = 0.0, double end = 1.0,
                                   std::optional<easing> easing_function = {}, finished_fn finished = {});
        explicit lerping_animation(std::vector<std::shared_ptr<animation>> animations);

        // C# ValueChanged — invoked whenever current_value changes.
        void set_value_changed(value_changed_fn value);

        // C# StartValue / EndValue / CurrentValue.
        [[nodiscard]] const std::any& start_value() const;
        void set_start_value(std::any value);
        [[nodiscard]] const std::any& end_value() const;
        void set_end_value(std::any value);
        [[nodiscard]] const std::any& current_value() const;

        // C# Lerp property: explicit lerp, or the one resolved from the start/end value's type.
        [[nodiscard]] const lerp* current_lerp() const;
        void set_lerp(lerp value);

        // C# Update override: ease + step via the base, then publish the interpolated value.
        void update(double percent) override;

    private:
        value_changed_fn value_changed_;
        std::any start_value_;
        std::any end_value_;
        std::any current_value_;
        std::optional<lerp> lerp_;
    };
} // namespace maui::animations
