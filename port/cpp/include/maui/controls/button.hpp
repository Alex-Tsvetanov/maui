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
// All of the button's own surface is bindable + mapped: Text, the i_text_style appearance (TextColor,
// Font, CharacterSpacing), Padding, and the i_button_stroke border (StrokeColor, StrokeThickness,
// CornerRadius). The generic IView properties (Visibility/Opacity/transforms/…) gain their shared
// ViewMapper at M3/M4 with the visual-element + layout work.

#include <functional>
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
        // Declare the style TargetType so an implicit (TargetType-keyed) style or a class style targeting
        // `button` matches this control (VisualElement's implicit-style resolution). The inline property
        // member initializers still run under this user-declared constructor.
        // The named-event channels (W1-15) let an event_trigger subscribe Clicked/Pressed/Released by
        // name — the reflection-free seam replacing C#'s GetRuntimeEvent (see element.hpp).
        button()
        {
            this->set_style_target_type<button>();
            this->register_named_event("clicked", [this](std::function<void()> handler) {
                return maui::core::connect_scoped(clicked, std::move(handler));
            });
            this->register_named_event("pressed", [this](std::function<void()> handler) {
                return maui::core::connect_scoped(pressed, std::move(handler));
            });
            this->register_named_event("released", [this](std::function<void()> handler) {
                return maui::core::connect_scoped(released, std::move(handler));
            });
        }

        // Shared bindable-property descriptors (one instance per type, like Button.*Property).
        static const maui::core::bindable_property<std::string>& text_property();
        static const maui::core::bindable_property<maui::graphics::color>& text_color_property();
        static const maui::core::bindable_property<maui::core::font>& font_property();
        static const maui::core::bindable_property<double>& character_spacing_property();
        static const maui::core::bindable_property<maui::core::thickness>& padding_property();
        static const maui::core::bindable_property<maui::graphics::color>& stroke_color_property();
        static const maui::core::bindable_property<double>& stroke_thickness_property();
        static const maui::core::bindable_property<int>& corner_radius_property();

        // ---- i_text / i_text_style getters (read by the handler's mapper) ----
        [[nodiscard]] std::string_view text() const override
        {
            return text_.get();
        }
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

        // ---- i_padding / i_button_stroke getters ----
        [[nodiscard]] maui::core::thickness padding() const override
        {
            return padding_.get();
        }
        [[nodiscard]] maui::graphics::color stroke_color() const override
        {
            return stroke_color_.get();
        }
        [[nodiscard]] double stroke_thickness() const override
        {
            return stroke_thickness_.get();
        }
        [[nodiscard]] int corner_radius() const override
        {
            return corner_radius_.get();
        }

        // ---- public setters (each drives the handler via on_property_changed → update_value) ----
        void set_text(std::string value)
        {
            text_.set(std::move(value));
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
        void set_padding(maui::core::thickness value)
        {
            padding_.set(value);
        }
        void set_stroke_color(maui::graphics::color value)
        {
            stroke_color_.set(value);
        }
        void set_stroke_thickness(double value)
        {
            stroke_thickness_.set(value);
        }
        void set_corner_radius(int value)
        {
            corner_radius_.set(value);
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
        maui::core::property<maui::graphics::color> text_color_{*this, text_color_property()};
        maui::core::property<maui::core::font> font_{*this, font_property()};
        maui::core::property<double> character_spacing_{*this, character_spacing_property()};
        maui::core::property<maui::core::thickness> padding_{*this, padding_property()};
        maui::core::property<maui::graphics::color> stroke_color_{*this, stroke_color_property()};
        maui::core::property<double> stroke_thickness_{*this, stroke_thickness_property()};
        maui::core::property<int> corner_radius_{*this, corner_radius_property()};
        bool is_pressed_ = false;
    };
} // namespace maui::controls
