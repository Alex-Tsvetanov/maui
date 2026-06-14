#pragma once
// maui::core::i_swipe_item  <=  Microsoft.Maui.ISwipeItem
//
// An individual command in a SwipeView. Ported from src/Core/src/Core/ISwipeItem.cs (ISwipeItem :
// IElement): the AutomationId the native item carries and the OnInvoked() activation the swipe state
// machine fires when the item's command should execute. The C# IElement base (Handler/Parent) is not
// carried here — the swipe items are activated through this contract by the SwipeView's own state
// machine, not hosted by a per-item handler, so the minimal activation surface is what the handler
// needs (the concrete controls still derive the full element tree).

#include <string_view>

namespace maui::core
{
    class i_swipe_item
    {
    public:
        virtual ~i_swipe_item() = default;

        // C# ISwipeItem.AutomationId — the string that uniquely identifies the item.
        [[nodiscard]] virtual std::string_view automation_id() const = 0;

        // C# ISwipeItem.OnInvoked() — user interaction indicates this item's command should execute.
        virtual void on_invoked() = 0;

    protected:
        i_swipe_item() = default;
        i_swipe_item(const i_swipe_item&) = default;
        i_swipe_item(i_swipe_item&&) = default;
        i_swipe_item& operator=(const i_swipe_item&) = default;
        i_swipe_item& operator=(i_swipe_item&&) = default;
    };
} // namespace maui::core
