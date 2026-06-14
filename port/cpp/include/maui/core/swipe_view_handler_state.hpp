#pragma once
// maui::core::swipe_view_state / swipe_machine_state — the swipe state-machine snapshot the
// swipe_view_handler's platform struct carries (the MauiSwipeView internal-state twin). Not a MAUI
// public type — it's the port's observable model of the native MauiSwipeView's _isSwiping / _isOpen /
// _swipeDirection / _swipeOffset fields, surfaced so the headless tests can assert the machine
// (Idle → Swiping → Open) the C# native view drives invisibly. Derived from src/Core/src/Platform/iOS/
// MauiSwipeView.cs.

#include <cstdint>

#include "maui/core/swipe_direction.hpp"

namespace maui::core
{
    // The three machine states (the C# native view has no enum — it derives these from _isSwiping /
    // _isOpen; the port names them so a test can assert the transition).
    enum class swipe_machine_state : std::uint8_t
    {
        idle = 0, // not swiping, not open (the rest state)
        swiping,  // a swipe is in progress (between begin_swipe and end_swipe)
        open      // settled open after a swipe past the open threshold (or a programmatic open)
    };

    // A snapshot of the swipe machine: the current state, the active swipe direction, the current offset
    // (signed: negative for left/up, positive for right/down — matching the C# _swipeOffset sign), and
    // whether the view is open (the IsOpen the machine writes back to the virtual view).
    struct swipe_view_state
    {
        swipe_machine_state state = swipe_machine_state::idle;
        swipe_direction direction = swipe_direction::none;
        double offset = 0;
        bool is_open = false;

        [[nodiscard]] bool operator==(const swipe_view_state&) const = default;
    };
} // namespace maui::core
