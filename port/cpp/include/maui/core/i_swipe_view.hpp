#pragma once
// maui::core::i_swipe_view  <=  Microsoft.Maui.ISwipeView
//
// The virtual-view contract for a container that reveals contextual commands on a swipe. Ported from
// src/Core/src/Core/ISwipeView.cs (ISwipeView : IContentView): the swipe Threshold, the four directional
// item collections (Left/Right/Top/Bottom), the IsOpen state (settable — the platform writes it back as
// the user swipes), the SwipeTransitionMode, the three inbound swipe notifications the platform raises
// (SwipeStarted/SwipeChanging/SwipeEnded → the control's events), and the RequestOpen/RequestClose
// commands the control routes to the handler. Content + Padding arrive through the i_content_view base.

#include "maui/core/i_content_view.hpp"
#include "maui/core/swipe_transition_mode.hpp"
#include "maui/core/swipe_view_requests.hpp"

namespace maui::core
{
    class i_swipe_items;

    class i_swipe_view : public i_content_view
    {
    public:
        // C# ISwipeView.Threshold — the minimum swipe distance for a swipe to be recognized.
        [[nodiscard]] virtual double threshold() const = 0;

        // C# ISwipeView.LeftItems / RightItems / TopItems / BottomItems — the directional command sets.
        [[nodiscard]] virtual i_swipe_items* left_items() const = 0;
        [[nodiscard]] virtual i_swipe_items* right_items() const = 0;
        [[nodiscard]] virtual i_swipe_items* top_items() const = 0;
        [[nodiscard]] virtual i_swipe_items* bottom_items() const = 0;

        // C# ISwipeView.IsOpen — whether the swipe view is open; the setter is the platform's write-back.
        [[nodiscard]] virtual bool is_open() const = 0;
        virtual void set_is_open(bool value) = 0;

        // C# ISwipeView.SwipeTransitionMode — Reveal vs Drag (read-only at this layer; a platform config).
        [[nodiscard]] virtual swipe_transition_mode transition_mode() const = 0;

        // C# ISwipeView.SwipeStarted / SwipeChanging / SwipeEnded — the inbound notifications the platform
        // raises as the gesture progresses; the control forwards them to its swipe_started/-changing/-ended
        // EVENTS. Named notify_* (the port's inbound-channel convention, like i_scroll_view's write-back
        // setters) so the plain names stay free for the control's observable events.
        virtual void notify_swipe_started(const swipe_view_swipe_started& args) = 0;
        virtual void notify_swipe_changing(const swipe_view_swipe_changing& args) = 0;
        virtual void notify_swipe_ended(const swipe_view_swipe_ended& args) = 0;

        // C# ISwipeView.RequestOpen / RequestClose — the programmatic open/close requests (routed to the
        // handler, which drives the native reveal / reset).
        virtual void request_open(const swipe_view_open_request& request) = 0;
        virtual void request_close(const swipe_view_close_request& request) = 0;
    };
} // namespace maui::core
