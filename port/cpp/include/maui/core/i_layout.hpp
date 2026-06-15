#pragma once
// maui::core::i_layout  <=  Microsoft.Maui.ILayout
//
// A view that positions + sizes child elements. Ported from src/Core/src/Core/ILayout.cs
// (ILayout : IView, IContainer, ISafeAreaView, IPadding, ICrossPlatformLayout). Subset: IView +
// IContainer + IPadding + ClipsToBounds — the surface the layout managers + handler consume.
// ISafeAreaView (i_safe_area_view.hpp) and ICrossPlatformLayout (i_cross_platform_layout.hpp) are NOT
// folded into this minimal contract; the concrete layout<> base implements both as standalone mixins
// (cross_platform_measure/arrange forward to its layout manager; ignore_safe_area is the obsolete
// Layout.IgnoreSafeArea auto-property), so i_layout stays the surface the managers + handler consume.

#include "maui/core/i_container.hpp"
#include "maui/core/i_padding.hpp"
#include "maui/core/i_view.hpp"

namespace maui::core
{
    class i_layout : public i_view, public i_container, public i_padding
    {
    public:
        // Whether the layout clips its children to its own bounds (ILayout.ClipsToBounds, default false).
        // The layout handler pushes this to the native panel (Apple: layer.masksToBounds).
        [[nodiscard]] virtual bool clips_to_bounds() const = 0;
    };
} // namespace maui::core
