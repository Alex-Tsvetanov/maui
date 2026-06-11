#pragma once
// maui::core::i_scroll_view  <=  Microsoft.Maui.IScrollView
//
// The virtual-view contract for a view that scrolls its single content child. Ported from
// src/Core/src/Core/IScrollView.cs (IScrollView : IContentView): the scroll-bar visibilities, the
// orientation, the scrollable content size, the current offsets (settable — the PLATFORM writes them
// back as the user scrolls), the scroll-finished acknowledgement, and the scroll-to request the
// handler executes natively.

#include "maui/core/i_content_view.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/core/scroll_orientation.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    class i_scroll_view : public i_content_view
    {
    public:
        // C# IScrollView.HorizontalScrollBarVisibility / VerticalScrollBarVisibility.
        [[nodiscard]] virtual scroll_bar_visibility horizontal_scroll_bar_visibility() const = 0;
        [[nodiscard]] virtual scroll_bar_visibility vertical_scroll_bar_visibility() const = 0;

        // C# IScrollView.Orientation.
        [[nodiscard]] virtual scroll_orientation orientation() const = 0;

        // C# IScrollView.ContentSize — the size of the scrollable content.
        [[nodiscard]] virtual maui::graphics::size content_size() const = 0;

        // C# IScrollView.HorizontalOffset / VerticalOffset — the current scroll position; the setters
        // are the platform's write-back path as the native view scrolls.
        [[nodiscard]] virtual double horizontal_offset() const = 0;
        virtual void set_horizontal_offset(double value) = 0;
        [[nodiscard]] virtual double vertical_offset() const = 0;
        virtual void set_vertical_offset(double value) = 0;

        // C# IScrollView.ScrollFinished — the platform reports a completed scroll operation.
        virtual void scroll_finished() = 0;

        // C# IScrollView.RequestScrollTo — scroll to the given offsets (instant = not animated).
        virtual void request_scroll_to(double horizontal_offset, double vertical_offset, bool instant) = 0;
    };
} // namespace maui::core
