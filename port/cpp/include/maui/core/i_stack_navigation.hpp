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
//   navigation_bar_title() / navigation_back_button_visible()
//                                          — the navigation chrome state the handler reads to drive the
//                                            bar (title + back button). AppKit has no UINavigationController,
//                                            so the handler builds a custom bar (NSTextField title +
//                                            back NSButton) and reads these to populate it. The C# analog
//                                            is the IToolbar info (Title / BackButtonVisible) the
//                                            NavigationPage drives onto its NavigationPageToolbar; folded
//                                            onto this contract here to keep the bar info on the seam the
//                                            handler already dynamic_casts to (no separate IToolbar element).
//
// C#'s IStackNavigationView (IView + IStackNavigation) is collapsed here: the navigation_page already
// derives i_view via its view<> base, so it just additionally derives this contract — no separate
// combined interface is needed at this layer.

#include <string_view>
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

        // The navigation-bar title — the current page's Title (C# NavigationPageToolbar reads CurrentPage.
        // Title). Empty when the stack is empty. The handler reads this to populate the custom bar.
        [[nodiscard]] virtual std::string_view navigation_bar_title() const = 0;

        // Whether the bar shows a back button — C# IToolbar.BackButtonVisible, driven by
        // NavigationPageController.StackDepth > 1 (back is shown only above the root page).
        [[nodiscard]] virtual bool navigation_back_button_visible() const = 0;

        // The bar's back button (and a hardware back press) routes here — C# NavigationPage.
        // OnBackButtonPressed: pop the current page when above the root and report handled (true), else
        // unhandled (false). The handler's back-button target-action invokes this without knowing the
        // concrete navigation-view type.
        virtual bool send_back_button_pressed() = 0;

    protected:
        i_stack_navigation() = default;
        i_stack_navigation(const i_stack_navigation&) = default;
        i_stack_navigation(i_stack_navigation&&) = default;
        i_stack_navigation& operator=(const i_stack_navigation&) = default;
        i_stack_navigation& operator=(i_stack_navigation&&) = default;
    };
} // namespace maui::core
