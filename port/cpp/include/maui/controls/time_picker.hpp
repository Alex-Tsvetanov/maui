#pragma once
// maui::controls::time_picker  <=  Microsoft.Maui.Controls.TimePicker
//
// A time-of-day selection control. Ported from src/Controls/src/Core/TimePicker/TimePicker.cs.
//
// Validation semantics (the TimePickerUnitTest.cs oracle): Time defaults to TimeSpan.Zero (NOT null)
// and validates `null || (TotalHours < 24 && TotalMilliseconds >= 0)` — an out-of-range value is
// silently rejected (C# logs a warning and keeps the stored value). time_selected carries (old, new)
// like TimeChangedEventArgs and raises only on a real change.
//
// IsOpen (TimePicker.IsOpenProperty, default false, TwoWay) tracks whether the native dialog is
// visible; a transition raises Opened/Closed (TimePicker.OnIsOpenPropertyChanged). The events fire
// AFTER the value is stored (the property_changed callback runs post-store), so a handler reading
// is_open() sees the new value.
//
// Specificity note (documented deviation, the entry/slider precedent): C#'s explicit ITimePicker.Time
// setter writes at FromHandler; the port's set_time doubles as the developer setter, so it stores at
// manual_value_setter.
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
#include "maui/core/i_time_picker.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class time_picker : public view<maui::core::i_time_picker>
    {
    public:
        time_picker()
        {
            this->set_style_target_type<time_picker>();
        }

        // Shared bindable-property descriptors (one instance per type, like TimePicker.*Property).
        static const maui::core::bindable_property<std::string>& format_property();
        static const maui::core::bindable_property<std::optional<maui::core::time_span>>& time_property();
        static const maui::core::bindable_property<bool>& is_open_property();
        static const maui::core::bindable_property<maui::graphics::color>& text_color_property();
        static const maui::core::bindable_property<maui::core::font>& font_property();
        static const maui::core::bindable_property<double>& character_spacing_property();

        // ---- i_time_picker ----
        [[nodiscard]] std::string_view format() const override
        {
            return format_.get();
        }
        [[nodiscard]] std::optional<maui::core::time_span> time() const override
        {
            return time_.get();
        }
        // The developer setter AND the handler write-back channel (see the specificity note above).
        void set_time(std::optional<maui::core::time_span> value) override
        {
            time_.set(value);
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
        void set_format(std::string value)
        {
            format_.set(std::move(value));
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
        // TimePicker.TimeSelected — (old, new), the TimeChangedEventArgs pair.
        maui::core::event<std::optional<maui::core::time_span>, std::optional<maui::core::time_span>> time_selected;
        maui::core::event<> opened; // TimePicker.Opened
        maui::core::event<> closed; // TimePicker.Closed

    private:
        // The descriptor callbacks (time_picker.cpp) reach the event above.
        friend struct time_picker_descriptor_access;

        // TimePicker.HandleIsOpenChanged: raise Opened when the new value is true, else Closed.
        void on_is_open_changed(bool new_value);

        maui::core::property<std::string> format_{*this, format_property()};
        maui::core::property<std::optional<maui::core::time_span>> time_{*this, time_property()};
        maui::core::property<bool> is_open_{*this, is_open_property()};
        maui::core::property<maui::graphics::color> text_color_{*this, text_color_property()};
        maui::core::property<maui::core::font> font_{*this, font_property()};
        maui::core::property<double> character_spacing_{*this, character_spacing_property()};
    };
} // namespace maui::controls
