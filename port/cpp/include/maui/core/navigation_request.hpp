#pragma once
// maui::core::navigation_request  <=  Microsoft.Maui.NavigationRequest
//
// The payload a stack-navigation view hands to its handler to drive a push/pop transition: the new
// navigation stack (top-most last) plus whether the transition should animate. Ported from
// src/Core/src/Primitives/NavigationRequestedEventArgs.cs (NavigationRequest).
//
// The stack is a list of NON-owning view pointers — the caller owns the pages' lifetime (PROFILE §8),
// matching C#'s IReadOnlyList<IView> (the views are referenced, not owned, by the request). The handler
// reads `stack` to decide which view is now current and hosts that view's native view.

#include <vector>

namespace maui::core
{
    class i_view;

    struct navigation_request
    {
        // C# NavigationRequest.NavigationStack — the new stack, top-most page last. Non-owning pointers.
        std::vector<i_view*> stack;
        // C# NavigationRequest.Animated.
        bool animated = false;
    };
} // namespace maui::core
