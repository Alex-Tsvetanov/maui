#pragma once
// maui::controls::menu_item_tracker<TItem>  <=  Microsoft.Maui.Controls.MenuItemTracker<TMenuItem>
//
// Aggregates the menu/toolbar items of a page HIERARCHY into one flat, priority-sorted list, and raises
// collection_changed whenever that aggregate may have changed. Ported from src/Controls/src/Core/Menu/
// MenuItemTracker.cs, scoped to the page types this port has:
//   - the aggregate = the target page's own items + (when the target is a navigation_page, C#'s
//     IPageContainer<Page> branch) the CURRENT page's items, recursively — then the additional targets'
//     items (deduplicated) — then one stable sort by the derived comparer (C# returnValue.Sort);
//   - tracking = subscribe the target's item collection, and for a navigation_page also its `navigated`
//     event (the port's CurrentPage-changed signal, fired by push/pop/pop_to_root) plus every stack
//     page's item collection (C# RegisterChildPage on each descendant page); a navigation re-registers
//     the stack pages and re-raises collection_changed.
// Scope notes (documented): FlyoutPage/Shell/TabbedPage branches and the PageAppearing/PagePropertyChanged
// relays wait for their controls; C#'s weak target refs become the port's caller-owns convention —
// tracked pages must outlive the tracker (or be untracked first), per PROFILE §8.

#include <algorithm>
#include <cstddef>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/menu_element_list.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    template <class TItem> class menu_item_tracker
    {
    public:
        menu_item_tracker() = default;
        virtual ~menu_item_tracker() = default;
        menu_item_tracker(const menu_item_tracker&) = delete;
        menu_item_tracker(menu_item_tracker&&) = delete;
        menu_item_tracker& operator=(const menu_item_tracker&) = delete;
        menu_item_tracker& operator=(menu_item_tracker&&) = delete;

        // C# MenuItemTracker.Target — the tracked root page (non-owning; null = untracked).
        [[nodiscard]] element* target() const
        {
            return target_;
        }
        void set_target(element* value)
        {
            if (target_ == value)
            {
                return;
            }
            untrack_target();
            target_ = value;
            if (target_ != nullptr)
            {
                track_target(*target_);
            }
            emit_collection_changed();
        }

        // C# MenuItemTracker.AdditionalTargets — extra pages whose items join the aggregate (untracked:
        // their collections are read at aggregation time only, like C#'s plain enumeration).
        [[nodiscard]] const std::vector<element*>& additional_targets() const
        {
            return additional_targets_;
        }
        void set_additional_targets(std::vector<element*> value)
        {
            additional_targets_ = std::move(value);
        }

        // C# MenuItemTracker.ToolbarItems — the aggregate, rebuilt (and re-sorted) on every read.
        [[nodiscard]] std::vector<TItem*> toolbar_items() const
        {
            std::vector<TItem*> result;
            if (target_ == nullptr)
            {
                return result;
            }
            collect(*target_, result);
            for (element* const page : additional_targets_)
            {
                if (page == nullptr)
                {
                    continue;
                }
                if (auto* list = get_menu_items(*page))
                {
                    for (TItem* const item : list->items())
                    {
                        if (std::ranges::find(result, item) == result.end())
                        {
                            result.push_back(item);
                        }
                    }
                }
            }
            std::stable_sort(result.begin(), result.end(),
                             [this](const TItem* lhs, const TItem* rhs) { return less(*lhs, *rhs); });
            return result;
        }

        // C# MenuItemTracker.CollectionChanged.
        maui::core::event<> collection_changed;

        // Re-register the target's child-page subscriptions and re-raise collection_changed. The C#
        // tracker hears stack edits through Page.DescendantAdded/Removed; the port's navigation events
        // cover push/pop/pop_to_root, so the SILENT stack changes (the root the NavigationPage(root)
        // ctor seeds, insert_page_before, remove_page) call this explicitly instead.
        void retrack()
        {
            if (auto* container = dynamic_cast<navigation_page*>(target_))
            {
                retrack_child_pages(*container);
            }
            emit_collection_changed();
        }

    protected:
        // C# GetMenuItems(Page) — the page's item collection for this tracker's item type (null when the
        // element is not a page carrying one).
        [[nodiscard]] virtual menu_element_list<TItem>* get_menu_items(element& page) const = 0;
        // C# CreateComparer() — the sort order (strict-weak less).
        [[nodiscard]] virtual bool less(const TItem& lhs, const TItem& rhs) const = 0;

    private:
        void emit_collection_changed()
        {
            collection_changed.raise();
        }

        // C# GetCurrentToolbarItems: the page's own items, then — for a page container — the CURRENT
        // page's, recursively (the IPageContainer<Page> branch; FlyoutPage/Shell are out of scope).
        void collect(element& page, std::vector<TItem*>& out) const
        {
            if (auto* list = get_menu_items(page))
            {
                for (TItem* const item : list->items())
                {
                    out.push_back(item);
                }
            }
            if (auto* container = dynamic_cast<navigation_page*>(&page))
            {
                if (content_page* const current = container->current_page())
                {
                    collect(*current, out);
                }
            }
        }

        // C# TrackTarget: subscribe the target's own collection; a navigation_page additionally reports
        // navigation (CurrentPage changes) and gets every stack page registered as a child page.
        void track_target(element& page)
        {
            if (auto* list = get_menu_items(page))
            {
                target_connections_.emplace_back(list->changed,
                                                 list->changed.connect([this] { emit_collection_changed(); }));
            }
            if (auto* container = dynamic_cast<navigation_page*>(&page))
            {
                target_connections_.emplace_back(container->navigated,
                                                 container->navigated.connect([this, container](content_page* const&) {
                                                     retrack_child_pages(*container);
                                                     emit_collection_changed();
                                                 }));
                retrack_child_pages(*container);
            }
        }

        void untrack_target()
        {
            target_connections_.clear();
            child_connections_.clear();
        }

        // C# RegisterChildPage for every page on the (new) stack — rebuilt whole on each navigation.
        void retrack_child_pages(navigation_page& container)
        {
            child_connections_.clear();
            for (content_page* const page : container.navigation_stack())
            {
                if (auto* list = get_menu_items(*page))
                {
                    child_connections_.emplace_back(list->changed,
                                                    list->changed.connect([this] { emit_collection_changed(); }));
                }
            }
        }

        element* target_ = nullptr;                // non-owning (C# holds a WeakReference)
        std::vector<element*> additional_targets_; // non-owning
        std::vector<maui::core::scoped_connection> target_connections_;
        std::vector<maui::core::scoped_connection> child_connections_;
    };
} // namespace maui::controls
