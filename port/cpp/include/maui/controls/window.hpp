#pragma once
// maui::controls::window  <=  Microsoft.Maui.Controls.Window (+ Microsoft.Maui.IWindow lifecycle)
//
// Hosts a single root page and carries the window lifecycle: Created / Activated / Deactivated /
// Destroying. A window is an element, so the hosted page is its one logical child — activating the window
// flows the window reference down the page's subtree (firing each element's Loaded) and drives the page's
// Appearing; deactivating reverses it (Disappearing + Unloaded). The window's own BindingContext also
// inherits down to the page. Ported from Window.cs (the IWindow.Created/Activated/Deactivated/Destroying
// drive + SendWindowAppearing / SendWindowDisppearing).
//
// The drive methods are send_* (the inbound channel — the platform's native window calls these, exactly
// like a control's send_clicked), so the matching observable events can keep the plain names. C# throws on
// a double Created/Activated; the port no-ops idempotently (gentler, and there is no platform contract to
// enforce here). Overlays, the modal stack, visual diagnostics, geometry (X/Y/Width/Height), multi-content,
// and Resumed/Stopped/Backgrounding are out of scope (STATUS.md).

#include <functional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    class content_page; // forward — the convenience ctor hosts one; appearing is driven by dynamic_cast in .cpp

    class window : public element
    {
    public:
        window() = default;
        explicit window(content_page& page);

        // The hosted root page (Window.Page / IWindow.Content). NON-owning — the caller owns the page.
        [[nodiscard]] element* content() const
        {
            return content_;
        }
        // Host (or replace) the root page. The page inherits the window's BindingContext immediately; if the
        // window is already activated it is also attached to the window (Loaded + Appearing) at once.
        void set_content(element& page);

        [[nodiscard]] std::string_view title() const
        {
            return title_;
        }
        void set_title(std::string value)
        {
            title_ = std::move(value);
        }

        // ---- IWindow lifecycle — driven by the host (the application or the native window_handler) ----
        void send_created();
        void send_activated();
        void send_deactivated();
        void send_destroying();

        [[nodiscard]] bool is_created() const
        {
            return is_created_;
        }
        [[nodiscard]] bool is_activated() const
        {
            return is_activated_;
        }

        // Observable lifecycle (Window.Created/Activated/Deactivated/Destroying — EventHandler, no args).
        maui::core::event<> created;
        maui::core::event<> activated;
        maui::core::event<> deactivated;
        maui::core::event<> destroying;

    protected:
        // The hosted page is the window's one logical child (so the window's BindingContext inherits to it).
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            if (content_ != nullptr)
            {
                visit(*content_);
            }
        }

    private:
        // Attach / detach the page to this window: set its containing window (firing Loaded/Unloaded down the
        // subtree) and drive Appearing/Disappearing when the page is a content_page (Window.SendWindow*).
        void attach_page();
        void detach_page();

        element* content_ = nullptr; // NON-owning root page (Window.Page)
        std::string title_;
        bool is_created_ = false;
        bool is_activated_ = false;
    };
} // namespace maui::controls
