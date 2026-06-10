#pragma once
// maui::controls::stepper  <=  Microsoft.Maui.Controls.Stepper
//
// Minus/plus buttons that adjust a numeric value by a fixed increment within [Minimum, Maximum].
// Ported from src/Controls/src/Core/Stepper/Stepper.cs (+ Stepper.Mapper.cs).
//
// Value semantics (the StepperUnitTests oracle): Value coerces through
// Math.Round(value, digits).Clamp(Minimum, Maximum), where `digits` derives from Increment as
// '-log10(increment) + 4' clamped to [1, 15] — 4 significant decimal digits after the most significant
// one ("If your increment uses more than 4 significant digits, you're holding it wrong"). Rounding is
// .NET Math.Round's banker's rounding (half to even). A Minimum/Maximum change RECOERCES Value with
// the requested value remembered, exactly like slider. Unlike slider, Minimum/Maximum ARE validated
// (Minimum <= Maximum / Maximum >= Minimum; an invalid set is ignored) — Minimum == Maximum is legal
// (#28330).

#include <stdexcept>
#include <string_view>

#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_stepper.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class stepper : public view<maui::core::i_stepper>
    {
    public:
        stepper()
        {
            this->set_style_target_type<stepper>();
        }

        // Stepper(double min, double max, double val, double increment): min must be strictly below
        // max; the larger bound is applied first; val clamps into [min, max].
        stepper(double min, double max, double val, double increment) : stepper()
        {
            if (min >= max)
            {
                throw std::out_of_range("stepper: min must be less than max");
            }
            if (max > minimum())
            {
                set_maximum(max);
                set_minimum(min);
            }
            else
            {
                set_minimum(min);
                set_maximum(max);
            }
            set_increment(increment);
            set_value(clamp(val, min, max));
        }

        // Shared bindable-property descriptors (one instance per type, like Stepper.*Property).
        static const maui::core::bindable_property<double>& minimum_property();
        static const maui::core::bindable_property<double>& maximum_property();
        static const maui::core::bindable_property<double>& value_property();
        static const maui::core::bindable_property<double>& increment_property();

        // ---- i_range (the mutable Value doubles as the developer setter, as in C#) ----
        [[nodiscard]] double minimum() const override
        {
            return minimum_.get();
        }
        void set_minimum(double value)
        {
            minimum_.set(value);
        }
        [[nodiscard]] double maximum() const override
        {
            return maximum_.get();
        }
        void set_maximum(double value)
        {
            maximum_.set(value);
        }
        [[nodiscard]] double value() const override
        {
            return value_.get();
        }
        void set_value(double value) override
        {
            value_.set(value);
        }

        // ---- Increment (IStepper.Interval => Increment) ----
        [[nodiscard]] double increment() const
        {
            return increment_.get();
        }
        void set_increment(double value)
        {
            increment_.set(value);
        }
        [[nodiscard]] double interval() const override
        {
            return increment_.get();
        }

        // Occurs when Value changes — (old, new), like ValueChangedEventArgs.
        maui::core::event<double, double> value_changed;

    private:
        // Graphics NumericExtensions.Clamp: max wins when the range is inverted.
        [[nodiscard]] static double clamp(double self, double min, double max)
        {
            if (max < min)
            {
                return max;
            }
            if (self < min)
            {
                return min;
            }
            if (self > max)
            {
                return max;
            }
            return self;
        }

        // Stepper.RecoerceValue: re-run the Value coercion after a range change — restoring the value
        // the user originally requested when it fits the new range.
        void recoerce_value()
        {
            is_recoercing_ = true;
            if (user_set_value_)
            {
                value_.set(requested_value_);
            }
            else
            {
                value_.set(clamp(value_.get(), minimum(), maximum()));
            }
            is_recoercing_ = false;
        }

        // The descriptors' callbacks (stepper.cpp) reach the recoercion + digits state above.
        friend struct stepper_descriptor_access;

        maui::core::property<double> minimum_{*this, minimum_property()};
        maui::core::property<double> maximum_{*this, maximum_property()};
        maui::core::property<double> value_{*this, value_property()};
        maui::core::property<double> increment_{*this, increment_property()};
        // '-log10(increment) + 4' rounding digits — "4 significant decimal digits after the most
        // significant one" (recomputed whenever Increment changes; starts at 4 for the default 1.0).
        int digits_ = 4;
        double requested_value_ = 0;  // the value the user asked for, before rounding/clamping
        bool user_set_value_ = false; // whether Value was ever explicitly set (vs recoercion)
        bool is_recoercing_ = false;
    };
} // namespace maui::controls
