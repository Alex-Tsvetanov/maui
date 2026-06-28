// maui::xaml — XAML registration for the "pickers" control group:
//   Picker, DatePicker, TimePicker
//   (Microsoft.Maui.Controls.Picker / DatePicker / TimePicker)
//
// Derivations: see xaml_specs.json "pickers" group entry for the full property/accessor table.
// Pattern: mirrors register_xaml_text_input.cpp — same include style, same namespace,
// same register_type / register_view_properties / register_bindable_property call sequence.
//
// Deferred / out-of-scope:
//   - Picker.SelectedItem (std::optional<std::string>): registered as a register_property<std::string>
//     lambda (wrapping set_selected_item with a string→optional<string> adaptor) since no
//     type-keyed converter exists for std::optional<std::string>.
//   - DatePicker.Date / MinimumDate / MaximumDate (std::optional<maui::core::date_time>): no
//     convert_optional_date_time exists; these attributes cannot be set from XAML markup text yet.
//     Deferred until a date converter is added.
//   - TimePicker.Time (std::optional<maui::core::time_span>): no convert_optional_time_span exists;
//     deferred until a time-span converter is added.
//   - FontSize/FontAttributes/FontFamily: registered via register_font_properties<T> (W6) — they
//     compose onto the single maui::core::font value (font().with_size / with_attributes / of_size),
//     parsed as string properties so NamedSize and the [Flags] FontAttributes route through one entry.
//   - Picker.Items child sink (W12, DONE): the element form <Picker.Items><x:String>…</x:String> is
//     handled in xaml_visitors.cpp (try_add_picker_item), NOT here — the <x:String> children mint plain
//     std::string VALUES (the x:String primitive route), which bypass both the registered-property
//     surface and the bindable child sink (the port's item_list has no static bindable_property
//     accessor), so they push straight onto the picker's Items face. Mirrors the W2 Grid-definitions
//     pattern. (A SelectedIndex attribute still coerces against the then-empty Items — attributes apply
//     before property-element children, exactly as MAUI's BottomUp ApplyPropertiesVisitor does.)
//   - Picker.ItemsSource (std::shared_ptr<observable_collection<std::string>>): no text converter
//     makes sense; deferred (needs a binding/static-collection seam).

#include "register_xaml_groups.hpp"
#include "register_xaml_helpers.hpp"

#include "maui/controls/date_picker.hpp"
#include "maui/controls/picker.hpp"
#include "maui/controls/time_picker.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    void register_xaml_pickers(xaml_type_registry& types, xaml_property_registry& properties,
                               xaml_converter_registry& /*converters*/)
    {
        namespace controls = maui::controls;

        // ---- Picker (Picker.cs; items-source / font / date converters deferred) ----
        types.register_type<controls::picker>("Picker");
        register_view_properties<controls::picker>(properties);
        register_font_properties<controls::picker>(properties); // W6
        properties.register_bindable_property<controls::picker>("Title", controls::picker::title_property());
        properties.register_bindable_property<controls::picker>("TitleColor", controls::picker::title_color_property());
        properties.register_bindable_property<controls::picker>("SelectedIndex",
                                                                controls::picker::selected_index_property());
        properties.register_bindable_property<controls::picker>("IsOpen", controls::picker::is_open_property());
        // SelectedItem: std::optional<std::string> — no optional converter exists; register via a
        // string→optional<string> adaptor lambda (the register_property<..., string> pattern from Grid
        // RowDefinitions) so a plain string attribute value routes to set_selected_item.
        properties.register_property<controls::picker, std::string>(
            "SelectedItem",
            [](controls::picker& picker, const std::string& value) { picker.set_selected_item(value); });
        properties.register_bindable_property<controls::picker>("TextColor", controls::picker::text_color_property());
        properties.register_bindable_property<controls::picker>("CharacterSpacing",
                                                                controls::picker::character_spacing_property());
        properties.register_bindable_property<controls::picker>("HorizontalTextAlignment",
                                                                controls::picker::horizontal_text_alignment_property());
        properties.register_bindable_property<controls::picker>("VerticalTextAlignment",
                                                                controls::picker::vertical_text_alignment_property());
        // No content/children sink: Picker has no [ContentProperty]; the Items / ItemsSource child
        // sinks are deferred (see file header). No register_add_child or register_content_property.

        // ---- DatePicker (DatePicker.cs; Date/MinimumDate/MaximumDate deferred; font deferred) ----
        types.register_type<controls::date_picker>("DatePicker");
        register_view_properties<controls::date_picker>(properties);
        register_font_properties<controls::date_picker>(properties); // W6
        properties.register_bindable_property<controls::date_picker>("Format",
                                                                     controls::date_picker::format_property());
        // Date / MinimumDate / MaximumDate: std::optional<maui::core::date_time>. The optional converter
        // IS registered (register_xaml_extra_converters.cpp), so these set from "YYYY-MM-DD" / "M/d/yyyy"
        // text (an empty string -> nullopt).
        properties.register_bindable_property<controls::date_picker>("Date", controls::date_picker::date_property());
        properties.register_bindable_property<controls::date_picker>("MinimumDate",
                                                                     controls::date_picker::minimum_date_property());
        properties.register_bindable_property<controls::date_picker>("MaximumDate",
                                                                     controls::date_picker::maximum_date_property());
        properties.register_bindable_property<controls::date_picker>("IsOpen",
                                                                     controls::date_picker::is_open_property());
        properties.register_bindable_property<controls::date_picker>("TextColor",
                                                                     controls::date_picker::text_color_property());
        properties.register_bindable_property<controls::date_picker>(
            "CharacterSpacing", controls::date_picker::character_spacing_property());
        // No content/children sink: DatePicker is a leaf control with no [ContentProperty].

        // ---- TimePicker (TimePicker.cs; Time deferred; font deferred) ----
        types.register_type<controls::time_picker>("TimePicker");
        register_view_properties<controls::time_picker>(properties);
        register_font_properties<controls::time_picker>(properties); // W6
        properties.register_bindable_property<controls::time_picker>("Format",
                                                                     controls::time_picker::format_property());
        // Time: std::optional<maui::core::time_span>. The optional converter IS registered, so this sets
        // from "h:mm:ss" / "hh:mm:ss" text (an empty string -> nullopt).
        properties.register_bindable_property<controls::time_picker>("Time", controls::time_picker::time_property());
        properties.register_bindable_property<controls::time_picker>("IsOpen",
                                                                     controls::time_picker::is_open_property());
        properties.register_bindable_property<controls::time_picker>("TextColor",
                                                                     controls::time_picker::text_color_property());
        properties.register_bindable_property<controls::time_picker>(
            "CharacterSpacing", controls::time_picker::character_spacing_property());
        // No content/children sink: TimePicker is a leaf control with no [ContentProperty].
    }
} // namespace maui::xaml
