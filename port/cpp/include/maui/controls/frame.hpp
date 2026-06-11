#pragma once
// maui::controls::frame  <=  Microsoft.Maui.Controls.Frame
//
// The legacy single-child framing container: BorderColor + CornerRadius + HasShadow over a default
// Padding of 20. Ported from src/Controls/src/Core/Frame/Frame.cs — which C# marks
// [Obsolete("Frame is obsolete as of .NET 9. Please use Border instead.")] — so the port implements
// frame as a THIN FACADE over the border machinery (the C# guidance made structural): frame derives
// border and translates its legacy surface onto the stroke/shape/shadow primitives:
//   - BorderColor  → Stroke = solid_paint(color) with StrokeThickness 1 (C# IBorderElement.BorderWidth
//     => 1, the FrameRenderer's fixed border width). Until a BorderColor is set the facade keeps
//     StrokeThickness 0, so measure/arrange inset by Padding only — exactly Frame.CrossPlatformMeasure/
//     Arrange's `BorderColor is not null ? BorderWidth : 0` term, reproduced through border's own
//     Padding + StrokeThickness formulas.
//   - CornerRadius → StrokeShape = round_rectangle(radius) when >= 0, the default rectangle at -1
//     (C# CornerRadiusProperty default -1, validated to -1 or >= 0).
//   - HasShadow    → the view Shadow: radius 5, opacity 0.8, offset (0,0), black — the hard-coded
//     shadow C#'s Frame returns for IView.Shadow on iOS (Frame.cs); default true.
// The facade resolves to the same border_handler (no Frame-specific native code — the C#
// FrameRenderer lives in the out-of-scope Compatibility layer).
//
// NOT modeled (documented): clearing BorderColor back to null (the facade only adds a border; C#'s
// IBorderElement nullable color round-trip serves the legacy renderer), and a developer-set custom
// Shadow overriding HasShadow (C# base.Shadow ?? hard-coded — set_shadow after construction wins here
// by ordering, which matches the observable C# precedence).

#include <optional>

#include "maui/controls/border.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class frame : public border
    {
    public:
        frame();

        // Shared bindable-property descriptors. padding_property shadows border's with the Frame
        // default (20 — Frame's IPaddingElement.PaddingDefaultValueCreator).
        static const maui::core::bindable_property<maui::core::thickness>& padding_property();
        static const maui::core::bindable_property<maui::graphics::color>& border_color_property();
        static const maui::core::bindable_property<float>& corner_radius_property();
        static const maui::core::bindable_property<bool>& has_shadow_property();

        // ---- BorderColor (C# BorderElement.BorderColorProperty; default null → nullopt) ----
        [[nodiscard]] std::optional<maui::graphics::color> border_color() const
        {
            if (!border_color_.is_set())
            {
                return std::nullopt;
            }
            return border_color_.get();
        }
        void set_border_color(maui::graphics::color value);

        // ---- CornerRadius (C# default -1; valid values are -1 or >= 0) ----
        [[nodiscard]] float corner_radius() const
        {
            return corner_radius_.get();
        }
        // Throws std::invalid_argument on a negative radius other than -1 (the C# validateValue).
        void set_corner_radius(float value);

        // ---- HasShadow (C# default true; drives the hard-coded iOS frame shadow) ----
        [[nodiscard]] bool has_shadow() const
        {
            return has_shadow_.get();
        }
        void set_has_shadow(bool value);

    private:
        maui::core::property<maui::graphics::color> border_color_{*this, border_color_property()};
        maui::core::property<float> corner_radius_{*this, corner_radius_property()};
        maui::core::property<bool> has_shadow_{*this, has_shadow_property()};
    };
} // namespace maui::controls
