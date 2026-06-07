#pragma once
// maui::core::i_stack_navigation  <=  Microsoft.Maui.IStackNavigation
//
// The stack-navigation contract a navigation view (navigation_page) implements so its handler can drive
// the native push/pop. Ported from src/Core/src/Core/IStackNavigation.cs.
//
//   request_navigation(request)            — the view asks the handler to transition to request.stack;
//                                            the handler does the native re-host, then reports completion.
//   navigation_finished(new_stack)         — the handler calls this back once the transition completes,
//                                            so the view can sync to the realized native stack (the seam
//                                            that "unblocks" the push/pop). In C# this is the async
//                                            completion signal; here (synchronous, macOS NSView swap) the
//                                            handler calls it inline at the end of the re-host.
//
// C#'s IStackNavigationView (IView + IStackNavigation) is collapsed here: the navigation_page already
// derives i_view via its view<> base, so it just additionally derives this contract — no separate
// combined interface is needed at this layer.

#include <vector>

#include "maui/core/navigation_request.hpp"

namespace maui::core
{
    class i_view;

    class i_stack_navigation
    {
    public:
        virtual ~i_stack_navigation() = default;

        // C# IStackNavigation.RequestNavigation: transition to the request's stack (the view's handler
        // hosts the new top-most page).
        virtual void request_navigation(const navigation_request& request) = 0;

        // C# IStackNavigation.NavigationFinished: the handler reports the realized native stack once the
        // transition completes, letting the view reconcile with it.
        virtual void navigation_finished(const std::vector<i_view*>& stack) = 0;

    protected:
        i_stack_navigation() = default;
        i_stack_navigation(const i_stack_navigation&) = default;
        i_stack_navigation(i_stack_navigation&&) = default;
        i_stack_navigation& operator=(const i_stack_navigation&) = default;
        i_stack_navigation& operator=(i_stack_navigation&&) = default;
    };
} // namespace maui::core
