// maui::controls::slider — out-of-line definitions: the shared bindable-property descriptors (one
// instance per type, like Slider.*Property) and the default-handler self-registration.

#include "maui/controls/slider.hpp"

#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/slider_handler.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    // Grants the descriptor callbacks below access to the slider's recoercion state (the C# analog is
    // the static coerceValue/propertyChanged delegates living inside the Slider class itself).
    struct slider_descriptor_access
    {
        static double coerce_value(slider& self, double value)
        {
            // Slider.ValueProperty coerceValue: remember what the user actually requested (unless this
            // set IS the recoercion), then clamp into the current range.
            if (!self.is_recoercing_)
            {
                self.requested_value_ = value;
                self.user_set_value_ = true;
            }
            return slider::clamp(value, self.minimum(), self.maximum());
        }

        static void recoerce(slider& self)
        {
            self.recoerce_value();
        }
    };

    // Slider.MinimumProperty / MaximumProperty: defaults 0 / 1, no validation (a temporarily inverted
    // range is legal); a change recoerces Value against the new range.
    const maui::core::bindable_property<double>& slider::minimum_property()
    {
        static const maui::core::bindable_property<double> descriptor{
            "minimum",
            0.0,
            {.property_changed = [](maui::core::bindable_object& bindable, const double&, const double&) {
                slider_descriptor_access::recoerce(static_cast<slider&>(bindable));
            }}};
        return descriptor;
    }

    const maui::core::bindable_property<double>& slider::maximum_property()
    {
        static const maui::core::bindable_property<double> descriptor{
            "maximum",
            1.0,
            {.property_changed = [](maui::core::bindable_object& bindable, const double&, const double&) {
                slider_descriptor_access::recoerce(static_cast<slider&>(bindable));
            }}};
        return descriptor;
    }

    // Slider.ValueProperty: default 0, TwoWay; coercion clamps (remembering the requested value) and a
    // change raises ValueChanged(old, new).
    const maui::core::bindable_property<double>& slider::value_property()
    {
        static const maui::core::bindable_property<double> descriptor{
            "value",
            0.0,
            {.property_changed =
                 [](maui::core::bindable_object& bindable, const double& old_value, const double& new_value) {
                     static_cast<slider&>(bindable).value_changed.raise(old_value, new_value);
                 },
             .coerce_value =
                 [](maui::core::bindable_object& bindable, const double& value) {
                     return slider_descriptor_access::coerce_value(static_cast<slider&>(bindable), value);
                 },
             .default_binding_mode = maui::core::binding_mode::two_way}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& slider::minimum_track_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"minimum_track_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& slider::maximum_track_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"maximum_track_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& slider::thumb_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"thumb_color"};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler (opt-in, PROFILE §6). This TU is always linked (the descriptors
// above are referenced by every user of the control), so the registrar runs.
MAUI_REGISTER_HANDLER(maui::controls::slider, maui::core::slider_handler)
