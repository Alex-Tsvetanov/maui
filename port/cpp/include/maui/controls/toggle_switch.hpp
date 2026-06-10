#pragma once
// maui::controls::toggle_switch  <=  Microsoft.Maui.Controls.Switch
//
// A control the user can toggle between two states: on or off. Ported from
// src/Controls/src/Core/Switch/Switch.cs. RENAMED: `switch` is a C++ keyword, so the control is
// `toggle_switch` (a project-level naming decision); everything else keeps the C# shape.
//
// Surface: IsToggled (TwoWay; raising `toggled`, driving the On/Off visual states and re-running the
// effective TrackColor), OnColor / OffColor (each re-runs TrackColor), ThumbColor. The i_switch
// channel: is_on() reads IsToggled; set_is_on() is the INBOUND write the handler performs on a native
// toggle (stored at the from-handler specificity, like C#'s `ISwitch.IsOn` explicit implementation);
// track_color() computes the effective on/off color (Switch's `ISwitch.TrackColor`).
//
// Color collapse (port convention, as button/label): C#'s nullable Colors (null = platform default)
// collapse to the non-nullable maui::graphics::color value type, so the platform-default fallback
// branches of SwitchExtensions have no analog here.

#include <string_view>

#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_switch.hpp"
#include "maui/core/property.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class toggle_switch : public view<maui::core::i_switch>
    {
    public:
        // The visual state names the switch drives (Switch.SwitchOnVisualState / SwitchOffVisualState).
        static constexpr std::string_view switch_on_visual_state = "On";
        static constexpr std::string_view switch_off_visual_state = "Off";

        toggle_switch()
        {
            this->set_style_target_type<toggle_switch>();
        }

        // Shared bindable-property descriptors (one instance per type, like Switch.*Property).
        static const maui::core::bindable_property<bool>& is_toggled_property();
        static const maui::core::bindable_property<maui::graphics::color>& on_color_property();
        static const maui::core::bindable_property<maui::graphics::color>& off_color_property();
        static const maui::core::bindable_property<maui::graphics::color>& thumb_color_property();

        // ---- the developer-facing surface ----
        [[nodiscard]] bool is_toggled() const
        {
            return is_toggled_.get();
        }
        void set_is_toggled(bool value)
        {
            is_toggled_.set(value);
        }
        [[nodiscard]] maui::graphics::color on_color() const
        {
            return on_color_.get();
        }
        void set_on_color(maui::graphics::color value)
        {
            on_color_.set(value);
        }
        [[nodiscard]] maui::graphics::color off_color() const
        {
            return off_color_.get();
        }
        void set_off_color(maui::graphics::color value)
        {
            off_color_.set(value);
        }

        // ---- i_switch (read by the handler's mapper / written by the native toggle) ----
        [[nodiscard]] bool is_on() const override
        {
            return is_toggled_.get();
        }
        // The inbound native channel: stores at the from-handler specificity (C# `ISwitch.IsOn` set =>
        // SetValue(IsToggledProperty, value, SetterSpecificity.FromHandler)).
        void set_is_on(bool value) override
        {
            is_toggled_.set(value, maui::core::setter_specificity::from_handler);
        }
        // The effective track color: OnColor when toggled, OffColor otherwise (`ISwitch.TrackColor`).
        [[nodiscard]] maui::graphics::color track_color() const override
        {
            return is_toggled_.get() ? on_color_.get() : off_color_.get();
        }
        [[nodiscard]] maui::graphics::color thumb_color() const override
        {
            return thumb_color_.get();
        }
        void set_thumb_color(maui::graphics::color value)
        {
            thumb_color_.set(value);
        }

        // Occurs when IsToggled changes (raised from the descriptor's property-changed callback with the
        // new value, like C#'s Toggled(ToggledEventArgs)).
        maui::core::event<bool> toggled;

        // Switch.ChangeVisualState: the base common states first, then — when enabled — the On/Off state
        // matching the toggle (a no-op when the developer configured no such states).
        void change_visual_state() override
        {
            view::change_visual_state();
            if (is_enabled() && is_toggled())
            {
                visual_states().go_to_state(*this, switch_on_visual_state);
            }
            else if (is_enabled() && !is_toggled())
            {
                visual_states().go_to_state(*this, switch_off_visual_state);
            }
        }

    protected:
        // Switch.OnPropertyChanged: an IsToggled change re-runs the IsOn mapper (the bindable property
        // name is "is_toggled" but the i_switch/mapper key is "is_on" — C# bridges the same mismatch).
        void on_property_changed(std::string_view name) override
        {
            view::on_property_changed(name);
            if (name == "is_toggled")
            {
                if (const auto& element_handler = handler())
                {
                    element_handler->update_value("is_on");
                }
            }
        }

    private:
        maui::core::property<bool> is_toggled_{*this, is_toggled_property()};
        maui::core::property<maui::graphics::color> on_color_{*this, on_color_property()};
        maui::core::property<maui::graphics::color> off_color_{*this, off_color_property()};
        maui::core::property<maui::graphics::color> thumb_color_{*this, thumb_color_property()};
    };
} // namespace maui::controls
