// refresh_view_handler — headless platform recipe. The "native pull-to-refresh control" is the mirror set
// in refresh_view_platform (refreshing / refresh_enabled / the spinner color / the hosted content).
// request_refresh() is the MauiRefreshViewProxy.OnRefresh twin: it writes IsRefreshing=true back through
// the virtual view (which re-enters the control's coercion → Refreshing event + the command), exactly as
// the native control's ValueChanged does. The Apple twin (no native pull on AppKit — a documented
// deviation) is src/platform/apple/refresh_view_handler.mm; the iOS twin (a real UIRefreshControl) is
// src/platform/ios/refresh_view_handler.mm.

#include "maui/core/refresh_view_handler.hpp"

#include <memory>

#include "maui/core/i_refresh_view.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::core
{
    refresh_view_platform::~refresh_view_platform() = default;

    std::unique_ptr<refresh_view_platform> refresh_view_handler::create_platform_view()
    {
        return std::make_unique<refresh_view_platform>();
    }

    void refresh_view_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
    }

    void refresh_view_handler::update_is_refreshing()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->refreshing = virtual_view()->is_refreshing();
    }

    void refresh_view_handler::update_refresh_color()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        // C# UpdateRefreshColor: the spinner tint follows RefreshColor (null = platform default).
        const maui::graphics::paint* const paint = virtual_view()->refresh_color();
        if (paint != nullptr)
        {
            platform->has_refresh_color = true;
            platform->refresh_color_argb = paint->background_color().to_uint();
        }
        else
        {
            platform->has_refresh_color = false;
            platform->refresh_color_argb = 0;
        }
    }

    void refresh_view_handler::update_is_refresh_enabled()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->refresh_enabled = virtual_view()->is_refresh_enabled();
    }

    // The native pull stand-in (MauiRefreshViewProxy.OnRefresh): write IsRefreshing=true back.
    void refresh_view_handler::request_refresh()
    {
        if (auto* view = virtual_view())
        {
            view->set_is_refreshing(true);
        }
    }

    void refresh_view_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native container to frame; the content is arranged by the control directly.
    }
} // namespace maui::core
