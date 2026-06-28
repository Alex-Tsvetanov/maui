// maui::xaml — XAML registrations for FormattedString / Span in ELEMENT form (W8):
//   <Label><Label.FormattedText><FormattedString>
//     <Span Text="Bold red" TextColor="Red" FontAttributes="Bold"/>
//     <Span Text=" underlined" TextDecorations="Underline"/>
//   </FormattedString></Label.FormattedText></Label>
//
// formatted_string + span are bindable_objects (: element), default-constructible, so register_type
// creates them; span's attributes are bindable properties; its <Span> children are FormattedString's
// [ContentProperty] Spans, routed through the child sink. The created formatted_string (boxed as
// shared_ptr<bindable_object>) reaches Label.FormattedText — which expects shared_ptr<formatted_string>
// — via the object-coercion in apply_properties_visitor (xaml_visitors.cpp). label::set_formatted_text
// already subscribes to the formatted_string's `changed` signal, so spans added by the loader AFTER the
// FormattedText assignment re-build the label's attributed text (no view-side fix needed, unlike W7).

#include "register_xaml_groups.hpp"

#include <array>
#include <memory>
#include <string>
#include <string_view>

#include "maui/controls/formatted_string.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/span.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/font_attributes.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_converters.hpp" // enum_entry / parse_enum / xaml_convert_error
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    namespace
    {
        // convert_font_attributes <= Microsoft.Maui.Controls.FontAttributes ([Flags] None/Bold/Italic). A
        // span carries FontAttributes as a direct enum bindable (unlike Label, whose FontAttributes folds
        // into its single font value — register_font_properties parses that inline). No converter existed
        // for the enum, so register one here (reusable wherever a font_attributes property is text-set).
        [[nodiscard]] maui::core::font_attributes convert_font_attributes(std::string_view text)
        {
            using maui::core::font_attributes;
            maui::core::font_attributes result = font_attributes::none;
            std::size_t start = 0;
            while (start <= text.size())
            {
                const std::size_t comma = text.find(',', start);
                const std::size_t end = comma == std::string_view::npos ? text.size() : comma;
                std::string_view token = text.substr(start, end - start);
                while (!token.empty() && token.front() == ' ')
                {
                    token.remove_prefix(1);
                }
                while (!token.empty() && token.back() == ' ')
                {
                    token.remove_suffix(1);
                }
                if (token == "Bold")
                {
                    result |= font_attributes::bold;
                }
                else if (token == "Italic")
                {
                    result |= font_attributes::italic;
                }
                else if (!token.empty() && token != "None")
                {
                    throw xaml_convert_error("Invalid FontAttributes value: " + std::string(token));
                }
                if (comma == std::string_view::npos)
                {
                    break;
                }
                start = comma + 1;
            }
            return result;
        }

        template <class T> [[nodiscard]] auto registry_converter(T (*convert)(std::string_view))
        {
            return [convert](const std::string& text) -> T {
                try
                {
                    return convert(text);
                }
                catch (const xaml_convert_error& error)
                {
                    throw xaml_parse_exception(error.what());
                }
            };
        }
    } // namespace

    void register_xaml_formatted_text(xaml_type_registry& types, xaml_property_registry& properties,
                                      xaml_converter_registry& converters)
    {
        namespace controls = maui::controls;

        // The font_attributes converter (span FontAttributes; reusable). Registered once.
        converters.register_converter<maui::core::font_attributes>(registry_converter(&convert_font_attributes));

        // ---- Span (the attributed run; all properties have existing converters except FontAttributes above) ----
        types.register_type<controls::span>("Span");
        properties.register_bindable_property<controls::span>("Text", controls::span::text_property());
        properties.register_bindable_property<controls::span>("FontFamily", controls::span::font_family_property());
        properties.register_bindable_property<controls::span>("FontSize", controls::span::font_size_property());
        properties.register_bindable_property<controls::span>("FontAttributes",
                                                              controls::span::font_attributes_property());
        properties.register_bindable_property<controls::span>("TextColor", controls::span::text_color_property());
        properties.register_bindable_property<controls::span>("BackgroundColor",
                                                              controls::span::background_color_property());
        properties.register_bindable_property<controls::span>("CharacterSpacing",
                                                              controls::span::character_spacing_property());
        properties.register_bindable_property<controls::span>("TextDecorations",
                                                              controls::span::text_decorations_property());
        properties.register_bindable_property<controls::span>("LineHeight", controls::span::line_height_property());

        // ---- FormattedString ([ContentProperty("Spans")]: each <Span> child is added to the spans
        //      collection. The span is owned by the XAML graph, so add_span takes a NON-OWNING aliasing
        //      shared_ptr (the W7 gradient-stop pattern) — no double free). ----
        types.register_type<controls::formatted_string>("FormattedString");
        properties.register_add_child<controls::formatted_string>(
            [](controls::formatted_string& formatted, maui::core::bindable_object& child) {
                auto* span = dynamic_cast<controls::span*>(&child);
                if (span == nullptr)
                {
                    return false;
                }
                formatted.add_span(std::shared_ptr<controls::span>(std::shared_ptr<void>{}, span));
                return true;
            });

        // ---- Label.FormattedText (object route: the created formatted_string reaches set_formatted_text
        //      via the apply object-coercion). ----
        properties.register_property<controls::label, std::shared_ptr<controls::formatted_string>>(
            "FormattedText", [](controls::label& label, const std::shared_ptr<controls::formatted_string>& value) {
                label.set_formatted_text(value);
            });
    }
} // namespace maui::xaml
