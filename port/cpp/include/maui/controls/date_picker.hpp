#pragma once
// maui::controls::date_picker  <=  Microsoft.Maui.Controls.DatePicker
//
// A calendar-date selection control. Ported from src/Controls/src/Core/DatePicker/DatePicker.cs.
//
// Clamping semantics (the DatePickerUnitTest.cs oracle):
//   - Date coerces on every set: truncate to .Date, clamp to MaximumDate, then MinimumDate (the C#
//     CoerceDate order); a null Date stays null (C#'s lifted comparisons answer false on null).
//   - Minimum/MaximumDate validate against the OTHER bound (invalid values are silently rejected,
//     like C#'s logged warning) and coerce by truncating to .Date AND clamping the current Date from
//     INSIDE the coercion — so the "date" change notifies before the bound's own (the event-order
//     test).
//   - Date defaults via a defaultValueCreator to DateTime.Today (materialized on first read).
//
// date_selected carries (old, new) like DateChangedEventArgs; it raises only on a real change.
//
// IsOpen (DatePicker.IsOpenProperty, default false, TwoWay) tracks whether the native dialog is
// visible; a transition raises Opened/Closed (DatePicker.OnIsOpenPropertyChanged). The events fire
// AFTER the value is stored (the property_changed callback runs post-store), so a handler reading
// is_open() sees the new value.
//
// Specificity note (documented deviation, the entry/slider precedent): C#'s explicit IDatePicker
// setters write at FromHandler; the port's set_date/set_format double as the developer setters, so
// they store at manual_value_setter.
//
// Deferred (documented, not stubbed): TextTransform.

#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/event.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_date_picker.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class date_picker : public view<maui::core::i_date_picker>
    {
    public:
        date_picker()
        {
            this->set_style_target_type<date_picker>();
        }

        // Shared bindable-property descriptors (one instance per type, like DatePicker.*Property).
        static const maui::core::bindable_property<std::string>& format_property();
        static const maui::core::bindable_property<std::optional<maui::core::date_time>>& date_property();
        static const maui::core::bindable_property<std::optional<maui::core::date_time>>& minimum_date_property();
        static const maui::core::bindable_property<std::optional<maui::core::date_time>>& maximum_date_property();
        static const maui::core::bindable_property<bool>& is_open_property();
        static const maui::core::bindable_property<maui::graphics::color>& text_color_property();
        static const maui::core::bindable_property<maui::core::font>& font_property();
        static const maui::core::bindable_property<double>& character_spacing_property();

        // ---- i_date_picker ----
        [[nodiscard]] std::string_view format() const override
        {
            return format_.get();
        }
        void set_format(std::string_view value) override
        {
            format_.set(std::string(value));
        }
        [[nodiscard]] std::optional<maui::core::date_time> date() const override
        {
            return date_.get();
        }
        // The developer setter AND the handler write-back channel (see the specificity note above).
        void set_date(std::optional<maui::core::date_time> value) override
        {
            date_.set(value);
        }
        [[nodiscard]] std::optional<maui::core::date_time> minimum_date() const override
        {
            return minimum_date_.get();
        }
        [[nodiscard]] std::optional<maui::core::date_time> maximum_date() const override
        {
            return maximum_date_.get();
        }
        [[nodiscard]] bool is_open() const override
        {
            return is_open_.get();
        }
        // The developer setter AND the handler write-back channel (editing-begin/end on iOS).
        void set_is_open(bool value) override
        {
            is_open_.set(value);
        }

        // ---- i_text_style ----
        [[nodiscard]] maui::graphics::color text_color() const override
        {
            return text_color_.get();
        }
        [[nodiscard]] maui::core::font font() const override
        {
            return font_.get();
        }
        [[nodiscard]] double character_spacing() const override
        {
            return character_spacing_.get();
        }

        // ---- public setters (drive the handler via on_property_changed → update_value) ----
        void set_minimum_date(std::optional<maui::core::date_time> value)
        {
            minimum_date_.set(value);
        }
        void set_maximum_date(std::optional<maui::core::date_time> value)
        {
            maximum_date_.set(value);
        }
        void set_text_color(maui::graphics::color value)
        {
            text_color_.set(value);
        }
        void set_font(maui::core::font value)
        {
            font_.set(std::move(value));
        }
        void set_character_spacing(double value)
        {
            character_spacing_.set(value);
        }

        // ---- developer-facing events ----
        // DatePicker.DateSelected — (old, new), the DateChangedEventArgs pair.
        maui::core::event<std::optional<maui::core::date_time>, std::optional<maui::core::date_time>> date_selected;
        maui::core::event<> opened; // DatePicker.Opened
        maui::core::event<> closed; // DatePicker.Closed

    private:
        // The descriptor callbacks (date_picker.cpp) reach the typed properties below.
        friend struct date_picker_descriptor_access;

        // DatePicker.HandleIsOpenChanged: raise Opened when the new value is true, else Closed.
        void on_is_open_changed(bool new_value) const;

        maui::core::property<std::string> format_{*this, format_property()};
        maui::core::property<std::optional<maui::core::date_time>> date_{*this, date_property()};
        maui::core::property<std::optional<maui::core::date_time>> minimum_date_{*this, minimum_date_property()};
        maui::core::property<std::optional<maui::core::date_time>> maximum_date_{*this, maximum_date_property()};
        maui::core::property<bool> is_open_{*this, is_open_property()};
        maui::core::property<maui::graphics::color> text_color_{*this, text_color_property()};
        maui::core::property<maui::core::font> font_{*this, font_property()};
        maui::core::property<double> character_spacing_{*this, character_spacing_property()};
    };
} // namespace maui::controls
