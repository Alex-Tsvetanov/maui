// maui::controls::stepper — out-of-line definitions: the shared bindable-property descriptors (one
// instance per type, like Stepper.*Property) and the default-handler self-registration.

#include "maui/controls/stepper.hpp"

#include <cmath>

#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/stepper_handler.hpp"

namespace maui::controls
{
    namespace
    {
        // .NET Math.Round(double, int digits) — banker's rounding (half to even) at `digits` decimal
        // places: scale by 10^digits, round to nearest-even, unscale; values at or beyond 1e16 are
        // returned unchanged (they carry no fractional precision anyway). std::nearbyint under the
        // default FE_TONEAREST mode IS round-half-to-even, matching .NET's midpoint behavior.
        double net_round(double value, int digits)
        {
            if (!(std::fabs(value) < 1e16))
            {
                return value;
            }
            const double power = std::pow(10.0, digits);
            return std::nearbyint(value * power) / power;
        }
    } // namespace

    // Grants the descriptor callbacks below access to the stepper's recoercion + digits state (the C#
    // analog is the static delegates living inside the Stepper class itself).
    struct stepper_descriptor_access
    {
        static double coerce_value(stepper& self, double value)
        {
            // Stepper.ValueProperty coerceValue: remember what the user actually requested (unless
            // this set IS the recoercion), then Math.Round(value, digits).Clamp(Minimum, Maximum).
            if (!self.is_recoercing_)
            {
                self.requested_value_ = value;
                self.user_set_value_ = true;
            }
            return stepper::clamp(net_round(value, self.digits_), self.minimum(), self.maximum());
        }

        static void recoerce(stepper& self)
        {
            self.recoerce_value();
        }

        static void update_digits(stepper& self, double increment)
        {
            // Stepper.IncrementProperty propertyChanged: digits = (-log10(increment) + 4).Clamp(1, 15)
            // (int-truncated after the double clamp, exactly as C#).
            const double raw = -std::log10(increment) + 4;
            self.digits_ = static_cast<int>(stepper::clamp(raw, 1, 15));
        }
    };

    // Stepper.MinimumProperty: default 0, valid only at or below Maximum (an invalid set is ignored);
    // a change recoerces Value.
    const maui::core::bindable_property<double>& stepper::minimum_property()
    {
        static const maui::core::bindable_property<double> descriptor{
            "minimum",
            0.0,
            {.property_changed =
                 [](maui::core::bindable_object& bindable, const double&, const double&) {
                     stepper_descriptor_access::recoerce(dynamic_cast<stepper&>(bindable));
                 },
             .validate_value = [](maui::core::bindable_object& bindable,
                                  const double& value) { return value <= dynamic_cast<stepper&>(bindable).maximum(); }}};
        return descriptor;
    }

    // Stepper.MaximumProperty: default 100, valid only at or above Minimum.
    const maui::core::bindable_property<double>& stepper::maximum_property()
    {
        static const maui::core::bindable_property<double> descriptor{
            "maximum",
            100.0,
            {.property_changed =
                 [](maui::core::bindable_object& bindable, const double&, const double&) {
                     stepper_descriptor_access::recoerce(dynamic_cast<stepper&>(bindable));
                 },
             .validate_value = [](maui::core::bindable_object& bindable,
                                  const double& value) { return value >= dynamic_cast<stepper&>(bindable).minimum(); }}};
        return descriptor;
    }

    // Stepper.ValueProperty: default 0, TwoWay; coercion rounds-then-clamps (remembering the requested
    // value) and a change raises ValueChanged(old, new).
    const maui::core::bindable_property<double>& stepper::value_property()
    {
        static const maui::core::bindable_property<double> descriptor{
            "value",
            0.0,
            {.property_changed =
                 [](maui::core::bindable_object& bindable, const double& old_value, const double& new_value) {
                     dynamic_cast<stepper&>(bindable).value_changed.raise(old_value, new_value);
                 },
             .coerce_value =
                 [](maui::core::bindable_object& bindable, const double& value) {
                     return stepper_descriptor_access::coerce_value(dynamic_cast<stepper&>(bindable), value);
                 },
             .default_binding_mode = maui::core::binding_mode::two_way}};
        return descriptor;
    }

    // Stepper.IncrementProperty: default 1; a change recomputes the rounding digits.
    const maui::core::bindable_property<double>& stepper::increment_property()
    {
        static const maui::core::bindable_property<double> descriptor{
            "increment",
            1.0,
            {.property_changed = [](maui::core::bindable_object& bindable, const double&, const double& new_value) {
                stepper_descriptor_access::update_digits(dynamic_cast<stepper&>(bindable), new_value);
            }}};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler (opt-in, PROFILE §6). This TU is always linked (the descriptors
// above are referenced by every user of the control), so the registrar runs.
MAUI_REGISTER_HANDLER(maui::controls::stepper, maui::core::stepper_handler)
