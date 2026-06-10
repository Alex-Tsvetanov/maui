// maui::controls::toggle_switch — out-of-line definitions: the shared bindable-property descriptors
// (one instance per type, like Switch.*Property) and the default-handler self-registration.

#include "maui/controls/toggle_switch.hpp"

#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/switch_handler.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    namespace
    {
        // Re-run the effective-TrackColor mapper (the C# `((IView)bindable)?.Handler?.UpdateValue(
        // nameof(ISwitch.TrackColor))` shared by the IsToggled / OnColor / OffColor callbacks).
        void update_track_color(maui::core::bindable_object& bindable)
        {
            auto& self = static_cast<toggle_switch&>(bindable); // descriptor owner is always a toggle_switch
            if (const auto& handler = self.handler())
            {
                handler->update_value("track_color");
            }
        }
    } // namespace

    // Switch.IsToggledProperty: default false, TwoWay; a change raises Toggled, drives the On/Off
    // visual state, and re-runs the TrackColor mapper (the effective color depends on the state).
    const maui::core::bindable_property<bool>& toggle_switch::is_toggled_property()
    {
        static const maui::core::bindable_property<bool> descriptor{
            "is_toggled",
            false,
            {.property_changed =
                 [](maui::core::bindable_object& bindable, const bool& /*old_value*/, const bool& new_value) {
                     auto& self = static_cast<toggle_switch&>(bindable);
                     self.toggled.raise(new_value);
                     self.change_visual_state();
                     update_track_color(bindable);
                 },
             .default_binding_mode = maui::core::binding_mode::two_way}};
        return descriptor;
    }

    // Switch.OnColorProperty / OffColorProperty: a change re-runs the TrackColor mapper.
    const maui::core::bindable_property<maui::graphics::color>& toggle_switch::on_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{
            "on_color",
            maui::graphics::color{},
            {.property_changed = [](maui::core::bindable_object& bindable, const maui::graphics::color&,
                                    const maui::graphics::color&) { update_track_color(bindable); }}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& toggle_switch::off_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{
            "off_color",
            maui::graphics::color{},
            {.property_changed = [](maui::core::bindable_object& bindable, const maui::graphics::color&,
                                    const maui::graphics::color&) { update_track_color(bindable); }}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& toggle_switch::thumb_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"thumb_color"};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler (opt-in, PROFILE §6). This TU is always linked (the descriptors
// above are referenced by every user of the control), so the registrar runs.
MAUI_REGISTER_HANDLER(maui::controls::toggle_switch, maui::core::switch_handler)
