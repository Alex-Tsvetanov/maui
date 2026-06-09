#pragma once
// maui::controls::navigation_page  <=  Microsoft.Maui.Controls.NavigationPage
//
// A page that manages a push/pop STACK of content_pages, showing the top-most one, with a navigation
// bar (title + back button), a separate modal stack overlaying the page stack, and INavigation stack
// edits. Ported from NavigationPage.cs (the MauiNavigationImpl push/pop orchestration) +
// IStackNavigationView + INavigation, and NavigationModel.cs (the modal PushModal/PopModal ordering):
//   - The stack is a vector of NON-owning content_page* (PROFILE §8: the caller owns the pages).
//   - push/pop/pop_to_root mutate the stack, fire the page Appearing/Disappearing lifecycle in C#'s
//     ORDER, then notify the handler (a "request_navigation" command) which re-hosts the new top-most
//     page's native view in its container and synchronously reports completion (navigation_finished).
//   - CHROME: the handler builds a custom navigation bar (AppKit has no UINavigationController) showing
//     bar_title() (the current page's Title) + a back button enabled when back_button_visible() (depth
//     > 1). send_back_button_pressed() routes to pop() (C# OnBackButtonPressed → SafePop).
//   - ANIMATION: push/pop/pop_to_root take an `animated` flag (default true, like C# PushAsync(page)),
//     threaded into the navigation_request. The transition stays synchronous (navigation_finished inline)
//     on every backend; the apple twin cross-fades the content swap when animated, headless is a no-op.
//   - MODAL: a SEPARATE stack (modal_stack()) overlaying the page stack — push_modal/pop_modal fire the
//     lifecycle per NavigationModel.PushModal/PopModal and drive a "request_modal_navigation" command so
//     the handler overlays the modal's native view (popping restores the underlying page).
//   - STACK EDITS: insert_page_before / remove_page (INavigation) port MauiNavigationImpl.
//     OnInsertPageBefore / OnRemovePage (incl. removing the current page routes through pop()).
//   - Shell, BarBackground/BarTextColor styling, TitleView, hardware-back Page.SendBackButtonPressed,
//     and overlapping-navigation queueing remain DEFERRED (documented in PROJECT/STATUS).
//
// ORDER (NavigationPage.cs MauiNavigationImpl.OnPushAsync/OnPopAsync/OnPopToRootAsync via
// SendHandlerUpdateAsync): mutate the stack + set the new current page, then fire FireDisappearing(prev)
// + FireAppearing(new) — BOTH before RequestNavigation — then RequestNavigation(stack); the handler does
// the native transition and (here synchronously) calls navigation_finished. We collapse C#'s async
// machinery (semaphore / TaskCompletionSource / overlapping-request queue) to a synchronous single
// transition: there is no overlap to guard, and the AppKit NSView swap completes inline.
//
// MODAL ORDER (NavigationModel.PushModal/PopModal): push_modal — previousVisible.SendDisappearing() then
// modal.SendAppearing(); pop_modal — modal.SendDisappearing() then newVisible.SendAppearing(). The
// "visible" page is the top modal if any, else the navigation stack's current page; the modal stack
// lives ON the navigation_page here (C# keeps it on Window.Navigation — a documented simplification since
// this layer has no Window-level modal host).
//
// HasAppeared gating: C#'s FireDisappearing/FireAppearing only fire once the navigation page itself has
// appeared (its window made it visible). There is no window lifecycle at this layer, so the
// navigation_page drives the ROOT page's initial send_appearing() itself when constructed with a root
// (or on the first push onto an empty stack) — standing in for the window — so a subsequent push then
// correctly fires send_disappearing() on the now-hidden root. Documented deviation.

#include <functional>
#include <memory>
#include <string_view>
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
        navigation_page()
        {
            this->set_style_target_type<navigation_page>(); // implicit / class style match
        }
        // Construct with a root page already pushed (C# NavigationPage(Page root) → PushPage(root)). The
        // root is made the initial current page and appears (the window's role, stood in for here).
        explicit navigation_page(content_page& root)
        {
            this->set_style_target_type<navigation_page>();
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

        // ---- the modal stack (top-most modal LAST), C# INavigation.ModalStack ----
        // A SEPARATE stack overlaying the navigation stack; empty until the first push_modal.
        [[nodiscard]] const std::vector<content_page*>& modal_stack() const
        {
            return modal_stack_;
        }

        // C# NavigationPage.PushAsync (MauiNavigationImpl.OnPushAsync): push `page` onto the stack and
        // make it current. Pushing a page already on the stack is a no-op (C#: InternalChildren.Contains).
        // `animated` (default true, like C# PushAsync(page) → PushAsync(page, true)) is threaded into the
        // navigation_request for the handler's transition.
        void push(content_page& page, bool animated = true);

        // C# NavigationPage.PopAsync (OnPopAsync): pop the top-most page, returning it (non-owning), and
        // make the page beneath it current. The root cannot be popped — popping a single-page stack is a
        // no-op and returns null (C#: InternalChildren.Count == 1 → return null).
        content_page* pop(bool animated = true);

        // C# NavigationPage.PopToRootAsync (OnPopToRootAsync): pop every page above the root, making the
        // root current. A no-op on a single-page (or empty) stack.
        void pop_to_root(bool animated = true);

        // ---- modal navigation (C# INavigation.PushModalAsync / PopModalAsync; ordering from
        // NavigationModel.PushModal / PopModal) ----
        // Push `page` as a modal overlaying the current visible page; the underlying page disappears and
        // the modal appears (in that order). Pushing a page already on a stack is a no-op.
        void push_modal(content_page& page, bool animated = true);
        // Pop the top modal, returning it (non-owning); the modal disappears and the revealed page
        // reappears. A no-op returning null when there is no modal (C# throws; we return null — deviation).
        content_page* pop_modal(bool animated = true);

        // ---- INavigation stack edits ----
        // C# MauiNavigationImpl.OnInsertPageBefore: insert `page` immediately before `before` in the
        // stack (RootPage updates if inserted at index 0). C# throws if `before` is absent or `page` is
        // already present; we no-op those cases (documented deviation — no exceptions in the seam). The
        // inserted page is off-screen, so no lifecycle fires.
        void insert_page_before(content_page& page, content_page& before);

        // C# MauiNavigationImpl.OnRemovePage: remove `page` from the stack. Removing the CURRENT page
        // routes through pop() (the revealed page appears). Removing an inner/root non-current page just
        // drops it with no lifecycle. C# throws when removing the sole root page; we no-op it. A page not
        // on the stack is ignored.
        void remove_page(content_page& page);

        // ---- navigation chrome (read by the handler to populate the custom bar) ----
        // C# NavigationPageToolbar shows CurrentPage.Title; empty when the stack is empty.
        [[nodiscard]] std::string_view bar_title() const
        {
            return stack_.empty() ? std::string_view{} : stack_.back()->title();
        }
        // C# IToolbar.BackButtonVisible — back is shown only above the root (StackDepth > 1).
        [[nodiscard]] bool back_button_visible() const
        {
            return stack_.size() > 1;
        }
        // C# NavigationPage.OnBackButtonPressed: above the root → pop() and report handled (true); at the
        // root → unhandled (false). The custom bar's back button invokes this. (Page.SendBackButtonPressed
        // — the page-level hardware-back hook — is deferred; this is the StackDepth > 1 → SafePop branch.)
        // Overrides i_stack_navigation::send_back_button_pressed so the core handler can route the back
        // button without knowing this concrete type.
        bool send_back_button_pressed() override;

        // ---- i_stack_navigation chrome getters (the handler reads these to build the bar) ----
        [[nodiscard]] std::string_view navigation_bar_title() const override
        {
            return bar_title();
        }
        [[nodiscard]] bool navigation_back_button_visible() const override
        {
            return back_button_visible();
        }

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
        // Every page in the stack — and every modal — is a logical child, so BindingContext + Window
        // inherit down to them. content_page is-a element, so visiting needs no cast.
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            for (content_page* const page : stack_)
            {
                visit(*page);
            }
            for (content_page* const modal : modal_stack_)
            {
                visit(*modal);
            }
        }

    private:
        // Make `root` the initial current page + appear it (stands in for the window). Used by the
        // root-ctor and by the first push onto an empty stack.
        void push_initial(content_page& root);

        // Build the request payload (the current stack as i_view*) and hand it to the handler via the
        // "request_navigation" command (C# Handler.Invoke(nameof(RequestNavigation), request)).
        void notify_request_navigation(bool animated);

        // Build the modal request payload (the modal stack as i_view*) and hand it to the handler via the
        // "request_modal_navigation" command so the handler overlays the top modal (or clears the overlay).
        void notify_request_modal_navigation(bool animated);

        // The current visible page: the top modal if any, else the navigation stack's current page
        // (NavigationModel.CurrentPage — _navTree.Last().Last(), the modal roots being separate stacks).
        [[nodiscard]] content_page* current_visible_page() const;

        // A given stack as i_view* (the navigation_request payload shape).
        [[nodiscard]] static std::vector<maui::core::i_view*> as_views(const std::vector<content_page*>& pages);

        // Whether `page` is on either the navigation stack or the modal stack (C# treats a page already in
        // any navigation context as ineligible to be pushed again).
        [[nodiscard]] bool contains(content_page* page) const;

        std::vector<content_page*> stack_;       // NON-owning: the caller owns the pages' lifetime
        std::vector<content_page*> modal_stack_; // NON-owning: the modal overlay stack
    };
} // namespace maui::controls
