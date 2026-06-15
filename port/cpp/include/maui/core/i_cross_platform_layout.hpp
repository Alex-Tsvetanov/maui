#pragma once
// maui::core::i_cross_platform_layout  <=  Microsoft.Maui.ICrossPlatformLayout
//
// The cross-platform measure/arrange face a native layout host calls back into. Ported from
// src/Core/src/Core/ICrossPlatformLayout.cs: CrossPlatformMeasure(width, height) and
// CrossPlatformArrange(bounds). In MAUI the native panel's OnMeasure/OnLayout (Android),
// SizeThatFits/LayoutSubviews (iOS), etc. invoke these to delegate sizing back to the cross-platform
// layout, which sizes itself + positions its children.
//
// PORT WIRING (see i_layout.hpp / i_content_view.hpp): the port already drives the layout pass through
// i_view::measure/arrange — a layout<> computes its own geometry via its layout manager, and the
// content_page does MeasureContent/ArrangeContent inside measure()/arrange(). This contract makes the
// C# face EXPLICIT so the seam is discoverable and a future native host can resolve it by
// dynamic_cast<i_cross_platform_layout*>. layout<> implements it by forwarding to measure/arrange (the
// same computation C# routes through CrossPlatformMeasure/Arrange), so the two stay in lock-step. It is
// a STANDALONE abstract mixin (no i_view base — matching C#, which keeps ICrossPlatformLayout separate
// from IView) so adding it to a control introduces no diamond.

#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    class i_cross_platform_layout
    {
    public:
        virtual ~i_cross_platform_layout() = default;

        // ICrossPlatformLayout.CrossPlatformMeasure — the desired size within the constraints.
        [[nodiscard]] virtual maui::graphics::size cross_platform_measure(double width_constraint,
                                                                          double height_constraint) = 0;
        // ICrossPlatformLayout.CrossPlatformArrange — arrange the children within `bounds`; returns the
        // arranged size.
        virtual maui::graphics::size cross_platform_arrange(const maui::graphics::rect& bounds) = 0;

    protected:
        i_cross_platform_layout() = default;
        i_cross_platform_layout(const i_cross_platform_layout&) = default;
        i_cross_platform_layout(i_cross_platform_layout&&) = default;
        i_cross_platform_layout& operator=(const i_cross_platform_layout&) = default;
        i_cross_platform_layout& operator=(i_cross_platform_layout&&) = default;
    };
} // namespace maui::core
