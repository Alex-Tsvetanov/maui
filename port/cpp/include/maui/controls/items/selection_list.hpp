#pragma once
// maui::controls::selection_list  <=  Microsoft.Maui.Controls.SelectionList (internal)
//
// The IList<object> face of SelectableItemsView.SelectedItems: every mutation notifies the owning
// view (SelectedItemsPropertyChanged) with the PRE-change shadow copy as the old selection and the
// live list as the new, then brings the shadow up to date — the C# choreography, verbatim.
//
// The list may wrap an EXTERNAL observable collection (a viewmodel-bound SelectedItems): changes made
// directly on that collection (not through this face) also raise a selection change — the
// _externalChange flag suppresses the doubled notification for mutations this list itself performs.
// A plain vector (the C# non-INCC IList) and the default empty list get no subscription.
//
// Storage is core::observable_collection<boxed_item> in every flavor (owned when not supplied), so
// the change-feed wiring is uniform; boxed_item equality drives Contains/IndexOf/Remove (C#
// object.Equals).

#include <cstddef>
#include <memory>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/core/event.hpp"
#include "maui/core/observable_collection.hpp"

namespace maui::controls
{
    class selectable_items_view;

    class selection_list
    {
    public:
        // SelectionList(view) — the default empty list.
        explicit selection_list(selectable_items_view& owner);
        // SelectionList(view, IList<object> that is INotifyCollectionChanged) — wraps + subscribes.
        selection_list(selectable_items_view& owner,
                       std::shared_ptr<maui::core::observable_collection<boxed_item>> items);
        // SelectionList(view, a plain IList<object>) — wraps a copy, no subscription.
        selection_list(selectable_items_view& owner, std::vector<boxed_item> items);

        selection_list(const selection_list&) = delete;
        selection_list(selection_list&&) = delete;
        selection_list& operator=(const selection_list&) = delete;
        selection_list& operator=(selection_list&&) = delete;
        ~selection_list() = default;

        // ---- IList<object> (the exercised subset) ----
        void add(boxed_item item);
        void insert(std::size_t index, boxed_item item);
        // Remove the first equal occurrence; false when absent (no notification then, like C#).
        bool remove(const boxed_item& item);
        void remove_at(std::size_t index);
        void clear();

        [[nodiscard]] bool contains(const boxed_item& item) const;
        [[nodiscard]] int index_of(const boxed_item& item) const;
        [[nodiscard]] std::size_t count() const;
        [[nodiscard]] const boxed_item& at(std::size_t index) const;
        // The live list's contents (the C# enumerator view).
        [[nodiscard]] const std::vector<boxed_item>& items() const;

    private:
        void on_collection_changed();
        [[nodiscard]] std::vector<boxed_item> copy() const;

        selectable_items_view* owner_;
        std::shared_ptr<maui::core::observable_collection<boxed_item>> internal_;
        std::vector<boxed_item> shadow_;
        bool external_change_ = false;
        // Declared after internal_ (§8): the subscription disconnects before the pinned collection dies.
        maui::core::scoped_connection subscription_;
    };
} // namespace maui::controls
