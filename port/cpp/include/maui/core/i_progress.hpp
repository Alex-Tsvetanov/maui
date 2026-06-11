#pragma once
// maui::core::i_progress  <=  Microsoft.Maui.IProgress
//
// The virtual-view contract for a determinate progress bar: a horizontal bar filled to a [0, 1]
// fraction. Ported from src/Core/src/Core/IProgress.cs.

#include "maui/core/i_view.hpp"
#include "maui/graphics/color.hpp"

namespace maui::core
{
    class i_progress : public i_view
    {
    public:
        // The current progress, 0 to 1 (values outside the range are clamped by the control).
        [[nodiscard]] virtual double progress() const = 0;

        // The color of the progress bar.
        [[nodiscard]] virtual maui::graphics::color progress_color() const = 0;
    };
} // namespace maui::core
