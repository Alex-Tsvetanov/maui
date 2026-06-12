#pragma once
// maui::controls::radio_button  <=  Microsoft.Maui.Controls.RadioButton
//
// A button selected from a group where only one can be checked at a time. Ported from RadioButton.cs
// (+ RadioButtonGroup.cs / RadioButtonGroupController.cs — see radio_button_group.hpp). Same API shape
// as check_box: bare-noun interface getters + method accessors, each backed by a private property<T>
// whose change flows through view::on_property_changed to the handler; send_is_checked is the INBOUND
// write the handler performs on a native tap (stored at the from-handler specificity, like C#
// RadioButton's `IRadioButton.IsChecked` explicit implementation).
//
// IsChecked semantics (OnIsCheckedPropertyChanged, ported faithfully): checking a button first runs
// the group's mutual exclusion (RadioButtonGroup.UpdateRadioButtonGroup — uncheck the others in
// scope, then record the selection on the group controller), then drives the Checked/Unchecked visual
// state, then raises checked_changed with the new value. Unchecking only drives the state + event.
//
// Value is C#'s `object Value` — the port's boxed std::any, compared via maui::core::boxed_equals.
// DEVIATION (documented): Value is a plain member, not a property<T> — the typed property engine
// requires operator== on T, which std::any lacks; set_value performs the same change-detection +
// propertyChanged flow (OnValuePropertyChanged) by hand. It is therefore not bindable/stylable yet.
//
// Content (documented deviation — the STRING content path): C#'s `object Content` renders a string
// natively and a View through the ControlTemplate machinery. The port's cut is the native string path
// only — content is a std::string pushed to the native control's title; View content + DefaultTemplate
// (+ ContentAsString's View warning) are deferred to the templates layer. corner_radius defaults to 0
// (the port button's convention; C#'s -1 "platform default" sentinel has no layer analog).
//
// Deferred (OUT OF SCOPE this cut, documented not stubbed): TextTransform + FontAttributes/AutoScaling
// (no TextTransform/FontElement subsystem in the port's font yet — font carries family/size/weight),
// the ControlTemplate path above, and the IBorderElement compatibility face.

#include <any>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/radio_button_group.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/boxed_value.hpp"
#include "maui/core/event.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_radio_button.hpp"
#include "maui/core/property.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class radio_button : public view<maui::core::i_radio_button>
    {
    public:
        // The framework-driven visual state names (RadioButton.CheckedVisualState/UncheckedVisualState).
        static constexpr std::string_view checked_visual_state = "Checked";
        static constexpr std::string_view unchecked_visual_state = "Unchecked";

        radio_button()
        {
            this->set_style_target_type<radio_button>();
        }

        // Shared bindable-property descriptors (one instance per type, like RadioButton.*Property).
        static const maui::core::bindable_property<bool>& is_checked_property();
        static const maui::core::bindable_property<std::string>& group_name_property();
        static const maui::core::bindable_property<std::string>& content_property();
        static const maui::core::bindable_property<maui::graphics::color>& text_color_property();
        static const maui::core::bindable_property<maui::core::font>& font_property();
        static const maui::core::bindable_property<double>& character_spacing_property();
        static const maui::core::bindable_property<maui::graphics::color>& stroke_color_property();
        static const maui::core::bindable_property<double>& stroke_thickness_property();
        static const maui::core::bindable_property<int>& corner_radius_property();

        // ---- the developer-facing surface ----
        [[nodiscard]] bool is_checked() const override
        {
            return is_checked_.get();
        }
        // The manual (developer) setter — C# RadioButton.IsChecked's class property (TwoWay).
        void set_is_checked(bool value)
        {
            is_checked_.set(value);
        }

        [[nodiscard]] const std::string& group_name() const
        {
            return group_name_.get();
        }
        void set_group_name(std::string value)
        {
            group_name_.set(std::move(value));
        }

        // C# `object Value`, boxed (see the header note). set_value hand-rolls the bindable change flow.
        [[nodiscard]] const std::any& value() const
        {
            return value_;
        }
        void set_value(std::any value)
        {
            if (maui::core::boxed_equals(value_, value))
            {
                return;
            }
            value_ = std::move(value);
            on_value_changed();
        }

        [[nodiscard]] const std::string& content() const
        {
            return content_.get();
        }
        void set_content(std::string value)
        {
            content_.set(std::move(value));
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

        // ---- i_button_stroke (C# spells these BorderColor/BorderWidth/CornerRadius and forwards to
        // IButtonStroke; the port keeps the IButtonStroke spelling — the button control's convention) ----
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

        // ---- i_radio_button (read by the handler's mapper / written by the native tap) ----
        // The inbound native channel: stores at the from-handler specificity (C# SelectRadioButton →
        // SetValue(IsCheckedProperty, ..., SetterSpecificity.FromHandler)).
        void send_is_checked(bool value) override
        {
            is_checked_.set(value, maui::core::setter_specificity::from_handler);
        }
        [[nodiscard]] std::string_view content_as_string() const override
        {
            return content_.get();
        }

        // Occurs when IsChecked changes (raised with the new value, like C#'s
        // CheckedChanged(CheckedChangedEventArgs)).
        maui::core::event<bool> checked_changed;

        // RadioButton.ChangeVisualState: apply the IsChecked state (Checked/Unchecked) first
        // (ApplyIsCheckedState), then the base common states.
        void change_visual_state() override
        {
            this->visual_states().go_to_state(*this, is_checked() ? checked_visual_state : unchecked_visual_state);
            view::change_visual_state();
        }

        // The group association (the C# groupControllers ConditionalWeakTable entry, held per button).
        // Weak: the controller is owned by its container element, never by the buttons it manages.
        [[nodiscard]] std::shared_ptr<radio_button_group_controller> group_controller() const
        {
            return group_controller_.lock();
        }
        void set_group_controller(const std::shared_ptr<radio_button_group_controller>& value)
        {
            group_controller_ = value;
        }

    protected:
        // The DescendantAdded translation (see radio_button_group.hpp): whenever this button's ancestor
        // chain changes, re-resolve the nearest named group controller up the tree and run its
        // AddRadioButton semantics; detached from every grouped ancestor, the association is dropped
        // (the DescendantRemoved analog). Defined in radio_button.cpp (needs the element walk).
        void on_resource_chain_changed() override;

    private:
        // OnIsCheckedPropertyChanged / OnValuePropertyChanged / OnGroupNamePropertyChanged — wired as the
        // descriptors' property_changed callbacks (radio_button.cpp), exactly like C#'s static delegates.
        void on_is_checked_changed(bool is_checked);
        void on_value_changed();
        void on_group_name_changed(const std::string& old_group_name, const std::string& new_group_name);
        friend struct radio_button_descriptor_access;

        maui::core::property<bool> is_checked_{*this, is_checked_property()};
        maui::core::property<std::string> group_name_{*this, group_name_property()};
        maui::core::property<std::string> content_{*this, content_property()};
        maui::core::property<maui::graphics::color> text_color_{*this, text_color_property()};
        maui::core::property<maui::core::font> font_{*this, font_property()};
        maui::core::property<double> character_spacing_{*this, character_spacing_property()};
        maui::core::property<maui::graphics::color> stroke_color_{*this, stroke_color_property()};
        maui::core::property<double> stroke_thickness_{*this, stroke_thickness_property()};
        maui::core::property<int> corner_radius_{*this, corner_radius_property()};
        std::any value_; // C# ValueProperty (see the deviation note above)
        std::weak_ptr<radio_button_group_controller> group_controller_;
    };
} // namespace maui::controls
