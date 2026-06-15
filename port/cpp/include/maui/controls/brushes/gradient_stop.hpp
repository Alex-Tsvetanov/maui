#pragma once
// maui::controls::gradient_stop  <=  Microsoft.Maui.Controls.GradientStop
//
// One color/offset stop in a controls-level gradient_brush. Ported from
// src/Controls/src/Core/GradientStop.cs: an Element carrying a (nullable) Color and a float Offset, both
// bindable, with value equality (Color value-equal AND |offset - other.offset| < 0.00001) and a stable
// hash (C# returns base.GetHashCode() — object identity; the port keeps the inherited element hash via a
// trivial get_hash_code so TestGetGradientStopHashCode's "doesn't throw" intent holds).
//
// NULLABLE COLOR (the central fidelity point): C# Color is a reference type, so the default GradientStop
// has Color == null. The port's maui::graphics::color is a value type, so the stop's color is held as a
// std::optional<color> (nullopt == C#'s null). This is what lets the brush→paint bridge reproduce C#'s
// Paint.IsNullOrEmpty (an all-null-color gradient is empty) — see brush_paint_bridge.hpp.
//
// It is an Element (logical child of its gradient_brush) so it inherits the brush's BindingContext
// (GradientBrush.OnBindingContextChanged → SetInheritedBindingContext), verified by
// BrushTypeConverterUnitTests.TestBindingContextPropagation. The bindable-property change on Color/Offset
// re-raises through the brush (the WeakBrushChangedProxy analog) — wired by gradient_brush.
//
// Out-of-line definitions live in gradient_stop.cpp.

#include <cstddef>
#include <optional>

#include "maui/controls/element.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class gradient_stop : public element
    {
    public:
        // C# GradientStop() — Color null, Offset 0.
        gradient_stop();
        // C# GradientStop(Color color, float offset).
        gradient_stop(maui::graphics::color color, float offset);

        // Shared bindable-property descriptors (the C# GradientStop.*Property fields).
        static const maui::core::bindable_property<std::optional<maui::graphics::color>>& color_property();
        static const maui::core::bindable_property<float>& offset_property();

        // C# GradientStop.Color — the stop color (nullopt == C#'s null default). This is a bindable property.
        [[nodiscard]] const std::optional<maui::graphics::color>& color() const
        {
            return color_.get();
        }
        void set_color(std::optional<maui::graphics::color> value)
        {
            color_.set(std::move(value));
        }

        // C# GradientStop.Offset — position 0..1 along the gradient. This is a bindable property.
        [[nodiscard]] float offset() const
        {
            return offset_.get();
        }
        void set_offset(float value)
        {
            offset_.set(value);
        }

        // C# GradientStop.Equals — Color value-equal AND offsets within 0.00001 (the issue #27281 rule:
        // compare color VALUES, never references).
        [[nodiscard]] bool equals(const gradient_stop& other) const;
        friend bool operator==(const gradient_stop& a, const gradient_stop& b)
        {
            return a.equals(b);
        }
        // C# GradientStop.GetHashCode — base.GetHashCode() (identity); the port returns a stable per-instance
        // value (the address) so the "GetHashCode doesn't throw" oracle holds without implying value-hash.
        [[nodiscard]] std::size_t get_hash_code() const;

    private:
        maui::core::property<std::optional<maui::graphics::color>> color_{*this, color_property()};
        maui::core::property<float> offset_{*this, offset_property()};
    };
} // namespace maui::controls
