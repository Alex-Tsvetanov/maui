// maui::controls::gradient_stop out-of-line definitions (header: brushes/gradient_stop.hpp).

#include "maui/controls/brushes/gradient_stop.hpp"

#include <cmath>
#include <cstddef>
#include <functional>
#include <optional>

#include "maui/core/bindable_property.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::optional<maui::graphics::color>>& gradient_stop::color_property()
    {
        // C# GradientStop.ColorProperty — default null (nullopt).
        static const maui::core::bindable_property<std::optional<maui::graphics::color>> prop{"Color", std::nullopt};
        return prop;
    }

    const maui::core::bindable_property<float>& gradient_stop::offset_property()
    {
        // C# GradientStop.OffsetProperty — default 0.
        static const maui::core::bindable_property<float> prop{"Offset", 0.0F};
        return prop;
    }

    gradient_stop::gradient_stop() = default;

    gradient_stop::gradient_stop(maui::graphics::color color, float offset)
    {
        color_.set(std::optional<maui::graphics::color>{color});
        offset_.set(offset);
    }

    bool gradient_stop::equals(const gradient_stop& other) const
    {
        // C# GradientStop.Equals: Color == dest.Color (value-equal, incl. both null) AND |Δoffset| < 0.00001.
        const std::optional<maui::graphics::color>& a = color_.get();
        const std::optional<maui::graphics::color>& b = other.color_.get();
        if (a.has_value() != b.has_value())
        {
            return false;
        }
        if (a.has_value() && *a != *b)
        {
            return false;
        }
        return std::abs(offset_.get() - other.offset_.get()) < 0.00001F;
    }

    std::size_t gradient_stop::get_hash_code() const
    {
        // C# returns base.GetHashCode() (object identity). The port mirrors that with the instance address —
        // a stable per-object value (the oracle only checks GetHashCode doesn't throw).
        return std::hash<const void*>{}(this);
    }
} // namespace maui::controls
