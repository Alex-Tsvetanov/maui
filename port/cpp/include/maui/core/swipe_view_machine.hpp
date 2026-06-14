#pragma once
// maui::core::swipe_machine — the cross-platform swipe state-machine logic shared by every backend's
// swipe_view_handler (the pure part of the native MauiSwipeView.cs machine). The headless backend drives
// it with synthetic offsets; the apple/ios backends drive it from a real pan recognizer. Keeping the
// machine here (free functions over a swipe_view_state + the i_swipe_view) means the three backends share
// ONE faithful port instead of three copies. Derived from src/Core/src/Platform/iOS/MauiSwipeView.cs +
// SwipeViewExtensions.cs.
//
// The functions mutate the passed state and fan the SwipeStarted/Changing/Ended notifications +
// IsOpen write-back out through the i_swipe_view. They are platform-agnostic — a backend layers its
// native reveal/animation on top (the visual; the machine is the behavior).

#include "maui/core/swipe_direction.hpp"
#include "maui/core/swipe_view_handler_state.hpp"
#include "maui/core/swipe_view_requests.hpp"

namespace maui::core
{
    class i_swipe_view;

    namespace swipe_machine
    {
        // C# MauiSwipeView constants (exposed so on-device tests can assert against the same thresholds).
        inline constexpr double minimum_open_threshold_percent = 0.15; // MinimumOpenSwipeThresholdPercentage
        inline constexpr double open_threshold_percent = 0.60;         // OpenSwipeThresholdPercentage
        inline constexpr double default_swipe_threshold = 250;         // SwipeViewExtensions.SwipeThreshold
        inline constexpr double swipe_item_width = 100;                // SwipeViewExtensions.SwipeItemWidth

        // C# MauiSwipeView.GetSwipeThreshold(SwipeDirection): the open distance for a swipe in `direction`.
        [[nodiscard]] double swipe_threshold(i_swipe_view& view, swipe_direction direction);

        // C# ProcessTouchMove (part 1): a swipe begins — set the direction + reset the offset. Idle state.
        void begin_swipe(swipe_view_state& state, swipe_direction direction);

        // C# ProcessTouchMove (part 2): push a new swipe offset. Raises SwipeStarted on the first move,
        // SwipeChanging on every move, and updates IsOpen (offset != 0).
        void swipe_to(swipe_view_state& state, i_swipe_view& view, double offset);

        // C# ProcessTouchUp: end the swipe — RaiseSwipeEnded, then ValidateSwipeThreshold (Execute mode
        // invokes the first visible item then closes unless RemainOpen; Reveal settles open past 60%; below
        // resets).
        void end_swipe(swipe_view_state& state, i_swipe_view& view);

        // C# ProgrammaticallyOpenSwipeItem: open toward the requested side (state → Open, IsOpen written
        // back).
        void programmatically_open(swipe_view_state& state, i_swipe_view& view, const swipe_view_open_request& request);

        // C# ResetSwipe: close (state → Idle, IsOpen false, written back through the view).
        void reset_swipe(swipe_view_state& state, i_swipe_view& view);
    } // namespace swipe_machine
} // namespace maui::core
