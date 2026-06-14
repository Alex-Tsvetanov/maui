#pragma once
// maui::core::i_swipe_items  <=  Microsoft.Maui.ISwipeItems
//
// A collection of swipe items, plus the Mode + SwipeBehaviorOnInvoked that govern how a swipe over them
// behaves. Ported from src/Core/src/Core/ISwipeItems.cs (ISwipeItems : IList<ISwipeItem>). The IList<>
// base collapses to the indexable read surface the swipe state machine needs (count() + at()) — the
// handler enumerates the items to find the first visible one to invoke and to size the reveal; it never
// MUTATES the collection through this contract (mutation is on the concrete swipe_items control), so the
// add/remove face is intentionally omitted from the virtual-view contract.

#include <cstddef>

#include "maui/core/swipe_behavior_on_invoked.hpp"
#include "maui/core/swipe_mode.hpp"

namespace maui::core
{
    class i_swipe_item;

    class i_swipe_items
    {
    public:
        virtual ~i_swipe_items() = default;

        // C# ISwipeItems.Mode — the effect of a swipe interaction (Reveal / Execute).
        [[nodiscard]] virtual swipe_mode mode() const = 0;

        // C# ISwipeItems.SwipeBehaviorOnInvoked — how the SwipeView behaves after an item is invoked.
        [[nodiscard]] virtual swipe_behavior_on_invoked behavior_on_invoked() const = 0;

        // The IList<ISwipeItem> read surface the state machine enumerates.
        [[nodiscard]] virtual std::size_t count() const = 0;
        [[nodiscard]] virtual i_swipe_item* at(std::size_t index) const = 0;

    protected:
        i_swipe_items() = default;
        i_swipe_items(const i_swipe_items&) = default;
        i_swipe_items(i_swipe_items&&) = default;
        i_swipe_items& operator=(const i_swipe_items&) = default;
        i_swipe_items& operator=(i_swipe_items&&) = default;
    };
} // namespace maui::core
