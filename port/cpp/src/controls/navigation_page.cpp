// maui::controls::navigation_page — the push/pop stack orchestration + the page lifecycle ordering, plus
// the i_stack_navigation back-channel and the default-handler self-registration. See navigation_page.hpp.
//
// The push/pop ORDER mirrors NavigationPage.cs's MauiNavigationImpl (OnPushAsync / OnPopAsync /
// OnPopToRootAsync) run through SendHandlerUpdateAsync: mutate the stack + set the current page, then
// fire Disappearing(previous) + Appearing(new) — BOTH before RequestNavigation — then notify the handler.
// C#'s async/overlap machinery is collapsed to a synchronous single transition (the AppKit NSView swap
// completes inline; there is no overlap to guard at this layer).

#include "maui/controls/navigation_page.hpp"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/navigation_page_handler.hpp"
#include "maui/core/navigation_request.hpp"

namespace maui::controls
{
    std::vector<maui::core::i_view*> navigation_page::stack_as_views() const
    {
        std::vector<maui::core::i_view*> views;
        views.reserve(stack_.size());
        for (content_page* const page : stack_)
        {
            views.push_back(page);
        }
        return views;
    }

    void navigation_page::push_initial(content_page& root)
    {
        stack_.push_back(&root);
        // The window's role at this layer: make the root visible. A later push then fires the root's
        // send_disappearing() correctly (it is now in the appeared state).
        root.send_appearing();
        // Host the root on the handler (if already attached) so the native container shows it.
        notify_request_navigation();
    }

    void navigation_page::push(content_page& page)
    {
        // C# OnPushAsync: pushing a page already on the stack is a no-op (InternalChildren.Contains).
        if (std::ranges::find(stack_, &page) != stack_.end())
        {
            return;
        }
        if (stack_.empty())
        {
            push_initial(page);
            return;
        }

        content_page* const previous = current_page();
        // C# PushPage(root): the new page becomes the top-most (and CurrentPage).
        stack_.push_back(&page);

        // C# OnPushAsync firePostNavigatingEvents: FireDisappearing(previous) + FireAppearing(new), both
        // fired BEFORE RequestNavigation.
        if (previous != nullptr)
        {
            previous->send_disappearing();
        }
        page.send_appearing();

        notify_request_navigation();
    }

    content_page* navigation_page::pop()
    {
        // C# OnPopAsync: the root cannot be popped (InternalChildren.Count == 1 → return null).
        if (stack_.size() <= 1)
        {
            return nullptr;
        }

        content_page* const current = current_page();
        content_page* const new_current = stack_[stack_.size() - 2];

        // C# OnPopAsync processStackChanges: FireDisappearing(current) THEN remove + set CurrentPage=new.
        current->send_disappearing();
        stack_.pop_back();
        // C# OnPopAsync firePostNavigatingEvents: FireAppearing(new) — still before RequestNavigation.
        new_current->send_appearing();

        notify_request_navigation();
        return current;
    }

    void navigation_page::pop_to_root()
    {
        // C# OnPopToRootAsync: a single-page (or empty) stack is a no-op.
        if (stack_.size() <= 1)
        {
            return;
        }

        content_page* const previous = current_page();
        content_page* const root = root_page();

        // C# OnPopToRootAsync processStackChanges: FireDisappearing(previous) THEN remove every page above
        // the root + set CurrentPage=root.
        previous->send_disappearing();
        stack_.erase(stack_.begin() + 1, stack_.end());
        // C# firePostNavigatingEvents: FireAppearing(root) — still before RequestNavigation.
        root->send_appearing();

        notify_request_navigation();
    }

    void navigation_page::notify_request_navigation()
    {
        // C# IStackNavigation.RequestNavigation: build the request from the current stack and hand it to
        // the handler. The handler hosts the new top-most page and (synchronously) reports completion.
        const maui::core::navigation_request request{.stack = stack_as_views(), .animated = false};
        request_navigation(request);
    }

    void navigation_page::set_handler(std::shared_ptr<maui::core::i_element_handler> value)
    {
        // Wire the seam through the base (creates the platform view + runs the property mapper), then host
        // the current page on the freshly-attached native container — C# NavigationPage.OnHandlerChangedCore
        // hosts the current page (and the platform mapper has no "request_navigation" PROPERTY to do it on
        // connect, since it is a command). A no-op when detaching (value null) or the stack is empty.
        view<maui::core::i_view>::set_handler(std::move(value));
        if (handler() && !stack_.empty())
        {
            notify_request_navigation();
        }
    }

    void navigation_page::request_navigation(const maui::core::navigation_request& request)
    {
        // C# NavigationPage: Handler?.Invoke(nameof(IStackNavigation.RequestNavigation), eventArgs). The
        // handler re-hosts the request's top-most page; with no handler attached this is a silent no-op.
        if (const auto& element_handler = handler())
        {
            element_handler->invoke("request_navigation", request);
        }
    }

    void navigation_page::navigation_finished(const std::vector<maui::core::i_view*>& /*stack*/)
    {
        // C# IStackNavigation.NavigationFinished: in C# this syncs the xplat stack to the realized native
        // stack and completes the pending navigation task. Here the xplat stack is already authoritative
        // (we mutate it before calling the handler, and there is no overlapping-navigation queue to
        // reconcile), so this is the synchronous completion acknowledgement — no reconciliation needed.
    }
} // namespace maui::controls

// Self-register the default handler for navigation_page (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::navigation_page, maui::core::navigation_page_handler)
