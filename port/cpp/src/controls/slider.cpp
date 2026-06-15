// maui::controls::slider — out-of-line definitions: the shared bindable-property descriptors (one
// instance per type, like Slider.*Property) and the default-handler self-registration.

#include "maui/controls/slider.hpp"

#include <memory>

#include "maui/controls/platform_configuration/ios_specific/slider.hpp" // the UpdateOnTap knob (key + getter)
#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_image_source.hpp"
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
                slider_descriptor_access::recoerce(dynamic_cast<slider&>(bindable));
            }}};
        return descriptor;
    }

    const maui::core::bindable_property<double>& slider::maximum_property()
    {
        static const maui::core::bindable_property<double> descriptor{
            "maximum",
            1.0,
            {.property_changed = [](maui::core::bindable_object& bindable, const double&, const double&) {
                slider_descriptor_access::recoerce(dynamic_cast<slider&>(bindable));
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
                     dynamic_cast<slider&>(bindable).value_changed.raise(old_value, new_value);
                 },
             .coerce_value =
                 [](maui::core::bindable_object& bindable, const double& value) {
                     return slider_descriptor_access::coerce_value(dynamic_cast<slider&>(bindable), value);
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

    // Slider.ThumbImageSourceProperty: default null ImageSource. The key matches the handler's mapper
    // entry so a change re-runs MapThumbImageSource (the async image-service load + thumb swap).
    const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& slider::
        thumb_image_source_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>> descriptor{
            "thumb_image_source"};
        return descriptor;
    }

    // i_ios_slider_specifics: read the iOSSpecific.Slider.UpdateOnTap platform-spec store (the
    // entry::cursor_color pattern — the knob header lives here in the .cpp to avoid a header cycle).
    bool slider::update_on_tap() const
    {
        return platform_configuration::ios_specific::slider::get_update_on_tap(*this);
    }
} // namespace maui::controls

// Self-register the default handler (opt-in, PROFILE §6). This TU is always linked (the descriptors
// above are referenced by every user of the control), so the registrar runs.
MAUI_REGISTER_HANDLER(maui::controls::slider, maui::core::slider_handler)
