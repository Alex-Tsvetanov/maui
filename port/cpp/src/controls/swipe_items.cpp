// maui::controls::swipe_items — out-of-line definitions: the shared bindable-property descriptors and
// the IList<ISwipeItem> mutation surface (each add/insert/remove/clear parents/un-parents the item as a
// logical child, then raises `changed`). Ported from SwipeItems.cs.

#include "maui/controls/swipe_items.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <string_view>

#include "maui/controls/element.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_swipe_item.hpp"
#include "maui/core/swipe_behavior_on_invoked.hpp"
#include "maui/core/swipe_mode.hpp"

namespace maui::controls
{
    namespace
    {
        // The concrete swipe items (swipe_item / swipe_item_view) are both element and i_swipe_item; the
        // list stores the i_swipe_item face, so cross-cast to element for the logical-tree hooks.
        element* as_element(maui::core::i_swipe_item* item)
        {
            return dynamic_cast<element*>(item);
        }
    } // namespace

    const maui::core::bindable_property<maui::core::swipe_mode>& swipe_items::mode_property()
    {
        // C# SwipeItems.ModeProperty default is SwipeMode.Reveal.
        static const maui::core::bindable_property<maui::core::swipe_mode> descriptor{"mode",
                                                                                      maui::core::swipe_mode::reveal};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::swipe_behavior_on_invoked>& swipe_items::
        behavior_on_invoked_property()
    {
        // C# SwipeItems.SwipeBehaviorOnInvokedProperty default is SwipeBehaviorOnInvoked.Auto.
        static const maui::core::bindable_property<maui::core::swipe_behavior_on_invoked> descriptor{
            "swipe_behavior_on_invoked", maui::core::swipe_behavior_on_invoked::automatic};
        return descriptor;
    }

    void swipe_items::add(maui::core::i_swipe_item& item)
    {
        items_.push_back(&item);
        if (auto* child = as_element(&item))
        {
            attach_logical_child(*child);
        }
        changed.raise();
    }

    void swipe_items::insert(std::size_t index, maui::core::i_swipe_item& item)
    {
        index = std::min(index, items_.size()); // clamp to the end (C# List.Insert range)
        items_.insert(items_.begin() + static_cast<std::ptrdiff_t>(index), &item);
        if (auto* child = as_element(&item))
        {
            attach_logical_child(*child);
        }
        changed.raise();
    }

    bool swipe_items::remove(maui::core::i_swipe_item& item)
    {
        const std::ptrdiff_t index = index_of(item);
        if (index < 0)
        {
            return false;
        }
        remove_at(static_cast<std::size_t>(index));
        return true;
    }

    void swipe_items::remove_at(std::size_t index)
    {
        if (index >= items_.size())
        {
            return;
        }
        maui::core::i_swipe_item* const item = items_[index];
        items_.erase(items_.begin() + static_cast<std::ptrdiff_t>(index));
        if (auto* child = as_element(item))
        {
            detach_logical_child(*child);
        }
        changed.raise();
    }

    void swipe_items::clear()
    {
        // C# SwipeItems.Clear: RemoveLogicalChild for every item, then clear the backing collection. The
        // port un-parents back-to-front and raises a single `changed` (the Clear → Reset notification).
        for (maui::core::i_swipe_item* const item : items_)
        {
            if (auto* child = as_element(item))
            {
                detach_logical_child(*child);
            }
        }
        items_.clear();
        changed.raise();
    }

    std::ptrdiff_t swipe_items::index_of(const maui::core::i_swipe_item& item) const
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

    void swipe_items::for_each_logical_child(const std::function<void(element&)>& visit) const
    {
        for (maui::core::i_swipe_item* const item : items_)
        {
            if (auto* child = as_element(item))
            {
                visit(*child);
            }
        }
    }

    void swipe_items::on_property_changed(std::string_view name)
    {
        element::on_property_changed(name);
        // C# forwards SwipeItems.PropertyChanged (Mode / SwipeBehaviorOnInvoked) to the SwipeView handler
        // through the same path a collection change uses; mirror that by raising `changed` so the
        // SwipeView re-pushes the items.
        if (name == mode_property().name() || name == behavior_on_invoked_property().name())
        {
            changed.raise();
        }
    }
} // namespace maui::controls
