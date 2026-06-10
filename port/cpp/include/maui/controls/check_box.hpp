#pragma once
// maui::controls::check_box  <=  Microsoft.Maui.Controls.CheckBox
//
// A control the user can select or clear. Ported from src/Controls/src/Core/CheckBox/CheckBox.cs
// (+ CheckBox.Mapper.cs).
//
// Surface: IsChecked (TwoWay; raising `checked_changed`, running the command, driving the IsChecked
// visual state and re-running the Foreground mapper) and Color. The i_check_box channel: set_is_checked
// is the INBOUND write the handler performs on a native toggle (stored at the from-handler specificity,
// like C#'s `ICheckBox.IsChecked` explicit implementation); foreground() is C#'s `Color?.AsPaint()` —
// a solid paint the control owns, rebuilt when Color changes, null while Color was never set.
//
// Command: the port models C#'s ICommand as the plain `command` callable (the button convention), so
// the CanExecute gate / IsEnabledCore coupling of CommandElement is NOT ported (documented deviation —
// the port has no ICommand abstraction yet). C#'s callback order is preserved: Foreground remap →
// CheckedChanged → Command → ChangeVisualState.

#include <memory>
#include <string_view>

#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_check_box.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::controls
{
    class check_box : public view<maui::core::i_check_box>
    {
    public:
        // The visual state name for the checked state (CheckBox.IsCheckedVisualState).
        static constexpr std::string_view is_checked_visual_state = "IsChecked";

        check_box()
        {
            this->set_style_target_type<check_box>();
        }

        // Shared bindable-property descriptors (one instance per type, like CheckBox.*Property).
        static const maui::core::bindable_property<bool>& is_checked_property();
        static const maui::core::bindable_property<maui::graphics::color>& color_property();

        // ---- the developer-facing surface ----
        [[nodiscard]] bool is_checked() const override
        {
            return is_checked_.get();
        }
        // The manual (developer) setter — C# CheckBox.IsChecked's class property.
        void set_is_checked(bool value)
        {
            is_checked_.set(value);
        }
        [[nodiscard]] maui::graphics::color color() const
        {
            return color_.get();
        }
        void set_color(maui::graphics::color value)
        {
            color_.set(value);
        }

        // ---- i_check_box (read by the handler's mapper / written by the native toggle) ----
        // The inbound native channel: stores at the from-handler specificity (C# `ICheckBox.IsChecked`
        // set => SetValue(IsCheckedProperty, value, SetterSpecificity.FromHandler)).
        void send_is_checked(bool value) override
        {
            is_checked_.set(value, maui::core::setter_specificity::from_handler);
        }
        // CheckBox.Foreground => Color?.AsPaint(): a solid paint over Color, null while Color is unset.
        [[nodiscard]] const maui::graphics::paint* foreground() const override
        {
            return foreground_.get();
        }

        // Occurs when IsChecked changes (raised with the new value, like C#'s
        // CheckedChanged(CheckedChangedEventArgs)).
        maui::core::event<bool> checked_changed;

        // Executed when IsChecked changes (C#'s Command, as a plain callable — see the header note).
        maui::core::move_only_function<void()> command;

        // CheckBox.ChangeVisualState: when enabled and checked, go to the IsChecked state if the
        // developer configured one (falling back to Normal otherwise); else the base common states.
        void change_visual_state() override
        {
            if (is_enabled() && is_checked())
            {
                if (!visual_states().go_to_state(*this, is_checked_visual_state))
                {
                    visual_states().go_to_state(*this, common_states::normal);
                }
            }
            else
            {
                view::change_visual_state();
            }
        }

    private:
        // Rebuild the owned foreground paint from Color (the `Color?.AsPaint()` analog). Called from the
        // color descriptor's property-changed callback (see check_box.cpp); null until Color is set.
        void refresh_foreground()
        {
            foreground_ = std::make_shared<maui::graphics::solid_paint>(color_.get());
        }
        // The descriptors' property-changed callbacks (check_box.cpp) reach refresh_foreground above.
        friend struct check_box_descriptor_access;

        maui::core::property<bool> is_checked_{*this, is_checked_property()};
        maui::core::property<maui::graphics::color> color_{*this, color_property()};
        std::shared_ptr<maui::graphics::paint> foreground_;
    };
} // namespace maui::controls
