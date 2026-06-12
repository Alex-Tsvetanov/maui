#pragma once
// maui::controls::scroll_to_request_event_args  <=  Microsoft.Maui.Controls.ScrollToRequestEventArgs
// (+ ScrollToMode)
//
// The payload of ItemsView.ScrollTo: either a {index, group_index} position request or an
// {item, group} element request (the two C# ctors → the two factories; Mode records which). It rides
// the handler command seam as a std::any payload (the scroll_view scroll_to_request precedent) and
// is also delivered through the control's scroll_to_requested event.

#include <cstdint>
#include <utility>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/scroll_to_position.hpp"

namespace maui::controls
{
    // <= Microsoft.Maui.Controls.ScrollToMode.
    enum class scroll_to_mode : std::uint8_t
    {
        position = 0,
        element,
    };

    struct scroll_to_request_event_args
    {
        scroll_to_mode mode = scroll_to_mode::position;
        controls::scroll_to_position scroll_to_position = controls::scroll_to_position::make_visible;
        bool is_animated = true;
        // Position mode:
        int index = 0;
        int group_index = -1;
        // Element mode:
        boxed_item item{};
        boxed_item group{};

        // ScrollToRequestEventArgs(int, int, ScrollToPosition, bool).
        [[nodiscard]] static scroll_to_request_event_args for_position(int index, int group_index,
                                                                       controls::scroll_to_position position,
                                                                       bool is_animated)
        {
            scroll_to_request_event_args args;
            args.mode = scroll_to_mode::position;
            args.index = index;
            args.group_index = group_index;
            args.scroll_to_position = position;
            args.is_animated = is_animated;
            return args;
        }

        // ScrollToRequestEventArgs(object, object, ScrollToPosition, bool).
        [[nodiscard]] static scroll_to_request_event_args for_element(boxed_item item, boxed_item group,
                                                                      controls::scroll_to_position position,
                                                                      bool is_animated)
        {
            scroll_to_request_event_args args;
            args.mode = scroll_to_mode::element;
            args.item = std::move(item);
            args.group = std::move(group);
            args.scroll_to_position = position;
            args.is_animated = is_animated;
            return args;
        }
    };
} // namespace maui::controls
