#pragma once
// maui::core::i_border_view  <=  Microsoft.Maui.IBorderView
//
// The virtual-view contract for a container that draws a border (and clips its content) around a single
// child: a content view that is also a border stroke. Ported from src/Core/src/Core/IBorderView.cs
// (IBorderView : IContentView, IBorderStroke — an empty marker combining the two faces).

#include "maui/core/i_border_stroke.hpp"
#include "maui/core/i_content_view.hpp"

namespace maui::core
{
    class i_border_view : public i_content_view, public i_border_stroke
    {
    };
} // namespace maui::core
