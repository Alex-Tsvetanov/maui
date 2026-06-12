// maui::controls::time_picker — out-of-line definitions: the shared bindable-property descriptors
// (TimePicker.*Property — the [0, 24h) validateValue + the TimeSelected propertyChanged, 1:1 from
// TimePicker.cs) and the default-handler self-registration.

#include "maui/controls/time_picker.hpp"

#include <optional>
#include <string>

#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/graphics/color.hpp"
#include "maui/core/time_picker_handler.hpp"

namespace maui::controls
{
    namespace
    {
        using opt_time = std::optional<maui::core::time_span>;
    } // namespace

    // Grants the descriptor callbacks access to the event (the C# analog is the static delegates
    // living inside the TimePicker class itself).
    struct time_picker_descriptor_access
    {
        static void raise_time_selected(time_picker& self, const opt_time& old_value, const opt_time& new_value)
        {
            self.time_selected.raise(old_value, new_value);
        }
    };

    // TimePicker.FormatProperty: default "t" (the short-time pattern).
    const maui::core::bindable_property<std::string>& time_picker::format_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"format", std::string{"t"}};
        return descriptor;
    }

    // TimePicker.TimeProperty: default TimeSpan.Zero (not null), TwoWay, validated to
    // `null || (TotalHours < 24 && TotalMilliseconds >= 0)`; a change raises TimeSelected(old, new).
    const maui::core::bindable_property<std::optional<maui::core::time_span>>& time_picker::time_property()
    {
        static const maui::core::bindable_property<opt_time> descriptor{
            "time",
            opt_time(maui::core::time_span{}),
            {.property_changed =
                 [](maui::core::bindable_object& bindable, const opt_time& old_value, const opt_time& new_value) {
                     time_picker_descriptor_access::raise_time_selected(dynamic_cast<time_picker&>(bindable), old_value,
                                                                        new_value);
                 },
             .validate_value =
                 [](maui::core::bindable_object&, const opt_time& value) {
                     return !value || (value->total_hours() < 24.0 && value->total_milliseconds() >= 0.0);
                 },
             .default_binding_mode = maui::core::binding_mode::two_way}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& time_picker::text_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"text_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::font>& time_picker::font_property()
    {
        static const maui::core::bindable_property<maui::core::font> descriptor{"font"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& time_picker::character_spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{"character_spacing", 0.0};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler for time_picker (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::time_picker, maui::core::time_picker_handler)
