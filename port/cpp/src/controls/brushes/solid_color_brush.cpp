// maui::controls::solid_color_brush out-of-line definitions (header: brushes/solid_color_brush.hpp).

#include "maui/controls/brushes/solid_color_brush.hpp"

#include <cstddef>
#include <functional>
#include <optional>

#include "maui/core/bindable_property.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::optional<maui::graphics::color>>& solid_color_brush::color_property()
    {
        // C# SolidColorBrush.ColorProperty — default null (nullopt).
        static const maui::core::bindable_property<std::optional<maui::graphics::color>> prop{"Color", std::nullopt};
        return prop;
    }

    solid_color_brush::solid_color_brush() = default;

    solid_color_brush::solid_color_brush(maui::graphics::color color)
    {
        color_.set(std::optional<maui::graphics::color>{color});
    }

    solid_color_brush::solid_color_brush(std::optional<maui::graphics::color> color)
    {
        color_.set(color);
    }

    bool solid_color_brush::equals(const solid_color_brush& other) const
    {
        // C# SolidColorBrush.Equals: Equals(Color, dest.Color) — value comparison (both null, or equal values;
        // the #27281 fix). color()/other.color() route through the virtual getter (immutable_brush shares it).
        const std::optional<maui::graphics::color>& a = color();
        const std::optional<maui::graphics::color>& b = other.color();
        if (a.has_value() != b.has_value())
        {
            return false;
        }
        return !a.has_value() || *a == *b;
    }

    std::size_t solid_color_brush::get_hash_code() const
    {
        // C# returns base.GetHashCode() (identity); mirror with the instance address.
        return std::hash<const void*>{}(this);
    }
} // namespace maui::controls
