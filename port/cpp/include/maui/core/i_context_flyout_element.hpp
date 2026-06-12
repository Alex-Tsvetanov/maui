#pragma once
// maui::core::i_context_flyout_element  <=  Microsoft.Maui.IContextFlyoutElement
//
// Marks a view as carrying a context flyout (the right-click / long-press menu). The shared
// view_mapper's map_context_flyout cross-casts the i_view to this (C# ViewHandler.MapContextFlyout
// type-checks `view is IContextFlyoutElement`) and materializes the menu on the native view
// (NSView.menu on AppKit; a UIContextMenuInteraction on iOS). Ported from
// src/Core/src/Core/IContextFlyoutElement.cs.

namespace maui::core
{
    class i_flyout;

    class i_context_flyout_element
    {
    public:
        virtual ~i_context_flyout_element() = default;

        // The attached context flyout, or null when none. Non-owning (the caller owns the flyout).
        [[nodiscard]] virtual i_flyout* context_flyout() const = 0;

    protected:
        i_context_flyout_element() = default;
        i_context_flyout_element(const i_context_flyout_element&) = default;
        i_context_flyout_element(i_context_flyout_element&&) = default;
        i_context_flyout_element& operator=(const i_context_flyout_element&) = default;
        i_context_flyout_element& operator=(i_context_flyout_element&&) = default;
    };
} // namespace maui::core
