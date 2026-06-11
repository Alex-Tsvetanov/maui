// scroll_view_handler — headless platform recipe. The "native scroller" is the mirror set in
// scroll_view_platform (orientation / bar visibilities / hosted content / offsets) plus the recorded
// scroll_to trail. A scroll_to executes the C# MapRequestScrollTo loop synchronously: clamp the target
// to the available scroll range (content size vs the arranged frame), write the offsets back through
// the virtual view (the ScrollEventProxy.Scrolled write-back), and acknowledge scroll_finished — the
// native scroll collapsed to its observable effects, exactly how the headless navigation transition
// stands in for the native swap. The Apple twin (a real NSScrollView) is
// src/platform/apple/scroll_view_handler.mm.

#include "maui/core/scroll_view_handler.hpp"

#include <algorithm>
#include <memory>

#include "maui/core/i_scroll_view.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    scroll_view_platform::~scroll_view_platform() = default;

    std::unique_ptr<scroll_view_platform> scroll_view_handler::create_platform_view()
    {
        return std::make_unique<scroll_view_platform>();
    }

    void scroll_view_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
    }

    void scroll_view_handler::update_orientation()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->orientation = virtual_view()->orientation();
    }

    void scroll_view_handler::update_horizontal_scroll_bar_visibility()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->horizontal_bar_visibility = virtual_view()->horizontal_scroll_bar_visibility();
    }

    void scroll_view_handler::update_vertical_scroll_bar_visibility()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->vertical_bar_visibility = virtual_view()->vertical_scroll_bar_visibility();
    }

    // C# MapRequestScrollTo, collapsed to its observable effects: record the request, clamp the target
    // to the available scroll range, write the offsets back, and acknowledge ScrollFinished.
    void scroll_view_handler::scroll_to(const scroll_to_request& request)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        platform->scroll_requests.push_back(request);

        // availableScroll = max(ContentSize - Frame, 0); the target clamps into [0, available].
        const maui::graphics::size content = view->content_size();
        const maui::graphics::rect frame = view->frame();
        const double available_x = std::max(content.width - frame.width, 0.0);
        const double available_y = std::max(content.height - frame.height, 0.0);
        const double target_x = std::clamp(request.horizontal_offset, 0.0, available_x);
        const double target_y = std::clamp(request.vertical_offset, 0.0, available_y);

        platform->offset_x = target_x;
        platform->offset_y = target_y;
        // The platform write-back (ScrollEventProxy.Scrolled): the virtual offsets follow the native
        // ones, raising the control's Scrolled event.
        view->set_horizontal_offset(target_x);
        view->set_vertical_offset(target_y);
        // Instant or animated, the headless scroll completes synchronously.
        view->scroll_finished();
    }

    void scroll_view_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native scroller to frame; the content is arranged by the control directly and
        // the scrollable extent lives in the control's content_size().
    }
} // namespace maui::core
