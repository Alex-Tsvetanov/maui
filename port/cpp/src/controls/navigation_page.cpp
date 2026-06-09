// maui::controls::navigation_page — the push/pop stack orchestration + the page lifecycle ordering, the
// navigation chrome state, the modal overlay stack, the INavigation stack edits, plus the
// i_stack_navigation back-channel and the default-handler self-registration. See navigation_page.hpp.
//
// The push/pop ORDER mirrors NavigationPage.cs's MauiNavigationImpl (OnPushAsync / OnPopAsync /
// OnPopToRootAsync) run through SendHandlerUpdateAsync: mutate the stack + set the current page, then
// fire Disappearing(previous) + Appearing(new) — BOTH before RequestNavigation — then notify the handler.
// The modal ORDER mirrors NavigationModel.PushModal/PopModal. C#'s async/overlap machinery is collapsed
// to a synchronous single transition (the AppKit NSView swap completes inline; no overlap to guard here).

#include "maui/controls/navigation_page.hpp"

#include <algorithm>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/navigation_page_handler.hpp"
#include "maui/core/navigation_request.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    // Shared bindable-property descriptors for the bar styling (C# NavigationPage.BarBackgroundColorProperty /
    // BarTextColorProperty). One instance per type; the default is an unset (default-constructed) color — the
    // navigation_page's *_set_ flag distinguishes "never set" so the handler keeps the system default.
    const maui::core::bindable_property<maui::graphics::color>& navigation_page::bar_background_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"bar_background_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& navigation_page::bar_text_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"bar_text_color"};
        return descriptor;
    }

    std::vector<maui::core::i_view*> navigation_page::as_views(const std::vector<content_page*>& pages)
    {
        std::vector<maui::core::i_view*> views;
        views.reserve(pages.size());
        for (content_page* const page : pages)
        {
            views.push_back(page);
        }
        return views;
    }

    content_page* navigation_page::current_visible_page() const
    {
        // NavigationModel.CurrentPage: the top of the last nav tree — i.e. the top modal if any, else the
        // navigation stack's current page (each modal is its own root in C#'s _navTree).
        if (!modal_stack_.empty())
        {
            return modal_stack_.back();
        }
        return current_page();
    }

    bool navigation_page::contains(content_page* page) const
    {
        return std::ranges::find(stack_, page) != stack_.end() ||
               std::ranges::find(modal_stack_, page) != modal_stack_.end();
    }

    void navigation_page::push_initial(content_page& root)
    {
        stack_.push_back(&root);
        attach_logical_child(root); // the page inherits this nav page's BindingContext + Window
        // The window's role at this layer: make the root visible. A later push then fires the root's
        // send_disappearing() correctly (it is now in the appeared state).
        root.send_appearing();
        // Host the root on the handler (if already attached) so the native container shows it.
        notify_request_navigation(false);
    }

    void navigation_page::push(content_page& page, bool animated)
    {
        // C# OnPushAsync: pushing a page already in any navigation context is a no-op (Contains).
        if (contains(&page))
        {
            return;
        }

        // C# PushAsync → OnPushAsync fires events even for the first page (unlike the NavigationPage(root)
        // ctor, which calls PushPage directly with no events — that path is push_initial). previousPage =
        // CurrentPage (null for an empty stack); SendNavigating with a null page is a no-op in C#.
        content_page* const previous = current_page();

        // C# OnPushAsync processStackChanges: SendNavigating(navType, previousPage) is fired FIRST, before
        // the stack is modified (so event ordering is consistent across platforms).
        if (previous != nullptr)
        {
            navigating.raise(previous);
        }

        // C# PushPage(root): the new page becomes the top-most (and CurrentPage).
        stack_.push_back(&page);
        attach_logical_child(page); // inherit this nav page's BindingContext + Window

        // C# OnPushAsync firePostNavigatingEvents: FireDisappearing(previous) + FireAppearing(new), both
        // fired BEFORE RequestNavigation. (On the first page the navigation_page stands in for the window and
        // appears the new page; there is no previous to disappear.)
        if (previous != nullptr)
        {
            previous->send_disappearing();
        }
        page.send_appearing();

        notify_request_navigation(animated);

        // C# OnPushAsync fireNavigatedEvents: SendNavigated(previousPage) then Pushed.Invoke(root) — AFTER
        // the transition.
        navigated.raise(previous);
        pushed.raise(&page);
    }

    content_page* navigation_page::pop(bool animated)
    {
        // C# OnPopAsync: the root cannot be popped (InternalChildren.Count == 1 → return null).
        if (stack_.size() <= 1)
        {
            return nullptr;
        }

        content_page* const current = current_page();
        content_page* const new_current = stack_[stack_.size() - 2];

        // C# OnPopAsync processStackChanges: SendNavigating(Pop, currentPage) FIRST, then
        // FireDisappearing(current), remove, set CurrentPage=new.
        navigating.raise(current);
        current->send_disappearing();
        stack_.pop_back();
        detach_logical_child(*current); // the popped page leaves the tree (loses Window + inherited context)
        // C# OnPopAsync firePostNavigatingEvents: FireAppearing(new) — still before RequestNavigation.
        new_current->send_appearing();

        notify_request_navigation(animated);

        // C# OnPopAsync fireNavigatedEvents: SendNavigated(currentPage) then Popped.Invoke(currentPage).
        navigated.raise(current);
        popped.raise(current);
        return current;
    }

    void navigation_page::pop_to_root(bool animated)
    {
        // C# OnPopToRootAsync: a single-page (or empty) stack is a no-op.
        if (stack_.size() <= 1)
        {
            return;
        }

        content_page* const previous = current_page();
        content_page* const root = root_page();

        // C# OnPopToRootAsync builds pagesToRemove = every page above the root, in stack order (bottom→top):
        // it pops from the top inserting each at index 0, so the result is stack_[1 .. end). Collect it
        // before erasing, for the PoppedToRoot event arg.
        const std::vector<content_page*> popped_pages(stack_.begin() + 1, stack_.end());

        // C# OnPopToRootAsync processStackChanges: SendNavigating(PopToRoot, previous) FIRST, then
        // FireDisappearing(previous), remove every page above the root, set CurrentPage=root.
        navigating.raise(previous);
        previous->send_disappearing();
        for (auto it = stack_.begin() + 1; it != stack_.end(); ++it)
        {
            detach_logical_child(**it); // each popped page leaves the tree
        }
        stack_.erase(stack_.begin() + 1, stack_.end());
        // C# firePostNavigatingEvents: FireAppearing(root) — still before RequestNavigation.
        root->send_appearing();

        notify_request_navigation(animated);

        // C# OnPopToRootAsync fireNavigatedEvents: SendNavigated(previous) then PoppedToRoot.Invoke(newPage,
        // pagesToRemove) — newPage is the root now current.
        navigated.raise(previous);
        popped_to_root.raise(root, popped_pages);
    }

    void navigation_page::push_modal(content_page& page, bool animated)
    {
        // Pushing a page already in any navigation context is a no-op (mirrors the push/InsertPageBefore
        // duplicate guards; C# would reparent-fault).
        if (contains(&page))
        {
            return;
        }

        // C# NavigationModel.PushModal: previousPage = the current visible page (top modal or nav current),
        // captured BEFORE the push.
        content_page* const previous = current_visible_page();
        modal_stack_.push_back(&page);
        attach_logical_child(page); // the modal inherits this nav page's BindingContext + Window

        // C# PushModal: previousPage.SendDisappearing() then page.SendAppearing().
        if (previous != nullptr)
        {
            previous->send_disappearing();
        }
        page.send_appearing();

        notify_request_modal_navigation(animated);
    }

    content_page* navigation_page::pop_modal(bool animated)
    {
        // C# NavigationModel.PopModal throws on an empty modal stack; we return null (documented deviation).
        if (modal_stack_.empty())
        {
            return nullptr;
        }

        // C# PopModal: previousPage = the current visible page (the top modal being popped), captured first.
        content_page* const modal = modal_stack_.back();
        content_page* const previous = current_visible_page(); // == modal (top of the modal stack)
        modal_stack_.pop_back();
        detach_logical_child(*modal);                             // the popped modal leaves the tree
        content_page* const new_visible = current_visible_page(); // the revealed page (next modal or nav top)

        // C# PopModal: previousPage.SendDisappearing() then CurrentPage.SendAppearing().
        previous->send_disappearing();
        if (new_visible != nullptr)
        {
            new_visible->send_appearing();
        }

        notify_request_modal_navigation(animated);
        return modal;
    }

    void navigation_page::insert_page_before(content_page& page, content_page& before)
    {
        // C# MauiNavigationImpl.OnInsertPageBefore: `before` must be on the stack and `page` must not
        // already be present (C# throws otherwise; we no-op those cases — no exceptions in the seam).
        const auto before_it = std::ranges::find(stack_, &before);
        if (before_it == stack_.end() || contains(&page))
        {
            return;
        }
        stack_.insert(before_it, &page);
        attach_logical_child(page); // the inserted (off-screen) page still inherits context + window
        // C# OnInsertPageBefore: if inserted at index 0, RootPage becomes the inserted page — root_page()
        // is derived from stack_.front(), so the insert already updated it (no explicit field to set).
        // The inserted page is off-screen — no lifecycle fires and the current page is unchanged — but the
        // realized stack changed, so re-issue the (non-animated) navigation request (C# NavigationType.Insert
        // is unanimated). hosted_page stays the same; this keeps the handler's stack mirror in sync.
        notify_request_navigation(false);
    }

    void navigation_page::remove_page(content_page& page)
    {
        // C# MauiNavigationImpl.OnRemovePage: removing the current page (when it is NOT the sole root)
        // routes through PopAsync.
        if (&page == current_page())
        {
            if (stack_.size() <= 1)
            {
                // C# throws InvalidOperationException (cannot remove the root when it is also current); we
                // no-op (documented deviation).
                return;
            }
            pop();
            return;
        }

        // An inner / non-current page: drop it with no lifecycle (C# RemoveFromInnerChildren + the page's
        // handler disconnect). A page not on the stack is ignored (C# throws; we no-op).
        const auto it = std::ranges::find(stack_, &page);
        if (it == stack_.end())
        {
            return;
        }
        stack_.erase(it);
        detach_logical_child(page); // the removed page leaves the tree (loses Window + inherited context)
        // The realized stack changed but the visible page did not; sync the handler's mirror (unanimated).
        notify_request_navigation(false);
    }

    bool navigation_page::send_back_button_pressed()
    {
        // C# NavigationPage.OnBackButtonPressed: StackDepth > 1 → SafePop() and report handled. (The
        // page-level CurrentPage.SendBackButtonPressed() hook is deferred — see the header.)
        if (back_button_visible())
        {
            pop();
            return true;
        }
        return false;
    }

    void navigation_page::set_title_view(maui::core::i_view* value)
    {
        // C# NavigationPage.SetTitleView (the attached TitleView): swap the bar's title view. NON-owning; a
        // null clears it (the title label returns). Re-issue the navigation request so host_current re-reads
        // the chrome and hosts (or clears) the title view in the bar. A no-op when unchanged.
        if (title_view_ == value)
        {
            return;
        }
        title_view_ = value;
        if (!stack_.empty())
        {
            notify_request_navigation(false); // refresh the bar; the visible page is unchanged (unanimated)
        }
    }

    void navigation_page::on_property_changed(std::string_view name)
    {
        // Route the change through the view<> base first (generic IView property maps + visual state).
        view<maui::core::i_view>::on_property_changed(name);
        // The bar colors have no property map on the navigation handler (the bar is driven by the navigation
        // COMMAND, which reads the chrome state). Re-issue the navigation request so host_current re-applies
        // the bar styling. Only do so when there is a stack to host (else there is no bar content yet).
        if ((name == "bar_background_color" || name == "bar_text_color") && !stack_.empty())
        {
            notify_request_navigation(false);
        }
    }

    void navigation_page::notify_request_navigation(bool animated)
    {
        // C# IStackNavigation.RequestNavigation: build the request from the current stack and hand it to
        // the handler. The handler hosts the new top-most page and (synchronously) reports completion.
        const maui::core::navigation_request request{.stack = as_views(stack_), .animated = animated};
        request_navigation(request);
    }

    void navigation_page::notify_request_modal_navigation(bool animated)
    {
        // Drive the modal overlay: hand the modal stack to the handler so it overlays the top modal (or
        // clears the overlay when the modal stack is empty). The cross-platform shape reuses
        // navigation_request (the modal stack as i_view* + animated).
        if (const auto& element_handler = handler())
        {
            const maui::core::navigation_request request{.stack = as_views(modal_stack_), .animated = animated};
            element_handler->invoke("request_modal_navigation", request);
        }
    }

    void navigation_page::set_handler(std::shared_ptr<maui::core::i_element_handler> value)
    {
        // Wire the seam through the base (creates the platform view + runs the property mapper), then host
        // the current page on the freshly-attached native container — C# NavigationPage.OnHandlerChangedCore
        // hosts the current page (and the platform mapper has no "request_navigation" PROPERTY to do it on
        // connect, since it is a command). A no-op when detaching (value null) or the stack is empty.
        view<maui::core::i_view>::set_handler(std::move(value));
        if (!handler())
        {
            return; // detaching — nothing to host
        }
        if (!stack_.empty())
        {
            notify_request_navigation(false);
        }
        // Re-establish any modal overlay too (a handler attached after a modal was pushed) — independent of
        // the nav stack, since the modal lives on its own stack.
        if (!modal_stack_.empty())
        {
            notify_request_modal_navigation(false);
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
