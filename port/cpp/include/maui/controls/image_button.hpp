#pragma once
// maui::controls::image_button  <=  Microsoft.Maui.Controls.ImageButton
//
// A button that displays an image and reacts to touch. Ported from ImageButton.cs (+ ButtonElement.cs /
// ImageElement.cs). Same API shape as button + image: bare-noun interface getters + method accessors,
// each backed by a private property<T> whose change flows through view::on_property_changed to the
// handler. The reverse direction is the send_* methods: the handler calls them on native touches and
// the control raises its public events.
//
// Inbound semantics (ButtonElement.ElementPressed/Released/Clicked via IButtonController, the button
// control's exact convention): send_pressed/send_clicked are IsEnabled-gated; send_released ALWAYS
// raises (the ImageButtonTests comment: a press that disables the button must not leave it Pressed).
// is_pressed mirrors the read-only IsPressed. send_clicked runs the optional `command` stand-in before
// raising clicked (the C# command-then-event order); the Pressed VSM state rides send_pressed/released
// (ChangeVisualState goes to Pressed while enabled + pressed).
//
// source: the control OWNS the source (property<shared_ptr<i_image_source>>), i_image_button::source()
// returns the raw borrow — the image control's exact convention. is_loading is the read-only state the
// loader pushes via update_is_loading. is_animation_playing is pinned false
// (C# ImageButton: IImageSourcePart.IsAnimationPlaying => false).
//
// Border naming: C#'s ImageButton spells the stroke surface BorderColor/BorderWidth/CornerRadius and
// forwards them to IButtonStroke.StrokeColor/StrokeThickness/CornerRadius; the port keeps the
// IButtonStroke (stroke_*) spelling the button control already uses (one name per concept).
// corner_radius defaults to 0 (the port button's convention; C#'s -1 "platform default" sentinel has
// no layer analog — documented).
//
// Deferred (OUT OF SCOPE this cut, documented not stubbed): ICommand CanExecute → IsEnabledCore
// coupling, the Controls-layer ImageElement.Measure aspect math (the handler's measure is
// authoritative, as for the image control).

#include <memory>
#include <string>
#include <utility>

#include "maui/controls/view.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_image_button.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class image_button : public view<maui::core::i_image_button>
    {
    public:
        // Declare the style TargetType so an implicit / class style targeting `image_button` matches.
        image_button()
        {
            this->set_style_target_type<image_button>();
        }

        // Shared bindable-property descriptors (one instance per type, like ImageButton.*Property).
        static const maui::core::bindable_property<maui::core::aspect>& aspect_property();
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& source_property();
        static const maui::core::bindable_property<bool>& is_opaque_property();
        static const maui::core::bindable_property<bool>& is_loading_property();
        static const maui::core::bindable_property<maui::core::thickness>& padding_property();
        static const maui::core::bindable_property<maui::graphics::color>& stroke_color_property();
        static const maui::core::bindable_property<double>& stroke_thickness_property();
        static const maui::core::bindable_property<int>& corner_radius_property();

        // ---- the IImage surface ----
        [[nodiscard]] maui::core::aspect aspect() const override
        {
            return aspect_.get();
        }
        [[nodiscard]] maui::core::i_image_source* source() const override
        {
            return source_.get().get();
        }
        [[nodiscard]] bool is_opaque() const override
        {
            return is_opaque_.get();
        }
        // C# ImageButton: IImageSourcePart.IsAnimationPlaying => false (constant — no bindable).
        [[nodiscard]] bool is_animation_playing() const override
        {
            return false;
        }
        // The loader pushes its in-flight state here (drives the read-only IsLoading).
        void update_is_loading(bool is_loading) override
        {
            is_loading_.set(is_loading);
        }
        [[nodiscard]] bool is_loading() const
        {
            return is_loading_.get();
        }

        // ---- i_padding / i_button_stroke ----
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

        // ---- public setters (drive the handler via on_property_changed → update_value) ----
        void set_aspect(maui::core::aspect value)
        {
            aspect_.set(value);
        }
        // The control takes ownership of the source. Passing a distinct instance fires the change.
        void set_source(std::shared_ptr<maui::core::i_image_source> value)
        {
            source_.set(std::move(value));
        }
        void set_is_opaque(bool value)
        {
            is_opaque_.set(value);
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

        // ---- i_button inbound channel (the button control's exact convention) ----
        void send_pressed() override
        {
            if (!is_enabled())
            {
                return;
            }
            is_pressed_ = true;
            change_visual_state_for_pressed();
            pressed.raise();
        }
        void send_released() override
        {
            // Released always fires, even disabled — otherwise a press that disables the button would
            // leave it Pressed forever (the ImageButtonTests comment, ported faithfully).
            is_pressed_ = false;
            this->change_visual_state();
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
        // ImageButton.ChangeVisualState: enabled + pressed goes to the Pressed state (the
        // ButtonElement.PressedVisualState drive); everything else falls back to the common drive.
        void change_visual_state_for_pressed()
        {
            if (is_enabled() && is_pressed_)
            {
                this->visual_states().go_to_state(*this, maui::controls::common_states::pressed);
            }
            else
            {
                this->change_visual_state();
            }
        }

        maui::core::property<maui::core::aspect> aspect_{*this, aspect_property()};
        maui::core::property<std::shared_ptr<maui::core::i_image_source>> source_{*this, source_property()};
        maui::core::property<bool> is_opaque_{*this, is_opaque_property()};
        maui::core::property<bool> is_loading_{*this, is_loading_property()};
        maui::core::property<maui::core::thickness> padding_{*this, padding_property()};
        maui::core::property<maui::graphics::color> stroke_color_{*this, stroke_color_property()};
        maui::core::property<double> stroke_thickness_{*this, stroke_thickness_property()};
        maui::core::property<int> corner_radius_{*this, corner_radius_property()};
        bool is_pressed_ = false;
    };
} // namespace maui::controls
