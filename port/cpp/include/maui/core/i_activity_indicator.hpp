#pragma once
// maui::core::i_activity_indicator  <=  Microsoft.Maui.IActivityIndicator
//
// The virtual-view contract for the indeterminate busy spinner: visible-and-animating while running.
// Ported from src/Core/src/Core/IActivityIndicator.cs.

#include "maui/core/i_view.hpp"
#include "maui/graphics/color.hpp"

namespace maui::core
{
    class i_activity_indicator : public i_view
    {
    public:
        // Whether the indicator should be visible and animating (false = hidden).
        [[nodiscard]] virtual bool is_running() const = 0;

        // The display color of the indicator.
        [[nodiscard]] virtual maui::graphics::color color() const = 0;
    };
} // namespace maui::core
