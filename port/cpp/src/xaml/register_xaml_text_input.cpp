// maui::xaml — XAML registrations for the "text_input" control group: Editor, SearchBar.
//
// Both are leaf text controls (no child sink). Mirrors the Entry block in xaml_standard_types.cpp and
// the register_view_properties<T> / register_font_properties<T> helpers from register_xaml_helpers.hpp
// (W6 wired FontSize/FontAttributes/FontFamily composition onto the single core::font value).
//
// See register_xaml_groups.hpp for the function-signature declaration; this is the reference group the
// other register_xaml_<group>.cpp files mirror.

#include "register_xaml_groups.hpp"
#include "register_xaml_helpers.hpp"

#include "maui/controls/editor.hpp"
#include "maui/controls/search_bar.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    void register_xaml_text_input(xaml_type_registry& types, xaml_property_registry& properties,
                                  xaml_converter_registry& /*converters*/)
    {
        namespace controls = maui::controls;

        // ---- Editor (Editor.cs: multi-line text input; AutoSize grows with content) ----
        types.register_type<controls::editor>("Editor");
        register_view_properties<controls::editor>(properties);
        register_font_properties<controls::editor>(properties); // W6
        properties.register_bindable_property<controls::editor>("Text", controls::editor::text_property());
        properties.register_bindable_property<controls::editor>("Placeholder",
                                                                controls::editor::placeholder_property());
        properties.register_bindable_property<controls::editor>("PlaceholderColor",
                                                                controls::editor::placeholder_color_property());
        properties.register_bindable_property<controls::editor>("IsReadOnly",
                                                                controls::editor::is_read_only_property());
        properties.register_bindable_property<controls::editor>("MaxLength", controls::editor::max_length_property());
        properties.register_bindable_property<controls::editor>(
            "IsTextPredictionEnabled", controls::editor::is_text_prediction_enabled_property());
        properties.register_bindable_property<controls::editor>("IsSpellCheckEnabled",
                                                                controls::editor::is_spell_check_enabled_property());
        properties.register_bindable_property<controls::editor>("CursorPosition",
                                                                controls::editor::cursor_position_property());
        properties.register_bindable_property<controls::editor>("SelectionLength",
                                                                controls::editor::selection_length_property());
        properties.register_bindable_property<controls::editor>("TextColor", controls::editor::text_color_property());
        properties.register_bindable_property<controls::editor>("CharacterSpacing",
                                                                controls::editor::character_spacing_property());
        properties.register_bindable_property<controls::editor>("HorizontalTextAlignment",
                                                                controls::editor::horizontal_text_alignment_property());
        properties.register_bindable_property<controls::editor>("VerticalTextAlignment",
                                                                controls::editor::vertical_text_alignment_property());
        properties.register_bindable_property<controls::editor>("AutoSize", controls::editor::auto_size_property());
        properties.register_bindable_property<controls::editor>("Keyboard", controls::editor::keyboard_property());

        // ---- SearchBar (SearchBar.cs: single-line query input + cancel/search affordances) ----
        types.register_type<controls::search_bar>("SearchBar");
        register_view_properties<controls::search_bar>(properties);
        register_font_properties<controls::search_bar>(properties); // W6
        properties.register_bindable_property<controls::search_bar>("Text", controls::search_bar::text_property());
        properties.register_bindable_property<controls::search_bar>("Placeholder",
                                                                    controls::search_bar::placeholder_property());
        properties.register_bindable_property<controls::search_bar>("PlaceholderColor",
                                                                    controls::search_bar::placeholder_color_property());
        properties.register_bindable_property<controls::search_bar>(
            "CancelButtonColor", controls::search_bar::cancel_button_color_property());
        properties.register_bindable_property<controls::search_bar>("IsReadOnly",
                                                                    controls::search_bar::is_read_only_property());
        properties.register_bindable_property<controls::search_bar>("MaxLength",
                                                                    controls::search_bar::max_length_property());
        properties.register_bindable_property<controls::search_bar>(
            "IsTextPredictionEnabled", controls::search_bar::is_text_prediction_enabled_property());
        properties.register_bindable_property<controls::search_bar>(
            "IsSpellCheckEnabled", controls::search_bar::is_spell_check_enabled_property());
        properties.register_bindable_property<controls::search_bar>("CursorPosition",
                                                                    controls::search_bar::cursor_position_property());
        properties.register_bindable_property<controls::search_bar>("SelectionLength",
                                                                    controls::search_bar::selection_length_property());
        properties.register_bindable_property<controls::search_bar>("ReturnType",
                                                                    controls::search_bar::return_type_property());
        properties.register_bindable_property<controls::search_bar>("TextColor",
                                                                    controls::search_bar::text_color_property());
        properties.register_bindable_property<controls::search_bar>("CharacterSpacing",
                                                                    controls::search_bar::character_spacing_property());
        properties.register_bindable_property<controls::search_bar>(
            "HorizontalTextAlignment", controls::search_bar::horizontal_text_alignment_property());
        properties.register_bindable_property<controls::search_bar>(
            "VerticalTextAlignment", controls::search_bar::vertical_text_alignment_property());
        properties.register_bindable_property<controls::search_bar>("Keyboard",
                                                                    controls::search_bar::keyboard_property());
    }
} // namespace maui::xaml
