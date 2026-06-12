// window_handler — cross-platform part: the property mapper table, the ctor, and the hand-written
// i_element_handler lifecycle (connect → create-window → host-page → run-mapper → connected, and the
// disconnect teardown). The platform recipe (create the native window, host the page's native view, push
// the title/geometry, wire the notifications) lives in the per-backend partial. Ported from
// WindowHandler.cs (+ ElementHandler.cs for the SetVirtualView/DisconnectHandler shape). It services the
// Core i_window contract — never the concrete controls::window — so this stays a Core-layer file (the
// self-registration window→window_handler lives in the controls layer, in window.cpp, like every other
// control).

#include "maui/core/window_handler.hpp"

#include <any>
#include <memory>
#include <string_view>
#include <utility>

#include "maui/core/i_element.hpp"
#include "maui/core/i_menu_bar_element.hpp"  // --- chrome (W1-11) ---
#include "maui/core/i_title_bar_element.hpp" // --- chrome (W1-11) ---
#include "maui/core/i_toolbar_element.hpp"   // --- chrome (W1-11) ---
#include "maui/core/i_window.hpp"
#include "maui/core/property_mapper.hpp"

namespace maui::core
{
    // WindowHandler.Mapper — Title / Content / X / Y / Width / Height. NON-chained (a window is not an
    // i_view, so there is no generic-IView/element mapper to fall back to; C#'s WindowHandler chains the
    // ElementHandler.ElementMapper, which is empty in this port). Keys match the window's property names.
    property_mapper<i_window, window_handler>& window_handler::mapper()
    {
        static property_mapper<i_window, window_handler> table{
            {"title", &window_handler::map_title},
            {"content", &window_handler::map_content},
            {"x", &window_handler::map_x},
            {"y", &window_handler::map_y},
            {"width", &window_handler::map_width},
            {"height", &window_handler::map_height},
            // chrome (W1-11): IToolbarElement.Toolbar / IMenuBarElement.MenuBar / IWindow.TitleBar.
            {"toolbar", &window_handler::map_toolbar},
            {"menu_bar", &window_handler::map_menu_bar},
            {"title_bar", &window_handler::map_title_bar},
        };
        return table;
    }

    window_handler::window_handler() : mapper_(&mapper())
    {
    }

    // Defined here (not =default in the header) so the window_platform's backend destructor is reachable —
    // the unique_ptr<window_platform> member needs the complete type at the point of destruction.
    window_handler::~window_handler() = default;

    void window_handler::set_maui_context(i_maui_context* context)
    {
        maui_context_ = context;
    }

    // ElementHandler.SetVirtualView: create the platform window on first connect, connect once (host the
    // page + wire the notifications), then run the full property mapper. Re-running with the same view is a
    // no-op.
    void window_handler::set_virtual_view(i_element& view)
    {
        auto* incoming = dynamic_cast<i_window*>(&view);
        if (window_view_ == incoming)
        {
            return;
        }
        const bool first_setup = (virtual_view_ == nullptr);
        virtual_view_ = &view;
        window_view_ = incoming;

        if (!platform_view_)
        {
            platform_view_ = create_platform_view();
        }
        if (platform_view_)
        {
            platform_view_->hosted_window = window_view_;
        }

        if (first_setup && platform_view_)
        {
            connect(); // host the page's native view + wire the native window notifications
        }

        if (mapper_ != nullptr && window_view_ != nullptr)
        {
            mapper_->update_properties(*this, *window_view_);
        }
    }

    void window_handler::update_value(std::string_view property)
    {
        if (window_view_ == nullptr || mapper_ == nullptr)
        {
            return;
        }
        mapper_->update_property(*this, *window_view_, property);
    }

    void window_handler::invoke(std::string_view /*command*/, const std::any& /*args*/)
    {
        // A window has no commands in this cut (C#'s only WindowHandler command is RequestDisplayDensity,
        // out of scope). Content changes flow through the "content" PROPERTY map, not a command.
    }

    // ElementHandler.DisconnectHandler: tear down the notifications through the (still-live) platform
    // window, then drop both references. Idempotent — a no-op once disconnected.
    void window_handler::disconnect_handler()
    {
        if (platform_view_ && virtual_view_ != nullptr)
        {
            disconnect();
            const std::unique_ptr<window_platform> old = std::move(platform_view_);
            virtual_view_ = nullptr;
            window_view_ = nullptr;
        }
    }

    // The mapper functions delegate to the per-backend recipe (each pushes one property to the native
    // window). map_x/map_y/map_width/map_height all funnel to apply_frame (the native window is positioned
    // and sized in one move, like AppKit's setFrame:display:), matching C#'s UpdateX/Y/Width/Height which
    // each call into the same frame update on the platform window.
    void window_handler::map_title(window_handler& handler, i_window& /*view*/)
    {
        handler.apply_title();
    }
    void window_handler::map_content(window_handler& handler, i_window& /*view*/)
    {
        handler.host_content();
    }
    void window_handler::map_x(window_handler& handler, i_window& /*view*/)
    {
        handler.apply_frame();
    }
    void window_handler::map_y(window_handler& handler, i_window& /*view*/)
    {
        handler.apply_frame();
    }
    void window_handler::map_width(window_handler& handler, i_window& /*view*/)
    {
        handler.apply_frame();
    }
    void window_handler::map_height(window_handler& handler, i_window& /*view*/)
    {
        handler.apply_frame();
    }

    // --- chrome (W1-11): each map cross-casts the window to its chrome element interface (the C#
    // `window as IToolbarElement/IMenuBarElement` probes; TitleBar sits on IWindow in C# — the port's
    // i_title_bar_element is the documented cast-interface stand-in) and hands the chrome to the
    // per-backend recipe. ---
    void window_handler::map_toolbar(window_handler& handler, i_window& view)
    {
        auto* element = dynamic_cast<i_toolbar_element*>(&view);
        handler.apply_toolbar(element != nullptr ? element->toolbar() : nullptr);
    }

    void window_handler::map_menu_bar(window_handler& handler, i_window& view)
    {
        auto* element = dynamic_cast<i_menu_bar_element*>(&view);
        handler.apply_menu_bar(element != nullptr ? element->menu_bar() : nullptr);
    }

    void window_handler::map_title_bar(window_handler& handler, i_window& view)
    {
        auto* element = dynamic_cast<i_title_bar_element*>(&view);
        handler.apply_title_bar(element != nullptr ? element->title_bar() : nullptr);
    }
} // namespace maui::core
