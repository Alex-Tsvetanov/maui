#pragma once
// maui::controls::shell_section (+ tab)  <=  Microsoft.Maui.Controls.ShellSection / Tab
//
// A group of shell_content tabs inside a shell_item, owning the section's NAVIGATION STACK (the
// pages pushed over the selected content). Ported from ShellSection.cs:
//   - items are OWNED shared_ptr<shell_content> children (PROFILE §8); current_item auto-selects
//     the first visible child on add and re-selects (or clears) when the current child is removed;
//   - the nav stack mirrors C#'s `List<Page> _navStack = { null }` — slot 0 is a nullptr standing
//     for the selected content's page, pushed pages follow; stack() exposes it verbatim (tests
//     index it exactly like C#);
//   - the stack mutators are PUBLIC VIRTUAL with the C# names (on_push / on_pop / on_pop_to_root /
//     on_remove_page / on_insert_page_before — Task-returning *Async collapsed to synchronous, the
//     handler seam being wave 4) so a test/subclass can observe them like C#'s protected overrides;
//     each proposes the navigation through the owning shell (ShellNavigatingEventArgs + deferral)
//     before mutating, and fires the page Appearing/Disappearing lifecycle in C#'s exact order;
//   - go_to applies a resolved shell_navigation_request to the stack (ShellSection.GoToAsync +
//     PrepareCurrentStackForBeingReplaced) — creating pages from routes, inserting middles,
//     removing excess paths and popping down to the target, all 1:1 minus the MODAL branches (the
//     port's shell has no modal stack; PresentationMode is likewise out of scope, so the per-page
//     "is navigation animated" probe collapses to the `animate` parameter / true);
//   - pages created FROM ROUTES are kept alive by this section while on the stack (owned_pages_);
//     externally-pushed pages stay caller-owned (their keep-alive entry is a non-owning handle).
//
// tab  <=  Microsoft.Maui.Controls.Tab — the alias subclass, declared here like the C# file does.

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_group_item.hpp"
#include "maui/controls/shell/shell_navigation_source.hpp"
#include "maui/controls/shell/shell_route_parameters.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    class content_page;
    class shell;
    class shell_item;
    class shell_navigation_request;

    class shell_section : public shell_group_item
    {
    public:
        shell_section() = default;
        ~shell_section() override;
        shell_section(const shell_section&) = delete;
        shell_section(shell_section&&) = delete;
        shell_section& operator=(const shell_section&) = delete;
        shell_section& operator=(shell_section&&) = delete;

        // ---- Items (ShellSection.Items — owned children) ----
        [[nodiscard]] const std::vector<std::shared_ptr<shell_content>>& items() const
        {
            return items_;
        }
        void add(std::shared_ptr<shell_content> content);
        // Adopt + add a bare page (the C# implicit ShellContent conversion at the collection seam).
        std::shared_ptr<shell_content> add(content_page& page);
        void remove(const shell_content& content);
        // IShellSectionController.GetItems — the visible (IsVisible) children, point-in-time.
        [[nodiscard]] std::vector<shell_content*> visible_items() const;

        // ---- CurrentItem (TwoWay-bindable in C#; the changed flow ported from OnCurrentItemChanged) ----
        [[nodiscard]] shell_content* current_item() const
        {
            return current_item_;
        }
        void set_current_item(shell_content* value);

        // ---- the navigation stack (ShellSection.Stack — slot 0 is the nullptr root marker) ----
        [[nodiscard]] const std::vector<content_page*>& stack() const
        {
            return nav_stack_;
        }
        // IShellSectionController.PresentedPage: the top pushed page, else the current content's page.
        [[nodiscard]] content_page* presented_page() const;
        [[nodiscard]] content_page* displayed_page() const
        {
            return displayed_page_;
        }
        void update_displayed_page();

        // ShellSection.IsVisibleSection: this is the shell's current item's current section.
        [[nodiscard]] bool is_visible_section() const;

        // The explicit `implicit operator ShellSection(ShellContent)` (CreateFromShellContent): an
        // already-parented content returns (and selects into) its existing section; otherwise a new
        // section adopts it (IMPL_ route + live Title/Icon/FlyoutIcon sync from the content).
        [[nodiscard]] static std::shared_ptr<shell_section> create_from_shell_content(
            std::shared_ptr<shell_content> content);

        // ---- stack mutators (C# protected virtual OnPushAsync/OnPopAsync/... — see header note) ----
        virtual void on_push(content_page& page, bool animated);
        virtual content_page* on_pop(bool animated);
        virtual void on_pop_to_root(bool animated);
        virtual void on_remove_page(content_page& page);
        virtual void on_insert_page_before(content_page& page, content_page& before);

        // Apply a resolved navigation request to this section's stack (ShellSection.GoToAsync).
        void go_to(const shell_navigation_request& request, shell_route_parameters& query_data,
                   std::optional<bool> animate, bool is_relative_popping);

        // ---- lifecycle fan-out (ShellSection.SendAppearing/SendDisappearing) ----
        void send_appearing() override;
        void send_disappearing() override;

        // The owning shell (Parent?.Parent as Shell) / shell_item (Parent), or null.
        [[nodiscard]] shell* containing_shell() const;
        [[nodiscard]] shell_item* parent_item() const;

        // SendStructureChanged: a visible section's structural change reaches the shell.
        void send_structure_changed() const;

        // Keep a route-created page alive while it sits on this section's stack (called by go_to's
        // creation path before the page enters the stack; dropped when the page leaves the stack).
        void retain_page(const std::shared_ptr<content_page>& page);

    protected:
        // Items + the pushed pages are this section's logical children (BindingContext inheritance).
        void for_each_logical_child(const std::function<void(element&)>& visit) const override;

    private:
        // C# PresentedPageDisappearing/PresentedPageAppearing (the ParentSet wait collapsed).
        void presented_page_disappearing();
        void presented_page_appearing();

        // ProposeNavigation through the owning shell; allowed when there is no shell yet.
        [[nodiscard]] bool propose_navigation(shell_navigation_source source, const std::vector<content_page*>* stack);

        // The GoToAsync helpers (ShellSection.PrepareCurrentStackForBeingReplaced et al, no modal).
        void prepare_current_stack_for_being_replaced(const shell_navigation_request& request,
                                                      shell_route_parameters& query_data, std::optional<bool> animate,
                                                      const std::vector<std::string>& global_routes,
                                                      bool is_relative_popping);
        void remove_excess_paths_within_the_route(const std::vector<std::string>& global_routes);
        [[nodiscard]] static std::shared_ptr<content_page> get_or_create_from_route(const std::string& route,
                                                                                    shell_route_parameters& query_data,
                                                                                    bool is_last, bool is_popping);
        void push_stack_of_pages(const std::vector<std::shared_ptr<content_page>>& pages, std::optional<bool> animate);

        // AddPage/RemovePage: the logical-child attach/detach around stack membership (+ keep-alive).
        void add_page(content_page& page);
        void remove_page(content_page& page);
        // The owning strong ref for a route-created page (empty for an externally-owned page) — used to
        // keep a page alive across the remove sequence (§8; C# relies on the GC + the caller's ref).
        [[nodiscard]] std::shared_ptr<content_page> page_owner(content_page& page);

        std::vector<std::shared_ptr<shell_content>> items_; // OWNED children
        shell_content* current_item_ = nullptr;             // borrowed view of one items_ entry
        std::vector<content_page*> nav_stack_{nullptr};     // C# _navStack — slot 0 is the root marker
        content_page* displayed_page_ = nullptr;
        // Route-created pages pinned while on the stack (externally-pushed entries are non-owning).
        std::map<content_page*, std::shared_ptr<content_page>> owned_pages_;
        // The CreateFromShellContent live sync: source + its subscription. Declared AFTER items_ so
        // the token disconnects before the owned source dies (§8); reset when the source is removed.
        shell_content* wrapper_source_ = nullptr;
        maui::core::scoped_connection wrapper_sync_token_;

        friend class shell_item; // create_from_shell_section reads/sets the wrapper sync
    };

    class tab : public shell_section
    {
    };
} // namespace maui::controls
