// maui::controls::date_picker — out-of-line definitions: the shared bindable-property descriptors
// (DatePicker.*Property — the CoerceDate / CoerceMinimumDate / CoerceMaximumDate / Validate* logic,
// 1:1 from DatePicker.cs) and the default-handler self-registration.

#include "maui/controls/date_picker.hpp"

#include <optional>
#include <string>

#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/date_picker_handler.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    namespace
    {
        using opt_date = std::optional<maui::core::date_time>;

        // DateTime?.Date — truncate the time-of-day, propagating null.
        opt_date truncated(const opt_date& value)
        {
            return value ? opt_date(value->date()) : std::nullopt;
        }
    } // namespace

    // Grants the descriptor callbacks access to the typed properties (the C# analog is the static
    // coerceValue/validateValue delegates living inside the DatePicker class itself).
    struct date_picker_descriptor_access
    {
        // DatePicker.CoerceDate: truncate, clamp to MaximumDate then MinimumDate. C#'s lifted
        // comparisons answer false when either side is null, so a null date stays null.
        static opt_date coerce_date(date_picker& self, const opt_date& value)
        {
            opt_date date_value = truncated(value);
            const opt_date max = self.maximum_date_.get();
            if (date_value && max && *date_value > *max)
            {
                date_value = max;
            }
            const opt_date min = self.minimum_date_.get();
            if (date_value && min && *date_value < *min)
            {
                date_value = min;
            }
            return date_value;
        }

        // DatePicker.CoerceMaximumDate: truncate; clamp the current Date down from INSIDE the
        // coercion (so the "date" change notifies before "maximum_date" — the event-order oracle).
        static opt_date coerce_maximum_date(date_picker& self, const opt_date& value)
        {
            const opt_date date_value = truncated(value);
            const opt_date current = self.date_.get();
            if (current && date_value && *current > *date_value)
            {
                self.set_date(date_value);
            }
            return date_value;
        }

        // DatePicker.CoerceMinimumDate — the mirror image.
        static opt_date coerce_minimum_date(date_picker& self, const opt_date& value)
        {
            const opt_date date_value = truncated(value);
            const opt_date current = self.date_.get();
            if (current && date_value && *current < *date_value)
            {
                self.set_date(date_value);
            }
            return date_value;
        }

        // DatePicker.ValidateMaximumDate: a null candidate or a null opposing bound is always valid.
        static bool validate_maximum_date(const date_picker& self, const opt_date& value)
        {
            const opt_date min = truncated(self.minimum_date_.get());
            if (!value || !min)
            {
                return true;
            }
            return value->date() >= *min;
        }

        static bool validate_minimum_date(const date_picker& self, const opt_date& value)
        {
            const opt_date max = truncated(self.maximum_date_.get());
            if (!value || !max)
            {
                return true;
            }
            return value->date() <= *max;
        }

        static void raise_date_selected(date_picker& self, const opt_date& old_value, const opt_date& new_value)
        {
            self.date_selected.raise(old_value, new_value);
        }

        // DatePicker.OnIsOpenPropertyChanged → HandleIsOpenChanged (raise Opened/Closed by transition).
        static void on_is_open_changed(date_picker& self, bool new_value)
        {
            self.on_is_open_changed(new_value);
        }
    };

    // DatePicker.FormatProperty: default "d" (the short-date pattern).
    const maui::core::bindable_property<std::string>& date_picker::format_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"format", std::string{"d"}};
        return descriptor;
    }

    // DatePicker.DateProperty: default null with a DateTime.Today defaultValueCreator, TwoWay,
    // coerced (truncate + clamp); a change raises DateSelected(old, new).
    const maui::core::bindable_property<std::optional<maui::core::date_time>>& date_picker::date_property()
    {
        static const maui::core::bindable_property<opt_date> descriptor{
            "date",
            std::nullopt,
            {.property_changed =
                 [](maui::core::bindable_object& bindable, const opt_date& old_value, const opt_date& new_value) {
                     date_picker_descriptor_access::raise_date_selected(dynamic_cast<date_picker&>(bindable), old_value,
                                                                        new_value);
                 },
             .coerce_value =
                 [](maui::core::bindable_object& bindable, const opt_date& value) {
                     return date_picker_descriptor_access::coerce_date(dynamic_cast<date_picker&>(bindable), value);
                 },
             .default_value_creator =
                 [](const maui::core::bindable_object&) { return opt_date(maui::core::date_time::today()); },
             .default_binding_mode = maui::core::binding_mode::two_way}};
        return descriptor;
    }

    // DatePicker.MinimumDateProperty: default 1900-01-01, validated against MaximumDate, coerced
    // (truncate + clamp the current Date up).
    const maui::core::bindable_property<std::optional<maui::core::date_time>>& date_picker::minimum_date_property()
    {
        static const maui::core::bindable_property<opt_date> descriptor{
            "minimum_date",
            opt_date(maui::core::date_time(1900, 1, 1)),
            {.coerce_value =
                 [](maui::core::bindable_object& bindable, const opt_date& value) {
                     return date_picker_descriptor_access::coerce_minimum_date(dynamic_cast<date_picker&>(bindable),
                                                                               value);
                 },
             .validate_value =
                 [](maui::core::bindable_object& bindable, const opt_date& value) {
                     return date_picker_descriptor_access::validate_minimum_date(dynamic_cast<date_picker&>(bindable),
                                                                                 value);
                 }}};
        return descriptor;
    }

    // DatePicker.MaximumDateProperty: default 2100-12-31 — the mirror image.
    const maui::core::bindable_property<std::optional<maui::core::date_time>>& date_picker::maximum_date_property()
    {
        static const maui::core::bindable_property<opt_date> descriptor{
            "maximum_date",
            opt_date(maui::core::date_time(2100, 12, 31)),
            {.coerce_value =
                 [](maui::core::bindable_object& bindable, const opt_date& value) {
                     return date_picker_descriptor_access::coerce_maximum_date(dynamic_cast<date_picker&>(bindable),
                                                                               value);
                 },
             .validate_value =
                 [](maui::core::bindable_object& bindable, const opt_date& value) {
                     return date_picker_descriptor_access::validate_maximum_date(dynamic_cast<date_picker&>(bindable),
                                                                                 value);
                 }}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& date_picker::text_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"text_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::font>& date_picker::font_property()
    {
        static const maui::core::bindable_property<maui::core::font> descriptor{"font"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& date_picker::character_spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{"character_spacing", 0.0};
        return descriptor;
    }

    // DatePicker.IsOpenProperty: default false, TwoWay; a change raises Opened/Closed by transition.
    const maui::core::bindable_property<bool>& date_picker::is_open_property()
    {
        static const maui::core::bindable_property<bool> descriptor{
            "is_open",
            false,
            {.property_changed =
                 [](maui::core::bindable_object& bindable, const bool&, const bool& new_value) {
                     date_picker_descriptor_access::on_is_open_changed(dynamic_cast<date_picker&>(bindable), new_value);
                 },
             .default_binding_mode = maui::core::binding_mode::two_way}};
        return descriptor;
    }

    void date_picker::on_is_open_changed(bool new_value) const
    {
        // DatePicker.HandleIsOpenChanged: the value is already stored, so raise Opened when it turned
        // true and Closed when it turned false (a handler reading is_open() observes the transition).
        if (new_value)
        {
            opened.raise();
        }
        else
        {
            closed.raise();
        }
    }
} // namespace maui::controls

// Self-register the default handler for date_picker (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::date_picker, maui::core::date_picker_handler)
