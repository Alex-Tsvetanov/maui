#pragma once
// maui::controls::swipe_items  <=  Microsoft.Maui.Controls.SwipeItems
//
// A collection of swipe items, plus the Mode + SwipeBehaviorOnInvoked that govern a swipe over them.
// Ported from src/Controls/src/Core/SwipeView/SwipeItems.cs (SwipeItems : Element, IList<ISwipeItem>,
// INotifyCollectionChanged): an ObservableCollection<ISwipeItem> with two bindable properties (Mode
// default Reveal, SwipeBehaviorOnInvoked default Auto) whose items are parented as the collection's
// logical children (AddLogicalChild on add, RemoveLogicalChild on remove/clear — so BindingContext
// inherits down into each item). The collection is itself an Element so it joins the SwipeView's logical
// tree.
//
// OWNERSHIP: the item pointers are NON-owning (the caller owns each item's lifetime, PROFILE §8 — the
// C# ObservableCollection co-owns via GC; in the port the test/developer owns the items, declared before
// the collection per the teardown doctrine). The concrete items (swipe_item / swipe_item_view) are both
// `element` and i_swipe_item; the list stores the i_swipe_item face and cross-casts to element for the
// logical-tree hooks.
//
// The collection raises `changed` after every mutation (the ObservableCollection.CollectionChanged the
// SwipeView subscribes to to re-push the items to the handler). Mode/SwipeBehaviorOnInvoked changes flow
// through the bindable property_changed signal (the SwipeItems.PropertyChanged the SwipeView also
// forwards to the handler).

#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_swipe_item.hpp"
#include "maui/core/i_swipe_items.hpp"
#include "maui/core/property.hpp"
#include "maui/core/swipe_behavior_on_invoked.hpp"
#include "maui/core/swipe_mode.hpp"

namespace maui::controls
{
    class swipe_items : public element, public maui::core::i_swipe_items
    {
    public:
        swipe_items() = default;

        // Shared bindable-property descriptors (SwipeItems.ModeProperty / SwipeBehaviorOnInvokedProperty).
        static const maui::core::bindable_property<maui::core::swipe_mode>& mode_property();
        static const maui::core::bindable_property<maui::core::swipe_behavior_on_invoked>&
        behavior_on_invoked_property();

        // ObservableCollection.CollectionChanged — raised after every add/insert/remove/clear.
        maui::core::event<> changed;

        // ---- i_swipe_items (Mode + SwipeBehaviorOnInvoked + the IList<> read surface) ----
        [[nodiscard]] maui::core::swipe_mode mode() const override
        {
            return mode_.get();
        }
        void set_mode(maui::core::swipe_mode value)
        {
            mode_.set(value);
        }

        [[nodiscard]] maui::core::swipe_behavior_on_invoked behavior_on_invoked() const override
        {
            return behavior_on_invoked_.get();
        }
        void set_behavior_on_invoked(maui::core::swipe_behavior_on_invoked value)
        {
            behavior_on_invoked_.set(value);
        }

        [[nodiscard]] std::size_t count() const override
        {
            return items_.size();
        }
        [[nodiscard]] maui::core::i_swipe_item* at(std::size_t index) const override
        {
            return index < items_.size() ? items_[index] : nullptr;
        }

        // ---- the IList<ISwipeItem> mutation surface ----
        // Add/Insert/Remove/Clear parent (and un-parent) each item as a logical child, exactly where C#'s
        // OnSwipeItemsChanged calls AddLogicalChild/RemoveLogicalChild, then raise `changed`.
        void add(maui::core::i_swipe_item& item);
        void insert(std::size_t index, maui::core::i_swipe_item& item);
        bool remove(maui::core::i_swipe_item& item);
        void remove_at(std::size_t index);
        void clear();

        [[nodiscard]] bool empty() const
        {
            return items_.empty();
        }
        [[nodiscard]] std::ptrdiff_t index_of(const maui::core::i_swipe_item& item) const;

    protected:
        // The items are this collection's logical children, so BindingContext inherits down into them
        // (C# SwipeItems parents every item via AddLogicalChild).
        void for_each_logical_child(const std::function<void(element&)>& visit) const override;

        // Mode/SwipeBehaviorOnInvoked changes raise `changed` too (C# forwards SwipeItems.PropertyChanged
        // to the handler the same as a collection change). Protected to match the bindable_object base.
        void on_property_changed(std::string_view name) override;

    private:
        std::vector<maui::core::i_swipe_item*> items_; // NON-owning (see header)
        maui::core::property<maui::core::swipe_mode> mode_{*this, mode_property()};
        maui::core::property<maui::core::swipe_behavior_on_invoked> behavior_on_invoked_{
            *this, behavior_on_invoked_property()};
    };
} // namespace maui::controls
