#pragma once
// maui::controls::navigation_page  <=  Microsoft.Maui.Controls.NavigationPage
//
// A page that manages a push/pop STACK of content_pages, showing the top-most one. Ported from
// NavigationPage.cs (the MauiNavigationImpl push/pop orchestration) + IStackNavigationView; this is the
// FIRST cut, deliberately minimal:
//   - The stack is a vector of NON-owning content_page* (PROFILE §8: the caller owns the pages).
//   - push/pop/pop_to_root mutate the stack, fire the page Appearing/Disappearing lifecycle in C#'s
//     ORDER, then notify the handler (a "request_navigation" command) which re-hosts the new top-most
//     page's native view in its container and synchronously reports completion (navigation_finished).
//   - There is NO animation, navigation bar, back button, title bar, modal stack, InsertPageBefore /
//     RemovePage, or Shell — those are DEFERRED (documented in PROJECT/STATUS).
//
// ORDER (NavigationPage.cs MauiNavigationImpl.OnPushAsync/OnPopAsync/OnPopToRootAsync via
// SendHandlerUpdateAsync): mutate the stack + set the new current page, then fire FireDisappearing(prev)
// + FireAppearing(new) — BOTH before RequestNavigation — then RequestNavigation(stack); the handler does
// the native transition and (here synchronously) calls navigation_finished. We collapse C#'s async
// machinery (semaphore / TaskCompletionSource / overlapping-request queue) to a synchronous single
// transition: there is no overlap to guard, and the AppKit NSView swap completes inline.
//
// HasAppeared gating: C#'s FireDisappearing/FireAppearing only fire once the navigation page itself has
// appeared (its window made it visible). There is no window lifecycle at this layer, so the
// navigation_page drives the ROOT page's initial send_appearing() itself when constructed with a root
// (or on the first push onto an empty stack) — standing in for the window — so a subsequent push then
// correctly fires send_disappearing() on the now-hidden root. Documented deviation.

#include <functional>
#include <memory>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_stack_navigation.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/navigation_request.hpp"

namespace maui::controls
{
    class navigation_page : public view<maui::core::i_view>, public maui::core::i_stack_navigation
    {
    public:
        navigation_page() = default;
        // Construct with a root page already pushed (C# NavigationPage(Page root) → PushPage(root)). The
        // root is made the initial current page and appears (the window's role, stood in for here).
        explicit navigation_page(content_page& root)
        {
            push_initial(root);
        }

        // ---- the navigation stack (top-most page LAST), C# NavigationPage.NavigationStack ----
        [[nodiscard]] const std::vector<content_page*>& navigation_stack() const
        {
            return stack_;
        }
        // C# NavigationPage.CurrentPage — the top-most page, or null when the stack is empty.
        [[nodiscard]] content_page* current_page() const
        {
            return stack_.empty() ? nullptr : stack_.back();
        }
        // C# NavigationPage.RootPage — the bottom of the stack, or null when empty.
        [[nodiscard]] content_page* root_page() const
        {
            return stack_.empty() ? nullptr : stack_.front();
        }

        // C# NavigationPage.PushAsync (MauiNavigationImpl.OnPushAsync): push `page` onto the stack and
        // make it current. Pushing a page already on the stack is a no-op (C#: InternalChildren.Contains).
        void push(content_page& page);

        // C# NavigationPage.PopAsync (OnPopAsync): pop the top-most page, returning it (non-owning), and
        // make the page beneath it current. The root cannot be popped — popping a single-page stack is a
        // no-op and returns null (C#: InternalChildren.Count == 1 → return null).
        content_page* pop();

        // C# NavigationPage.PopToRootAsync (OnPopToRootAsync): pop every page above the root, making the
        // root current. A no-op on a single-page (or empty) stack.
        void pop_to_root();

        // ---- i_element (override to host the current page once a handler attaches) ----
        // C# NavigationPage.OnHandlerChangedCore: when the handler connects to a navigation page that
        // already has pages, host the current page (and, in C#, fire its appearing). Here we re-issue the
        // navigation request after the base wires the handler, so the freshly-attached native container
        // shows the current page immediately (rather than staying empty until the next push/pop).
        void set_handler(std::shared_ptr<maui::core::i_element_handler> value) override;

        // ---- i_stack_navigation (the handler reports the realized native stack back here) ----
        void request_navigation(const maui::core::navigation_request& request) override;
        void navigation_finished(const std::vector<maui::core::i_view*>& stack) override;

    protected:
        // Every page in the stack is a logical child, so BindingContext + Window inherit down to them.
        // content_page is-a element, so visiting needs no cast.
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            for (content_page* const page : stack_)
            {
                visit(*page);
            }
        }

    private:
        // Make `root` the initial current page + appear it (stands in for the window). Used by the
        // root-ctor and by the first push onto an empty stack.
        void push_initial(content_page& root);

        // Build the request payload (the current stack as i_view*) and hand it to the handler via the
        // "request_navigation" command (C# Handler.Invoke(nameof(RequestNavigation), request)).
        void notify_request_navigation();

        // The current stack as i_view* (the navigation_request payload shape).
        [[nodiscard]] std::vector<maui::core::i_view*> stack_as_views() const;

        std::vector<content_page*> stack_; // NON-owning: the caller owns the pages' lifetime
    };
} // namespace maui::controls
