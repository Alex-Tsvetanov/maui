#pragma once
// maui::core::i_switch  <=  Microsoft.Maui.ISwitch
//
// The virtual-view contract for a two-state toggle: an IView with a mutable on/off value plus the
// track/thumb colors the platform pushes. Ported from src/Core/src/Core/ISwitch.cs.
//
// IsOn is MUTABLE in C# — the setter is the INBOUND channel: the native control's value-changed
// handler writes the user's toggle back through it (the concrete control stores it at the
// from-handler specificity, exactly like C# Switch's `ISwitch.IsOn` explicit implementation).
//
// TrackColor is the EFFECTIVE track color (the control computes OnColor-or-OffColor from its toggle
// state — see Switch's `ISwitch.TrackColor` getter); ThumbColor is pushed as-is.

#include "maui/core/i_view.hpp"
#include "maui/graphics/color.hpp"

namespace maui::core
{
    class i_switch : public i_view
    {
    public:
        // Gets or sets whether this switch is toggled on (set = the inbound native channel).
        [[nodiscard]] virtual bool is_on() const = 0;
        virtual void set_is_on(bool value) = 0;

        // Gets the switch track color (the effective on/off color).
        [[nodiscard]] virtual maui::graphics::color track_color() const = 0;

        // Gets the switch thumb color.
        [[nodiscard]] virtual maui::graphics::color thumb_color() const = 0;
    };
} // namespace maui::core
