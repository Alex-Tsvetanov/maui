// maui::controls::check_box — out-of-line definitions: the shared bindable-property descriptors (one
// instance per type, like CheckBox.*Property) and the default-handler self-registration.

#include "maui/controls/check_box.hpp"

#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/check_box_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    // Grants the descriptor callbacks below access to check_box::refresh_foreground (the C# analog is
    // the static propertyChanged delegates living inside the CheckBox class itself).
    struct check_box_descriptor_access
    {
        static void refresh_foreground(check_box& self)
        {
            self.refresh_foreground();
        }
    };

    // CheckBox.IsCheckedProperty: default false, TwoWay; a change re-runs the Foreground mapper, raises
    // CheckedChanged, executes the command, and drives the IsChecked visual state — in that C# order.
    const maui::core::bindable_property<bool>& check_box::is_checked_property()
    {
        static const maui::core::bindable_property<bool> descriptor{
            "is_checked",
            false,
            {.property_changed =
                 [](maui::core::bindable_object& bindable, const bool& /*old_value*/, const bool& new_value) {
                     auto& self = static_cast<check_box&>(bindable); // descriptor owner is always a check_box
                     if (const auto& handler = self.handler())
                     {
                         handler->update_value("foreground");
                     }
                     self.checked_changed.raise(new_value);
                     if (self.command)
                     {
                         self.command();
                     }
                     self.change_visual_state();
                 },
             .default_binding_mode = maui::core::binding_mode::two_way}};
        return descriptor;
    }

    // CheckBox.ColorProperty (ColorElement.ColorProperty): a change rebuilds the owned foreground paint
    // (Color?.AsPaint()) and re-runs the Foreground mapper (CheckBox.Mapper.cs MapColor).
    const maui::core::bindable_property<maui::graphics::color>& check_box::color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{
            "color",
            maui::graphics::color{},
            {.property_changed = [](maui::core::bindable_object& bindable, const maui::graphics::color&,
                                    const maui::graphics::color&) {
                auto& self = static_cast<check_box&>(bindable);
                check_box_descriptor_access::refresh_foreground(self);
                if (const auto& handler = self.handler())
                {
                    handler->update_value("foreground");
                }
            }}};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler (opt-in, PROFILE §6). This TU is always linked (the descriptors
// above are referenced by every user of the control), so the registrar runs.
MAUI_REGISTER_HANDLER(maui::controls::check_box, maui::core::check_box_handler)
