// layout_manager — the shared constraint-resolution rule. See layout_manager.hpp.

#include "maui/layouts/layout_manager.hpp"

#include <algorithm>

#include "maui/core/dimension.hpp"

namespace maui::layouts
{
    double layout_manager::resolve_constraints(double external_constraint, double explicit_length,
                                               double measured_length, double min, double max)
    {
        double length = maui::core::dimension::is_explicit_set(explicit_length) ? explicit_length : measured_length;

        length = std::min(length, max); // C#: if (max < length) length = max;
        length = std::max(length, min); // C#: if (min > length) length = min;

        return std::min(length, external_constraint);
    }
} // namespace maui::layouts
