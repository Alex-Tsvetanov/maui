#pragma once
// maui::controls::button  <=  Microsoft.Maui.Controls.Button (M2 subset)
//
// The first concrete, developer-facing control — the Rosetta Stone's virtual view. A text-bearing
// button (ITextButton: it is both an i_button and an i_text). Ported from Button.cs + ButtonElement.cs.
//
// API shape (per the M2 decision): the virtual-view interfaces stay bare-noun method getters
// (text(), is_enabled(), …) and the public control API is method accessors — text()/set_text(),
// is_enabled()/set_is_enabled() — each backed by a private property<T> that carries the value
// precedence + change notification. A property change flows through view::on_property_changed to the
// handler (update_value), which re-runs the mapper (virtual→native). The reverse direction is the
// send_* methods: the handler calls them on a native tap and the control raises its public events.
//
// M2 first cut: Text is the live bindable property (mapped to the native title). text_color / font /
// character_spacing / padding / stroke are present (to satisfy the i_text / i_button surface) with
// plain defaults; they become bindable + mapped in later M2/M4 cuts (documented in STATUS).

#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_button.hpp"
#include "maui/core/i_text.hpp"
#include "maui/core/i_text_button.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class button : public view<maui::core::i_text_button>
    {
    public:
        // The shared descriptor for the Text bindable property (Button.TextProperty).
        static const maui::core::bindable_property<std::string>& text_property();

        // ---- i_text / i_text_style getters (read by the handler's mapper) ----
        [[nodiscard]] std::string_view text() const override
        {
            return text_.get();
        }
        [[nodiscard]] maui::graphics::color text_color() const override
        {
            return text_color_;
        }
        [[nodiscard]] maui::core::font font() const override
        {
            return font_;
        }
        [[nodiscard]] double character_spacing() const override
        {
            return character_spacing_;
        }

        // ---- public setters (drive the handler via on_property_changed) ----
        void set_text(std::string value)
        {
            text_.set(std::move(value));
        }

        // ---- i_padding / i_button_stroke (defaults for the M2 cut) ----
        [[nodiscard]] maui::core::thickness padding() const override
        {
            return padding_;
        }
        [[nodiscard]] maui::graphics::color stroke_color() const override
        {
            return stroke_color_;
        }
        [[nodiscard]] double stroke_thickness() const override
        {
            return stroke_thickness_;
        }
        [[nodiscard]] int corner_radius() const override
        {
            return corner_radius_;
        }

        // ---- i_button inbound channel (called by the handler on native touch events) ----
        // Mirrors ButtonElement.ElementPressed/Released/Clicked, including the IsEnabled gating and the
        // command-before-event order; Released always clears IsPressed (even when disabled).
        void send_pressed() override
        {
            if (!is_enabled())
            {
                return;
            }
            is_pressed_ = true;
            pressed.raise();
        }
        void send_released() override
        {
            is_pressed_ = false;
            released.raise();
        }
        void send_clicked() override
        {
            if (!is_enabled())
            {
                return;
            }
            if (command)
            {
                command();
            }
            clicked.raise();
        }

        [[nodiscard]] bool is_pressed() const
        {
            return is_pressed_;
        }

        // ---- developer-facing events + command (the outbound channel) ----
        maui::core::event<> clicked;
        maui::core::event<> pressed;
        maui::core::event<> released;
        maui::core::move_only_function<void()> command;

    private:
        maui::core::property<std::string> text_{*this, text_property()};
        maui::graphics::color text_color_;
        maui::core::font font_;
        double character_spacing_ = 0;
        maui::core::thickness padding_;
        maui::graphics::color stroke_color_;
        double stroke_thickness_ = 0;
        int corner_radius_ = 0;
        bool is_pressed_ = false;
    };
} // namespace maui::controls
