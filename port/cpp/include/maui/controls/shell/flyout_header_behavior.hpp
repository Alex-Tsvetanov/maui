#pragma once
// maui::controls::flyout_header_behavior  <=  Microsoft.Maui.Controls.FlyoutHeaderBehavior
//
// How a shell's flyout HEADER behaves when the flyout list scrolls. Ported from
// FlyoutHeaderBehavior.cs (the explicit C# enum values are load-bearing — the iOS
// ShellFlyoutLayoutManager switches on them):
//   default_behavior   — platform-specific default scroll behavior (iOS: content below the header, no inset).
//   fixed_behavior     — the header stays pinned; content sits below it (no inset push).
//   scroll             — the header scrolls off with the content (the scroll view overlaps it via a content
//                        inset that pushes the content down by the header height).
//   collapse_on_scroll — the header shrinks to a minimum height as the user scrolls down.

namespace maui::controls
{
    enum class flyout_header_behavior
    {
        default_behavior = 0,   // C# FlyoutHeaderBehavior.Default (`default` is a C++ keyword)
        fixed_behavior = 1,     // C# FlyoutHeaderBehavior.Fixed (`fixed`-suffixed to parallel default_behavior)
        scroll = 2,             // C# FlyoutHeaderBehavior.Scroll
        collapse_on_scroll = 3, // C# FlyoutHeaderBehavior.CollapseOnScroll
    };
} // namespace maui::controls
