#pragma once
// maui::core::dimension  <=  Microsoft.Maui.Primitives.Dimension
//
// The sentinel values + helpers the layout system uses to distinguish "explicitly set" sizes from
// "unset" ones: Unset is NaN (no explicit width/height request), Minimum is 0, Maximum is +inf.
// Ported from src/Core/src/Primitives/Dimension.cs.

#include <cmath>
#include <limits>

namespace maui::core::dimension
{
    inline constexpr double minimum = 0.0;
    inline constexpr double unset = std::numeric_limits<double>::quiet_NaN();
    inline constexpr double maximum = std::numeric_limits<double>::infinity();

    [[nodiscard]] inline bool is_explicit_set(double value)
    {
        return !std::isnan(value);
    }

    [[nodiscard]] inline bool is_maximum_set(double value)
    {
        return value != std::numeric_limits<double>::infinity(); // C# !double.IsPositiveInfinity
    }

    [[nodiscard]] inline bool is_minimum_set(double value)
    {
        return !std::isnan(value);
    }

    [[nodiscard]] inline double resolve_minimum(double value)
    {
        return is_minimum_set(value) ? value : minimum;
    }
} // namespace maui::core::dimension
