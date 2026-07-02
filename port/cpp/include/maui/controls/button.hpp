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
//
// ImageSource + ContentLayout (Button.cs) are added NARROW: Button stays an i_text_button (it is NOT
// widened to i_image — PROFILE.md forbids the diamond i_text + i_image would create). The control owns
// the ImageSource and the handler maps it through a handler-owned async image_source_loader (the
// ImageButtonMapper keyed on the source); ContentLayout is stored + pushed (change → re-measure in C#),
// but the text+image composition is deferred (no container infrastructure on any backend yet).

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/button_content_layout.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_button.hpp"
#include "maui/core/i_image_source.hpp"
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
        // Button.ImageSourceProperty (= ImageElement.ImageSourceProperty, default null) and
        // Button.ContentLayoutProperty (default {Left, DefaultSpacing}). NARROW image support: the control
        // OWNS the source as a property<shared_ptr<i_image_source>> (like image.hpp) so a change re-runs the
        // handler's image mapper; ContentLayout is stored + pushed (change → InvalidateMeasureInternal in C#),
        // but the actual text+image composition is deferred (no container infra). Button is NOT widened to
        // i_image — see button_handler.hpp (the ImageButtonMapper is keyed on the source via the handler).
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>&
        image_source_property();
        static const maui::core::bindable_property<button_content_layout>& content_layout_property();

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

        // ---- image surface (NARROW: own the source, read it through the handler's image mapper) ----
        // Raw borrow into the owned shared_ptr (null when unset) — the i_text_button::image_source() read
        // path the handler's image mapper calls (C# IImageSourcePart.Source => ImageSource). The control
        // retains ownership.
        [[nodiscard]] maui::core::i_image_source* image_source() const override
        {
            return image_source_.get().get();
        }
        [[nodiscard]] button_content_layout content_layout() const
        {
            return content_layout_.get();
        }
        // The i_text_button seam the iOS get_desired_size reads to compose the image+title (Left/Right →
        // width, Top/Bottom → height) — the measure-relevant projection of ContentLayout. image_position
        // mirrors ButtonContentLayout.ImagePosition {Left, Top, Right, Bottom} 1:1 by ordinal, so we convert
        // by ordinal. (See i_text_button::content_layout_spec — NARROW, like image_source().)
        [[nodiscard]] maui::core::button_content_spec content_layout_spec() const override
        {
            const button_content_layout layout = content_layout_.get();
            return {static_cast<maui::core::button_content_spec::image_position>(static_cast<int>(layout.position)),
                    layout.spacing};
        }
        // C# Button.IImageSourcePart.UpdateIsLoading (Button.cs:499-505) — the handler/loader pushes the
        // in-flight loading state through the i_text_button seam (overrides the defaulted no-op there;
        // narrow approach — Button is not widened to i_image). Unlike ImageButton, Button has NO public
        // IsLoading (C# Button.IImageElement.IsLoading => false); the only effect is the re-measure on a load
        // FINISH: when the previous push was loading and this one is not, re-push ContentLayout (C# does
        // Handler?.UpdateValue(nameof(ContentLayout))) so the text+image composition re-measures. The
        // re-measure itself is deferred here (no container infra) — the UpdateValue call is the faithful seam.
        void update_is_loading(bool loading) override
        {
            if (!loading && was_image_loading_ && handler())
            {
                handler()->update_value("content_layout");
            }
            was_image_loading_ = loading;
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
        // The control takes ownership of the source (a distinct instance fires the change → the image
        // mapper re-runs). Mirrors image.hpp::set_source / C# Button.ImageSource setter.
        void set_image_source(std::shared_ptr<maui::core::i_image_source> value)
        {
            image_source_.set(std::move(value));
        }
        void set_content_layout(button_content_layout value)
        {
            content_layout_.set(value);
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
        maui::core::property<std::shared_ptr<maui::core::i_image_source>> image_source_{*this, image_source_property()};
        maui::core::property<button_content_layout> content_layout_{*this, content_layout_property()};
        bool is_pressed_ = false;
        // C# Button._wasImageLoading — tracks the prior loading state so a load FINISH triggers the
        // ContentLayout re-push (see update_is_loading). Not exposed; purely internal.
        bool was_image_loading_ = false;
    };
} // namespace maui::controls
