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

    double layout_manager::resolve_size_request(double measured, double exact, double min, double max)
    {
        // C# ViewHandlerExtensions.ResolveConstraints(measured, exact, min, max): an unset minimum resolves
        // to 0 (Dimension.ResolveMinimum), an explicit exact value overrides measured, then max caps and
        // min floors — so max beats exact and min beats both. NaN comparisons are false, so an unset
        // (NaN) max/exact leaves the value as-is, matching the C# guard order.
        // An explicit exact value overrides measured; std::min/std::max then cap/floor. NaN comparisons are
        // false, so a NaN max leaves the value as-is (std::min returns the first arg) — the same outcome as
        // C#'s `if (resolved > max)` guard. min is already resolved (NaN -> 0), so it never spuriously floors.
        double resolved = maui::core::dimension::is_explicit_set(exact) ? exact : measured;
        resolved = std::min(resolved, max);                                         // max wins over exact
        resolved = std::max(resolved, maui::core::dimension::resolve_minimum(min)); // min wins over both
        return resolved;
    }
} // namespace maui::layouts
