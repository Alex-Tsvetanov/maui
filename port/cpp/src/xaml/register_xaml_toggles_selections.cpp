// maui::xaml — XAML registration for the toggles_selections group:
//   CheckBox, RadioButton, Switch (port name: toggle_switch).
//
// Source of truth: xaml_specs.json group "toggles_selections" (property list + content_or_children),
// cross-checked against check_box.hpp, radio_button.hpp, toggle_switch.hpp.
//
// Pattern mirrors register_xaml_text_input.cpp (the reference group):
//   1. types.register_type<cpp_type>("XamlElement")
//   2. register_view_properties<cpp_type>(properties)  — IView/VisualElement shared surface
//   3. properties.register_bindable_property<cpp_type>("Attr", cpp_type::attr_property())  — per property
//   4. Content / child sinks per content_or_children (none for leaf controls here, except RadioButton
//      which has a string [ContentProperty("Content")]).
//
// Documented deferrals in this file:
//   - font_property() on radio_button is DEFERRED: maui::core::font has no registered converter in
//     register_standard_xaml_converters, and no convert_font free function exists in xaml_converters.hpp.
//     The spec lists "FontFamily" as the markup name but the backing property is the composite
//     font_property(); defer the entire slot until convert_font is added (as with Button/Label/Entry).
//   - radio_button::value_ (std::any) has NO bindable_property accessor and is explicitly excluded per
//     the spec note and the header deviation comment.

#include "register_xaml_groups.hpp"

#include "maui/controls/check_box.hpp"
#include "maui/controls/radio_button.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/i_view.hpp"
#include "maui/graphics/color.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"
#include "register_xaml_helpers.hpp"

namespace maui::xaml
{
    void register_xaml_toggles_selections(xaml_type_registry& types, xaml_property_registry& properties,
                                          xaml_converter_registry& /*converters*/)
    {
        namespace controls = maui::controls;

        // ---- CheckBox (CheckBox.cs) ---------------------------------------------------------------
        // Leaf control; no [ContentProperty] — no register_add_child or register_content_property.
        types.register_type<controls::check_box>("CheckBox");
        register_view_properties<controls::check_box>(properties);
        properties.register_bindable_property<controls::check_box>("IsChecked",
                                                                   controls::check_box::is_checked_property());
        properties.register_bindable_property<controls::check_box>("Color", controls::check_box::color_property());

        // ---- RadioButton (RadioButton.cs; [ContentProperty("Content")] — string path only) --------
        // The port's string content cut registers the Content attribute AND a content_property so that
        // inline element text in XAML (e.g. <RadioButton>Yes</RadioButton>) routes to the Content string.
        // font_property() is DEFERRED — no convert_font converter exists yet (see file header note).
        // value_ (std::any) has no bindable_property accessor and is excluded.
        types.register_type<controls::radio_button>("RadioButton");
        register_view_properties<controls::radio_button>(properties);
        register_font_properties<controls::radio_button>(properties); // W6
        properties.register_bindable_property<controls::radio_button>("IsChecked",
                                                                      controls::radio_button::is_checked_property());
        properties.register_bindable_property<controls::radio_button>("GroupName",
                                                                      controls::radio_button::group_name_property());
        properties.register_bindable_property<controls::radio_button>("Content",
                                                                      controls::radio_button::content_property());
        properties.register_bindable_property<controls::radio_button>("TextColor",
                                                                      controls::radio_button::text_color_property());
        // FontFamily / font_property() — DEFERRED (no convert_font converter; see file header note).
        properties.register_bindable_property<controls::radio_button>(
            "CharacterSpacing", controls::radio_button::character_spacing_property());
        // C# markup names for the IButtonStroke surface are BorderColor / BorderWidth / CornerRadius;
        // the port's descriptor names are stroke_color_property / stroke_thickness_property / corner_radius_property.
        properties.register_bindable_property<controls::radio_button>("BorderColor",
                                                                      controls::radio_button::stroke_color_property());
        properties.register_bindable_property<controls::radio_button>(
            "BorderWidth", controls::radio_button::stroke_thickness_property());
        properties.register_bindable_property<controls::radio_button>("CornerRadius",
                                                                      controls::radio_button::corner_radius_property());
        // [ContentProperty("Content")]: inline text in markup routes to the Content string attribute.
        properties.register_content_property<controls::radio_button>("Content");

        // ---- Switch / toggle_switch (Switch.cs; XAML element name is "Switch") -------------------
        // Leaf control; no [ContentProperty] — no register_add_child or register_content_property.
        // The C++ class is named toggle_switch (C++ keyword collision with 'switch'); the XAML name
        // "Switch" is registered here matching the C# MAUI markup name.
        types.register_type<controls::toggle_switch>("Switch");
        register_view_properties<controls::toggle_switch>(properties);
        properties.register_bindable_property<controls::toggle_switch>("IsToggled",
                                                                       controls::toggle_switch::is_toggled_property());
        properties.register_bindable_property<controls::toggle_switch>("OnColor",
                                                                       controls::toggle_switch::on_color_property());
        properties.register_bindable_property<controls::toggle_switch>("OffColor",
                                                                       controls::toggle_switch::off_color_property());
        properties.register_bindable_property<controls::toggle_switch>("ThumbColor",
                                                                       controls::toggle_switch::thumb_color_property());
    }
} // namespace maui::xaml
