#pragma once
// maui::core::i_layout  <=  Microsoft.Maui.ILayout
//
// A view that positions + sizes child elements. Ported from src/Core/src/Core/ILayout.cs
// (ILayout : IView, IContainer, ISafeAreaView, IPadding, ICrossPlatformLayout). M3 subset: IView +
// IContainer + IPadding — the surface the layout managers consume. ClipsToBounds, ISafeAreaView, and
// the ICrossPlatformLayout measure/arrange bridge (used by the layout handler) arrive at M4.

#include "maui/core/i_container.hpp"
#include "maui/core/i_padding.hpp"
#include "maui/core/i_view.hpp"

namespace maui::core
{
    class i_layout : public i_view, public i_container, public i_padding
    {
    };
} // namespace maui::core
