#pragma once
// maui::core::swipe_view_{swipe_started,swipe_changing,swipe_ended,open_request,close_request}
//   <=  Microsoft.Maui.SwipeViewSwipeStarted / SwipeViewSwipeChanging / SwipeViewSwipeEnded /
//       SwipeViewOpenRequest / SwipeViewCloseRequest
//
// The tightly-coupled family of small POD records the ISwipeView swipe pipeline + open/close commands
// pass between the virtual view and its handler. Ported from src/Core/src/Primitives/
// SwipeViewSwipeStarted.cs / SwipeViewSwipeChanging.cs / SwipeViewSwipeEnded.cs /
// SwipeViewOpenRequest.cs / SwipeViewCloseRequest.cs — C# `record` types (value-equality POD), so they
// share one header (a value family, like point/size — PROFILE §3). Aggregate-initialized at the call
// sites; the defaulted operator== gives the C# record value-equality the swipe state machine compares on.

#include "maui/core/open_swipe_item.hpp"
#include "maui/core/swipe_direction.hpp"

namespace maui::core
{
    // C# SwipeViewSwipeStarted(SwipeDirection): the swipe-started notification the platform raises.
    struct swipe_view_swipe_started
    {
        swipe_direction direction = swipe_direction::none;

        [[nodiscard]] bool operator==(const swipe_view_swipe_started&) const = default;
    };

    // C# SwipeViewSwipeChanging(SwipeDirection, Offset): the per-frame swipe progress notification.
    struct swipe_view_swipe_changing
    {
        swipe_direction direction = swipe_direction::none;
        double offset = 0;

        [[nodiscard]] bool operator==(const swipe_view_swipe_changing&) const = default;
    };

    // C# SwipeViewSwipeEnded(SwipeDirection, IsOpen): the swipe-finished notification (IsOpen = whether
    // the view settled open).
    struct swipe_view_swipe_ended
    {
        swipe_direction direction = swipe_direction::none;
        bool is_open = false;

        [[nodiscard]] bool operator==(const swipe_view_swipe_ended&) const = default;
    };

    // C# SwipeViewOpenRequest(OpenSwipeItem, Animated): a programmatic Open request.
    struct swipe_view_open_request
    {
        open_swipe_item item = open_swipe_item::left_items;
        bool animated = true;

        [[nodiscard]] bool operator==(const swipe_view_open_request&) const = default;
    };

    // C# SwipeViewCloseRequest(Animated): a programmatic Close request.
    struct swipe_view_close_request
    {
        bool animated = true;

        [[nodiscard]] bool operator==(const swipe_view_close_request&) const = default;
    };
} // namespace maui::core
