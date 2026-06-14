#pragma once
// maui::controls::flyout_behavior  <=  Microsoft.Maui.FlyoutBehavior
//
// How a shell presents its flyout: disabled (no flyout), flyout (the swipe-in/hamburger drawer,
// the default), or locked (always visible alongside the content). Ported from FlyoutBehavior.cs.

namespace maui::controls
{
    enum class flyout_behavior
    {
        disabled,
        flyout,
        locked,
    };
} // namespace maui::controls
