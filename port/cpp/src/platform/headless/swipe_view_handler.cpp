// swipe_view_handler — headless platform recipe: the swipe state machine driven by SYNTHETIC swipe
// offsets the tests inject (no real pan gesture without a UI). The machine itself is the shared
// cross-platform maui::core::swipe_machine (src/core/swipe_view_machine.cpp — the pure MauiSwipeView.cs
// port); this partial only wires the handler drivers to it and mirrors the content host. The Apple/iOS
// twins host the content on a real NSView/UIView with a pan recognizer and drive the SAME machine.

#include "maui/core/swipe_view_handler.hpp"

#include <memory>

#include "maui/core/i_swipe_view.hpp"
#include "maui/core/swipe_direction.hpp"
#include "maui/core/swipe_view_machine.hpp"
#include "maui/core/swipe_view_requests.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::core
{
    swipe_view_platform::~swipe_view_platform() = default;

    std::unique_ptr<swipe_view_platform> swipe_view_handler::create_platform_view()
    {
        return std::make_unique<swipe_view_platform>();
    }

    void swipe_view_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
    }

    void swipe_view_handler::update_transition_mode()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->transition = virtual_view()->transition_mode();
    }

    void swipe_view_handler::update_items()
    {
        // C# MapLeftItems/... are empty (the gesture-time UpdateSwipeItems re-reads the live collection);
        // the machine reads the collections directly on each swipe, so there is nothing to cache here.
    }

    void swipe_view_handler::programmatically_open(const swipe_view_open_request& request)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        swipe_machine::programmatically_open(platform->state, *view, request);
    }

    void swipe_view_handler::reset_swipe(bool /*animated*/)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        swipe_machine::reset_swipe(platform->state, *view);
    }

    void swipe_view_handler::begin_swipe(swipe_direction direction)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        swipe_machine::begin_swipe(platform->state, direction);
    }

    void swipe_view_handler::swipe_to(double offset)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        swipe_machine::swipe_to(platform->state, *view, offset);
    }

    void swipe_view_handler::end_swipe()
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        swipe_machine::end_swipe(platform->state, *view);
    }

    void swipe_view_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native container to frame; the content is arranged by the control directly.
    }
} // namespace maui::core
