#pragma once
// maui::core::i_flyout  <=  Microsoft.Maui.IFlyout
//
// The marker contract for an attachable flyout (a view's ContextFlyout is typed as this). Ported from
// src/Core/src/IFlyout.cs (IFlyout : IElement — the IElement base is dropped here: the port's menu tree
// does not carry per-element handlers; the window/view chrome materializes whole menus instead, see
// STATUS.md W1-11).

namespace maui::core
{
    class i_flyout
    {
    public:
        virtual ~i_flyout() = default;

    protected:
        i_flyout() = default;
        i_flyout(const i_flyout&) = default;
        i_flyout(i_flyout&&) = default;
        i_flyout& operator=(const i_flyout&) = default;
        i_flyout& operator=(i_flyout&&) = default;
    };
} // namespace maui::core
