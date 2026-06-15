// maui::core::swipe_machine — the cross-platform swipe state-machine logic (the pure part of the native
// MauiSwipeView.cs machine), shared by every backend's swipe_view_handler. Ported from
// src/Core/src/Platform/iOS/MauiSwipeView.cs + SwipeViewExtensions.cs. See swipe_view_machine.hpp.

#include "maui/core/swipe_view_machine.hpp"

#include <cmath>
#include <cstddef>
#include <numbers>

#include "maui/core/i_swipe_item.hpp"
#include "maui/core/i_swipe_item_menu_item.hpp"
#include "maui/core/i_swipe_item_view.hpp"
#include "maui/core/i_swipe_items.hpp"
#include "maui/core/i_swipe_view.hpp"
#include "maui/core/open_swipe_item.hpp"
#include "maui/core/swipe_behavior_on_invoked.hpp"
#include "maui/core/swipe_direction.hpp"
#include "maui/core/swipe_mode.hpp"
#include "maui/core/swipe_view_handler_state.hpp"
#include "maui/core/swipe_view_requests.hpp"
#include "maui/core/visibility.hpp"

namespace maui::core::swipe_machine
{
    namespace
    {
        // C# MauiSwipeView.GetIsVisible — skip collapsed items.
        bool get_is_visible(i_swipe_item& item)
        {
            if (auto* view = dynamic_cast<i_swipe_item_view*>(&item))
            {
                return view->visibility() == visibility::visible;
            }
            if (auto* menu = dynamic_cast<i_swipe_item_menu_item*>(&item))
            {
                return menu->visibility() == visibility::visible;
            }
            return true;
        }

        // C# MauiSwipeView.ExecuteSwipeItem — invoke only an ENABLED item.
        void execute_swipe_item(i_swipe_item& item)
        {
            bool is_enabled = true;
            if (auto* menu = dynamic_cast<i_swipe_item_menu_item*>(&item))
            {
                is_enabled = menu->is_enabled();
            }
            else if (auto* view = dynamic_cast<i_swipe_item_view*>(&item))
            {
                is_enabled = view->is_enabled();
            }
            if (is_enabled)
            {
                item.on_invoked();
            }
        }

        // C# MauiSwipeView.GetSwipeItemsByDirection(SwipeDirection): a Left swipe reveals the RIGHT items,
        // a Right swipe the LEFT items, an Up swipe the BOTTOM items, a Down swipe the TOP items.
        i_swipe_items* items_for_direction(i_swipe_view& view, swipe_direction direction)
        {
            switch (direction)
            {
                case swipe_direction::left:
                    return view.right_items();
                case swipe_direction::right:
                    return view.left_items();
                case swipe_direction::up:
                    return view.bottom_items();
                case swipe_direction::down:
                    return view.top_items();
                case swipe_direction::none:
                default:
                    return nullptr;
            }
        }

        bool is_horizontal(swipe_direction direction)
        {
            return direction == swipe_direction::left || direction == swipe_direction::right;
        }

        // The first VISIBLE item in a set (Execute mode invokes only it — #7580).
        i_swipe_item* first_visible(i_swipe_items& items)
        {
            for (std::size_t i = 0; i < items.count(); ++i)
            {
                if (i_swipe_item* const item = items.at(i); item != nullptr && get_is_visible(*item))
                {
                    return item;
                }
            }
            return nullptr;
        }

        // The signed open offset for a direction (left/up negative, right/down positive — the C#
        // _swipeOffset sign convention).
        double signed_threshold(swipe_direction direction, double threshold)
        {
            return (direction == swipe_direction::left || direction == swipe_direction::up) ? -threshold : threshold;
        }
    } // namespace

    // C# SwipeDirectionHelper.GetSwipeDirection / GetAngleFromPoints / GetSwipeDirectionFromAngle /
    // IsAngleInRange (Primitives/SwipeDirection.cs), ported verbatim: classify a drag into the single
    // dominant direction by the atan2 angle (0..360, screen coordinates with y down).
    swipe_direction get_swipe_direction(double x1, double y1, double x2, double y2)
    {
        const double rad = std::atan2(y1 - y2, x2 - x1) + std::numbers::pi;
        const double angle = std::fmod((rad * 180.0 / std::numbers::pi) + 180.0, 360.0);

        const auto in_range = [angle](double init, double end) { return angle >= init && angle < end; };
        if (in_range(45, 135))
        {
            return swipe_direction::up;
        }
        if (in_range(0, 45) || in_range(315, 360))
        {
            return swipe_direction::right;
        }
        if (in_range(225, 315))
        {
            return swipe_direction::down;
        }
        return swipe_direction::left;
    }

    // C# MauiSwipeView.GetSwipeThreshold: Element.Threshold wins; else Reveal sums the visible items'
    // widths and Execute uses the default. (The content-frame-relative sizing — contentWidth * 0.8,
    // ValidateSwipeThreshold's frame clamp — has no analog without a native content frame; the headless /
    // AppKit backends use this simplified form, documented in swipe_view_handler.hpp.)
    double swipe_threshold(i_swipe_view& view, swipe_direction direction)
    {
        const double explicit_threshold = view.threshold();
        if (explicit_threshold > 0)
        {
            return explicit_threshold;
        }
        const i_swipe_items* const items = items_for_direction(view, direction);
        if (items == nullptr)
        {
            return 0;
        }
        if (items->mode() == swipe_mode::reveal && is_horizontal(direction))
        {
            double total = 0;
            for (std::size_t i = 0; i < items->count(); ++i)
            {
                if (i_swipe_item* const item = items->at(i); item != nullptr && get_is_visible(*item))
                {
                    total += swipe_item_width;
                }
            }
            return total > 0 ? total : default_swipe_threshold;
        }
        // Vertical reveal (content-height-derived) and Execute mode both collapse to the default here.
        return default_swipe_threshold;
    }

    void begin_swipe(swipe_view_state& state, swipe_direction direction)
    {
        state = {.state = swipe_machine_state::idle, .direction = direction, .offset = 0, .is_open = state.is_open};
    }

    void swipe_to(swipe_view_state& state, i_swipe_view& view, double offset)
    {
        if (state.direction == swipe_direction::none)
        {
            return;
        }
        const bool first_move = state.state != swipe_machine_state::swiping;
        if (first_move)
        {
            view.notify_swipe_started({.direction = state.direction});
            state.state = swipe_machine_state::swiping;
        }
        state.offset = offset;
        const bool open = offset != 0;
        state.is_open = open;
        view.set_is_open(open);
        view.notify_swipe_changing({.direction = state.direction, .offset = offset});
    }

    void end_swipe(swipe_view_state& state, i_swipe_view& view)
    {
        if (state.state != swipe_machine_state::swiping)
        {
            return;
        }
        const swipe_direction direction = state.direction;
        view.notify_swipe_ended({.direction = direction, .is_open = state.is_open});

        // C# ValidateSwipeThreshold: if |offset| >= 60% of the threshold, open; else reset.
        const double threshold = swipe_threshold(view, direction);
        const double open_threshold = open_threshold_percent * threshold;
        if (open_threshold <= 0 || std::abs(state.offset) < open_threshold)
        {
            reset_swipe(state, view);
            return;
        }

        i_swipe_items* const items = items_for_direction(view, direction);
        if (items == nullptr)
        {
            reset_swipe(state, view);
            return;
        }
        if (items->mode() == swipe_mode::execute)
        {
            // Execute only the FIRST visible item (#7580).
            if (i_swipe_item* const item = first_visible(*items))
            {
                execute_swipe_item(*item);
            }
            if (items->behavior_on_invoked() != swipe_behavior_on_invoked::remain_open)
            {
                reset_swipe(state, view);
            }
            else
            {
                state.state = swipe_machine_state::open;
            }
        }
        else
        {
            // Reveal: settle open at the threshold.
            state.state = swipe_machine_state::open;
            state.offset = signed_threshold(direction, threshold);
            state.is_open = true;
            view.set_is_open(true);
        }
    }

    void programmatically_open(swipe_view_state& state, i_swipe_view& view, const swipe_view_open_request& request)
    {
        swipe_direction direction = swipe_direction::none;
        switch (request.item)
        {
            case open_swipe_item::left_items:
                direction = swipe_direction::right;
                break;
            case open_swipe_item::top_items:
                direction = swipe_direction::down;
                break;
            case open_swipe_item::right_items:
                direction = swipe_direction::left;
                break;
            case open_swipe_item::bottom_items:
                direction = swipe_direction::up;
                break;
        }
        const double threshold = swipe_threshold(view, direction);
        state = {.state = swipe_machine_state::open,
                 .direction = direction,
                 .offset = signed_threshold(direction, threshold),
                 .is_open = threshold != 0};
        view.set_is_open(state.is_open);
    }

    void reset_swipe(swipe_view_state& state, i_swipe_view& view)
    {
        state = {.state = swipe_machine_state::idle, .direction = swipe_direction::none, .offset = 0, .is_open = false};
        view.set_is_open(false);
    }
} // namespace maui::core::swipe_machine
