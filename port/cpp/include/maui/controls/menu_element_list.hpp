#pragma once
// maui::controls::menu_element_list<TItem>  <=  the IList<T> + handler-notification shape shared by
// Microsoft.Maui.Controls.{MenuBar, MenuBarItem, MenuFlyout, MenuFlyoutSubItem} and the Page item
// collections (Page.ToolbarItems / Page.MenuBarItems, ObservableCollections in C#).
//
// One small owned-by-the-control list of NON-owning item pointers (the caller owns each item's
// lifetime, PROFILE §8) carrying the three behaviors every C# menu container repeats:
//   - LOGICAL-TREE hooks: the owner passes attach/detach callbacks (its protected
//     attach_logical_child/detach_logical_child), run exactly where C# Add/Insert/Remove call
//     AddLogicalChild/InsertLogicalChild/RemoveLogicalChild. A container that does NOT parent its
//     children (MenuBar) passes custom (or empty) hooks.
//   - HANDLER notification: NotifyHandler(action, index, item) — C# invokes the per-element handler's
//     Add/Remove/Insert command with a *HandlerUpdate payload; the port's menu tree carries no
//     per-element handlers (the window/view chrome rebuilds whole menus), so the same (action, index,
//     item) triple goes to an optional std::function seam the tests/chrome observe. Actions are
//     "add" / "remove" / "insert" (nameof(IMenuBarHandler.Add/Remove/Insert), snake-cased).
//   - a `changed` event raised after every mutation (the ObservableCollection.CollectionChanged the
//     menu/toolbar trackers subscribe to).
//
// this[index] setter semantics follow C#: RemoveAt(index) then Insert(index, value) — both
// notifications fire. clear() removes back-to-front via remove_at (C#'s Clear loop).

#include <cstddef>
#include <functional>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/core/event.hpp"

namespace maui::controls
{
    template <class TItem> class menu_element_list
    {
    public:
        using notify_fn = std::function<void(std::string_view action, std::size_t index, TItem*)>;
        using tree_hook = std::function<void(TItem&)>;

        menu_element_list() = default;
        menu_element_list(tree_hook attach, tree_hook detach) : attach_(std::move(attach)), detach_(std::move(detach))
        {
        }

        // The handler-notification seam (C# NotifyHandler → Handler?.Invoke(action, update)).
        void set_handler_notify(notify_fn value)
        {
            notify_ = std::move(value);
        }

        void add(TItem& item)
        {
            const std::size_t index = items_.size();
            items_.push_back(&item);
            run_attach(item);
            notify("add", index, &item);
        }

        void insert(std::size_t index, TItem& item)
        {
            if (index > items_.size())
            {
                index = items_.size();
            }
            items_.insert(items_.begin() + static_cast<std::ptrdiff_t>(index), &item);
            run_attach(item);
            notify("insert", index, &item);
        }

        bool remove(TItem& item)
        {
            const std::ptrdiff_t index = index_of(item);
            if (index < 0)
            {
                // C# Remove on an absent item: List.Remove returns false and the handler is still
                // notified with index -1; the port skips the meaningless notification.
                return false;
            }
            remove_at(static_cast<std::size_t>(index));
            return true;
        }

        void remove_at(std::size_t index)
        {
            if (index >= items_.size())
            {
                return;
            }
            TItem* const item = items_[index];
            items_.erase(items_.begin() + static_cast<std::ptrdiff_t>(index));
            run_detach(*item);
            notify("remove", index, item);
        }

        // C# this[index] = value: RemoveAt(index) then Insert(index, value).
        void set_at(std::size_t index, TItem& item)
        {
            remove_at(index);
            insert(index, item);
        }

        void clear()
        {
            for (std::size_t i = items_.size(); i > 0; --i)
            {
                remove_at(i - 1);
            }
        }

        [[nodiscard]] std::size_t count() const
        {
            return items_.size();
        }
        [[nodiscard]] TItem* at(std::size_t index) const
        {
            return index < items_.size() ? items_[index] : nullptr;
        }
        [[nodiscard]] bool contains(const TItem& item) const
        {
            return index_of(item) >= 0;
        }
        [[nodiscard]] std::ptrdiff_t index_of(const TItem& item) const
        {
            for (std::size_t i = 0; i < items_.size(); ++i)
            {
                if (items_[i] == &item)
                {
                    return static_cast<std::ptrdiff_t>(i);
                }
            }
            return -1;
        }
        [[nodiscard]] const std::vector<TItem*>& items() const
        {
            return items_;
        }

        // ObservableCollection.CollectionChanged — raised after every add/insert/remove (the trackers
        // subscribe to this).
        maui::core::event<> changed;

    private:
        void run_attach(TItem& item)
        {
            if (attach_)
            {
                attach_(item);
            }
        }
        void run_detach(TItem& item)
        {
            if (detach_)
            {
                detach_(item);
            }
        }
        void notify(std::string_view action, std::size_t index, TItem* item)
        {
            if (notify_)
            {
                notify_(action, index, item);
            }
            changed.raise();
        }

        std::vector<TItem*> items_; // NON-owning: the caller owns each item's lifetime
        tree_hook attach_;
        tree_hook detach_;
        notify_fn notify_;
    };
} // namespace maui::controls
