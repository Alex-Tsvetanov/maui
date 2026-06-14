#pragma once
// maui::core::i_swipe_item_view  <=  Microsoft.Maui.ISwipeItemView
//
// A swipe item that displays custom content (the SwipeItemView control's contract). Ported from
// src/Core/src/Core/ISwipeItemView.cs (ISwipeItemView : IContentView, ISwipeItem) — it hosts a single
// Content like any content view and is activated as a swipe item. The IContentView base carries the
// hosted Content + Padding + the IView surface (is_enabled, which the swipe state machine's
// ExecuteSwipeItem reads to decide whether to invoke).

#include "maui/core/i_content_view.hpp"
#include "maui/core/i_swipe_item.hpp"

namespace maui::core
{
    class i_swipe_item_view : public i_content_view, public i_swipe_item
    {
    };
} // namespace maui::core
