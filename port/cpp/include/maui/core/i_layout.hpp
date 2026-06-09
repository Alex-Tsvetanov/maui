#pragma once
// maui::core::i_layout  <=  Microsoft.Maui.ILayout
//
// A view that positions + sizes child elements. Ported from src/Core/src/Core/ILayout.cs
// (ILayout : IView, IContainer, ISafeAreaView, IPadding, ICrossPlatformLayout). Subset: IView +
// IContainer + IPadding + ClipsToBounds — the surface the layout managers + handler consume.
// ISafeAreaView and the ICrossPlatformLayout measure/arrange bridge remain deferred.

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
