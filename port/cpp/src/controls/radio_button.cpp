// maui::controls::radio_button — out-of-line definitions: the shared bindable-property descriptors
// (one instance per type, like RadioButton.*Property), the group-coupling callbacks
// (OnIsCheckedPropertyChanged / OnValuePropertyChanged / OnGroupNamePropertyChanged + the
// DescendantAdded translation), and the default-handler self-registration. See radio_button.hpp.

#include "maui/controls/radio_button.hpp"

#include <string>

#include "maui/controls/element.hpp"
#include "maui/controls/radio_button_group.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/radio_button_handler.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    // Grants the descriptor callbacks below access to the private on_* members (the C# analog is the
    // static propertyChanged delegates living inside the RadioButton class itself).
    struct radio_button_descriptor_access
    {
        static void is_checked_changed(radio_button& self, bool is_checked)
        {
            self.on_is_checked_changed(is_checked);
        }
        static void group_name_changed(radio_button& self, const std::string& old_name, const std::string& new_name)
        {
            self.on_group_name_changed(old_name, new_name);
        }
    };

    // RadioButton.IsCheckedProperty: default false, TwoWay; a change runs OnIsCheckedPropertyChanged
    // (group exclusion when checking → visual state → CheckedChanged), in that C# order.
    const maui::core::bindable_property<bool>& radio_button::is_checked_property()
    {
        static const maui::core::bindable_property<bool> descriptor{
            "is_checked",
            false,
            {.property_changed =
                 [](maui::core::bindable_object& bindable, const bool& /*old_value*/, const bool& new_value) {
                     // The descriptor's owner is always a radio_button.
                     radio_button_descriptor_access::is_checked_changed(dynamic_cast<radio_button&>(bindable),
                                                                        new_value);
                 },
             .default_binding_mode = maui::core::binding_mode::two_way}};
        return descriptor;
    }

    // RadioButton.GroupNameProperty: default null (empty); a change runs OnGroupNamePropertyChanged.
    const maui::core::bindable_property<std::string>& radio_button::group_name_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{
            "group_name",
            std::string{},
            {.property_changed = [](maui::core::bindable_object& bindable, const std::string& old_value,
                                    const std::string& new_value) {
                radio_button_descriptor_access::group_name_changed(dynamic_cast<radio_button&>(bindable), old_value,
                                                                   new_value);
            }}};
        return descriptor;
    }

    // RadioButton.ContentProperty (the port's string-content cut — see the header deviation note).
    const maui::core::bindable_property<std::string>& radio_button::content_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"content", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& radio_button::text_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"text_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::font>& radio_button::font_property()
    {
        static const maui::core::bindable_property<maui::core::font> descriptor{"font"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& radio_button::character_spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{"character_spacing", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& radio_button::stroke_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"stroke_color"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& radio_button::stroke_thickness_property()
    {
        // -1, NOT 0: C#'s BorderElement.BorderWidthProperty defaults to -1d (a "not set" SENTINEL), and
        // every handler's UpdateStrokeThickness keys off `>= 0` to decide whether to override the native
        // default at all. With 0 here the port pushed an EXPLICIT zero border on every button - invisible
        // on iOS/Android (their native default is already 0) but on WinUI it erased the theme's button
        // border outright. An explicit BorderWidth=0 must still mean "no border"; that is exactly why the
        // unset value has to be a distinct sentinel.
        static const maui::core::bindable_property<double> descriptor{"stroke_thickness", -1.0};
        return descriptor;
    }

    const maui::core::bindable_property<int>& radio_button::corner_radius_property()
    {
        // -1 (BorderElement.DefaultCornerRadius), NOT 0 - same sentinel argument as stroke_thickness
        // above: 0 is a legitimate explicit "square corners" request and must be distinguishable from
        // "unset". On WinUI the 0 default squared off every themed button.
        static const maui::core::bindable_property<int> descriptor{"corner_radius", -1};
        return descriptor;
    }

    // RadioButton.OnIsCheckedPropertyChanged: checking runs the group update first (uncheck the others
    // in scope + record the selection), then the visual state, then CheckedChanged — unchecking skips
    // the group update.
    void radio_button::on_is_checked_changed(bool is_checked)
    {
        if (is_checked)
        {
            radio_button_group::update_radio_button_group(*this);
        }
        change_visual_state();
        checked_changed.raise(is_checked);
    }

    // RadioButton.OnValuePropertyChanged: only a CHECKED button with a group name refreshes the
    // group's selected value.
    void radio_button::on_value_changed()
    {
        if (!is_checked() || group_name().empty())
        {
            return;
        }
        if (const auto controller = group_controller())
        {
            controller->handle_radio_button_value_changed(*this);
        }
    }

    // RadioButton.OnGroupNamePropertyChanged: moving between two NAMED groups clears the old group's
    // selection (via the still-associated controller).
    void radio_button::on_group_name_changed(const std::string& old_group_name, const std::string& new_group_name) const
    {
        if (old_group_name.empty() || new_group_name.empty() || new_group_name == old_group_name)
        {
            return;
        }
        if (const auto controller = group_controller())
        {
            controller->handle_radio_button_group_name_changed(old_group_name);
        }
    }

    // The DescendantAdded/DescendantRemoved translation (see radio_button_group.hpp): re-resolve the
    // nearest named controller up the (changed) ancestor chain. Runs after the base re-resolution so
    // styles/resources settle first.
    void radio_button::on_resource_chain_changed()
    {
        view::on_resource_chain_changed();
        for (element* ancestor = logical_parent(); ancestor != nullptr; ancestor = ancestor->logical_parent())
        {
            const auto controller = radio_button_group::existing_controller_of(*ancestor);
            if (controller && !controller->group_name().empty())
            {
                controller->add_radio_button(*this);
                return;
            }
        }
        // No grouped ancestor remains: drop the association (the DescendantRemoved analog).
        group_controller_.reset();
    }
} // namespace maui::controls

// Self-register the default handler for radio_button (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::radio_button, maui::core::radio_button_handler)
