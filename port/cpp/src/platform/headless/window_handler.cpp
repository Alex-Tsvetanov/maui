// window_handler — headless platform recipe. There is no native NSWindow; the "native host" is the
// window_platform mirror (title / content_hosted / activated) so tests can observe what the host tracks as
// the window's properties + lifecycle drive it. The Apple twin (a real NSWindow + an NSWindowDelegate
// notification trampoline) is src/platform/apple/window_handler.mm.

#include "maui/core/window_handler.hpp"

#include <memory>
#include <string>

#include "maui/core/i_element.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/i_window.hpp"

namespace maui::core
{
    window_platform::~window_platform() = default;

    std::unique_ptr<window_platform> window_handler::create_platform_view()
    {
        return std::make_unique<window_platform>();
    }

    // C# ConnectHandler — headless: nothing native to wire; the lifecycle is driven directly on the window.
    void window_handler::connect()
    {
    }

    // C# DisconnectHandler — headless: nothing native to tear down.
    void window_handler::disconnect() const
    {
    }

    void window_handler::host_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // Mirror whether the root page's native view is hosted as the content view. The page's native view
        // comes from its view-handler's native_view(); on headless that is null (no native tree), so the
        // mirror tracks instead that a page with an attached view-handler IS the content (C#'s MapContent
        // sets RootViewController to the page's platform view).
        platform->content_hosted = false;
        if (window_view_ == nullptr)
        {
            return;
        }
        if (auto* page = window_view_->content())
        {
            if (dynamic_cast<i_view_handler*>(page->handler().get()) != nullptr)
            {
                platform->content_hosted = true;
            }
        }
    }

    void window_handler::apply_title()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || window_view_ == nullptr)
        {
            return;
        }
        platform->title = std::string(window_view_->title()); // C# MapTitle -> NSWindow.title
    }

    void window_handler::apply_frame()
    {
        // Headless: no native window to move/size; the geometry lives in the window's properties. (The Apple
        // twin calls NSWindow setFrame:display:.)
    }

    // --- chrome (W1-11): headless recipes — record the chrome borrows so the tests can observe each map
    // ran with the window's current chrome. (The Apple twin materializes the real NSToolbar / NSMenu main
    // menu / NSTitlebarAccessoryViewController from the same borrows.) ---
    void window_handler::apply_toolbar(i_toolbar* toolbar) const
    {
        if (auto* platform = typed_platform_view())
        {
            platform->hosted_toolbar = toolbar;
        }
    }

    void window_handler::apply_menu_bar(i_menu_bar* menu_bar) const
    {
        if (auto* platform = typed_platform_view())
        {
            platform->hosted_menu_bar = menu_bar;
        }
    }

    void window_handler::apply_title_bar(i_title_bar* title_bar) const
    {
        if (auto* platform = typed_platform_view())
        {
            platform->hosted_title_bar = title_bar;
        }
    }
} // namespace maui::core
