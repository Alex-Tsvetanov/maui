#pragma once
// maui::core::i_content_view  <=  Microsoft.Maui.IContentView
//
// The virtual-view contract for a view that hosts a single child (its Content). Ported from
// src/Core/src/Core/IContentView.cs (IContentView : IView, IPadding, ICrossPlatformLayout).
//
// This M4c cut keeps the minimal surface: Content (the hosted child) + Padding (via i_padding) +
// the i_view base. C#'s PresentedContent — the content as actually rendered, after templates — is the
// same as Content here (no control templates yet), so the two collapse to a single content() accessor.
// ICrossPlatformLayout (CrossPlatformMeasure/Arrange) is also collapsed: the content_page control
// performs MeasureContent/ArrangeContent directly in its measure()/arrange() (see content_page.hpp),
// so no separate cross-platform-layout face is needed at this layer.
//
// content() returns a non-owning pointer — the caller owns the content view's lifetime (PROFILE §8) —
// and may be null when no content is set.

#include "maui/core/i_padding.hpp"
#include "maui/core/i_view.hpp"

namespace maui::core
{
    class i_content_view : public i_view, public i_padding
    {
    public:
        // C# IContentView.Content / PresentedContent (collapsed here): the hosted child, or null.
        [[nodiscard]] virtual i_view* content() const = 0;
    };
} // namespace maui::core
