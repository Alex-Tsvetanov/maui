#pragma once
// maui::controls::switch_cell  <=  Microsoft.Maui.Controls.SwitchCell
//
// A cell with a label and an on/off switch. Ported from src/Controls/src/Core/Cells/SwitchCell.cs.
// The native row renders a toggle_switch (Microsoft.Maui.Controls.Switch) beside the text — the table
// handler builds that native switch; the cell itself just carries the data + the toggled event, exactly
// like the C# SwitchCell (which is data-only; the renderer owns the UISwitch/NSButton).
//
// Surface: On (bindable, default false, TwoWay; a change raises OnChanged(ToggledEventArgs)), Text
// (bindable), OnColor (bindable color). Color collapse (port convention): nullable Color → color value.

#include <string>
#include <utility>

#include "maui/controls/cells/cell.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class switch_cell : public cell
    {
    public:
        switch_cell()
        {
            this->set_style_target_type<switch_cell>();
        }

        // Shared bindable-property descriptors (one instance per type, like SwitchCell.*Property).
        static const maui::core::bindable_property<bool>& on_property();
        static const maui::core::bindable_property<std::string>& text_property();
        static const maui::core::bindable_property<maui::graphics::color>& on_color_property();

        // SwitchCell.OnChanged — raised with the new value whenever On changes (TwoWay).
        maui::core::event<bool> on_changed;

        // ---- On (SwitchCell.On; default false, TwoWay) ----
        [[nodiscard]] bool on() const
        {
            return on_.get();
        }
        void set_on(bool value)
        {
            on_.set(value);
        }
        // The inbound native channel: a user toggle writes through at the from-handler specificity
        // (matching the native-write convention; still raises OnChanged via the descriptor callback).
        void set_on_from_handler(bool value)
        {
            on_.set(value, maui::core::setter_specificity::from_handler);
        }

        // ---- Text / OnColor ----
        [[nodiscard]] std::string text() const
        {
            return text_.get();
        }
        void set_text(std::string value)
        {
            text_.set(std::move(value));
        }
        [[nodiscard]] maui::graphics::color on_color() const
        {
            return on_color_.get();
        }
        void set_on_color(maui::graphics::color value)
        {
            on_color_.set(value);
        }

    private:
        maui::core::property<bool> on_{*this, on_property()};
        maui::core::property<std::string> text_{*this, text_property()};
        maui::core::property<maui::graphics::color> on_color_{*this, on_color_property()};
    };
} // namespace maui::controls
