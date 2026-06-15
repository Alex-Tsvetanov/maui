#pragma once
// maui::controls::solid_color_brush  <=  Microsoft.Maui.Controls.SolidColorBrush
//
// A brush that paints with a single (nullable) Color. Ported from src/Controls/src/Core/SolidColorBrush.cs:
// the bindable Color (default null), IsEmpty (Color is null), and value Equals (compare color VALUES — the
// dotnet/maui#27281 fix; the port's color::operator== already compares the ARGB value). [ContentProperty]
// is Color (loader metadata, not modeled here).
//
// Color is std::optional<color> (nullopt == C#'s null), so a default solid_color_brush is empty and the
// brush→paint bridge maps a null color to a solid_paint whose color stays the value-type default.
//
// The named Brush statics (Brush.Red …) are inherited from `brush` and are immutable_brush instances; that
// derived (read-only) class lives in immutable_brush.hpp.
//
// Out-of-line definitions live in solid_color_brush.cpp.

#include <cstddef>
#include <optional>
#include <utility>

#include "maui/controls/brushes/brush.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class solid_color_brush : public brush
    {
    public:
        // C# SolidColorBrush() — Color null.
        solid_color_brush();
        // C# SolidColorBrush(Color color).
        explicit solid_color_brush(maui::graphics::color color);
        // The nullable-color ctor (the bridge + the converter's "no color set" path: new SolidColorBrush(null)).
        explicit solid_color_brush(std::optional<maui::graphics::color> color);

        // C# SolidColorBrush.ColorProperty.
        static const maui::core::bindable_property<std::optional<maui::graphics::color>>& color_property();

        // C# SolidColorBrush.Color — the fill color (nullopt == null). Virtual in C# (immutable_brush
        // overrides the setter to a no-op). This is a bindable property.
        [[nodiscard]] virtual const std::optional<maui::graphics::color>& color() const
        {
            return color_.get();
        }
        virtual void set_color(std::optional<maui::graphics::color> value)
        {
            color_.set(std::move(value));
        }

        // C# SolidColorBrush.IsEmpty — Color is null.
        [[nodiscard]] bool is_empty() const override
        {
            return !color_.get().has_value();
        }

        // C# SolidColorBrush.Equals — value-compares the colors (both null, or both equal values).
        [[nodiscard]] bool equals(const solid_color_brush& other) const;
        friend bool operator==(const solid_color_brush& a, const solid_color_brush& b)
        {
            return a.equals(b);
        }
        // C# SolidColorBrush.GetHashCode — base.GetHashCode() (identity); see gradient_stop note.
        [[nodiscard]] std::size_t get_hash_code() const;

    protected:
        // immutable_brush sets the stored color past the public setter (which it overrides to a no-op).
        void set_color_internal(std::optional<maui::graphics::color> value)
        {
            color_.set(std::move(value));
        }

    private:
        maui::core::property<std::optional<maui::graphics::color>> color_{*this, color_property()};
    };
} // namespace maui::controls
